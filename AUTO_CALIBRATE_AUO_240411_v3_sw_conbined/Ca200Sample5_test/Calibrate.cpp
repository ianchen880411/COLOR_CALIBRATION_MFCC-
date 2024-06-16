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

Calibrate::Calibrate() {
}

Calibrate::~Calibrate() {

}

void Calibrate::Configure_one_d_calibration_attribute(const uint16_t bit_depth = 10, const uint16_t one_d_calibration_lut_size = 16) {
	(bit_depth > 8) ? this->display_bit_depth = bit_depth : error = false;

	this->one_d_full_lut_size = pow(2, bit_depth);
	this->RGB_max = one_d_full_lut_size - 1;
	
	(one_d_calibration_lut_size > 0) ? this->one_d_calibration_lut_size = one_d_calibration_lut_size : error = false; //= the size of the 1D lut for calibration
	this->one_d_calibration_lut_interval = one_d_full_lut_size / this->one_d_calibration_lut_size;;

	this->test_one_d_LUT_ptr = new TEST_LUT[this->one_d_calibration_lut_size];
	this->calibrated_one_d_LUT_ptr = new CALIBRATED_LUT[this->one_d_calibration_lut_size];
}


void Calibrate::Virtual_Video_Pipeline(RGBColor RGB_raw, RGBColor& RGB_new, PRGBColor gamma_color_encode_LUT, MATRIX gamut_matrix, MATRIX cct_matrix, PRGBColor gamma_decode_LUT, PRGBColor base_LUT, double gamma) {
	// Gamma encode LUT -> CCT & Gamut Matrix -> Gamma decode LUT -> 1D D65 Base LUT
	RGBColor RGB_color_gamma_encode;
	RGBColor RGB_cct_gamut_matrix;
	RGBColor RGB_gamma_decode;
	RGBColor RGB_base_LUT;

	float R_m = pow(RGB_raw.Red / RGB_max, gamma) * RGB_max;
	float G_m = pow(RGB_raw.Green / RGB_max, gamma) * RGB_max;
	float B_m = pow(RGB_raw.Blue / RGB_max, gamma) * RGB_max;

	float r_m = R_m * gamut_matrix[0][0] + G_m * gamut_matrix[0][1] + B_m * gamut_matrix[0][2];
	float g_m = R_m * gamut_matrix[1][0] + G_m * gamut_matrix[1][1] + B_m * gamut_matrix[1][2];
	float b_m = R_m * gamut_matrix[2][0] + G_m * gamut_matrix[2][1] + B_m * gamut_matrix[2][2];

	r_m = min(max(r_m, 0), RGB_max);
	g_m = min(max(g_m, 0), RGB_max);
	b_m = min(max(b_m, 0), RGB_max);
	
	float R = pow(r_m / 1023.0, 1 / panel_tone_curve.w) * 1023;
	float G = pow(g_m / 1023.0, 1 / panel_tone_curve.w) * 1023;
	float B = pow(b_m / 1023.0, 1 / panel_tone_curve.w) * 1023;

	RGB_base_LUT.Red = base_LUT[int(round(R))].Red;
	RGB_base_LUT.Green = base_LUT[int(round(G))].Green;
	RGB_base_LUT.Blue = base_LUT[int(round(B))].Blue;

	RGB_new = RGB_base_LUT;
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

double Calibrate::calculate_trc(vector<double> lv) {
	vector<double> std_lv;
	double cnt;
	int step = (lv.size() >= 1024) ? 1 : one_d_full_lut_size / (lv.size() - 1);
	for (int i = 0; i < lv.size(); i++) {
		cnt = min(i * step, RGB_max);
		std_lv.push_back(cnt / RGB_max);
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


void Calibrate::Interpolate(vector<int>& index, const int interval) {
	vector<int> index_temp = index;
	int v;
	int i1, i2;
	int l;
	index.clear();
	for (int i = 0; i < 1024; i++) {
		i1 = floor(i / double(interval));
		i2 = ceil(i / double(interval));
		l = index_temp[i2] - index_temp[i1];
		v = round(index_temp[i1] + ((i % interval) / double(interval)) * l);
		index.push_back(v);
	}
}


void Calibrate::Generate_Gamma_LUT(PRGBColor gamma_LUT, const double gamma_target) {
	uint16_t v = 0;
	for (uint16_t i = 0; i < one_d_full_lut_size; i++) {
		v = round(pow(i / RGB_max, gamma_target) * RGB_max);
		gamma_LUT[i] = RGBColor{ v,v,v };
	}
}
#pragma endregion


#pragma region Color Matrix
void Calibrate::RGB_to_XYZ(MATRIX matrix_RGB_to_XYZ, RGBColor RGB, const double tone_curve, vector<double>XYZ_W, vector<double>XYZ_0, vector<double>& XYZ) {
	float r, g, b;
	
	r = (RGB.Red == 0) ? 0 : pow(RGB.Red / RGB_max, tone_curve);
	g = (RGB.Green == 0) ? 0 : pow(RGB.Green / RGB_max, tone_curve);
	b = (RGB.Blue == 0) ? 0 : pow(RGB.Blue / RGB_max, tone_curve);

	vector<float>rgb = { r,g,b };
	for (int j = 0; j < 3; j++) {
		XYZ.push_back(0);
		for (int k = 0; k < 3; k++) {
			XYZ[j] += matrix_RGB_to_XYZ[j][k] * rgb[k];
		}
		XYZ[j] *= (XYZ_W[1] - XYZ_0[1]);
		XYZ[j] += XYZ_0[j];
	}
}


MATRIX Calibrate::Build_Matrix_RGB_to_XYZ(vector<double>& XYZ_R, vector<double>& XYZ_G, vector<double>& XYZ_B, vector<double>& XYZ_W, vector<double>& XYZ_0) {
	vector<double>XYZ_R_nor;
	vector<double>XYZ_G_nor;
	vector<double>XYZ_B_nor;
	vector<double>XYZ_W_nor;;

	double Y_max = XYZ_W[1] - XYZ_0[1];

	for (int i = 0; i < 3; i++) {
		XYZ_R_nor.push_back((XYZ_R[i] - XYZ_0[i]) / Y_max);
		XYZ_G_nor.push_back((XYZ_G[i] - XYZ_0[i]) / Y_max);
		XYZ_B_nor.push_back((XYZ_B[i] - XYZ_0[i]) / Y_max);
		XYZ_W_nor.push_back((XYZ_W[i] - XYZ_0[i]) / Y_max);
	}


	float xR = XYZ_R_nor[0] / XYZ_R_nor[1];
	float yR = XYZ_R_nor[1] / XYZ_R_nor[1];
	float zR = XYZ_R_nor[2] / XYZ_R_nor[1];

	float xG = XYZ_G_nor[0] / XYZ_G_nor[1];
	float yG = XYZ_G_nor[1] / XYZ_G_nor[1];
	float zG = XYZ_G_nor[2] / XYZ_G_nor[1];

	float xB = XYZ_B_nor[0] / XYZ_B_nor[1];
	float yB = XYZ_B_nor[1] / XYZ_B_nor[1];
	float zB = XYZ_B_nor[2] / XYZ_B_nor[1];

	MATRIX Xi = 
	{ {xR, xG, xB},
	  {yR, yG, yB},
	  {zR, zG, zB}};

	MATRIX Xi_inv = Matrix_Inv(Xi);

	vector<float> w{ 0,0,0 };
	for (int j = 0; j < 3; j++) {
		for (int k = 0; k < 3; k++) {
			w[j] += Xi_inv[j][k] * XYZ_W_nor[k];
		}
	}

	MATRIX M =
	{ {xR * w[0], xG * w[1], xB * w[2]},
	  {yR * w[0], yG * w[1], yB * w[2]},
	  {zR * w[0], zG * w[1], zB * w[2]} };

	return M;
}


void Calibrate::Build_Matrix_GAMUT_Trans(const MATRIX& matrix_native_RGB_XYZ, MATRIX& matrix_Gamut_trans, const CString gamut_target) {
	MATRIX matrix_gamut_target = mp_matrix_gamut_target[gamut_target];
	MATRIX matrix_CCT_trans;
	if (gamut_target == "Native") {
		matrix_Gamut_trans = matrix_raw;
	}
	else {
		vector<float> w_adj = W_Transform(matrix_gamut_target, 5000);
		float w_adj_m_max =  *max_element(w_adj.begin(), w_adj.end());
		float r = 1.0 / w_adj_m_max;

		matrix_CCT_trans =
		{ {w_adj[0] * r, 0, 0},
		 {0, w_adj[1] * r, 0},
		 {0, 0, w_adj[2] * r} };

		MATRIX matrix_gamut_t = Multiply(matrix_gamut_target, matrix_CCT_trans);

		matrix_Gamut_trans = Multiply(Matrix_Inv(matrix_native_RGB_XYZ), matrix_gamut_t);
	}

}


void Calibrate::Build_Matrix_CCT_Trans(const MATRIX& matrix_native_RGB_XYZ, MATRIX& matrix_CCT_trans, const int cct_target, const double tone_curve) {
	vector<float> w_adj = W_Transform(matrix_native_RGB_XYZ, cct_target);
	double w_adj_m_max = *max_element(w_adj.begin(), w_adj.end());

	if (tone_curve == 0) {
		for (int i = 0; i < 3; i++) {
			w_adj[i] /= w_adj_m_max;
			w_adj[i] = w_adj[i];
		}
	}
	else {
		for (int i = 0; i < 3; i++) {
			w_adj[i] /= w_adj_m_max;
			w_adj[i] = pow(w_adj[i], 1.0 / tone_curve);
		}
	}

	matrix_CCT_trans =
	{ {w_adj[0], 0, 0},
	 {0, w_adj[1], 0},
	 {0, 0, w_adj[2]} };
}


void Calibrate::Build_Matrix_Gamut_CCT_Trans(const MATRIX& matrix_native_RGB_XYZ, MATRIX& matrix_Gamut_CCT_trans, vector<double>XYZ_W, vector<double>XYZ_0, const CString gamut_target, const int cct_target) {
	MATRIX matrix_gamut_target;
	MATRIX matrix_gamut_transed;

	if (gamut_target == "Native") {
		matrix_Gamut_CCT_trans = matrix_raw;
	}
	else {
		matrix_gamut_target = mp_matrix_gamut_target[gamut_target];

		vector<double> XYZ_R_gamut, XYZ_G_gamut, XYZ_B_gamut, XYZ_W_gamut, XYZ_0_gamut;
		RGB_to_XYZ(matrix_gamut_target, RGBColor{uint16_t(RGB_max), 0, 0}, 1.0, XYZ_W, XYZ_0, XYZ_R_gamut);
		RGB_to_XYZ(matrix_gamut_target, RGBColor{0, uint16_t(RGB_max), 0 }, 1.0, XYZ_W, XYZ_0, XYZ_G_gamut);
		RGB_to_XYZ(matrix_gamut_target, RGBColor{ 0, 0, uint16_t(RGB_max) }, 1.0, XYZ_W, XYZ_0, XYZ_B_gamut);
		RGB_to_XYZ(matrix_gamut_target, RGBColor{ uint16_t(RGB_max), uint16_t(RGB_max), uint16_t(RGB_max) }, 1.0, XYZ_W, XYZ_0, XYZ_W_gamut);
		RGB_to_XYZ(matrix_gamut_target, RGBColor{ 0, 0, 0 }, 1.0, XYZ_W, XYZ_0, XYZ_0_gamut);

		vector<double> XYZ_R_adp, XYZ_G_adp, XYZ_B_adp, XYZ_W_adp, XYZ_0_adp;
		MATRIX matrix_chromatic_adaptation;
		Chromatic_Adaptation(XYZ_R_gamut, XYZ_R_adp, matrix_chromatic_adaptation, XYZ_W_gamut, cct_target);
		Chromatic_Adaptation(XYZ_G_gamut, XYZ_G_adp, matrix_chromatic_adaptation, XYZ_W_gamut, cct_target);
		Chromatic_Adaptation(XYZ_B_gamut, XYZ_B_adp, matrix_chromatic_adaptation, XYZ_W_gamut, cct_target);
		Chromatic_Adaptation(XYZ_W_gamut, XYZ_W_adp, matrix_chromatic_adaptation, XYZ_W_gamut, cct_target);
		Chromatic_Adaptation(XYZ_0_gamut, XYZ_0_adp, matrix_chromatic_adaptation, XYZ_W_gamut, cct_target);

		matrix_Gamut_CCT_trans = Build_Matrix_RGB_to_XYZ(XYZ_R_adp, XYZ_G_adp, XYZ_B_adp, XYZ_W_adp, XYZ_0_adp);
		matrix_Gamut_CCT_trans = Multiply(Matrix_Inv(matrix_native_RGB_XYZ), matrix_Gamut_CCT_trans);

		double matrix_scale;
		for (int i = 0; i < 3; i++) {
			matrix_scale = 1.0 / (matrix_Gamut_CCT_trans[i][0] + matrix_Gamut_CCT_trans[i][1] + matrix_Gamut_CCT_trans[i][2]);
			matrix_Gamut_CCT_trans[i][0] *= matrix_scale;
			matrix_Gamut_CCT_trans[i][1] *= matrix_scale;
			matrix_Gamut_CCT_trans[i][2] *= matrix_scale;
		}
	}
}


void Calibrate::Chromatic_Adaptation(vector<double> XYZ_raw, vector<double>& XYZ_adp, MATRIX& matrix_chromatic_adaptation, vector<double> XYZ_raw_w, const int cct_target) {
	vector<float> XYZ_target_w = CCT_to_xyz_nor(cct_target);

	float XYZ_raw_w_max = *max_element(XYZ_raw_w.begin(), XYZ_raw_w.end());

	for (int i = 0; i < XYZ_raw_w.size(); i++) {
		XYZ_raw_w[i] /= XYZ_raw_w_max;
	}
	
	vector<float> cone_response_raw = { 0,0,0 };
	vector<float> cone_response_target = { 0,0,0 };
	for (int j = 0; j < 3; j++) {
		for (int k = 0; k < 3; k++) {
			cone_response_raw[j] += M_Bradford[j][k] * XYZ_raw_w[k];
			cone_response_target[j] += M_Bradford[j][k] * XYZ_target_w[k];
		}
	}

	MATRIX cone_response=
	{ {cone_response_target[0] / cone_response_raw[0], 0, 0},
	  {0, cone_response_target[1] / cone_response_raw[1], 0},
	  {0, 0, cone_response_target[2] / cone_response_raw[2]} };

	matrix_chromatic_adaptation = Multiply(Multiply(M_Bradford_inv, cone_response), M_Bradford);

	XYZ_adp = { 0,0,0 };
	for (int j = 0; j < 3; j++) {
		for (int k = 0; k < 3; k++) {
			XYZ_adp[j] += matrix_chromatic_adaptation[j][k] * XYZ_raw[k];
		}
	}
}
#pragma region CCT 1D LUT calibration

void Calibrate::Set_Calibration_Target(const int cct_target, const TRC tone_curve) {
	vector<float>xy = CCT_to_xy(cct_target);
	targetx = xy[0];
	targety = xy[1];
	this->panel_tone_curve = tone_curve;
}

void Calibrate::CCT_Calibration(RGBColor RGB_raw, RGBColor& RGB_new, MATRIX m, vector<vector<double> >& data, int index, int grayscale_ref, vector<double>& Lv, int& count) {
	vector<double> ref_white = { data[0][4] * targetx / targety, data[0][4], data[0][4] * (1 - targetx - targety) / targety };

	double dis_x = targetx - data[0][7];
	double dis_y = targety - data[0][8];
	double dis_z = (1.0 - targetx - targety) - data[0][9];

	double dis = sqrt(pow(dis_x, 2.0) + pow(dis_y, 2.0));

	static double lim;
	if (data[0][4] < 2) {
		count = 11;
		Lv[index] = data[0][4];
		calibrated_one_d_LUT_ptr[index].rgbColor = RGB_raw;
	}
	else {
		if (dis < calibrated_one_d_LUT_ptr[index].dis) {
			if (grayscale_ref == 1023) {
				lim = 0.00005;
			}
			else {
				lim = 0.001;
			}

			calibrated_one_d_LUT_ptr[index].index = count;
			if (abs(dis_x) <= lim && abs(dis_y) <= lim) {
				count = 11;
			}
			calibrated_one_d_LUT_ptr[index].rgbColor = RGB_raw;
			calibrated_one_d_LUT_ptr[index].xyz = { data[0][3], data[0][4], data[0][5] };
			calibrated_one_d_LUT_ptr[index].dis = dis;
			Lv[index] = data[0][4];
		}
	}

	MATRIX CCT_T;
	static vector<double> RGB_temp;
	static RGBColor RGB_me;
	static vector<double> xyz_t;
	static int mo_r;
	static int mo_b;

	static double dis_x_raw;
	static double dis_z_raw;

	RGB_temp = { 0,0,0 };
	if (grayscale_ref == 1023) {
		CCT_T = Matrix_Inv(m);
		xyz_t = { dis_x,dis_y,dis_z };
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				RGB_temp[j] += CCT_T[j][k] * xyz_t[k];
			}
		}

		int rv = round(RGB_temp[0] * RGB_raw.Red);
		int gv = round(RGB_temp[1] * RGB_raw.Green);
		int bv = round(RGB_temp[2] * RGB_raw.Blue);

		rv = (rv == 0) ? -1 : rv;
		gv = (gv == 0) ? -1 : gv;
		bv = (bv == 0) ? -1 : bv;
		                                         
		RGB_temp[0] = RGB_raw.Red + rv;
		RGB_temp[1] = RGB_raw.Green + gv;
		RGB_temp[2] = RGB_raw.Blue + bv;
	}
	else{
		switch (count)
		{
		case 0:
			CCT_T = Matrix_Inv(m);
			xyz_t = { dis_x,dis_y,dis_z };
			for (int j = 0; j < 3; j++) {
				for (int k = 0; k < 3; k++) {
					RGB_temp[j] += CCT_T[j][k] * xyz_t[k]; 
				}
			}

			mo_r = round(RGB_temp[0] * RGB_raw.Red) + round(RGB_temp[1] * RGB_raw.Green);
			mo_b = round(RGB_temp[2] * RGB_raw.Blue);

			mo_r = (abs(mo_r) == 0) ? 1 : mo_r;
			mo_b = (abs(mo_b) == 0) ? 1 : mo_b;

			RGB_me = RGB_raw;

			RGB_temp[0] = RGB_me.Red + mo_r;
			RGB_temp[2] = RGB_me.Blue;
			break;
		case 1:
			RGB_temp[0] = RGB_me.Red;
			RGB_temp[2] = RGB_me.Blue + mo_b;
			break;
		case 2:      
			RGB_temp[0] = RGB_me.Red + mo_r;
			RGB_temp[2] = RGB_me.Blue + mo_b;
			break;
		case 3:
			mo_r = (mo_r == 1) ? 2 : 1;
			mo_b = (mo_b == 1) ? 2 : 1;
			RGB_temp[0] = RGB_me.Red + mo_r;
			RGB_temp[2] = RGB_me.Blue + mo_b;
			break;
		case 4:
			RGB_temp[0] = RGB_me.Red + mo_r;
			RGB_temp[2] = RGB_me.Blue;
			break;
		case 5:
			RGB_temp[0] = RGB_me.Red;
			RGB_temp[2] = RGB_me.Blue + mo_b;
			break;
		case 6:
			RGB_temp[0] = RGB_me.Red - mo_r;
			RGB_temp[2] = RGB_me.Blue;
			break;
		case 7:
			RGB_temp[0] = RGB_me.Red;
			RGB_temp[2] = RGB_me.Blue - mo_b;
			break;
		case 8:
			RGB_temp[0] = RGB_me.Red + mo_r;
			RGB_temp[2] = RGB_me.Blue - mo_b;
			break;
		case 9:
			RGB_temp[0] = RGB_me.Red - mo_r;
			RGB_temp[2] = RGB_me.Blue + mo_b;
			break;
		case 10:
			RGB_temp[0] = RGB_me.Red - mo_r;
			RGB_temp[2] = RGB_me.Blue - mo_b;
			break;
		default:
			break;
		}

		RGB_temp[1] = RGB_me.Green;

		
		if (RGB_temp[0] > calibrated_one_d_LUT_ptr[grayscale_ref + 1].rgbColor.Red) {
			RGB_temp[0] = calibrated_one_d_LUT_ptr[grayscale_ref + 1].rgbColor.Red;
		}


		if (RGB_temp[2] > calibrated_one_d_LUT_ptr[grayscale_ref + 1].rgbColor.Blue) {
			RGB_temp[2] = calibrated_one_d_LUT_ptr[grayscale_ref + 1].rgbColor.Blue;
		}
		
	}


	RGB_new.Red = round(min(max(RGB_temp[0], 0), 1023));
	RGB_new.Green = round(min(max(RGB_temp[1], 0), 1023));
	RGB_new.Blue = round(min(max(RGB_temp[2], 0), 1023));
}


void Calibrate::Init_LUT() {
	for (uint16_t i = 0; i < one_d_calibration_lut_size; i++) {
		calibrated_one_d_LUT_ptr[i].dis = MAXINT;
		test_one_d_LUT_ptr[i].rgbColor.Red = (i + 1) * one_d_calibration_lut_interval - 1;
		test_one_d_LUT_ptr[i].rgbColor.Green = (i + 1) * one_d_calibration_lut_interval - 1;
		test_one_d_LUT_ptr[i].rgbColor.Blue = (i + 1) * one_d_calibration_lut_interval - 1;
	}
}

//分析測試 LUT 的校正測量結果
void Calibrate::Analysis() {

	double dis_x = MAXINT;
	double dis_y = MAXINT;
	double dis_z = MAXINT;
	double dis = MAXINT;


	vector<double> ref_white;
	double ref_Y;
	vector<double> Lab;
	vector<double> ref_Lab;

	ref_white = { test_one_d_LUT_ptr[one_d_calibration_lut_size - 1].XYZ.Y * targetx / targety, test_one_d_LUT_ptr[one_d_calibration_lut_size - 1].XYZ.Y, test_one_d_LUT_ptr[one_d_calibration_lut_size - 1].XYZ.Y * (1 - targetx - targety) / targety };

	for (int i = 0; i < one_d_calibration_lut_size; i++) {
		dis_x = targetx - test_one_d_LUT_ptr[i].xyz.x;
		dis_y = targety - test_one_d_LUT_ptr[i].xyz.y;
		dis_z = (1.0 - targetx - targety) - test_one_d_LUT_ptr[i].xyz.z;

		Lab = XYZ_to_Lab({ test_one_d_LUT_ptr[i].XYZ.X, test_one_d_LUT_ptr[i].XYZ.Y, test_one_d_LUT_ptr[i].XYZ.Z }, ref_white);
		ref_Y = test_one_d_LUT_ptr[i].XYZ.Y;
		ref_Lab = XYZ_to_Lab({ ref_Y * targetx / targety, ref_Y, ref_Y * (1.0 - targetx - targety) / targety, }, ref_white);

		dis = sqrt(pow(ref_Lab[1] - Lab[1], 2.0) + pow(ref_Lab[2] - Lab[2], 2.0));
		//dis = sqrt(pow(dis_x, 2.0) + pow(dis_y, 2.0) + pow(dis_z, 2.0));

		if (dis < calibrated_one_d_LUT_ptr[i].dis) {
			calibrated_one_d_LUT_ptr[i].rgbColor = test_one_d_LUT_ptr[i].rgbColor;
			calibrated_one_d_LUT_ptr[i].xyz = test_one_d_LUT_ptr[i].xyz;
			calibrated_one_d_LUT_ptr[i].dis = dis;
		}
	}
}

void Calibrate::Cal_CCT_LUT(vector<MATRIX>M, PRGBColor virtual_LUT) {
	MATRIX RGB_XYZ;
	MATRIX CCT_T;
	vector<float> xyz_d = CCT_to_xy(6500);
	xyz_d.push_back(1.0 - xyz_d[0] - xyz_d[1]);
	vector<float> xyz_t;
	float dis_x, dis_y, dis_z;
	vector<double> RGB_temp;
	int grayscale;
	double scale;
	for (int i = 0; i < one_d_calibration_lut_size; i++) {
		grayscale = (i + 1) * one_d_calibration_lut_interval - 1; //計算所對應的0~1023灰階階數
		if (grayscale < 16) {
			test_one_d_LUT_ptr[i].rgbColor.Red = grayscale;
			test_one_d_LUT_ptr[i].rgbColor.Green = grayscale;
			test_one_d_LUT_ptr[i].rgbColor.Blue = grayscale;

			//更新下一回合測試的 RGB 值
			virtual_LUT[grayscale].Red = test_one_d_LUT_ptr[i].rgbColor.Red;
			virtual_LUT[grayscale].Green = test_one_d_LUT_ptr[i].rgbColor.Green;
			virtual_LUT[grayscale].Blue = test_one_d_LUT_ptr[i].rgbColor.Blue;

			continue;
		}

		RGB_XYZ = M[i];

		dis_x = xyz_d[0] - test_one_d_LUT_ptr[i].xyz.x;
		dis_y = xyz_d[1] - test_one_d_LUT_ptr[i].xyz.y;
		dis_z = xyz_d[2] - test_one_d_LUT_ptr[i].xyz.z;

		CCT_T = Matrix_Inv(RGB_XYZ);
		xyz_t = { dis_x,dis_y,dis_z };
		RGB_temp = { 0,0,0 };
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				RGB_temp[j] += CCT_T[j][k] * xyz_t[k];
			}
		}
	
		RGB_temp[0] = RGB_temp[0] * test_one_d_LUT_ptr[i].rgbColor.Red + test_one_d_LUT_ptr[i].rgbColor.Red;
		RGB_temp[1] = RGB_temp[1] * test_one_d_LUT_ptr[i].rgbColor.Green + test_one_d_LUT_ptr[i].rgbColor.Green;
		RGB_temp[2] = RGB_temp[2] * test_one_d_LUT_ptr[i].rgbColor.Blue + test_one_d_LUT_ptr[i].rgbColor.Blue;
		

		if (i < one_d_calibration_lut_size - 1 && (abs(RGB_temp[1] - grayscale) / double(grayscale))>(calibrated_one_d_LUT_ptr[one_d_calibration_lut_size - 1].rgbColor.Green - RGB_max) / RGB_max) {
			scale = grayscale / RGB_temp[1];
			RGB_temp[0] *= scale;
			RGB_temp[1] *= scale;
			RGB_temp[2] *= scale;
		}

		test_one_d_LUT_ptr[i].rgbColor.Red = min(max(RGB_temp[0], 0), RGB_max);
		test_one_d_LUT_ptr[i].rgbColor.Green = min(max(RGB_temp[1], 0), RGB_max);
		test_one_d_LUT_ptr[i].rgbColor.Blue = min(max(RGB_temp[2], 0), RGB_max);

		//更新下一回合測試的 RGB 值
		virtual_LUT[grayscale].Red = test_one_d_LUT_ptr[i].rgbColor.Red;
		virtual_LUT[grayscale].Green = test_one_d_LUT_ptr[i].rgbColor.Green;
		virtual_LUT[grayscale].Blue = test_one_d_LUT_ptr[i].rgbColor.Blue;
	}
}

