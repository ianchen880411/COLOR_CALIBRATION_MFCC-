#include "stdafx.h"
#include "calibrate.h"
#include "Ca200SampleDlg.h"
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <numeric>

//////////////////////////////////////////////
using namespace std;
/////////////////////////////////////

const int lut_size = 16; //= interval
int interval = lut_size; //校正切分點數
int step = 1024 / interval; //間隔

TRC tone_curve; //raw tone curve(gamma) R、G、B、W、target
double targetx, targety; //目標色域座標
vector<double> ref_white; //參考白
extern double tone_curve_raw;

Test_LUT test_lut[lut_size]; //校正時的測試LUT 
Calibrated_LUT calibrated_lut[lut_size]; //校正時最優解LUT

RGBColor one_d_lut[LUT_array_size]; //實際傳送的LUT
RGBColor ca_lut[LUT_array_size]; //紀錄色溫校正後的LUT 供 tone curve 、dicom 運算時使用
RGBColor cct_r_lut[LUT_array_size];
RGBColor dicom_lut[LUT_array_size];

MATRIX allocateMatrix(int row, int col);
MATRIX Multiply(MATRIX m1, MATRIX m2);
MATRIX Matrix_Inv(MATRIX matrix);
void Mark(uint16_t& R, uint16_t& G, uint16_t& B, int i);
double L_to_JND(double L);
double JND_to_L(double JND);
double j_p(double Jmax, double Jmin, int n, int p);
vector<double> dicom_XYZ(double max_Lv);

MATRIX matrix_raw =
{ {1, 0, 0},
 {0, 1, 0},
 {0, 0, 1} };

MATRIX M_sRGB_RGB_XYZ =
{{0.4124, 0.3576, 0.1805},
 {0.2126, 0.7152, 0.0722},
 {0.0193, 0.1192, 0.9505}};

MATRIX M_ADOBE_RGB_XYZ =
{{0.5767309, 0.1855540, 0.1881852},
 {0.2973769, 0.6273491, 0.0752741},
 {0.0270343, 0.0706872, 0.9911085}};

MATRIX M_BT2020_RGB_XYZ =
{{0.6370, 0.1446, 0.1689},
 {0.2627, 0.6780, 0.0593},
 {0.0, 0.0281, 1.0610}};

map<CString, MATRIX> mp_matrix_gamut_target =
{{"sRGB", M_sRGB_RGB_XYZ},
 {"ADOBE", M_ADOBE_RGB_XYZ},
 {"BT2020", M_BT2020_RGB_XYZ}};

#pragma region  Colour
vector<double> XYZ_to_Lab(vector<double> XYZ, vector<double> ref_XYZ) {
	double e = 0.008856;
	double k = 903.3;
	double xr = XYZ[0] / ref_XYZ[0];
	double yr = XYZ[1] / ref_XYZ[1];
	double zr = XYZ[2] / ref_XYZ[2];

	double fx = (xr > e) ? pow(xr, 1.0 / 3.0) : (k * xr + 16.0) / 116.0;
	double fy = (yr > e) ? pow(yr, 1.0 / 3.0) : (k * yr + 16.0) / 116.0;
	double fz = (zr > e) ? pow(zr, 1.0 / 3.0) : (k * zr + 16.0) / 116.0;

	double L = 116.0 * fy - 16.0;
	double a = 500.0 * (fx - fy);
	double b = 200.0 * (fy - fz);
	return vector<double>{L, a, b};
}

double xy_to_CCT(double& x, double& y) {
	double n = (x - 0.332) / (0.1858 - y);
	double CCT = pow(449 * n, 3) + pow(3525 * n, 2) + 6823.3 * n + 5520.33;
	return CCT;
}

vector<float> CCT_to_xy(int& cct) {
	float x;
	if (cct < 7000) {
		x = (-4.6070 * pow(10.0, 9.0) / pow(cct, 3.0)) + (2.9678 * pow(10.0, 6.0) / pow(cct, 2.0)) + (0.09911 * pow(10.0, 3.0) / cct) + 0.244063;
	}
	else {
		x = (-2.0064 * pow(10.0, 9.0) / pow(cct, 3.0)) + (1.9018 * pow(10.0, 6.0) / pow(cct, 2.0)) + (0.24748 * pow(10.0, 3.0) / cct) + 0.237040;
	}
	float y = -3.0 * pow(x, 2) + 2.87 * x - 0.275;
	return vector<float>{x, y};
}

