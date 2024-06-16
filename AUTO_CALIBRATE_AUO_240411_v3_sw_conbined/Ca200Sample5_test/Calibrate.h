#pragma once
#include <vector>
#include <map>;



#ifndef CALIBRATE_H
#define CALIBRATE_H

using namespace std;

typedef struct _trc {
	double r; //for the red channel in sdr gamma
	double g; //for the green channel in sdr gamma
	double b; //for the blue channel in sdr gamma
	double w; //for the white channel in sdr gamma
}TRC, * PTRC;

typedef struct _RGBColor {
	uint16_t  Red = 0;
	uint16_t  Green = 0;
	uint16_t  Blue = 0;
} RGBColor, * PRGBColor;

typedef struct _XYZ {
	double X = 0.0;
	double Y = 0.0;
	double Z = 0.0;
}XYZ, * PXYZ;

typedef struct _sxyz {
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
}sxyz, * Psxyz;

typedef struct _TEST_LUT {
	RGBColor rgbColor;
	XYZ XYZ;
	sxyz xyz;
	double duv;
	double dis;
}TEST_LUT, * PTEST_LUT;

typedef struct _CALIBRATED_LUT {
	RGBColor rgbColor;
	sxyz xyz;
	double dis = MAXINT;
	int index;
}CALIBRATED_LUT, * PCALIBRATED_LUT;

#define  MATRIX vector<vector<float>>
#define LUT_3D vector<vector <vector<RGBColor>>>

class Calibrate
{
public:

	Calibrate();

	void Configure_one_d_calibration_attribute(const uint16_t bit_depth , const uint16_t one_d_calibration_lut_size);
	void Virtual_Video_Pipeline(RGBColor RGB_raw, RGBColor& RGB_new, PRGBColor gamma_color_encode_LUT, MATRIX gamut_matrix, MATRIX cct_matrix, PRGBColor gamma_decode_LUT, PRGBColor base_LUT, double gamma);

	bool error = true;

	TRC panel_tone_curve; //the tone curve(gamma) of the display
	PTEST_LUT test_one_d_LUT_ptr; //the 1D LUT used during the calibration process
	PCALIBRATED_LUT calibrated_one_d_LUT_ptr; //the 1D LUT with optimal calibration results


	void Init_LUT();
	void Set_Calibration_Target(const int cct_target, const TRC tone_curve);


	//Colour
	vector<float> CCT_to_xy(const int cct);
	vector<double> XYZ_to_Lab(const vector<double>& XYZ, const vector<double>& ref_XYZ);
	double xy_to_CCT(const double x, const double y);
	vector<float> W_Transform(const MATRIX& matrix_RGB_XYZ, const int cct_target);


	//TRC measureand Gamut¡BCCT RGB conversion matrix
	double calculate_trc(vector<double> lv);
	void Generate_Gamma_LUT(PRGBColor gamma_LUT, const double gamma_target);


	// Colour Matrix
	void RGB_to_XYZ(MATRIX matrix_RGB_to_XYZ, RGBColor RGB, const double tone_curve, vector<double>XYZ_W, vector<double>XYZ_0, vector<double>& XYZ);
	MATRIX Build_Matrix_RGB_to_XYZ(vector<double>& XYZ_R, vector<double>& XYZ_G, vector<double>& XYZ_B, vector<double>& XYZ_W, vector<double>& XYZ_0);
	void Calibrate::Build_Matrix_GAMUT_Trans(const MATRIX& matrix_native_RGB_XYZ, MATRIX& matrix_Gamut_trans, const CString gamut_target);
	void Calibrate::Build_Matrix_CCT_Trans(const MATRIX& matrix_native_RGB_XYZ, MATRIX& matrix_CCT_trans, const int cct_target, const double tone_curve);
	void Build_Matrix_Gamut_CCT_Trans(const MATRIX& matrix_native_RGB_XYZ, MATRIX& matrix_Gamut_CCT_trans, vector<double>XYZ_W, vector<double>XYZ_0, const CString gamut_target, const int cct_target);
	void Chromatic_Adaptation(vector<double> XYZ_raw, vector<double>& XYZ_adp, MATRIX& matrix_chromatic_adaptation, vector<double> XYZ_raw_w, const int cct_target);

	//CCT
	void CCT_Calibration(RGBColor RGB_raw, RGBColor& RGB_new, MATRIX m, vector<vector<double> >& data, int index, int grayscale_ref, vector<double>& Lv, int& round);
	void Build_CCT_LUT(PRGBColor calibrated_one_d_LUT_ptr);


