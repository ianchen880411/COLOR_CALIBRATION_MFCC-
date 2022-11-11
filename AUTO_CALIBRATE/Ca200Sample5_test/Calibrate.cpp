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

Calibrate::Calibrate(int panel_bitdepth) {
	Calibrate::panel_bitdepth = panel_bitdepth;
	panel_RGB_max = pow(2, panel_bitdepth) - 1;
	Init_LUT();
}

Calibrate::~Calibrate() {
}

#pragma region  Colour Science
vector<double> Calibrate::XYZ_to_Lab(const vector<double> XYZ, const vector<double> ref_XYZ) {
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

double Calibrate::xy_to_CCT(const double x, const double y) {
	double n = (x - 0.332) / (0.1858 - y);
	double CCT = pow(449 * n, 3) + pow(3525 * n, 2) + 6823.3 * n + 5520.33;
	return CCT;
}

MATRIX Calibrate::Build_Matrix_RGB_to_XYZ(const vector<double> XYZ_R, const vector<double> XYZ_G, const vector<double> XYZ_B, 
													const vector<double> XYZ_W, const vector<double> XYZ_0, const double Y_max) {
	vector<float>XYZ_R_nor;
	vector<float>XYZ_G_nor;
	vector<float>XYZ_B_nor;
	vector<float>XYZ_W_nor;
	for (int i = 0; i < 3; i++) {
		XYZ_R_nor.push_back((XYZ_R[i] - XYZ_0[i]) / Y_max);
		XYZ_G_nor.push_back((XYZ_G[i] - XYZ_0[i]) / Y_max);
		XYZ_B_nor.push_back((XYZ_B[i] - XYZ_0[i]) / Y_max);
		XYZ_W_nor.push_back((XYZ_W[i] - XYZ_0[i]) / Y_max);
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

vector<float> Calibrate::W_Transform(MATRIX& matrix_RGB_XYZ, const int cct_target) {
	vector<float> xyz = CCT_to_xy(cct_target);
	xyz.push_back(1.0 - xyz[0] - xyz[1]);
	float r = 1.0 / xyz[1];
	for (int i = 0; i < 3; i++) {
		xyz[i] *= r;
	}
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

MATRIX Calibrate::Build_Matrix_RGB_to_Gamut_RGB(MATRIX& matrix_native_RGB_XYZ, const CString gamut_target, const int cct_target) {

	MATRIX matrix_gamut_target = mp_matrix_gamut_target[gamut_target];
	MATRIX matrix_gamut_trans = Multiply(Matrix_Inv(matrix_native_RGB_XYZ), matrix_gamut_target);

	MATRIX matrix_gamut_transed = Multiply(matrix_native_RGB_XYZ, matrix_gamut_trans);

	vector<float> w_adj = W_Transform(matrix_gamut_transed, cct_target);
	double w_adj_m_max = *max_element(w_adj.begin(), w_adj.end());
	for (int i = 0; i < 3; i++) {
		w_adj[i] /= w_adj_m_max;
	}

	MATRIX matrix_cct_trans =
	{ {w_adj[0], 0, 0},
	 {0, w_adj[1], 0},
	 {0, 0, w_adj[2]} };
	matrix_gamut_trans = Multiply(matrix_gamut_trans, matrix_cct_trans);

	return matrix_gamut_trans;
}

MATRIX Calibrate::Build_Matrix_RGB_to_CCT_RGB(MATRIX& matrix_native_RGB_XYZ, const int cct_target) {
	vector<float> w_adj = W_Transform(matrix_native_RGB_XYZ, cct_target);
	double w_adj_m_max = *max_element(w_adj.begin(), w_adj.end());
	static const vector<double>t{ panel_tone_curve.r, panel_tone_curve.g, panel_tone_curve.b };
	for (int i = 0; i < 3; i++) {
		w_adj[i] = pow(w_adj[i] / w_adj_m_max, 1 / t[i]) * w_adj_m_max;
	}
	MATRIX matrix_cct_trans =
	{ {w_adj[0], 0, 0},
	 {0, w_adj[1], 0},
	 {0, 0, w_adj[2]} };

	return matrix_cct_trans;
}
#pragma endregion

#pragma region Fit gamma

void Calibrate::row_minus(vector<float>& X, vector<float>& Y, double multi) {
	for (int i = 0; i < 6; i++) {
		X[i] -= Y[i] * multi;
	}
}

void Calibrate::single_row_divide(vector<float>& X, double divide) {
	for (int i = 0; i < 6; i++) {
		X[i] /= divide;
	}
}

double Calibrate::Calculate_Gamma_MSE(const vector<double> lv, const vector<double> std_lv, double gamma) {
	double temp = 0;
	double temp2;
	for (int i = 0; i < lv.size(); i++) {
		temp2 = pow(lv[i], 1.0 / gamma) - std_lv[i];
		temp += temp2 * temp2;
	}
	return temp;
}

double Calibrate::Calculate_TC(const vector<double> lv) {
	vector<double> std_lv;
	int cnt;
	int interval;
	double tc = 3;
	double unit = 1;

	interval = pow(2, panel_bitdepth) / lv.size();
	for (int i = 0; i < lv.size(); i++) {
		cnt = min(i * interval, panel_RGB_max);
		std_lv.push_back(double(cnt) / panel_RGB_max);
	}

	for (int i = 1; i <= 4; i++) {
		double min = 999999999;
		unit *= 0.1;
		for (int j = 0; j <= 40; j++) {
			double temp = Calculate_Gamma_MSE(lv, std_lv, (tc - unit * 20) + j * unit);
			if (temp < min) {
				min = temp;
			}
			else {
				tc = (tc - unit * 20) + (j - 1) * unit;
				break;
			}
		}
	}
	return tc;
}
#pragma endregion

void Calibrate::Init_LUT() {
	uint16_t v;
	for (uint16_t i = 0; i < virtul_LUT_size; i++) {
		v = (i + 1) * virtul_LUT_interval - 1;

		test_lut[i].rgbColor = RGBColor{ v, v, v };
		test_lut[i].dis = MAXINT;
		test_lut[i].XYZ = XYZ{ 0.0,0.0,0.0 };
		test_lut[i].xyz = sxyz{ 0.0,0.0,0.0 };

		calibrated_lut[i].rgbColor = RGBColor{ v, v, v };
		calibrated_lut[i].dis = MAXINT;
		calibrated_lut[i].XYZ = XYZ{ 0.0,0.0,0.0 };
		calibrated_lut[i].xyz = sxyz{ 0.0,0.0,0.0 };
	}

	for (uint16_t i = 0; i < LUT_array_size; i++) {
		one_d_lut[i].rgbColor = RGBColor{ i,i,i };
	}
}

#pragma region CCT_AUTO
void Calibrate::Read_Test_Data(vector<vector<double>>* data) {
	for (int i = 0; i < virtul_LUT_size; i++) {
		test_lut[i].XYZ.X = (*data)[i][3];
		test_lut[i].XYZ.Y = (*data)[i][4];
		test_lut[i].XYZ.Z = (*data)[i][5];
		test_lut[i].xyz.x = (*data)[i][7];
		test_lut[i].xyz.y = (*data)[i][8];
		test_lut[i].xyz.z = (*data)[i][9];
	}
}
//計算測試 LUT
void Calibrate::Cal_CCT_LUT(vector<vector<double>>* data, const int cct) {
	double targetx, targety;
	double dis_x, dis_y, dis_z; //與target xy的距離 
	double random_r, random_g, random_b; //用於當調整值過小時 讓他跳出某個範圍 以利尋找最優解
	double s; //根據灰階大小來計算調整值大小 例如灰階100的調整值大概是1~2位數 而1000可能會到1~3位數
	double vR, vG, vB; //R、G、B 調整值
	int grayscale;

	Read_Test_Data(data);

	vector<float>xy = CCT_to_xy(cct);
	targetx = xy[0];
	targety = xy[1];

	for (int i = 0; i < virtul_LUT_size; i++) {
		dis_x = targetx - test_lut[i].xyz.x;
		dis_y = targety - test_lut[i].xyz.y;
		dis_z = (1.0 - targetx - targety) - test_lut[i].xyz.z;

		Analysis(test_lut[virtul_LUT_size - 1].XYZ.Y, targetx, targety);

		random_r = (2.0 - 1) * rand() / (RAND_MAX + 1.0) + 1;
		random_g = (2.0 - 1) * rand() / (RAND_MAX + 1.0) + 1;
		random_b = (2.0 - 1) * rand() / (RAND_MAX + 1.0) + 1;
		s = virtul_LUT_interval * (i + 1);
		vR = dis_x * s / targetx;
		vG = dis_y * s / targety;
		vB = dis_z * s;
		vR = (abs(vR) < 1.0) ? vR * random_r : vR;
		vG = (abs(vG) < 1.0) ? vG * random_g : vG;
		vB = (abs(vB) < 1.0) ? vB * random_b : vB;
		test_lut[i].rgbColor.Red += uint16_t(round(vR));
		test_lut[i].rgbColor.Green += uint16_t(round(vG));
		test_lut[i].rgbColor.Blue += uint16_t(round(vB));

		grayscale = (i + 1) * virtul_LUT_interval - 1; //計算所對應的0~1023灰階階數

		Mark(test_lut[i].rgbColor.Red, test_lut[i].rgbColor.Green, test_lut[i].rgbColor.Blue, grayscale);

		//更新下一回合測試的 RGB 值
		one_d_lut[grayscale].rgbColor.Red = test_lut[i].rgbColor.Red;
		one_d_lut[grayscale].rgbColor.Green = test_lut[i].rgbColor.Green;
		one_d_lut[grayscale].rgbColor.Blue = test_lut[i].rgbColor.Blue;
	}
}

//分析測試 LUT 的校正測量結果
void Calibrate::Analysis(const double Y, const double targetx, const double targety) {
	double dis;

	vector<double> ref_white;
	vector<double> Lab;
	vector<double> ref_Lab;

	ref_white.push_back(Y * targetx / targety);
	ref_white.push_back(Y);
	ref_white.push_back(Y * (1 - targetx - targety) / targety);

	for (int i = 0; i < virtul_LUT_size; i++) {
		Lab = XYZ_to_Lab({ test_lut[i].XYZ.X,test_lut[i].XYZ.Y,test_lut[i].XYZ.Z }, ref_white);
		ref_Lab = XYZ_to_Lab({ test_lut[i].XYZ.Y * targetx / targety,test_lut[i].XYZ.Y,test_lut[i].XYZ.Y * (1.0 - targetx - targety) / targety, }, ref_white);

		dis = sqrt(pow(ref_Lab[1] - Lab[1], 2.0) + pow(ref_Lab[2] - Lab[2], 2.0));

		if (dis < calibrated_lut[i].dis) {
			calibrated_lut[i].rgbColor = test_lut[i].rgbColor;
			calibrated_lut[i].xyz = test_lut[i].xyz;
			calibrated_lut[i].dis = dis;
		}
	}
}

//具體是做類似於根據灰階階數來做比例與tone curve mapping計算
void Calibrate::Mark(uint16_t& R, uint16_t& G, uint16_t& B, int i) {
	uint16_t max_RGB = max(R, G);
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

//待校正後確認16個點的 RGB 值後 將one_d_lut 0~1023 計算出來
void Calibrate::Get_CCT_LUT(LUTable1D cct_LUT[]) {
	int max_RGB;
	uint16_t R, G, B;
	int j;
	double ref_grayscale;
	for (int i = 0; i < 1024; i++) {
		j = ceil(double(i + 1) * virtul_LUT_size / 1024.0) - 1;
		ref_grayscale = (j + 1) * virtul_LUT_interval - 1;
		if ((i + 1) % int(virtul_LUT_interval) == 0) {
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

		cct_LUT[i].rgbColor.Red = R;
		cct_LUT[i].rgbColor.Green = G;
		cct_LUT[i].rgbColor.Blue = B;
	}
}
#pragma endregion

#pragma region Tone Curve Correction
vector<int> Calibrate::Tone_Curve_Correction(const vector<double>lv, const double tone_curve_target, const int interval) {
	vector<int> tc_index;
	vector<double> target_tc_lv;
	for (int i = 0; i <= LUT_array_size; i += interval) {
		i = (i == LUT_array_size) ? panel_RGB_max : i;
		target_tc_lv.push_back(pow(i / double(panel_RGB_max), tone_curve_target) * lv[lv.size() - 1]);
	}

	int index;
	double dis_n;
	double dis_ca;
	for (int i = 0; i < lv.size(); i++) {
		dis_ca = MAXINT;
		for (int j = 0; j < lv.size(); j++) {
			dis_n = abs(target_tc_lv[i] - lv[j]);
			if (abs(dis_n) < abs(dis_ca)) {
				dis_ca = dis_n;
				index = (j == lv.size() - 1) ? j * interval - 1 : j * interval;
			}
		}
		tc_index.push_back(index);
	}
	return tc_index;
}

void Calibrate::Interpolate(vector<int> &index, const double tone_curve_target, const int interval) {
	vector<int> index_temp = index;
	int v;
	int i1, i2;
	int l;
	index.clear();
	for (int i = 0; i < LUT_array_size; i++) {
		i = (i == LUT_array_size) ? panel_RGB_max : i;
		i1 = floor(i / double(interval));
		i2 = ceil(i / double(interval));
		l = index_temp[i2] - index_temp[i1];
		v = round(index_temp[i1] + pow((i % interval) / double(interval), tone_curve_target) * l);
		index.push_back(v);
	}
}

void Calibrate::CCT_LUT_Tone_Curve_Trans(LUTable1D cct_LUT[], const vector<double>lv, const double tone_curve_target, const int interval) {
	LUTable1D temp_LUT[LUT_array_size];
	vector<int> calibrated_tc_index;
	for (int i = 0; i < LUT_array_size; i++) {
		temp_LUT[i] = cct_LUT[i];
	}

	calibrated_tc_index = Tone_Curve_Correction(lv, tone_curve_target, interval);
	Interpolate(calibrated_tc_index, tone_curve_target, interval);
	int index;
	for (int i = 0; i < LUT_array_size; i++) {
		index = calibrated_tc_index[i];
		cct_LUT[i] = temp_LUT[index];
	}
}

void Calibrate::Generate_Gamma_LUT(LUTable1D LUT[], const double tone_curve_raw, const double tone_curve_target) {
	uint16_t v;
	for (int i = 0; i < 1024; i++) {
		v = round(pow((double)i / panel_RGB_max, tone_curve_target / tone_curve_raw) * panel_RGB_max);
		LUT[i].rgbColor = RGBColor{ v,v,v };
	}
}
#pragma endregion

#pragma region DICOM Correction
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
	double jp = Jmin + (double(p) / (pow(2, n) - 1)) * (Jmax - Jmin);
	return jp;
}

vector<double> Calibrate::dicom_XYZ(const double max_lv) {
	vector<double> target_dicom_list;
	double J_max = L_to_JND(max_lv);
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
			L_D /= max_lv;
		}
		target_dicom_list.push_back(L_D);
	}
	target_dicom_list[0] = 0;
	target_dicom_list[target_dicom_list.size() - 1] = 1;
	return target_dicom_list;
}

void Calibrate:: Generate_DICOM_LUT(const LUTable1D base_lut[], LUTable1D dicom_lut[], const double max_lv, const double tone_curve_raw) {
	vector<double> target_dicom_list = dicom_XYZ(max_lv);
	double index;
	uint16_t v;
	for (int i = 0; i < LUT_array_size; i++) {
		index = i * target_dicom_list[i];
		index = pow((index / base_lut[panel_RGB_max].rgbColor.Green), 1 / tone_curve_raw) * double(panel_RGB_max);
		v = round(index);
		dicom_lut[i].rgbColor = RGBColor{ v,v,v };
	}
}
#pragma endregion

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

MATRIX Calibrate::Multiply(const MATRIX m1, const MATRIX m2) {
	int row = m1.size();
	int col = m2[0].size();
	double v;
	MATRIX m = allocateMatrix(row, col);

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				m[i][j] += m1[i][k] * m2[k][j];
				v = m1[i][k] * m2[k][j];

			}
		}
	}
	return m;
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