void Write_measured(vector<double>jnd) {
	CFile fout;
	fout.Open("./measured_JND.csv", CFile::modeCreate | CFile::modeWrite);

	fout.Write("JND\n", 8);//

	for (int i = 0; i < (int)jnd.size(); i++) {
		//LUTable1D LUTElement5 = { 5, {0x3FF, 0x3FF, 0x3FF} };

		char str[255];
		sprintf_s(str, "%f\n,", jnd[i]);
		fout.Write(str, strlen(str));//儲存x
	}
	fout.Close();
}

void  Write_Calibrated_Data(CString filename, vector<vector<double>>data) {
	CFile fout;
	fout.Open(filename, CFile::modeCreate | CFile::modeWrite);

	fout.Write("R,G,B,X,Y,Z,sx,sy,sz, index \n", 256);//

	for (int i = 0; i < (int)data.size(); i++) {
		char str[255];
		for (int j = 0; j < data[i].size(); j++) {
			if (j != data[i].size() - 1) {
				sprintf_s(str, "%f,", data[i][j]);
				fout.Write(str, strlen(str));//儲存R
			}
			else if (j == data[i].size() - 1) {
				sprintf_s(str, "%f\n", data[i][j]);
				fout.Write(str, strlen(str));
			}
		}
	}
	fout.Close();
}