vector<float> CCT_to_xyz_nor(int& cct) {
	vector<float> xyz = CCT_to_xy(cct);
	xyz.push_back(1.0 - xyz[0] - xyz[1]);
	float r = 1.0 / xyz[1];
	for (int i = 0; i < 3; i++) {
		xyz[i] *= r;
	}
	return xyz;
}

vector<float> W_Transform(MATRIX& matrix_RGB_XYZ, int& cct_target) {
	vector<float> xyz = CCT_to_xyz_nor(cct_target);
	MATRIX m1 = Matrix_Inv(matrix_RGB_XYZ);
	vector<float> m2 = { xyz[0], xyz[1], xyz[2] };
	vector<float> matrix_w{ 0,0,0 };
	for (int j = 0; j < 3; j++) {
		for (int k = 0; k < 3; k++) {
			matrix_w[j] += m1[j][k] * m2[k];
		}
	}
	return matrix_w;
}
#pragma endregion

#pragma region TRC_measure_Gamut_CCT_RGB_Conversion 
double calculate_mse(vector<double>& lv, vector<double>& std_lv, double gamma) {
	double temp = 0;
	double temp2;
	for (int i = 0; i < lv.size(); i++) {
		temp2 = pow(lv[i], 1 / gamma) - std_lv[i];
		temp += temp2 * temp2;
	}
	return temp;
}

double calculate_trc(vector<double>& lv, vector<double>& std_lv) {
	double raw_trc = 3;
	double unit = 1;
	for (int i = 1; i <= 4; i++) {
		double min = 999999999;
		unit *= 0.1;
		for (int j = 0; j <= 40; j++) {
			double temp = calculate_mse(lv, std_lv, (raw_trc - unit * 20) + j * unit);
			if (temp < min) {
				min = temp;
			}
			else {
				raw_trc = (raw_trc - unit * 20) + (j - 1) * unit;
				break;
			}
		}
	}
	return raw_trc;
}

MATRIX Build_Matrix_RGB_to_XYZ(vector<double>& XYZ_R, vector<double>& XYZ_G, vector<double>& XYZ_B, vector<double>& XYZ_0, double& Y_max) {
	vector<float>	XYZ_R_nor;
	vector<float>	XYZ_G_nor;
	vector<float>	XYZ_B_nor;
	for (int i = 0; i < 3; i++) {
		XYZ_R_nor.push_back((XYZ_R[i] - XYZ_0[i]) / Y_max);
		XYZ_G_nor.push_back((XYZ_G[i] - XYZ_0[i]) / Y_max);
		XYZ_B_nor.push_back((XYZ_B[i] - XYZ_0[i]) / Y_max);
	}

	float XYZ_R_nor_sum = 0.0;
	float XYZ_G_nor_sum = 0.0;
	float XYZ_B_nor_sum = 0.0;
	for (int i = 0; i < 3; i++) {
		XYZ_R_nor_sum += XYZ_R_nor[i];
		XYZ_G_nor_sum += XYZ_G_nor[i];
		XYZ_B_nor_sum += XYZ_B_nor[i];
	}

	float xR = XYZ_R_nor[0] / XYZ_R_nor_sum;
	float xG = XYZ_G_nor[0] / XYZ_G_nor_sum;
	float xB = XYZ_B_nor[0] / XYZ_B_nor_sum;

	float yR = XYZ_R_nor[1] / XYZ_R_nor_sum;
	float yG = XYZ_G_nor[1] / XYZ_G_nor_sum;
	float yB = XYZ_B_nor[1] / XYZ_B_nor_sum;

	float zR = XYZ_R_nor[2] / XYZ_R_nor_sum;
	float zG = XYZ_G_nor[2] / XYZ_G_nor_sum;
	float zB = XYZ_B_nor[2] / XYZ_B_nor_sum;

	MATRIX	Xi =
	{ {xR, xG, xB},
	 {yR, yG, yB},
	 {zR, zG, zB} };

	MATRIX	Sm =
	{ {XYZ_R_nor_sum, 0, 0},
	 {0, XYZ_G_nor_sum, 0},
	 {0, 0, XYZ_B_nor_sum} };

	MATRIX	M = Multiply(Xi, Sm);
	return M;
}

