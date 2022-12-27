#include "stdafx.h"
#include "calibrate.h"
#include "Ca200SampleDlg.h"
#include <string>
#include <algorithm>
#include <vector>
#include <numeric>

//////////////////////////////////////////////
using namespace std;
/////////////////////////////////////

int interval = lut_size; //校正切分點數
int step = 1024 / interval; //間隔

Calibrate::Calibrate() {

}

Calibrate::~Calibrate() {

}

#pragma region  Colour
vector<double> Calibrate::XYZ_to_Lab(const vector<double>& XYZ, const vector<double>& ref_XYZ) {
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

double Calibrate::xy_to_CCT(const double x, const double y) {
	double n = (x - 0.332) / (0.1858 - y);
	double CCT = pow(449 * n, 3) + pow(3525 * n, 2) + 6823.3 * n + 5520.33;
	return CCT;
}

vector<float> Calibrate::CCT_to_xy(const int cct) {
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

vector<float> Calibrate::CCT_to_xyz_nor(const int cct) {
	vector<float> xyz = CCT_to_xy(cct);
	xyz.push_back(1.0 - xyz[0] - xyz[1]);
	float r = 1.0 / xyz[1];
	for (int i = 0; i < 3; i++) {
		xyz[i] *= r;
	}
	return xyz;
}

vector<float> Calibrate::W_Transform(const MATRIX& matrix_RGB_XYZ, const int cct_target) {
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

#pragma region TRC_measure

double Calibrate::calculate_trc(const vector<double>& lv) {
	vector<double> std_lv;
	double cnt;
	int step = LUT_array_size / (lv.size() - 1);
	for (int i = 0; i < lv.size(); i++) {
		cnt = min(i * step, 1023.0);
		std_lv.push_back(cnt / 1023.0);
	}

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

double Calibrate::calculate_mse(const vector<double>& lv, const vector<double>& std_lv, double gamma) {
	double temp = 0;
	double temp2;
	for (int i = 0; i < lv.size(); i++) {
		temp2 = pow(lv[i], 1 / gamma) - std_lv[i];
		temp += temp2 * temp2;
	}
	return temp;
}

void Calibrate::Generate_Gamma_LUT(PRGBColor gamma_LUT, const double gamma_target) {
	uint16_t v = 0;
	for (uint16_t i = 0; i < LUT_array_size; i++) {
		v = round(pow(i / 1023.0, gamma_target) * 1023.0);
		gamma_LUT[i] = RGBColor{ v,v,v };
	}
}
#pragma endregion

#pragma region Gamut CCT RGB Conversion

MATRIX Calibrate::Build_Matrix_RGB_to_XYZ(vector<double>& XYZ_R, vector<double>& XYZ_G, vector<double>& XYZ_B, vector<double>& XYZ_W) {
	vector<float>XYZ_R_nor;
	vector<float>XYZ_G_nor;
	vector<float>XYZ_B_nor;
	vector<float>XYZ_W_nor;;

	double Y_max = XYZ_W[1];

	for (int i = 0; i < 3; i++) {
		XYZ_R_nor.push_back((XYZ_R[i]) / Y_max);
		XYZ_G_nor.push_back((XYZ_G[i]) / Y_max);
		XYZ_B_nor.push_back((XYZ_B[i]) / Y_max);
		XYZ_W_nor.push_back((XYZ_W[i]) / Y_max);
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

	MATRIX	Xi_inv = Matrix_Inv(Xi);
	vector<float> matrix_s{ 0,0,0 };
	for (int j = 0; j < 3; j++) {
		for (int k = 0; k < 3; k++) {
			matrix_s[j] += Xi_inv[j][k] * XYZ_W_nor[k];
		}
	}
	MATRIX Sm =
	{ {float(matrix_s[0]),0,0},
	  {0,float(matrix_s[1]),0},
 	  {0,0,float(matrix_s[2])} };

	MATRIX	M = Multiply(Xi, Sm);
	return M;
}

void Calibrate::Build_Matrix_Gamut_CCT_Trans(const MATRIX& matrix_native_RGB_XYZ, MATRIX& matrix_Gamut_CCT_trans, const CString gamut_target, const int cct_target) {
	MATRIX matrix_gamut_trans;
	MATRIX matrix_gamut_transed;
	if (gamut_target == "Native") {
		matrix_gamut_trans = matrix_raw;
		matrix_gamut_transed = matrix_native_RGB_XYZ;
	}
	else {
		MATRIX matrix_gamut_target = mp_matrix_gamut_target[gamut_target];
		matrix_gamut_trans = Multiply(Matrix_Inv(matrix_native_RGB_XYZ), matrix_gamut_target);
		matrix_gamut_transed = Multiply(matrix_native_RGB_XYZ, matrix_gamut_trans);
	}

	MATRIX matrix_cct_trans;
	if (cct_target == 6500) {
		matrix_cct_trans = matrix_raw;
	}
	else {
		vector<float> w_adj = W_Transform(matrix_gamut_transed, cct_target);
		double w_adj_m_max = *max_element(w_adj.begin(), w_adj.end());
		for (int i = 0; i < 3; i++) {
			w_adj[i] /= w_adj_m_max;
		}
		matrix_cct_trans =
		{ {w_adj[0], 0, 0},
		 {0, w_adj[1], 0},
		 {0, 0, w_adj[2]} };
	}
	
	matrix_gamut_trans = Multiply(matrix_gamut_trans, matrix_cct_trans);

	matrix_Gamut_CCT_trans = matrix_gamut_trans;
}
#pragma endregion

#pragma region CCT_AUTO

void Calibrate::Set_Calibration_Target(const int cct_target, const TRC tone_curve) {
	vector<float>xy = CCT_to_xy(cct_target);
	targetx = xy[0];
	targety = xy[1];
	this->panel_tone_curve = tone_curve;
}

void Calibrate::CCT_Calibration(PRGBColor virtual_LUT, vector<vector<double> >& data) {
	for (int i = 0; i < interval; i++) {
		test_LUT[i].XYZ.X = data[i][3];
		test_LUT[i].XYZ.Y = data[i][4];
		test_LUT[i].XYZ.Z = data[i][5];
		test_LUT[i].xyz.x = data[i][7];
		test_LUT[i].xyz.y = data[i][8];
		test_LUT[i].xyz.z = data[i][9];
	}
	Analysis(); //分析結果
	Cal_CCT_LUT(virtual_LUT); //產生cct_lut
}

void Calibrate::Init_LUT() {
	for (uint16_t i = 0; i < interval; i++) {
		calibrated_LUT[i].dis = MAXINT;
		test_LUT[i].rgbColor.Red = (i + 1) * step - 1;
		test_LUT[i].rgbColor.Green = (i + 1) * step - 1;
		test_LUT[i].rgbColor.Blue = (i + 1) * step - 1;
	}
}

//分析測試 LUT 的校正測量結果
void Calibrate::Analysis() {

	double dis_x;
	double dis_y;
	double dis_z;
	double dis;

	vector<double> ref_white;
	vector<double> Lab;
	vector<double> ref_Lab;

	ref_white = { test_LUT[interval - 1].XYZ.Y * targetx / targety, test_LUT[interval - 1].XYZ.Y, test_LUT[interval - 1].XYZ.Y * (1 - targetx - targety) / targety };

	for (int i = 0; i < interval; i++) {
		dis_x = targetx - test_LUT[i].xyz.x;
		dis_y = targety - test_LUT[i].xyz.y;
		dis_z = (1.0 - targetx - targety) - test_LUT[i].xyz.z;

		Lab = XYZ_to_Lab({ test_LUT[i].XYZ.X,test_LUT[i].XYZ.Y,test_LUT[i].XYZ.Z }, ref_white);
		ref_Lab = XYZ_to_Lab({ test_LUT[i].XYZ.Y * targetx / targety,test_LUT[i].XYZ.Y,test_LUT[i].XYZ.Y * (1.0 - targetx - targety) / targety, }, ref_white);

		dis = sqrt(pow(ref_Lab[1] - Lab[1], 2.0) + pow(ref_Lab[2] - Lab[2], 2.0));

		if (dis < calibrated_LUT[i].dis) {
			calibrated_LUT[i].rgbColor = test_LUT[i].rgbColor;
			calibrated_LUT[i].xyz = test_LUT[i].xyz;
			calibrated_LUT[i].dis = dis;
		}
	}
}

void Calibrate::Cal_CCT_LUT(PRGBColor virtual_LUT) {
	double dis_x, dis_y, dis_z; //與target xy的距離 
	double random_r, random_g, random_b; //用於當調整值過小時 讓他跳出某個範圍 以利尋找最優解
	double s; //根據灰階大小來計算調整值大小 例如灰階100的調整值大概是1~2位數 而1000可能會到1~3位數
	double vR, vG, vB; //R、G、B 調整值
	int grayscale;
	for (int i = 0; i < interval; i++) {
		dis_x = targetx - test_LUT[i].xyz.x;
		dis_y = targety - test_LUT[i].xyz.y;
		dis_z = (1.0 - targetx - targety) - test_LUT[i].xyz.z;

		random_r = (2.0 - 1) * rand() / (RAND_MAX + 1.0) + 1;
		random_g = (2.0 - 1) * rand() / (RAND_MAX + 1.0) + 1;
		random_b = (2.0 - 1) * rand() / (RAND_MAX + 1.0) + 1;
		s = step * (i + 1);
		vR = dis_x * s / targetx;
		vG = dis_y * s / targety;
		vB = dis_z * s;
		vR = (abs(vR) < 2.0) ? vR * random_r : vR;
		vG = (abs(vG) < 2.0) ? vG * random_g : vG;
		vB = (abs(vB) < 2.0) ? vB * random_b : vB;
		test_LUT[i].rgbColor.Red += uint16_t(round(vR));
		test_LUT[i].rgbColor.Green += uint16_t(round(vG));
		test_LUT[i].rgbColor.Blue += uint16_t(round(vB));

		grayscale = (i + 1) * step - 1; //計算所對應的0~1023灰階階數

		Mark(test_LUT[i].rgbColor.Red, test_LUT[i].rgbColor.Green, test_LUT[i].rgbColor.Blue, grayscale);

		//更新下一回合測試的 RGB 值
		virtual_LUT[grayscale].Red = test_LUT[i].rgbColor.Red;
		virtual_LUT[grayscale].Green = test_LUT[i].rgbColor.Green;
		virtual_LUT[grayscale].Blue = test_LUT[i].rgbColor.Blue;
	}
}

//具體是做類似於根據灰階階數來做比例與tone curve mapping計算
void Calibrate::Mark(uint16_t& R, uint16_t& G, uint16_t& B, const int i) {
	double max_RGB = max(R, G);
	max_RGB = max(max_RGB, B);
	double tR, tG, tB;

	tR = (double)R * i / max_RGB;
	tG = (double)G * i / max_RGB;
	tB = (double)B * i / max_RGB;

	//tone curve mapping
	
	R = round(pow(tR / (double)R, panel_tone_curve.w / panel_tone_curve.r) * R);
	G = round(pow(tG / (double)G, panel_tone_curve.w / panel_tone_curve.g) * G);
	B = round(pow(tB / (double)B, panel_tone_curve.w / panel_tone_curve.b) * B);
	
	R = min(R, 1023);
	G = min(G, 1023);
	B = min(B, 1023);

	R = max(R, 0);
	G = max(G, 0);
	B = max(B, 0);
}

//待校正後確認16個點的 RGB 值後 將 calibrated_LUT 0~1023 計算出來
void Calibrate::Build_CCT_LUT(PRGBColor cct_calibrated_LUT) {
	int max_RGB;
	uint16_t R, G, B;
	int j;
	double ref_grayscale;
	for (int i = 0; i < 1024; i++) {
		j = ceil(double(i + 1) * interval / 1024.0) - 1;
		ref_grayscale = (j + 1) * step - 1;

		R = calibrated_LUT[j].rgbColor.Red;
		G = calibrated_LUT[j].rgbColor.Green;
		B = calibrated_LUT[j].rgbColor.Blue;

		if ((i + 1) % step != 0) {
			Mark(R, G, B, i);
		}
		cct_calibrated_LUT[i] = RGBColor{ R,G,B };
	}
}

void Calibrate::LUT_Tone_Curve_Trans(PRGBColor result_LUT, const vector<double>& Lv, const double tone_curve_target, const int interval) {
	double trc_raw;
	vector<int> trc_index;
	RGBColor temp[LUT_array_size];
	Tone_Curve_Correction(Lv, tone_curve_target, trc_index, interval);
	trc_raw = calculate_trc(Lv);
	Interpolate(trc_index, tone_curve_target, trc_raw, interval);
	for (int i = 0; i < LUT_array_size; i++) {
		temp[i] = result_LUT[trc_index[i]];
	}
	for (int i = 0; i < LUT_array_size; i++) {
		result_LUT[i] = temp[i];
	}
}

void Calibrate::Tone_Curve_Correction(const vector<double>& Lv, const double tone_curve_target, vector<int>& trc_index, const int interval) {
	vector<double> target_trc_Lv;
	for (int i = 0; i <= 1024; i += interval) {
		i = (i == 1024) ? 1023 : i;
		target_trc_Lv.push_back(pow(i / 1023.0, tone_curve_target) * Lv[Lv.size() - 1]);
	}

	int index;
	double dis_n;
	double dis_ca;
	for (int i = 0; i < Lv.size(); i++) {
		dis_ca = MAXINT;
		for (int j = 0; j < Lv.size(); j++) {
			dis_n = abs(target_trc_Lv[i] - Lv[j]);
			if (abs(dis_n) < abs(dis_ca)) {
				dis_ca = dis_n;
				index = (j == Lv.size() - 1) ? j * interval - 1 : j * interval;
			}
		}
		trc_index.push_back(index);
	}
	trc_index[trc_index.size() - 1] = LUT_array_size - 1;
}

void Calibrate::Interpolate(vector<int> &index, const double tone_curve_target, const double trc_raw, const int interval) {
	vector<int> index_temp = index;
	int v;
	int i1, i2;
	int l;
	//int steps = LUT_array_size / index.size();
	index.clear();
	for (int i = 0; i < LUT_array_size; i++) {
		//i = (i == 1024) ? 1023 : i;
		i1 = floor(i / double(interval));
		i2 = ceil(i / double(interval));
		l = index_temp[i2] - index_temp[i1];
		v = round(index_temp[i1] + (pow((i % interval) / double(interval), tone_curve_target / trc_raw)) * l);
		index.push_back(v);
	}
}

void Calibrate::Cal_DICOM_LUT(PRGBColor dicom_LUT, const double min_Lv, const double max_Lv) {
	vector<double> target_dicom_list = dicom_XYZ(min_Lv, max_Lv);
	double index;
	uint16_t v;
	for (int i = 0; i < 1024; i++) {
		index = 1023.0 * target_dicom_list[i];
		v = round(index); 
		dicom_LUT[i] = RGBColor{ v,v,v };
	}
}

double Calibrate::L_to_JND(const double L)
{
	int JND = 0;
	JND = round(71.498068 + 94.593053 * log10(L) + 41.912053 * (pow(log10(L), 2)) + 9.8247004 * (pow(log10(L), 3))
		+ 0.28175407 * (pow(log10(L), 4)) - 1.1878455 * (pow(log10(L), 5))
		- 0.18014349 * (pow(log10(L), 6)) + 0.14710899 * (pow(log10(L), 7)) - 0.017046845 * (pow(log10(L), 8)));
	return JND;
}

double Calibrate::JND_to_L(const double JND)
{
	double j = JND;
	double d = -1.3011877 + 0.080242636 * log(j) + 0.13646699 * (pow(log(j), 2)) - 0.025468404 * (pow(log(j), 3)) + 0.0013635334 * (pow(log(j), 4));
	double m = 1 - 0.025840191 * log(j) - 0.10320229 * (pow(log(j), 2)) + 0.028745620 * (pow(log(j), 3)) - 0.0031978977 * (pow(log(j), 4)) + 0.00012992634 * (pow(log(j), 5));
	double log10L = d / m;
	double L = pow(10, log10L);
	return L;
}

double Calibrate::j_p(const double Jmax, const double Jmin, const int n, const int p)
{
	double jp = Jmin + (p / (pow(2, n) - 1)) * (Jmax - Jmin);
	return jp;
}

vector<double> Calibrate::dicom_XYZ(const double min_Lv, const double max_Lv) {
	vector<double> target_dicom_list;
	double J_max = L_to_JND(max_Lv);
	double J_min = L_to_JND(min_Lv);
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
			L_D = (L_D + min_Lv) / max_Lv;
		}
		target_dicom_list.push_back(L_D);
	}
	target_dicom_list[0] = 0;
	target_dicom_list[target_dicom_list.size() - 1] = 1;
	return target_dicom_list;
}

#pragma endregion

void Calibrate::row_minus(vector<float>& X, const vector<float>& Y, const double multi) {
	for (int i = 0; i < 6; i++) {
		X[i] -= Y[i] * multi;
	}
}

void Calibrate::single_row_divide(vector<float>& X, const double divide) {
	for (int i = 0; i < 6; i++) {
		X[i] /= divide;
	}
}

MATRIX Calibrate::Matrix_Inv(MATRIX matrix) { // this function will get the  inverse of the matrix
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

MATRIX Calibrate::allocateMatrix(const int row, const int col)
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

MATRIX Calibrate::Multiply(const MATRIX& m1, const MATRIX& m2) {
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