//待校正後確認16個點的 RGB 值後 將 calibrated_one_d_LUT_ptr 0~1023 計算出來
void Calibrate::Build_CCT_LUT(PRGBColor cct_calibrated_LUT) {
	int max_RGB;
	uint16_t R_0, G_0, B_0, R_1, G_1, B_1, R, G, B;
	int ref_index;
	double interpolate_index_r, interpolate_index_g, interpolate_index_b;

	int index_pre = 64 / one_d_calibration_lut_interval - 1;


	for (int i = 0; i < 1024; i++) {
		if (i < 63) {
			R = calibrated_one_d_LUT_ptr[index_pre].rgbColor.Red * (i / 63.0);
			G = calibrated_one_d_LUT_ptr[index_pre].rgbColor.Green * (i / 63.0);
			B = calibrated_one_d_LUT_ptr[index_pre].rgbColor.Blue * (i / 63.0);
		}
		else {
			ref_index = ceil((i + 1) / one_d_calibration_lut_interval);
			if ((i + 1) % one_d_calibration_lut_interval == 0) {
				R = calibrated_one_d_LUT_ptr[ref_index - 1].rgbColor.Red;
				G = calibrated_one_d_LUT_ptr[ref_index - 1].rgbColor.Green;
				B = calibrated_one_d_LUT_ptr[ref_index - 1].rgbColor.Blue;
			}
			else {
				if (ref_index > 0) {
					R_0 = calibrated_one_d_LUT_ptr[ref_index - 1].rgbColor.Red;
					G_0 = calibrated_one_d_LUT_ptr[ref_index - 1].rgbColor.Green;
					B_0 = calibrated_one_d_LUT_ptr[ref_index - 1].rgbColor.Blue;
				}
				else
				{
					R_0 = 0;
					G_0 = 0;
					B_0 = 0;
				}
				R_1 = calibrated_one_d_LUT_ptr[ref_index].rgbColor.Red;
				G_1 = calibrated_one_d_LUT_ptr[ref_index].rgbColor.Green;
				B_1 = calibrated_one_d_LUT_ptr[ref_index].rgbColor.Blue;
				interpolate_index_r = pow(((i + 1) % one_d_calibration_lut_interval) / (double)one_d_calibration_lut_interval, 1);
				interpolate_index_g = pow(((i + 1) % one_d_calibration_lut_interval) / (double)one_d_calibration_lut_interval, 1);
				interpolate_index_b = pow(((i + 1) % one_d_calibration_lut_interval) / (double)one_d_calibration_lut_interval, 1);
				R = R_0 + round(interpolate_index_r * (R_1 - R_0));
				G = G_0 + round(interpolate_index_g * (G_1 - G_0));
				B = B_0 + ceil(interpolate_index_b * (B_1 - B_0));
			}
		}
		cct_calibrated_LUT[i] = RGBColor{ R,G,B };
	}

	vector<vector<double>>data_xy;

	for (int i = 0; i < one_d_calibration_lut_size; i++) {
		data_xy.push_back({ (double)calibrated_one_d_LUT_ptr[i].rgbColor.Red, (double)calibrated_one_d_LUT_ptr[i].rgbColor.Green, (double)calibrated_one_d_LUT_ptr[i].rgbColor.Blue , calibrated_one_d_LUT_ptr[i].xyz.x ,calibrated_one_d_LUT_ptr[i].xyz.y, calibrated_one_d_LUT_ptr[i].xyz.z, double(calibrated_one_d_LUT_ptr[i].index)});
	}
	Write_Calibrated_Data("./calibrated_xy.csv", data_xy);
}