MATRIX Build_Matrix_RGB_to_Gamut_RGB(MATRIX& matrix_native_RGB_XYZ, CString& gamut_target) {
	MATRIX matrix_gamut_target = mp_matrix_gamut_target[gamut_target];
	int cct = 6500;
	vector<float> w_adj = W_Transform(matrix_native_RGB_XYZ, cct);

	MATRIX matrix_RGB_XYZ_D = allocateMatrix(3, 3);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			matrix_RGB_XYZ_D[i][j] = matrix_native_RGB_XYZ[i][j] * w_adj[j];
		}
	}
	MATRIX matrix_gamut_trans = Multiply(Matrix_Inv(matrix_RGB_XYZ_D), matrix_gamut_target);
	float w_adj_m_max = *max_element(w_adj.begin(), w_adj.end());
	for (int i = 0; i < 3; i++) {
		w_adj[i] /= w_adj_m_max;
		//w_adj[i] = pow(w_adj[i] / w_adj_m_max,  1/tone_curve_raw) * w_adj_m_max;
	}
	MATRIX w_adj_m =
	{ {w_adj[0], 0, 0},
	 {0, w_adj[1], 0},
	 {0, 0, w_adj[2]} };

	matrix_gamut_trans = Multiply(w_adj_m, matrix_gamut_trans);

	return matrix_gamut_trans;
}

MATRIX Build_Matrix_RGB_to_CCT_RGB(MATRIX& matrix_native_RGB_XYZ, int& cct_target) {
	vector<float> w_adj = W_Transform(matrix_native_RGB_XYZ, cct_target);
	double w_adj_m_max = *max_element(w_adj.begin(), w_adj.end());
	for (int i = 0; i < 3; i++) {
		w_adj[i] /= w_adj_m_max;
	}
	for (int i = 0; i < 3; i++) {
		w_adj[i] = pow(w_adj[i], (1.0 / tone_curve_raw));
	}
	MATRIX matrix_cct_trans =
	{ {w_adj[0], 0, 0},
	 {0, w_adj[1], 0},
	 {0, 0, w_adj[2]} };

	return matrix_cct_trans;
}

vector<double> RGB_to_Gamut_RGB(vector<double>RGB, MATRIX& matrix_gamut_trans) {
	vector<double> RGB_gamut = { 0,0,0 };

	for (int i = 0; i < 3; i++) {
		RGB[i] = pow(RGB[i] / 1023.0, tone_curve_raw) * 1023.0;
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			RGB_gamut[i] += matrix_gamut_trans[i][j] * RGB[j];
		}
	}

	for (int i = 0; i < 3; i++) {
		RGB_gamut[i] = max(RGB_gamut[i], 0.0);
		RGB_gamut[i] = min(RGB_gamut[i], 1023.0);
		RGB_gamut[i] = pow(RGB_gamut[i] / 1023.0, 1.0 / tone_curve_raw) * 1023.0;
	}

	return RGB_gamut;
}

vector<double> RGB_to_CCT_RGB(vector<double>RGB, MATRIX& matrix_cct_trans) {
	vector<double> RGB_gamut = { 0,0,0 };

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			RGB_gamut[i] += matrix_cct_trans[i][j] * RGB[j];
		}
	}

	return RGB_gamut;
}
#pragma endregion

#pragma region CCT_AUTO

//計算測試 LUT
void Cal_CCT_LUT() {
	double dis_x, dis_y, dis_z; //與target xy的距離 
	double random_r, random_g, random_b; //用於當調整值過小時 讓他跳出某個範圍 以利尋找最優解
	double s; //根據灰階大小來計算調整值大小 例如灰階100的調整值大概是1~2位數 而1000可能會到1~3位數
	double vR, vG, vB; //R、G、B 調整值
	int grayscale;
	for (int i = 0; i < interval; i++) {
		dis_x = targetx - test_lut[i].xyz.x;
		dis_y = targety - test_lut[i].xyz.y;
		dis_z = (1.0 - targetx - targety) - test_lut[i].xyz.z;

		random_r = (2.0 - 1) * rand() / (RAND_MAX + 1.0) + 1;
		random_g = (2.0 - 1) * rand() / (RAND_MAX + 1.0) + 1;
		random_b = (2.0 - 1) * rand() / (RAND_MAX + 1.0) + 1;
		s = step * (i + 1);
		vR = dis_x * s / targetx;
		vG = dis_y * s / targety;
		vB = dis_z * s;
		vR = (abs(vR) < 1.0) ? vR * random_r : vR;
		vG = (abs(vG) < 1.0) ? vG * random_g : vG;
		vB = (abs(vB) < 1.0) ? vB * random_b : vB;
		test_lut[i].rgbColor.Red += uint16_t(vR);
		test_lut[i].rgbColor.Green += uint16_t(vG);
		test_lut[i].rgbColor.Blue += uint16_t(vB);

		grayscale = (i + 1) * step - 1; //計算所對應的0~1023灰階階數

		Mark(test_lut[i].rgbColor.Red, test_lut[i].rgbColor.Green, test_lut[i].rgbColor.Blue, grayscale);

		//更新下一回合測試的 RGB 值
		one_d_lut[grayscale].Red = test_lut[i].rgbColor.Red;
		one_d_lut[grayscale].Green = test_lut[i].rgbColor.Green;
		one_d_lut[grayscale].Blue = test_lut[i].rgbColor.Blue;
	}
}