	//Tone Curve
	void Gamma_Mapping(PRGBColor LUT_raw, PRGBColor LUT_Gamma, vector<double>  Lv_display_graysacle, double gamma_target, vector<int>& tone_index, int RGB_interval);
	void PQ_Mapping(PRGBColor LUT_raw, PRGBColor LUT_PQ, vector<double>  Lv_display_graysacle, vector<int>& tone_index, int RGB_interval);
	void HLG_Mapping(PRGBColor LUT_raw, PRGBColor LUT_HLG, vector<double>  Lv_display_graysacle, vector<int>& tone_index, int RGB_interval);
	void DICOM_Mapping(PRGBColor LUT_raw, PRGBColor LUT_DICOM, const double illuminance_amb, const double display_reflection, const double luminance_ratio, const double specific_dicom_Lv_min, const double specific_dicom_Lv_max, const vector<double> Lv_display_graysacle, vector<int>& tone_index, vector<double>& dicom_lv_nor, int RGB_interval);
	

	//3D LUT
	void Build_Three_D_LUT(vector<vector <vector<RGBColor>>>& three_d_lut, MATRIX matrix_gamut_cct, CString tone_curve, int lut_size);
	void trilinearInterpolation(const vector<vector<vector<RGBColor>>>& three_d_lut, int r, int g, int b, RGBColor& result);

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

	MATRIX M_Bradford =
	{ {0.8951000, 0.2664000, -0.1614000},
	  {-0.7502000, 1.7135000, 0.0367000},
	  {0.0389000, -0.0685000, 1.0296000} };

	MATRIX M_Bradford_inv = 
	{ {0.9869929, -0.1470543, 0.1599627},
	  {0.4323053, 0.5183603, 0.0492912},
	  {-0.0085287, 0.0400428, 0.9684867} };
	~Calibrate();
protected:

private:
	uint16_t display_bit_depth = 0;
	uint16_t one_d_full_lut_size = 0; //the size of the whole 1D lut, and must be a power of 4, ex:256 1024 4096
	uint16_t one_d_calibration_lut_size = 0; //the size of the 1D lut for calibration, and must be a multiple of 4, ex:4 8 16 24 32 
	uint16_t one_d_calibration_lut_interval = 0; // RGB value one_d_calibration_lut_size in 1D LUT
	float RGB_max = 0.0;
	double targetx = 0.0;
	double targety = 0.0;


	vector<float> CCT_to_xyz_nor(const int cct);
	double calculate_mse(const vector<double>& Lv, const vector<double>& std_Lv, double gamma);
	void Interpolate(vector<int>& index, const int one_d_calibration_lut_size);


	void Analysis();
	void Cal_CCT_LUT(vector<MATRIX>M, PRGBColor virtual_LUT);


	void Tone_Curve_Correction(PRGBColor LUT_raw, PRGBColor LUT_result, const vector<double>& Lv_raw, const vector<double>& Lv_target, vector<int>& tone_index, const int RGB_interval);

	void Generate_Gamma_Lv(const vector<double> Lv_display_graysacle, vector<double>& Lv_gamma, const double gamma_target);
	
	void Generate_PQ_Lv(vector<double> Lv_display_graysacle, vector<double>& Lv_gamma);
	double PQ_EOTF(double rgb, double Lv_max);

	void Generate_HLG_Lv(vector<double> Lv_display_graysacle, vector<double>& Lv_hlg);
	double HLG_OETF(double rgb);
	double HLG_OOTF(double rgb, const double peak_lv, const double system_gamma);
	double HLG_EOTF(double rgb, const double black_lv, const double peak_lv);

	void Generate_DICOM_GSDF_Lv(const double dicom_lv_min, const double dicom_lv_max, vector<double>& Lv_target);
	double L_to_JND(const double L);
	double JND_to_L(const double JND);
	double j_p(const double Jmax, const double Jmin, const int n, const int p);

	void row_minus(vector<float>& X, const vector<float>& Y, const double multi);
	void single_row_divide(vector<float>& X, const double divide);
	MATRIX Matrix_Inv(MATRIX matrix);
	MATRIX allocateMatrix(const int row, const int col);
	MATRIX Multiply(const MATRIX& m1, const MATRIX& m2);
};
#endif //CALIBRATE_H