void Calibrate::Tone_Curve_Correction(PRGBColor LUT_raw, PRGBColor LUT_result, const vector<double>& Lv_raw, const vector<double>& Lv_target, vector<int>& tone_index, const int RGB_interval) {

	if (Lv_target.size() != one_d_full_lut_size) {
		return;
	}

	int upper;
	int downer;
	int index = 0;
	double dis_n;
	double dis_ca;
	double ratio;
	for (int i = 0; i < one_d_full_lut_size - 1; i++) {
		dis_ca = MAXINT;
		for (int j = 0; j < 1024; j++) {
			dis_n = abs(Lv_target[i] - Lv_raw[j]);
			if (abs(dis_n) < abs(dis_ca)) {
				dis_ca = dis_n;
				index = j;
			}
		}
		tone_index.push_back(index);
		LUT_result[i] = LUT_raw[index];

	}
	tone_index.push_back(RGB_max);
}


void Calibrate::Gamma_Mapping(PRGBColor LUT_raw, PRGBColor LUT_Gamma, vector<double> Lv_display_graysacle, double gamma_target, vector<int>& tone_index, int RGB_interval) {
	vector<double> Lv_gamma;
	Generate_Gamma_Lv(Lv_display_graysacle, Lv_gamma, gamma_target);
	Tone_Curve_Correction(LUT_raw, LUT_Gamma, Lv_display_graysacle, Lv_gamma, tone_index, RGB_interval);
}