//分析測試 LUT 的校正測量結果
void Analysis() {
	double dis_x;
	double dis_y;
	double dis_z;
	double dis;

	vector<double> Lab;
	vector<double> ref_Lab;
	for (int i = 0; i < interval; i++) {
		dis_x = targetx - test_lut[i].xyz.x;
		dis_y = targety - test_lut[i].xyz.y;
		dis_z = (1.0 - targetx - targety) - test_lut[i].xyz.z;

		Lab = XYZ_to_Lab({ test_lut[i].XYZ.X,test_lut[i].XYZ.Y,test_lut[i].XYZ.Z }, ref_white);
		ref_Lab = XYZ_to_Lab({ test_lut[i].XYZ.Y * targetx / targety,test_lut[i].XYZ.Y,test_lut[i].XYZ.Y * (1.0 - targetx - targety) / targety, }, ref_white);

		dis = sqrt(pow(ref_Lab[1] - Lab[1], 2.0) + pow(ref_Lab[2] - Lab[2], 2.0));
		//dis = sqrt(pow(dis_x, 2.0) + pow(dis_y, 2.0));

		if (dis < calibrated_lut[i].dis) {
			calibrated_lut[i].rgbColor = test_lut[i].rgbColor;
			calibrated_lut[i].xyz = test_lut[i].xyz;
			calibrated_lut[i].dis = dis;
		}
	}
}

//具體是做類似於根據灰階階數來做比例與tone curve mapping計算
void Mark(uint16_t& R, uint16_t& G, uint16_t& B, int i) {
	double max_RGB = max(R, G);
	max_RGB = max(max_RGB, B);
	double tR, tG, tB;

	tR = (double)R * i / max_RGB;
	tG = (double)G * i / max_RGB;
	tB = (double)B * i / max_RGB;

	//tone curve mapping
	R = pow(tR / (double)R, tone_curve_raw / tone_curve.r) * R;
	G = pow(tG / (double)G, tone_curve_raw / tone_curve.g) * G;
	B = pow(tB / (double)B, tone_curve_raw / tone_curve.b) * B;

	R = min(R, 1023);
	G = min(G, 1023);
	B = min(B, 1023);

	R = max(R, 0);
	G = max(G, 0);
	B = max(B, 0);
}

//待校正後確認16個點的 RGB 值後 將one_d_lut 0~1023 計算出來
void Build_CCT_LUT() {
	int max_RGB;
	uint16_t R, G, B;
	int j;
	double ref_grayscale;
	for (int i = 0; i < 1024; i++) {
		j = ceil(double(i + 1) * interval / 1024.0) - 1;
		ref_grayscale = (j + 1) * step - 1;
		if ((i + 1) % step == 0) {
			R = calibrated_lut[j].rgbColor.Red;
			G = calibrated_lut[j].rgbColor.Green;
			B = calibrated_lut[j].rgbColor.Blue;
		}
		else {
			R = (double)calibrated_lut[j].rgbColor.Red;
			G = (double)calibrated_lut[j].rgbColor.Green;
			B = (double)calibrated_lut[j].rgbColor.Blue;
			Mark(R, G, B, i);
		}

		ca_lut[i].Red = R;
		ca_lut[i].Green = G;
		ca_lut[i].Blue = B;
	}
}

