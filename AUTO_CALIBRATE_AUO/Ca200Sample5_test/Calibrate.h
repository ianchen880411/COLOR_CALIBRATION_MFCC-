
#ifndef Calibrate_INCLUDED
#define Calibrate_INCLUDED

//#ifdef __cplusplus
//extern "C" {
//#endif

#pragma once
#include <vector>

#ifndef LUT_array_size
#define LUT_array_size    1024
#endif

#include "scaler_serial.h"
#include "scaler_internal.h"
#include <map>;

using namespace std;
#define  MATRIX vector<vector<float>>

const int lut_size = 16; //= interval

typedef struct _trc {
	double r;
	double g;
	double b;
	double w;
}TRC;

typedef struct _XYZ {
	double  X;
	double  Y;
	double  Z;
}XYZ;

typedef struct _sxyz {
	double  x;
	double  y;
	double  z;
}sxyz;

typedef struct _Test_LUT {
	RGBColor rgbColor;
	XYZ XYZ;
	sxyz xyz;
	double dis;
}Test_LUT;

typedef struct _Calibrated_LUT {
	RGBColor rgbColor;
	sxyz xyz;
	double dis = MAXINT;
}Calibrated_LUT;

class Calibrate
{
public:
	Calibrate();
	TRC panel_tone_curve;

	void Init_LUT();
	void Set_Calibration_Target(const int cct_target, const TRC tone_curve);
	//Colour
	vector<float> CCT_to_xy(const int cct);
	vector<double> XYZ_to_Lab(const vector<double>& XYZ, const vector<double>& ref_XYZ);
	double xy_to_CCT(const double x, const double y);
	vector<float> W_Transform(const MATRIX& matrix_RGB_XYZ, const int cct_target);

	//TRC measureand Gamut、CCT RGB conversion matrix
	double calculate_trc(const vector<double>& lv);
	void Generate_Gamma_LUT(PRGBColor gamma_LUT, const double gamma_target);

	// Colour Matrix
	MATRIX Build_Matrix_RGB_to_XYZ(vector<double>& XYZ_R, vector<double>& XYZ_G, vector<double>& XYZ_B, vector<double>& XYZ_W);
	void Build_Matrix_Gamut_CCT_Trans(const MATRIX& matrix_native_RGB_XYZ, MATRIX& matrix_Gamut_CCT_trans, const CString gamut_target, const int cct_target);

	//CCT
	void CCT_Calibration(PRGBColor virtual_LUT, vector<vector<double> >& data);
	void Build_CCT_LUT(PRGBColor calibrated_LUT);
	void LUT_Tone_Curve_Trans(PRGBColor result_LUT, const vector<double>& Lv, const double tone_curve_target, const int interval);

	//DICOM
	void Cal_DICOM_LUT(PRGBColor dicom_LUT, const double min_Lv, const double max_Lv);

	MATRIX matrix_raw =
	{ {1.0, 0, 0},
	 {0, 1.0, 0},
	 {0, 0, 1.0} };

	MATRIX M_sRGB_RGB_XYZ =
	{ {0.4124, 0.3576, 0.1805},
	 {0.2126, 0.7152, 0.0722},
	 {0.0193, 0.1192, 0.9505} };

	MATRIX M_ADOBE_RGB_XYZ =
	{ {0.5767309, 0.1855540, 0.1881852},
	 {0.2973769, 0.6273491, 0.0752741},
	 {0.0270343, 0.0706872, 0.9911085} };

	MATRIX M_BT2020_RGB_XYZ =
	{ {0.6370, 0.1446, 0.1689},
	 {0.2627, 0.6780, 0.0593},
	 {0.0, 0.0281, 1.0610} };

	map<CString, MATRIX> mp_matrix_gamut_target =
	{ {"Native", matrix_raw},
	 {"sRGB", M_sRGB_RGB_XYZ},
	 {"ADOBE", M_ADOBE_RGB_XYZ},
	 {"BT2020", M_BT2020_RGB_XYZ} };

	~Calibrate();
protected:

private:
	int interval = lut_size; //校正切分點數
	int step = LUT_array_size / interval; //間隔

	double targetx = 0.0;
	double targety = 0.0;

	Test_LUT test_LUT[lut_size]; //校正時的測試LUT 
	Calibrated_LUT calibrated_LUT[lut_size]; //校正時最優解LUT
	vector<vector<int>> send_index;

	vector<float> CCT_to_xyz_nor(const int cct);
	double calculate_mse(const vector<double>& lv, const vector<double>& std_lv, double gamma);

	void Analysis();
	void Cal_CCT_LUT(PRGBColor virtual_LUT);
	void Mark(uint16_t& R, uint16_t& G, uint16_t& B, const int i);

	void Tone_Curve_Correction(const vector<double>& Lv, const double tone_curve_target, vector<int>& trc_index, const int interval);
	void Interpolate(vector<int>& index, const double tone_curve_target, const double trc_raw, const int interval);

	double L_to_JND(const double L);
	double JND_to_L(const double JND);
	double j_p(const double Jmax, const double Jmin, const int n, const int p);
	vector<double> dicom_XYZ(const double min_Lv, const double max_Lv);

	void row_minus(vector<float>& X, const vector<float>& Y, const double multi);
	void single_row_divide(vector<float>& X, const double divide);
	MATRIX Matrix_Inv(MATRIX matrix);
	MATRIX allocateMatrix(const int row, const int col);
	MATRIX Multiply(const MATRIX& m1, const MATRIX& m2);
};
#endif