void Calibrate::Generate_Gamma_Lv(const vector<double> Lv_display_graysacle, vector<double>& Lv_gamma, const double gamma_target) {
	double Lv_min = Lv_display_graysacle[0];
	double Lv_max = Lv_display_graysacle[Lv_display_graysacle.size() - 1] - Lv_min;

	for (int i = 0; i < one_d_full_lut_size; i++) {
		Lv_gamma.push_back(pow(i / RGB_max, gamma_target) * Lv_max + Lv_min);
	}
}


void Calibrate::PQ_Mapping(PRGBColor LUT_raw, PRGBColor  LUT_PQ, vector<double> Lv_display_graysacle, vector<int>& tone_index, int RGB_interval) {
	vector<double> Lv_pq;
	Generate_PQ_Lv(Lv_display_graysacle, Lv_pq);
	Tone_Curve_Correction(LUT_raw, LUT_PQ, Lv_display_graysacle, Lv_pq, tone_index, RGB_interval);
}


void Calibrate::Generate_PQ_Lv(vector<double> Lv_display_graysacle, vector<double>& Lv_pq) {
	double Lv_min = Lv_display_graysacle[0];
	double Lv_max = Lv_display_graysacle[Lv_display_graysacle.size() - 1];
	for (int i = 0; i < one_d_full_lut_size; i++) {
		Lv_pq.push_back(PQ_EOTF(i / RGB_max, Lv_max));
	}
}