void Cal_DICOM_LUT(RGBColor lut[LUT_array_size], double max_Lv) {
	vector<double> target_dicom_list = dicom_XYZ(max_Lv);
	double index;
	for (int i = 0; i < 1024; i++) {
		index = lut[1023].Green * target_dicom_list[i];
		index = pow((double)(index / lut[1023].Green), 1 / tone_curve_raw) * 1023;
		dicom_lut[i] = lut[(int)index];
	}
}

double L_to_JND(double L)
{
	int JND = 0;
	JND = 71.498068 + 94.593053 * log10(L) + 41.912053 * (pow(log10(L), 2)) + 9.8247004 * (pow(log10(L), 3))
		+ 0.28175407 * (pow(log10(L), 4)) - 1.1878455 * (pow(log10(L), 5))
		- 0.18014349 * (pow(log10(L), 6)) + 0.14710899 * (pow(log10(L), 7)) - 0.017046845 * (pow(log10(L), 8));
	return JND;
}

double JND_to_L(double JND)
{
	double j = JND;
	double d = -1.3011877 + 0.080242636 * log(j) + 0.13646699 * (pow(log(j), 2)) - 0.025468404 * (pow(log(j), 3)) + 0.0013635334 * (pow(log(j), 4));
	double m = 1 - 0.025840191 * log(j) - 0.10320229 * (pow(log(j), 2)) + 0.028745620 * (pow(log(j), 3)) - 0.0031978977 * (pow(log(j), 4)) + 0.00012992634 * (pow(log(j), 5));
	double log10L = d / m;
	double L = pow(10, log10L);
	return L;
}

double j_p(double Jmax, double Jmin, int n, int p)
{
	double jp = Jmin + (p / (pow(2, n) - 1)) * (Jmax - Jmin);
	return jp;
}

vector<double>  dicom_XYZ(double max_Lv) {
	vector<double> target_dicom_list;
	double J_max = L_to_JND(max_Lv);
	double J_min = L_to_JND(0.01);
	int n = 10;
	double jp;
	double L_D;
	for (int p = 0; p < 1024; p++) {
		jp = j_p(J_max, J_min, n, p);
		if (jp <= 0) {
			L_D = 0;
		}
		else {
			L_D = JND_to_L(jp);
			L_D /= max_Lv;
		}
		target_dicom_list.push_back(L_D);
	}
	target_dicom_list[0] = 0;
	target_dicom_list[target_dicom_list.size() - 1] = 1;
	return target_dicom_list;
}

#pragma endregion

void row_minus(vector<float>& X, vector<float>& Y, double multi) {
	for (int i = 0; i < 6; i++) {
		X[i] -= Y[i] * multi;
	}
}

void single_row_divide(vector<float>& X, double divide) {
	for (int i = 0; i < 6; i++) {
		X[i] /= divide;
	}
}

MATRIX Matrix_Inv(MATRIX matrix) { // this function will get the  inverse of the matrix
	MATRIX matrix_inv;
	for (int i = 0; i < 3; i++) {
		for (int j = 1; j <= 3; j++) {
			if (i + 1 == j)
				matrix[i].push_back(1);
			else
				matrix[i].push_back(0);
		}
	}
	for (int i = 0; i < 3; i++) {
		float multi1 = matrix[(i + 1) % 3][i] / matrix[i][i];
		float multi2 = matrix[(i + 2) % 3][i] / matrix[i][i];
		row_minus(matrix[(i + 1) % 3], matrix[i], multi1);
		row_minus(matrix[(i + 2) % 3], matrix[i], multi2);
		single_row_divide(matrix[i], matrix[i][i]);
	}
	for (int i = 0; i < 3; i++) {
		vector<float> temp;
		for (int j = 3; j < 6; j++)temp.push_back(matrix[i][j]);
		matrix_inv.push_back(temp);
	}
	return matrix_inv;
}

MATRIX allocateMatrix(int row, int col)
{
	MATRIX m;
	vector<float>k;
	for (int j = 0; j < col; j++)
		k.push_back(0.0);
	for (int i = 0; i < row; i++) {
		m.push_back(k);
	}
	return m;
}

MATRIX Multiply(MATRIX m1, MATRIX m2) {
	int row = m1.size();
	int col = m2[0].size();
	MATRIX m = allocateMatrix(row, col);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				m[i][j] += m1[i][k] * m2[k][j];
			}
		}
	}
	return m;
}

