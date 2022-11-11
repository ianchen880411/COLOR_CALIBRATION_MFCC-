
#ifndef Calibrate_INCLUDED
#define Calibrate_INCLUDED

#pragma once
#include <vector>
#include <map>

#ifndef LUT_array_size
#define LUT_array_size    1024
#endif

#ifndef virtul_LUT_size
#define virtul_LUT_size     16 // 1D LUT calibration size
#endif

#ifndef virtul_LUT_interval
#define virtul_LUT_interval LUT_array_size / virtul_LUT_size  // 1D LUT calibration interval, ex: 64 128 192 ... 1023
#endif

#include "scaler_serial.h"
#include "scaler_internal.h"
using namespace std;
#define  MATRIX vector<vector<float>>

typedef struct _trc {
	double r;
	double g;
	double b;
	double w;
	double target;
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

typedef struct _Calibration_LUT {
	RGBColor rgbColor;
	XYZ XYZ;
	sxyz xyz;
	double dis;
}Calibration_LUT;

class Calibrate
{
public:
	Calibrate(int panel_bitdepth);
	~Calibrate();

	void Init_LUT();
	LUTable1D one_d_lut[LUT_array_size]; //實際傳送的LUT
	TRC  panel_tone_curve; // tone curve of R G B W and target

	vector<double> XYZ_to_Lab(const vector<double> XYZ, const vector<double> ref_XYZ);
	vector<float> CCT_to_xy(const int cct);
	double xy_to_CCT(const double x, const double y);
	MATRIX Calibrate::Build_Matrix_RGB_to_XYZ(const vector<double> XYZ_R, const vector<double> XYZ_G, const vector<double> XYZ_B, const vector<double> XYZ_W, const vector<double> XYZ_0, const double Y_max);
	vector<float> W_Transform(MATRIX& matrix_RGB_XYZ, const int cct_target);
	MATRIX Build_Matrix_RGB_to_Gamut_RGB(MATRIX& matrix_native_RGB_XYZ, const CString gamut_target, const int cct_target);
	MATRIX Build_Matrix_RGB_to_CCT_RGB(MATRIX& matrix_native_RGB_XYZ, const int cct_target);
	
	double Calculate_TC(const vector<double> lv);

	void Cal_CCT_LUT(vector<vector<double>>* data, const int cct);
	void Get_CCT_LUT(LUTable1D cct_LUT[]);
	void CCT_LUT_Tone_Curve_Trans(LUTable1D cct_LUT[], const vector<double>lv, const double tone_curve_target, const int interval);
	void Generate_Gamma_LUT(LUTable1D LUT[], const double tone_curve_raw, const double tone_curve_target);
	void Generate_DICOM_LUT(const LUTable1D base_lut[], LUTable1D dicom_lut[], const double max_lv, const double tone_curve_raw);

	MATRIX matrix_raw =
	{ {1, 0, 0},
	 {0, 1, 0},
	 {0, 0, 1} };

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

protected:

private:
	// panel attribute
	int panel_bitdepth; // panel's bitdepth ex:8、10、12
	int panel_RGB_max;

	Calibration_LUT test_lut[virtul_LUT_size]; //校正時的測試LUT 
	Calibration_LUT calibrated_lut[virtul_LUT_size]; //校正時最優解LUT

	//fit tone curve(gamma)
	void row_minus(vector<float>& X, vector<float>& Y, double multi);
	void single_row_divide(vector<float>& X, double divide);
	double Calculate_Gamma_MSE(const vector<double> lv, const vector<double> std_lv, double gamma);

	//cct LUT
	void Read_Test_Data(vector<vector<double>>* data);
	void Analysis(const double Y, const double targetx, const double targety);
	void Mark(uint16_t& R, uint16_t& G, uint16_t& B, int i);

	//LUT tone curve trans
	vector<int> Tone_Curve_Correction(const vector<double>lv, const double tone_curve_target, const int interval);
	void Interpolate(vector<int>& index, const double tone_curve_target, const int steps);

	//LUT dicom trans
	double L_to_JND(const double L);
	double JND_to_L(const double JND);
	double j_p(const double Jmax, const double Jmin, const int n, const int p);
	vector<double> dicom_XYZ(const double max_Lv);

	MATRIX allocateMatrix(const int row, const int col);
	MATRIX Multiply(const MATRIX m1, const MATRIX m2);
	MATRIX Matrix_Inv(MATRIX matrix);
};

#endif