double Calibrate::PQ_EOTF(double rgb, double Lv_max) {
	double m1 = 0.1593017578125;
	double m2 = 78.84375;
	double c2 = 18.8515625;
	double c3 = 18.6875;
	double c1 = c3 - c2 + 1;
	double E_f = rgb;


	double numerator = pow(E_f, 1.0 / m2) - c1;
	double denominator = c2 - c3 * pow(E_f, 1.0 / m2);
	double Y = max(10000 * pow(numerator / denominator, 1.0 / m1), 0);
	Y = (Y > Lv_max) ? Lv_max : Y;

	return Y;
}

double HLG(int E, double lv_max, double lv_min) {
	double E_f = E / 1023.0;
	double Lw = lv_max;
	double Lb = lv_min;
	double sys_gamma = 1.2 + 0.42 * log10(Lw / 1000);

	double	B = sqrt(3 * pow((Lb / Lw), 1 / sys_gamma));

	double X = max(0, (1 - B) * E_f + B);

	double a = 0.17883277;
	double b = 0.28466892;
	double c = 0.55991073;

	double ootf_e = 0.0;

	if (X <= 0.5) {
		ootf_e = pow(X, 2) / 3.0;
	}
	else {
		ootf_e = (exp((X - c) / a) + b) / 12.0;
	}

	double Y_s = 0.2627 * E_f + 0.6780 * E_f + 0.0593 * E_f;
	double Y = lv_max * pow(Y_s, sys_gamma - 1) * ootf_e;

	return Y;
}


