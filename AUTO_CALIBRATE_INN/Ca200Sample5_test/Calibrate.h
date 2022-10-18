
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

//Colour
vector<float> CCT_to_xy(int& cct);

//TRC measureand Gamut¡BCCT RGB conversion matrix
double calculate_trc(vector<double>& lv, vector<double>& std_lv);
MATRIX Build_Matrix_RGB_to_XYZ(vector<double>& XYZ_R, vector<double>& XYZ_G, vector<double>& XYZ_B, vector<double>& XYZ_0, double& Y_max);
MATRIX Build_Matrix_RGB_to_Gamut_RGB(MATRIX& matrix_native_RGB_XYZ, CString& gamut_target);
MATRIX Build_Matrix_RGB_to_CCT_RGB(MATRIX& matrix_native_RGB_XYZ, int& cct_target);
vector<double> RGB_to_CCT_RGB(vector<double>RGB, MATRIX& matrix_cct_trans);
//CCT
void Cal_CCT_LUT();
void Analysis();
void Build_CCT_LUT();

//DICOM
void Cal_DICOM_LUT(RGBColor lut[LUT_array_size], double max_Lv);
#endif