void Calibrate::HLG_Mapping(PRGBColor LUT_raw, PRGBColor LUT_HLG, vector<double> Lv_display_graysacle, vector<int>& tone_index, int RGB_interval) {
	vector<double> Lv_hlg;
	Generate_HLG_Lv(Lv_display_graysacle, Lv_hlg);

	vector<int> trc_index;
	Tone_Curve_Correction(LUT_raw, LUT_HLG, Lv_display_graysacle, Lv_hlg, tone_index,RGB_interval);
}


void Calibrate::Generate_HLG_Lv(vector<double> Lv_display_graysacle, vector<double>& Lv_hlg) {
	double Lv_min = Lv_display_graysacle[0];
	double Lv_max = Lv_display_graysacle[Lv_display_graysacle.size() - 1];
	for (int i = 0; i < one_d_full_lut_size; i++) {
		Lv_hlg.push_back(HLG(i, Lv_max, Lv_min));
	}
}


double Calibrate::HLG_OETF(double rgb) {
	double oetf_inverse_rgb;

	static const double a = 0.17883277;
	static const double b = 0.28466892;
	static const double c = 0.55991073;

	if (rgb <= 0.5) {
		oetf_inverse_rgb = pow(rgb, 2) / 3.0;
	}
	else {
		oetf_inverse_rgb = (exp((rgb - c) / a) + b) / 12.0;
	}

	return oetf_inverse_rgb;
}

double Calibrate::HLG_OOTF(double rgb, const double peak_lv, const double system_gamma) {
	double Y;

	double Y_s = 0.2627 * rgb + 0.6780 * rgb + 0.0593 * rgb;

	Y = peak_lv * pow(Y_s, system_gamma - 1) * rgb;

	return Y;
}

double Calibrate::HLG_EOTF(double rgb, const double black_lv, const double peak_lv) {
	double Y;

	double sys_gamma = 1.2 + 0.42 * log10(peak_lv / 1000.0);
	double B = sqrt(3 * pow((black_lv / peak_lv), 1.0 / sys_gamma));

	double rgb_s = max(0, (1 - B) * rgb + B);

	Y = (HLG_OOTF(HLG_OETF(rgb_s), peak_lv, sys_gamma));

	return Y;
}


/// <summary>
/// Generate 1D LUT for DICOM calibration
/// </summary>
/// <param name="raw_LUT">: The raw 1D lut.</param>
/// <param name="dicom_LUT">: The 1D DICOM lut that will be generated.</param>
/// <param name="illuminance_amb">: The ambient illuminance. (can also set 0 as the default)</param>
/// <param name="display_reflection">: The reflection coefficient of the display.</param>
/// <param name="luminance_ratio">: The luminance ratio. (if set 0, dicom max lv = specific_dicom_lv_max or display_raw_graysacle_lv[-1])</param>
/// <param name="specific_dicom_lv_min">: The specific minimum DICOM luminance. (can also set 0 as the default)</param>
/// <param name="specific_dicom_lv_max">: The specific maximum DICOM luminance. (can also set 0 as the default)</param>
/// <param name="display_raw_graysacle_lv">: The raw measured grayscale luminance of the display.</param>
/// /// <param name="interval">: The RGB interval of display_raw_graysacle_lv.</param>
void Calibrate::DICOM_Mapping(PRGBColor LUT_raw, PRGBColor LUT_DICOM, const double illuminance_amb, const double display_reflection, const double luminance_ratio, const double specific_dicom_Lv_min, const double specific_dicom_Lv_max, const vector<double> Lv_display_graysacle, vector<int>& tone_index, vector<double>& dicom_lv_nor, int RGB_interval) {

	double dicom_Lv_min = 0.0;
	double dicom_Lv_max = 0.0;
	double dicom_Lv_amb = illuminance_amb * display_reflection;

	if (specific_dicom_Lv_min == 0) {
		dicom_Lv_min = max(dicom_Lv_amb * 5, 1.0);
		if (dicom_Lv_amb == 0) {
			dicom_Lv_min = Lv_display_graysacle[0];
			dicom_Lv_min = 1.0;
		}
	}
	else {
		dicom_Lv_min = specific_dicom_Lv_min;
	}

	if (specific_dicom_Lv_max == 0) {
		if (luminance_ratio == 0) {
			dicom_Lv_max = Lv_display_graysacle[Lv_display_graysacle.size() - 1];
		}
		else {
			dicom_Lv_max = dicom_Lv_min * luminance_ratio;
		}
		dicom_Lv_max += dicom_Lv_amb;
	}
	else {
		dicom_Lv_max = specific_dicom_Lv_max;
	}

	vector<double> Lv_dicom;
	Generate_DICOM_GSDF_Lv(dicom_Lv_min, dicom_Lv_max, Lv_dicom);

	for (int i = 0; i < Lv_dicom.size(); i++) {
		dicom_lv_nor.push_back((Lv_dicom[i]) / (Lv_dicom[Lv_dicom.size() - 1]));
	}

	Tone_Curve_Correction(LUT_raw, LUT_DICOM, Lv_display_graysacle, Lv_dicom, tone_index, RGB_interval);
}


void Calibrate::Generate_DICOM_GSDF_Lv(const double dicom_lv_min, const double dicom_lv_max, vector<double>& Lv_target)
{
	double J_min = L_to_JND(dicom_lv_min);
	double J_max = L_to_JND(dicom_lv_max);
	double jp;
	double lv;

	for (int p = 0; p < one_d_full_lut_size; p++) {
		jp = j_p(J_max, J_min, this->display_bit_depth, p);
		if (jp <= 0) {
			lv = 0.0;
		}
		else {
			lv = JND_to_L(jp);
		}

		if (lv / dicom_lv_max > 0.999) {
			lv = dicom_lv_max;
		}
		Lv_target.push_back(lv);
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
#pragma endregion


void Calibrate::Build_Three_D_LUT(vector<vector <vector<RGBColor>>>& three_d_lut, MATRIX matrix_gamut_cct, CString tone_curve, int lut_size) {
	int three_d_lut_interval = 1024 / (lut_size - 1);
	int R = 0;
	int G = 0;
	int B = 0;

	uint16_t R_1;
	uint16_t G_1;
	uint16_t B_1;

	for (int i = 0; i < lut_size; ++i) {
		three_d_lut.push_back(vector<vector<RGBColor>>());
		for (int j = 0; j < lut_size; ++j) {
			three_d_lut[i].push_back(vector<RGBColor>());
			for (int k = 0; k < lut_size; ++k) {
				RGBColor color;
				three_d_lut[i][j].push_back(color);
			}
		}
	}

	double R_m;
	double G_m;
	double B_m;

	for (int b = 0; b < lut_size; b++) {
		for (int g = 0; g < lut_size; g++) {
			for (int r = 0; r < lut_size; r++) {
				R = max(r * three_d_lut_interval - 1, 0);
				G = max(g * three_d_lut_interval - 1, 0);
				B = max(b * three_d_lut_interval - 1, 0);

				R_m = pow(R / RGB_max, panel_tone_curve.w) * RGB_max;
				G_m = pow(G / RGB_max, panel_tone_curve.w) * RGB_max;
				B_m = pow(B / RGB_max, panel_tone_curve.w) * RGB_max;

				double r_m = R_m * matrix_gamut_cct[0][0] + G_m * matrix_gamut_cct[0][1] + B_m * matrix_gamut_cct[0][2];
				double g_m = R_m * matrix_gamut_cct[1][0] + G_m * matrix_gamut_cct[1][1] + B_m * matrix_gamut_cct[1][2];
				double b_m = R_m * matrix_gamut_cct[2][0] + G_m * matrix_gamut_cct[2][1] + B_m * matrix_gamut_cct[2][2];

				r_m = min(max(r_m, 0), RGB_max);
				g_m = min(max(g_m, 0), RGB_max);
				b_m = min(max(b_m, 0), RGB_max);

				R_1 = round(pow(r_m / 1023.0, 1.0 / panel_tone_curve.w) * 1023);
				G_1 = round(pow(g_m / 1023.0, 1.0 / panel_tone_curve.w) * 1023);
				B_1 = round(pow(b_m / 1023.0, 1.0 / panel_tone_curve.w) * 1023);

				if (r == g && g == b) {
					three_d_lut[b][g][r] = RGBColor{ uint16_t(R), uint16_t(G),uint16_t(B) };
				}
				else
				{
					three_d_lut[b][g][r] = RGBColor{ R_1,G_1,B_1 };
				}

			}
		}
	}
}


void Calibrate:: trilinearInterpolation(const vector<vector<vector<RGBColor>>>& three_d_lut, int b, int g, int r, RGBColor& result) {
	int x0 = floor((r + 1) / 64.0);
	int y0 = floor((g + 1) / 64.0);
	int z0 = floor((b + 1) / 64.0);

	int x1 = (x0 == 16) ? 16 : x0 + 1;
	int y1 = (y0 == 16) ? 16 : y0 + 1;
	int z1 = (z0 == 16) ? 16 : z0 + 1;

	double dx = (r == 0) ? 0 : ((r + 1) - x0 * 64) / 64.0;
	double dy = (g == 0) ? 0 : ((g + 1) - y0 * 64) / 64.0;
	double dz = (b == 0) ? 0 : ((b + 1) - z0 * 64) / 64.0;


	RGBColor c000 = three_d_lut[x0][y0][z0];
	RGBColor c001 = three_d_lut[x0][y0][z1];
	RGBColor c010 = three_d_lut[x0][y1][z0];
	RGBColor c011 = three_d_lut[x0][y1][z1];
	RGBColor c100 = three_d_lut[x1][y0][z0];
	RGBColor c101 = three_d_lut[x1][y0][z1];
	RGBColor c110 = three_d_lut[x1][y1][z0];
	RGBColor c111 = three_d_lut[x1][y1][z1];

	result.Red  = static_cast<uint16_t>((1 - dx) * (1 - dy) * (1 - dz) * c000.Red +
		(1 - dx) * (1 - dy) * dz * c001.Red +
		(1 - dx) * dy * (1 - dz) * c010.Red +
		(1 - dx) * dy * dz * c011.Red +
		dx * (1 - dy) * (1 - dz) * c100.Red +
		dx * (1 - dy) * dz * c101.Red +
		dx * dy * (1 - dz) * c110.Red +
		dx * dy * dz * c111.Red);
	result.Green = static_cast<uint16_t>((1 - dx) * (1 - dy) * (1 - dz) * c000.Green +
		(1 - dx) * (1 - dy) * dz * c001.Green +
		(1 - dx) * dy * (1 - dz) * c010.Green +
		(1 - dx) * dy * dz * c011.Green +
		dx * (1 - dy) * (1 - dz) * c100.Green +
		dx * (1 - dy) * dz * c101.Green +
		dx * dy * (1 - dz) * c110.Green +
		dx * dy * dz * c111.Green);
	result.Blue = static_cast<uint16_t>((1 - dx) * (1 - dy) * (1 - dz) * c000.Blue +
		(1 - dx) * (1 - dy) * dz * c001.Blue +
		(1 - dx) * dy * (1 - dz) * c010.Blue +
		(1 - dx) * dy * dz * c011.Blue +
		dx * (1 - dy) * (1 - dz) * c100.Blue +
		dx * (1 - dy) * dz * c101.Blue +
		dx * dy * (1 - dz) * c110.Blue +
		dx * dy * dz * c111.Blue);

	if (r == g && g == b && g != 0) {
		double mean_v = (result.Red + result.Green + result.Blue) / 3.0;
		result.Red = round(mean_v);
		result.Green = round(mean_v);
		result.Blue = round(mean_v);
	}
}


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

