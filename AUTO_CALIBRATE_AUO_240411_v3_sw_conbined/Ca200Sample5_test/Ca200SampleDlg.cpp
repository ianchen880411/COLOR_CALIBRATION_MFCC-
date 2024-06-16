#include <string> 
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <string>
#include <string.h>
#include <iostream>
#include <io.h>
#include <chrono>
#include <time.h>


//#include <iostream>
//#include <Windows.h>    // Includes the functions for serial communication via RS232
#include <stdlib.h>
#include <cstring> 

#include <iostream>
#include <stdio.h>
#include "stdafx.h"
#include "Ca200Sample.h"
#include "Ca200SampleDlg.h"
#include "DlgProxy.h"
#include "scaler_serial.h"
#include "scaler_internal.h"

// CA-SDK
#include "Const.h"
#include "CaEvent.h"
#import "C:\Program Files (x86)\KONICAMINOLTA\CA-SDK\SDK\CA200Srvr.dll" no_namespace implementation_only  



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define _LOAD_CVS_FILE_and_GENERATE_CHECKSUM_                           1
#define _KEEP_ENABLED_LUTS_after_TX_Video_Pipeline_LUT_Update_FPGA_     1
#define _KEEP_ENABLED_LUTS_after_TX_Video_Pipeline_LUT_Activate_FPGA_   1
#define _LUT_UPDATE_WITHOUT_SAVING_TO_FLASH_                            0

char strCOM[10] = "COM10";

Calibrate* CA = new Calibrate();
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCa200SampleDlg Dialog

IMPLEMENT_DYNAMIC(CCa200SampleDlg, CDialog);

CCa200SampleDlg::CCa200SampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCa200SampleDlg::IDD, pParent)
	, m_Reg_01E0(0x00002710)
	, m_Reg_01E4(0x00000000)
	, m_Reg_01E8(0x27100000)
	, m_Reg_01EC(0x00000000)
	, m_Reg_01F0(0x00000000)
	, m_Reg_01F4(0x00002710)
{
	//{{AFX_DATA_INIT(CCa200SampleDlg)
	m_strLv = _T("");
	m_strT = _T("");
	m_strx = _T("");
	m_stry = _T("");
	m_strduv = _T("");
	m_str232 = _T("123");
	//}}AFX_DATA_INIT
	
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pAutoProxy = NULL;
}

CCa200SampleDlg::CCa200SampleDlg(int i){
}

CCa200SampleDlg::~CCa200SampleDlg()
{
	if (m_pAutoProxy != NULL)
		m_pAutoProxy->m_pDialog = NULL;

}

void CCa200SampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCa200SampleDlg)
	DDX_Text(pDX, IDC_STATIC_LV, m_strLv);
	DDX_Text(pDX, IDC_STATIC_T, m_strT);
	DDX_Text(pDX, IDC_STATIC_X, m_strx);
	DDX_Text(pDX, IDC_STATIC_Y, m_stry);
	DDX_Text(pDX, IDC_STATIC_DUV, m_strduv);
	DDX_Text(pDX, IDC_STATIC_232, m_str232);

	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCa200SampleDlg, CDialog)
	//{{AFX_MSG_MAP(CCa200SampleDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_CAL0, OnButtonCal0)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDCANCEL, &CCa200SampleDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_ADDLIST_NATIVE, &CCa200SampleDlg::OnBnClickedButtonAddlistNative)
	ON_BN_CLICKED(IDOK, &CCa200SampleDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_Color, &CCa200SampleDlg::OnBnClickedButtonColor)
	ON_BN_CLICKED(IDC_BUTTON_TRC, &CCa200SampleDlg::OnBnClickedButtonTrc)
	ON_BN_CLICKED(IDC_BUTTON_Calibrate, &CCa200SampleDlg::OnBnClickedButtonCalibrate)
	ON_BN_CLICKED(IDC_BUTTON_Gray, &CCa200SampleDlg::OnBnClickedButtonGray)
	ON_BN_CLICKED(IDC_BUTTON_Pattern, &CCa200SampleDlg::OnBnClickedButtonPattern)
	ON_BN_CLICKED(IDC_BUTTON_ADDLIST_ADOBE, &CCa200SampleDlg::OnBnClickedButtonAddlistAdobe)
	ON_BN_CLICKED(IDC_BUTTON_ADDLIST_BT2020, &CCa200SampleDlg::OnBnClickedButtonAddlistBt2020)
	ON_BN_CLICKED(IDC_BUTTON_ADDLIST_SRGB, &CCa200SampleDlg::OnBnClickedButtonAddlistSrgb)
	ON_BN_CLICKED(IDC_BUTTON_SET_PIPELINE, &CCa200SampleDlg::OnBnClickedButtonSetPipeline)
	ON_CBN_SELCHANGE(IDC_COMBO_MODE, &CCa200SampleDlg::OnCbnSelchangeComboMode)
	ON_BN_CLICKED(IDC_CHECK_NATIVE_CCT_ALL, &CCa200SampleDlg::OnBnClickedCheckNativeCctAll)
	ON_BN_CLICKED(IDC_CHECK_NATIVE_TRC_ALL, &CCa200SampleDlg::OnBnClickedCheckNativeTrcAll)
	ON_BN_CLICKED(IDC_CHECK_SRGB_CCT_ALL, &CCa200SampleDlg::OnBnClickedCheckSrgbCctAll)
	ON_BN_CLICKED(IDC_CHECK_SRGB_TRC_ALL, &CCa200SampleDlg::OnBnClickedCheckSrgbTrcAll)
	ON_BN_CLICKED(IDC_CHECK_ADOBE_CCT_ALL, &CCa200SampleDlg::OnBnClickedCheckAdobeCctAll)
	ON_BN_CLICKED(IDC_CHECK_ADOBE_TRC_ALL, &CCa200SampleDlg::OnBnClickedCheckAdobeTrcAll)
	ON_BN_CLICKED(IDC_CHECK_BT2020_CCT_ALL, &CCa200SampleDlg::OnBnClickedCheckBt2020CctAll)
	ON_BN_CLICKED(IDC_CHECK_BT2020_TRC_ALL, &CCa200SampleDlg::OnBnClickedCheckBt2020TrcAll)
	ON_BN_CLICKED(IDC_BUTTON_VERIFY_ALL, &CCa200SampleDlg::OnBnClickedButtonVerifyAll)
	ON_BN_CLICKED(IDC_BUTTON_VERIFY_NONE, &CCa200SampleDlg::OnBnClickedButtonVerifyNone)
	ON_BN_CLICKED(IDC_BUTTON_DELETELIST_NATIVE, &CCa200SampleDlg::OnBnClickedButtonDeletelistNative)
	ON_BN_CLICKED(IDC_BUTTON_DELETELIST_SRGB, &CCa200SampleDlg::OnBnClickedButtonDeletelistSrgb)
	ON_BN_CLICKED(IDC_BUTTON_DELETELIST_ADOBE, &CCa200SampleDlg::OnBnClickedButtonDeletelistAdobe)
	ON_BN_CLICKED(IDC_BUTTON_DELETELIST_BT2020, &CCa200SampleDlg::OnBnClickedButtonDeletelistBt2020)
	ON_BN_CLICKED(IDC_BUTTON_Init, &CCa200SampleDlg::OnBnClickedButtonInit)
	ON_BN_CLICKED(IDC_BUTTON_OWN, &CCa200SampleDlg::OnBnClickedButtonOwn)
	ON_BN_CLICKED(IDC_CHECK_Image, &CCa200SampleDlg::OnBnClickedCheckImage)
	ON_CBN_SELCHANGE(IDC_COMBO_PIP_TRC, &CCa200SampleDlg::OnCbnSelchangeComboPipTrc)
	ON_CBN_SELCHANGE(IDC_COMBO_PIP_GAMUT, &CCa200SampleDlg::OnCbnSelchangeComboPipGamut)
	ON_CBN_SELCHANGE(IDC_COMBO_PIP_CCT, &CCa200SampleDlg::OnCbnSelchangeComboPipCct)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////

// Prototype the functions to be used
void initPort(HANDLE* hCom, wchar_t* COMPORT, int nComRate, int nComBits, COMMTIMEOUTS timeout);
void purgePort(HANDLE* hCom);
void outputToPort(HANDLE* hCom, LPCVOID buf, DWORD szBuf);
DWORD inputFromPort(HANDLE* hCom, LPVOID buf, DWORD szBuf);

// Sub functions
//void createPortFile(HANDLE* hCom, wchar_t* COMPORT);
void createPortFile(HANDLE* hCom, char* COMPORT);
static int SetComParms(HANDLE* hCom, int nComRate, int nComBits, COMMTIMEOUTS timeout);

#define EX_FATAL 1

#pragma region Save and Read excel file

void write_time_log(string str) {
	std::ofstream outputFile("calibration_time.txt", ios::app);
	outputFile << str;
	outputFile.close();
}

void  Write_Measure_Data(CString filename, vector<vector<double>>data) {
	CFile fout;
	fout.Open(filename, CFile::modeCreate | CFile::modeWrite);

	fout.Write("R,G,B,X,Y,Z,Lv,sx,sy,sz,lT,duv \n", 36);//

	for (int i = 0; i < (int)data.size(); i++) {
		char str[255];
		for (int j = 0; j < data[i].size(); j++) {
			if (j != data[i].size() - 1) {
				sprintf_s(str, "%f,", data[i][j]);
				fout.Write(str, strlen(str));
			}
			else if (j == data[i].size() - 1) {
				sprintf_s(str, "%f\n", data[i][j]);
				fout.Write(str, strlen(str));
			}
		}
	}
	fout.Close();
}


void Write_1D_LUT(CString filename, RGBColor lut[]) {
	CFile fout;
	fout.Open(filename, CFile::modeCreate | CFile::modeWrite);

	fout.Write("i,R,G,B\n", 8);//

	for (int i = 0; i < RGB_depth; i++) {

		char str[255];
		sprintf_s(str, "%d,", i);
		fout.Write(str, strlen(str));//儲存i
		sprintf_s(str, "%d,", lut[i].Red);
		fout.Write(str, strlen(str));//儲存R
		sprintf_s(str, "%d,", lut[i].Green);
		fout.Write(str, strlen(str));//儲存G
		sprintf_s(str, "%d\n", lut[i].Blue);
		fout.Write(str, strlen(str));//儲存B
	}
	fout.Close();
}


void Write_3D_LUT(CString filename, vector<vector<vector<RGBColor>>> three_d_lut) {
	CFile fout;
	fout.Open(filename, CFile::modeCreate | CFile::modeWrite);

	fout.Write("i,R,G,B\n", 8);//

	int count = 0;
	for (uint16_t r = 0; r < three_d_lut[0].size(); r++) {
		for (uint16_t g = 0; g < three_d_lut[0].size(); g++) {
			for (uint16_t b = 0; b < three_d_lut[0].size(); b++) {
				char str[255];
				sprintf_s(str, "%d,", count);
				fout.Write(str, strlen(str));//儲存i
				sprintf_s(str, "%d,", three_d_lut[r][g][b].Red);
				fout.Write(str, strlen(str));//儲存R
				sprintf_s(str, "%d,", three_d_lut[r][g][b].Green);
				fout.Write(str, strlen(str));//儲存G
				sprintf_s(str, "%d\n", three_d_lut[r][g][b].Blue);
				fout.Write(str, strlen(str));//儲存B
				count++;
			}
		}
	}
	fout.Close();
}


void Write_Matrix(CString filename, MATRIX matrix) {
	CFile fout;
	fout.Open(filename, CFile::modeCreate | CFile::modeWrite);
	char str[255];
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (j == 2) {
				sprintf_s(str, "%.4f\n", matrix[i][j]);
			}
			else {
				sprintf_s(str, "%.4f,", matrix[i][j]);
			}
			fout.Write(str, strlen(str));
		}
	}
	fout.Close();
}

void Write_Max_Lv(CString filename, vector<double> XYZ) {
	CFile fout;
	fout.Open(filename, CFile::modeCreate | CFile::modeWrite);
	char str[255];
	for (int i = 0; i < 3; i++) {
		sprintf_s(str, "%.4f,", XYZ[i]);
		fout.Write(str, strlen(str));
	}
	fout.Close();
}

void Write_measured_ND(vector<double>jnd) {
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

MATRIX CCa200SampleDlg::Load_CSV(string filename) {
	MATRIX m;
	fstream file;
	string line;
	string data;
	vector<float> tempvec;
	bool b = 0;
	file.open(filename);
	while (getline(file, line, '\n')) {
		istringstream temp(line);
		tempvec.clear();
		while (getline(temp, data, ',')) {
			if (float(*data.c_str())||int(*data.c_str())) {
				tempvec.push_back(atof(data.c_str()));
				b = 1;
			}
		}
		if (b) {
			m.push_back(tempvec);
		}
		b = 0;
	}
	return m;
}

MATRIX CCa200SampleDlg::Load_Matrix(CString filename) {
	MATRIX m;
	string s;
	s = filename;
	m = Load_CSV(s);
	return m;
}

MATRIX CCa200SampleDlg::Load_LUT_RGB(CString filename) {
	MATRIX m;
	MATRIX m1;
	string s;
	s = filename;
	m = Load_CSV(s);
	for (int i = 1; i < m.size(); i++) {
		m1.push_back(vector<float>{m[i][1], m[i][2], m[i][3]});
	}
	return m1;
}


void CCa200SampleDlg::Load_Panel_1D_LUT(CString filename, PRGBColor LUT) {
	MATRIX m; 
	RGBColor RGB;
	string s;
	s = filename;
	m = Load_CSV(s);
	for (int i = 1; i < m.size(); i++) {
		RGB = { uint16_t(m[i][1]), uint16_t(m[i][2]), uint16_t(m[i][3]) };
		LUT[i-1] = RGB;
	}
}


void CCa200SampleDlg::Load_Panel_3D_LUT(CString filename, vector<vector<vector<RGBColor>>>& three_d_lut, int lut_size) {
	MATRIX m;
	RGBColor RGB;
	string s;
	s = filename;
	m = Load_CSV(s);
	vector<RGBColor> RGB_lsit;
	for (int i = 1; i < m.size(); i++) {
		RGB = { uint16_t(m[i][1]), uint16_t(m[i][2]), uint16_t(m[i][3]) };
		RGB_lsit.push_back(RGB);
	}

	int count = 1;
	for (int i = 0; i < lut_size; ++i) {
		three_d_lut.push_back(vector<vector<RGBColor>>());
		for (int j = 0; j < lut_size; ++j) {
			three_d_lut[i].push_back(vector<RGBColor>());
			for (int k = 0; k < lut_size; ++k) {
				RGBColor color = { uint16_t(m[count][1]), uint16_t(m[count][2]), uint16_t(m[count][3]) };
				three_d_lut[i][j].push_back(color);
				count++;
			}
		}
	}
}


void CCa200SampleDlg::Load_Send_Index(CString filename) {
	send_index.clear();
	MATRIX m;
	string s;
	s = filename;
	m = Load_CSV(s);
	for (int i = 1; i < m.size(); i++) {
		send_index.push_back({ int(m[i][1]), int(m[i][2]), int(m[i][3]) });
	}
}

void CCa200SampleDlg::_Loas_All_File (char* lpPath, vector<CString>& fileList)
{
	char szFind[MAX_PATH];
	WIN32_FIND_DATA FindFileData;

	strcpy(szFind, lpPath);
	strcat(szFind, "\\*.*");

	HANDLE hFind = ::FindFirstFile(szFind, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)  return;

	while (true)
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (FindFileData.cFileName[0] != '.')
			{
				char szFile[MAX_PATH];
				strcpy(szFile, lpPath);
				strcat(szFile, "\\");
				strcat(szFile, (char*)(FindFileData.cFileName));
				_Loas_All_File(szFile, fileList);
			}
		}
		else
		{
			//std::cout << FindFileData.cFileName << std::endl; 
			char file_path[MAX_PATH];
			strcpy(file_path, lpPath);
			strcat(file_path, FindFileData.cFileName);
			fileList.push_back(file_path);
		}
		if (!FindNextFile(hFind, &FindFileData))  break;
	}
	FindClose(hFind);
}

void CCa200SampleDlg:: _Load_ALL_Matrix(CString file_directory, vector< MATRIX>& Matrix_Data) {
	vector<CString> file_list;
	//_Loas_All_File(file_directory.GetBuffer(), file_list);
	
	MATRIX m;
	CString str;
	for (int i = 0; i < gamut_list.size(); i++) {
		for (int j = 0; j < cct_list.size(); j++) {
			str.Format("./data/matrix/matrix_%s_%d.csv",gamut_list[i],cct_list[j]);
			m = Load_Matrix(str);
			Matrix_Data.push_back(m);
		}
	}
}

void CCa200SampleDlg::_Load_GamutMatrix_File(uint32_t* m_PageBuffer, const CString file_directory, int& buf_index)
{
	typedef struct _SCALER_CALIBRATE_DATA_ {
		union {
			float fData;
			uint32_t lData;
		}CaliData;
	} SCALER_CALIBRATE_DATA;

	SCALER_CALIBRATE_DATA _caliData;

	vector<MATRIX> Matrix_Data;
	_Load_ALL_Matrix(file_directory, Matrix_Data);

	for (int i = 0; i < Matrix_Data.size(); i++) {
		if (i % 8 == 0 || i == 0) {
			for (int j = 0; j < 32; j++) {
				m_PageBuffer[buf_index] = 0xFFFFFFFF;
				buf_index++;
			}
		}

		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				_caliData.CaliData.fData = Matrix_Data[i][j][k];
				m_PageBuffer[buf_index] = _caliData.CaliData.lData;
				buf_index++;
			}
			m_PageBuffer[buf_index] = 0xFFFFFFFF;
			buf_index++;
		}
	}
}

void CCa200SampleDlg::_Load_LUT_File(uint32_t* m_PageBuffer, int& buf_index)
{
	typedef struct _SCALER_CALIBRATE_DATA_ {
		union {
			float fData;
			uint32_t lData;
		}CaliData;
	} SCALER_CALIBRATE_DATA;

	SCALER_CALIBRATE_DATA _caliData;

	CString filename;
	vector<MATRIX> Matrix_Data;
	MATRIX lut_data;

	filename.Format("./data/lut/gamma_base_lut.csv");
	lut_data = Load_LUT_RGB(filename);
	Matrix_Data.push_back(lut_data);

	for (int i = 0; i < trc_list.size(); i++) {
		filename.Format("./data/lut/tone_curve/lut_%s.csv", trc_list[i]);
		lut_data = Load_LUT_RGB(filename);
		Matrix_Data.push_back(lut_data);
	}

	filename.Format("./data/lut/tone_curve/dicom_lut.csv");
	lut_data = Load_LUT_RGB(filename);
	Matrix_Data.push_back(lut_data);

	uint32_t dwColorTemp;
	uint32_t dwRGBData;
	for (int i = 0; i < Matrix_Data.size(); i++)
	{
		for (int j = 0; j < Matrix_Data[i].size(); j++) {
			/* Red Color */
			dwColorTemp = uint16_t(Matrix_Data[i][j][0]);
			dwRGBData = (dwColorTemp & MASK_SINGLE_COLOR /*0x3FF*/);
			/* Green Color */
			dwColorTemp = uint16_t(Matrix_Data[i][j][1]);
			dwRGBData = (dwRGBData | ((dwColorTemp & MASK_SINGLE_COLOR /*0x3FF*/) << BITS_SHIFT_4_GREEN /*10*/));
			/* Blue Color */
			dwColorTemp = uint16_t(Matrix_Data[i][j][2]);
			dwRGBData = (dwRGBData | ((dwColorTemp & MASK_SINGLE_COLOR /*0x3FF*/) << BITS_SHIFT_4_BLUE /*20*/));

			m_PageBuffer[buf_index] = dwRGBData;
			buf_index++;
		}
	}
}

void CCa200SampleDlg::Export_Data_Image() {
	CFile fout;

	for (int i = 0; i < sizeof(m_PageBuffer)/ sizeof(UINT32); i++) {
		m_PageBuffer[i] = 0xFFFFFFFF;
	}

	int buf_index = 128;
	_Load_GamutMatrix_File(m_PageBuffer, "./data//matrix//", buf_index);
	buf_index += 384;
	_Load_LUT_File(m_PageBuffer, buf_index);
	CFile file;
	CFileException e;
	CString strError;
	if (file.Open("./data//data_image.bin", CFile::modeCreate | CFile::modeWrite, &e) == FALSE)
	{
		strError.Format(TEXT("Failed to open the Calibration-image file %s, error=%d"), "./data//data_image.bin", e.m_cause);
		AfxMessageBox(strError);

		return;
	}

	int size = sizeof(m_PageBuffer) / sizeof(UINT32);
	try {
		file.Write(m_PageBuffer, 0x9000);
	}
	catch (CFileException* exp)
	{
		TCHAR szCause[255];
		exp->GetErrorMessage(szCause, 255);

		strError.Format(TEXT("Write Calibration-Image failed, Exception[%s]"), szCause);
		AfxMessageBox(strError);

		file.Close();
		return;
	}
}
#pragma endregion

void CCa200SampleDlg::CA_CAL_ZERO() {
	if (!cal_zero) {
		try {
			xy_str.Format("Cal Zero...");
			SetDlgItemText(IDC_STATIC_xy, xy_str);
			m_pCaObj->CalZero();
			cal_zero = true;
		}
		catch (_com_error e) {
			CString strerr;
			strerr.Format(_T("HR:0x%08x\nMSG:%s"), e.Error(), (LPCSTR)e.Description());
			AfxMessageBox((LPCSTR)strerr);
			cal_zero = false;
			return;
		}
	}
}

void CCa200SampleDlg::Check_State() {
	CA_CAL_ZERO();
	if (LinkNo == -1) {
		nRet = Connect_to_ScalerBoard(strCOM, &LinkNo);
		if (nRet != ERROR_SUCCESS)
		{
			CString strError;
			strError.Format("Connect_to_ScalerBoard() failed, error code=%d", nRet);
			AfxMessageBox(strError);
		}

		nRet = Adjust_BackLight_PWM(LinkNo, 100, 1);
		if (nRet != ERROR_SUCCESS)
		{
			CString strError;
			strError.Format("Adjust_BackLight_PWM() failed, error code=%d", nRet);
			AfxMessageBox(strError);
		}
	}
}

void CCa200SampleDlg::Init_Data_Vector() {
	vector<double> row;
	row.assign(12, 0);//配置一個 row 的大小  R,G,B,X,Y,Z,Lv,sx,sy,sz,lT,duv
	data.assign(0, row);//配置2維
}


void CCa200SampleDlg::Virtual_Video_Pipeline_Proccess(RGBColor RGB_raw, RGBColor& RGB_new, CString gamut, CString cct, CString tone_curve) {
	//MATRIX color_matrix;
	//CString cstr_matrix;

	//cstr_matrix.Format(".\\data\\matrix\\matrix_%s_%s.csv", gamut, cct);
	//color_matrix = Load_Matrix(cstr_matrix);


	MATRIX color_gamut_matrix;
	CString cstr_gamut_matrix;

	cstr_gamut_matrix.Format(".\\data\\matrix\\matrix_%s.csv", gamut);
	color_gamut_matrix = Load_Matrix(cstr_gamut_matrix);

	MATRIX color_cct_matrix;
	CString cstr_cct_matrix;
	cstr_cct_matrix.Format(".\\data\\matrix\\matrix_%s_%s.csv", gamut, cct);
	color_cct_matrix = Load_Matrix(cstr_cct_matrix);

	RGBColor tone_encode_LUT[RGB_depth];
	CString cstr_trc;
	if (tone_curve == "PQ" || tone_curve == "HLG" || tone_curve == "DICOMGSDF") {
		cstr_trc.Format(".\\data\\lut\\tone_curve\\%s_lut.csv", tone_curve);
	}
	else
	{
		cstr_trc.Format(".\\data\\lut\\tone_curve\\tone_encode_lut_%s.csv", tone_curve);
	}
	Load_Panel_1D_LUT(cstr_trc, tone_encode_LUT);

	RGBColor tone_decode_LUT[RGB_depth];
	if (tone_curve == "PQ" || tone_curve == "HLG" || tone_curve == "DICOMGSDF") {
		cstr_trc.Format(".\\data\\lut\\tone_curve\\tone_decode_lut_%.1f_inv.csv", 5.0);
	}
	else
	{
		cstr_trc.Format(".\\data\\lut\\tone_curve\\tone_decode_lut_%s_inv.csv", tone_curve);
	}
	Load_Panel_1D_LUT(cstr_trc, tone_decode_LUT);

	RGBColor base_LUT[RGB_depth];
	if (tone_curve == "PQ" || tone_curve == "HLG" || tone_curve == "DICOMGSDF") {
		cstr_trc.Format(".\\data\\lut\\gamma_base_lut_%.1f.csv", 5.0);
	}
	else
	{
		cstr_trc.Format(".\\data\\lut\\gamma_base_lut_%s.csv", tone_curve);
	}
	Load_Panel_1D_LUT(cstr_trc, base_LUT);

	CA->Virtual_Video_Pipeline(RGB_raw, RGB_new, tone_encode_LUT, color_gamut_matrix, color_cct_matrix, tone_decode_LUT, base_LUT, atof(tone_curve));
	//nRet = Generate_RGB_Pattern(LinkNo, RGB_new.Red, RGB_new.Green, RGB_new.Blue);
}


void CCa200SampleDlg::Set_FPGA_Video_Pipeline(CString gamut, CString cct, CString tone_curve) {
	MATRIX color_matrix;
	CString cstr_matrix;
	cstr_matrix.Format(".\\data\\matrix\\matrix_%s_%s.csv", gamut, cct);
	color_matrix = Load_Matrix(cstr_matrix);

	RGBColor tone_encode_LUT[RGB_depth];
	CString cstr_trc;
	if (tone_curve == "PQ" || tone_curve == "HLG" || tone_curve == "DICOMGSDF") {
		cstr_trc.Format(".\\data\\lut\\tone_curve\\%s_lut.csv", tone_curve);
	}
	else
	{
		cstr_trc.Format(".\\data\\lut\\tone_curve\\tone_encode_lut_%s.csv", tone_curve);
	}
	Load_Panel_1D_LUT(cstr_trc, tone_encode_LUT);

	RGBColor tone_decode_LUT[RGB_depth];
	if (tone_curve == "PQ" || tone_curve == "HLG" || tone_curve == "DICOMGSDF") {
		cstr_trc.Format(".\\data\\lut\\tone_curve\\tone_decode_lut_%.2f_inv.csv", 5.0);
	}
	else
	{
		cstr_trc.Format(".\\data\\lut\\tone_curve\\tone_decode_lut_%s_inv.csv", tone_curve);
	}
	Load_Panel_1D_LUT(cstr_trc, tone_decode_LUT);

	RGBColor base_LUT[RGB_depth];
	if (tone_curve == "PQ" || tone_curve == "HLG" || tone_curve == "DICOMGSDF") {
		cstr_trc.Format(".\\data\\lut\\gamma_base_lut_%.1f.csv", 5.0);
	}
	else
	{
		//cstr_trc.Format(".\\data\\lut\\gamma_base_lut_%s.csv", tone_curve);
		cstr_trc.Format(".\\data\\lut\\base_lut_raw.csv");
	}
	Load_Panel_1D_LUT(cstr_trc, base_LUT);

	if (video_pip_state["matrix"].c_str() != gamut + "_" + cct) {
		//Update_MCU_Matrix(color_matrix, gamut + "_" + cct);
	}

	if (video_pip_state["encode_lut"].c_str() != tone_curve) {
		//Update_MCU_LUT(tone_encode_LUT, SELECT_1D_LUT_GAMMA, 0, tone_curve);
	}

	if (video_pip_state["decode_lut"].c_str() != tone_curve) {
		//Update_MCU_LUT(tone_decode_LUT, SELECT_1D_LUT_DEGAMMA, 1, tone_curve);
	}

	if (video_pip_state["base_lut"].c_str() != tone_curve) {
		Update_MCU_LUT(base_LUT, SELECT_1D_LUT_CCT, 2, tone_curve);
	}
}


void CCa200SampleDlg::Send_Get(const vector<vector<int>> send_index, int16_t nRet, const int32_t LinkNo, vector<vector<double> >& data, bool virtual_video_pip) {
	// CA-SDK
	uint16_t R;
	uint16_t G;
	uint16_t B;
	float fLv;
	float fx;
	float fy;
	float fz;
	long lT;
	float fduv;
	float fX;
	float fY;
	float fZ;

	int mode;
	mode = ((CButton*)GetDlgItem(IDC_CHECK_Image))->GetCheck();

	if (mode) {
		((CButton*)GetDlgItem(IDC_CHECK_Image))->SetCheck(false);
	}

	RGBColor tone_encode_LUT[RGB_depth];
	CString tone_lut_path;
	CString three_d_lut_path;
	vector<vector <vector<RGBColor>>> three_d_lut;
	if (virtual_video_pip) {
		tone_lut_path.Format(".\\data\\lut\\tone_curve\\%s\\%s_%s.csv", cct_target, cct_target, tone_curve_target);
		Load_Panel_1D_LUT(tone_lut_path, tone_encode_LUT);

		RGBColor tone_encode_LUT[RGB_depth];

		three_d_lut_path.Format(".\\data\\lut\\three_d_lut\\%s\\%s_%s.csv", cct_target, gamut_target, cct_target);
		Load_Panel_3D_LUT(three_d_lut_path, three_d_lut, 17);
	}

	for (int i = 0; i < send_index.size(); i++) {
		R = send_index[i][0];
		G = send_index[i][1];
		B = send_index[i][2];

		//SetDlgItemText(IDC_STATIC_232, m_str232);

		if (virtual_video_pip == true) {

			RGBColor result;
			result.Red = tone_encode_LUT[R].Red;
			result.Green = tone_encode_LUT[G].Green;
			result.Blue = tone_encode_LUT[B].Blue;

			CA->trilinearInterpolation(three_d_lut, result.Red, result.Green, result.Blue, result);

			nRet = Generate_RGB_Pattern(LinkNo, result.Red, result.Green, result.Blue);
			m_str232.Format("Message send: %d,%d,%d", result.Red, result.Green, result.Blue);
	
		}
		else
		{
			nRet = Generate_RGB_Pattern(LinkNo, R, G, B);
		}
		m_str232.Format("Message send: %d,%d,%d", R, G, B);
		
		Sleep(60);
		
		m_pCaObj->put_AveragingMode(2);
		m_pCaObj->Measure(0);

		fX = m_pProbeObj->X;
		fY = m_pProbeObj->Y;
		fZ = m_pProbeObj->Z;

		fLv = m_pProbeObj->Lv;
		fx = m_pProbeObj->sx;
		fy = m_pProbeObj->sy;
		fz = 1.0 - fx - fy;
		lT = m_pProbeObj->T;
		fduv = m_pProbeObj->duv;

		data.push_back({ double(R), double(G), double(B),fX,fY,fZ,fLv,fx,fy,fz,(double)lT,fduv });//儲存R,G,B,X,Y,Z,Lv,sx,sy,sz,lT,duv

		m_strLv.Format("%.4f", fLv);
		m_strx.Format("%1.4f", fx);
		m_stry.Format("%1.4f", fy);
		m_strT.Format("%4d", lT);
		m_strduv.Format("%1.4f", fduv);
		UpdateData(FALSE);
	}
}

vector<double> CCa200SampleDlg::Measure_MaxLv(RGBColor RGB) {
	send_index.clear();
	Init_Data_Vector();
	Send_Get(send_index, nRet, LinkNo, data, false);
	return vector<double> { data[0][3], data[0][4], data[0][5] };
}

void CCa200SampleDlg::Measure_RGBW() {
	send_index.clear();
	send_index.push_back({ int(RGB_max), 0, 0 });
	send_index.push_back({ 0, int(RGB_max), 0 });
	send_index.push_back({ 0, 0, int(RGB_max) });
	send_index.push_back({ int(RGB_max), int(RGB_max), int(RGB_max) });

	Init_Data_Vector();
	Send_Get(send_index, nRet, LinkNo, data, false);
	XYZ_R = { data[0][3], data[0][4],data[0][5] };
	XYZ_G = { data[1][3], data[1][4],data[1][5] };
	XYZ_B = { data[2][3], data[2][4],data[2][5] };
	XYZ_W = { data[3][3], data[3][4],data[3][5] };
}

void CCa200SampleDlg::panel_CCT_Calibration(PRGBColor result_LUT, const int cct_target, vector<double>& Lv_calibrated, vector<MATRIX> panel_RGBW_maatrix) {
	vector<float>xy = CA->CCT_to_xy(cct_target);
	double targetx = xy[0];
	double targety = xy[1];
	xy_str.Format("Calibrating %s, %dK, x: %.3f, y: %.3f", gamut_target, cct_target, targetx, targety);
	SetDlgItemText(IDC_STATIC_xy, xy_str);

	CA->Init_LUT();
	

	const int lut_size = 1024;
	int lut_interval = 1024 / lut_size;

	Lv_calibrated.clear();

	for (int i = 0; i < lut_size; i++) {
		Lv_calibrated.push_back(0.0);
	}

	CA->Set_Calibration_Target(cct_target, panel_tone_curve);

	RGBColor RGB_raw;
	RGBColor RGB_new;
	int grayscale;

	int v;
	MATRIX m;
	for (int i = 0; i < lut_size; i++) {
		v = (lut_size - 1) - i;
		grayscale = uint16_t((v + 1) * lut_interval - 1);
		if (v == lut_size - 1) {
			RGB_raw = { uint16_t(grayscale),  uint16_t(grayscale), uint16_t(grayscale) };
			RGB_new = { uint16_t(grayscale),  uint16_t(grayscale), uint16_t(grayscale) };
		}
		else
		{
			double r_s = CA->calibrated_one_d_LUT_ptr[v + 1].rgbColor.Red / double(CA->calibrated_one_d_LUT_ptr[v + 1].rgbColor.Green);
			double b_s = CA->calibrated_one_d_LUT_ptr[v + 1].rgbColor.Blue / double(CA->calibrated_one_d_LUT_ptr[v + 1].rgbColor.Green);
			uint16_t G = round(CA->calibrated_one_d_LUT_ptr[lut_size - 1].rgbColor.Green * (((v + 1) * lut_interval - 1) / RGB_max));
			RGB_raw = { uint16_t(round(G * r_s)), G,uint16_t(round(G * b_s)) };
			RGB_new = RGB_raw;
		}

		int n_limit = 25;
		int n = 0;

		m = panel_RGBW_maatrix[ceil((v + 1) * lut_interval / 32.0) - 1];

		if (v <lut_size - 1) {
			n_limit = 11;
		}

		while (n < n_limit) {
			send_index.clear();
			send_index.push_back({ RGB_new.Red, RGB_new.Green, RGB_new.Blue });
			Init_Data_Vector();
			Send_Get(send_index, nRet, LinkNo, data, false);

			CA->CCT_Calibration(RGB_raw, RGB_new, m, data, v, grayscale, Lv_calibrated, n);
			RGB_raw = RGB_new;
			n++;
		}
	}

	CA->Build_CCT_LUT(result_LUT);

	xy_str.Format("Calibrating Complete!");
	SetDlgItemText(IDC_STATIC_xy, xy_str);
}

void CCa200SampleDlg::Measure_Raw() {
	xy_str.Format("Measure R、G、B、W Scale...");
	SetDlgItemText(IDC_STATIC_xy, xy_str);
	
	send_index.clear();
	//vector<vector<int>> send_index;
	for (int i = 64; i <= RGB_depth; i += 64) {
		i = min(i, int(RGB_max));
		send_index.push_back({ i, 0, 0 });
	}
	for (int i = 64; i <= RGB_depth; i += 64) {
		i = min(i, int(RGB_max));
		send_index.push_back({ 0, i, 0 });
	}
	for (int i = 64; i <= RGB_depth; i += 64) {
		i = min(i, int(RGB_max));
		send_index.push_back({ 0, 0, i });
	}
	for (int i = 0; i <= RGB_depth; i += 64) {
		i = min(i, int(RGB_max));
		send_index.push_back({ i, i, i });
	}

	Init_Data_Vector();
	Send_Get(send_index, nRet, LinkNo, data, false);
	vector<double> lv;
	double lv_0 = data[48][4];
	XYZ_0 = { data[48][3], data[48][4],data[48][5] };

	vector<double> tempvec;

	XYZ_R = { data[15][3], data[15][4],data[15][5] };
	lv.push_back(0);
	for (int i = 0; i < 16; i++) {
		tempvec = data[i];
		lv.push_back((tempvec[4] - lv_0) / (data[15][4] - lv_0));
	}
	panel_tone_curve.r = CA->calculate_trc(lv);
	lv.clear();

	XYZ_G = { data[31][3], data[31][4],data[31][5] };
	lv.push_back(0);
	for (int i = 16; i < 32; i++) {
		tempvec = data[i];
		lv.push_back((tempvec[4] - lv_0) / (data[31][4] - lv_0));
	}
	panel_tone_curve.g = CA->calculate_trc(lv);
	lv.clear();

	XYZ_B = { data[47][3], data[47][4],data[47][5] };
	lv.push_back(0);
	for (int i = 32; i < 48; i++) {
		tempvec = data[i];
		lv.push_back((tempvec[4] - lv_0) / (data[47][4] - lv_0));
	}
	panel_tone_curve.b = CA->calculate_trc(lv);
	lv.clear();

	XYZ_W = { data[64][3], data[64][4],data[64][5] };
	for (int i = 48; i < 65; i++) {
		tempvec = data[i];
		lv.push_back((tempvec[4] - lv_0) / (data[64][4] - lv_0));
	}
	panel_tone_curve.w = CA->calculate_trc(lv);

	m_raw_trc_r.Format("%.2f", panel_tone_curve.r);
	m_raw_trc_g.Format("%.2f", panel_tone_curve.g);
	m_raw_trc_b.Format("%.2f", panel_tone_curve.b);
	m_raw_trc_w.Format("%.2f", panel_tone_curve.w);
	m_raw_Lv.Format("%.2f", data[64][6]);
	m_raw_cct.Format("%d", int(data[64][10]));
	SetDlgItemText(IDC_STATIC_trc_r, m_raw_trc_r);
	SetDlgItemText(IDC_STATIC_trc_g, m_raw_trc_g);
	SetDlgItemText(IDC_STATIC_trc_b, m_raw_trc_b);
	SetDlgItemText(IDC_STATIC_trc_w, m_raw_trc_w);
	SetDlgItemText(IDC_STATIC_L, m_raw_Lv);
	SetDlgItemText(IDC_STATIC_CCT, m_raw_cct);
	xy_str.Format("Measure Complete!");
	SetDlgItemText(IDC_STATIC_xy, xy_str);
}

void CCa200SampleDlg::Verify_Btn_Enable(bool b) {
	for (int i = 0; i < cct_list.size(); i++) {
		native_cct_cb[i]->EnableWindow(b);
		srgb_cct_cb[i]->EnableWindow(b);
		adobe_cct_cb[i]->EnableWindow(b);
		bt2020_cct_cb[i]->EnableWindow(b);
	}
	for (int i = 0; i < trc_list.size(); i++) {
		native_trc_cb[i]->EnableWindow(b);
		srgb_trc_cb[i]->EnableWindow(b);
		adobe_trc_cb[i]->EnableWindow(b);
		bt2020_trc_cb[i]->EnableWindow(b);
	}
	(CButton*)GetDlgItem(IDC_CHECK_NATIVE_CCT_ALL)->EnableWindow(b);
	(CButton*)GetDlgItem(IDC_CHECK_NATIVE_TRC_ALL)->EnableWindow(b);
	(CButton*)GetDlgItem(IDC_CHECK_SRGB_CCT_ALL)->EnableWindow(b);
	(CButton*)GetDlgItem(IDC_CHECK_SRGB_TRC_ALL)->EnableWindow(b);
	(CButton*)GetDlgItem(IDC_CHECK_ADOBE_CCT_ALL)->EnableWindow(b);
	(CButton*)GetDlgItem(IDC_CHECK_ADOBE_TRC_ALL)->EnableWindow(b);
	(CButton*)GetDlgItem(IDC_CHECK_BT2020_CCT_ALL)->EnableWindow(b);
	(CButton*)GetDlgItem(IDC_CHECK_BT2020_TRC_ALL)->EnableWindow(b);
	(CButton*)GetDlgItem(IDC_BUTTON_DELETELIST_NATIVE)->EnableWindow(b);
	(CButton*)GetDlgItem(IDC_BUTTON_ADDLIST_NATIVE)->EnableWindow(b);
	(CButton*)GetDlgItem(IDC_BUTTON_DELETELIST_SRGB)->EnableWindow(b);
	(CButton*)GetDlgItem(IDC_BUTTON_ADDLIST_SRGB)->EnableWindow(b);
	(CButton*)GetDlgItem(IDC_BUTTON_DELETELIST_ADOBE)->EnableWindow(b);
	(CButton*)GetDlgItem(IDC_BUTTON_ADDLIST_ADOBE)->EnableWindow(b);
	(CButton*)GetDlgItem(IDC_BUTTON_DELETELIST_BT2020)->EnableWindow(b);
	(CButton*)GetDlgItem(IDC_BUTTON_ADDLIST_BT2020)->EnableWindow(b);
	(CButton*)GetDlgItem(IDC_BUTTON_VERIFY_ALL)->EnableWindow(b);
	(CButton*)GetDlgItem(IDC_BUTTON_VERIFY_NONE)->EnableWindow(b);
	(CListBox*)GetDlgItem(IDC_LIST_VF)->EnableWindow(b);

}

void CCa200SampleDlg::Init_Hash_Select(unordered_map<string, bool> & select) {
	CString str;
	string s;
	for (int i = 0; i < cct_list.size(); i++) {
		for (int j = 0; j < trc_list.size(); j++) {
			str.Format("%s_%s", cct_list[i], trc_list[j]);
			s = str;
			select[s] = false;
		}
	}
}

void CCa200SampleDlg::Display_Verifiy_List() {
	CString str;
	CString str_list;
	string s;
	unordered_map<string, bool> select;
	((CListBox*)GetDlgItem(IDC_LIST_VF))->ResetContent();
	for (int i = 0; i <gamut_list.size(); i++) {
		select = *select_list[i];
		for (int j = 0; j < cct_list.size(); j++) {
			for (int k = 0; k < trc_list.size(); k++) {
				str.Format("%s_%s", cct_list[j], trc_list[k]);
				s = str;
				if (select[s]) {
					str_list.Format("%s_%s_%s", gamut_list[i], cct_list[j], trc_list[k]);
					((CListBox*)GetDlgItem(IDC_LIST_VF))->AddString(str_list);
				}
			}
		}
	}
}

void CCa200SampleDlg::Check_Selct(vector<CButton*> button_cct_select, vector<CButton*> button_trc_select, unordered_map<string, bool>& select, CString g, bool mode) {
	
	CString str;
	string s;
	CString str_list;
	for (int i = 0; i < cct_list.size(); i++) {
		if (BST_CHECKED == button_cct_select[i]->GetCheck())
		{
			for (int j = 0; j < trc_list.size(); j++) {
				if (BST_CHECKED == button_trc_select[j]->GetCheck())
				{
					str.Format("%s_%s", cct_list[i], trc_list[j]);
					s = str;
					if(select[s] != mode)
						select[s] = mode;
				}
			}
		}
	}
	Display_Verifiy_List();
}

void CCa200SampleDlg::Init_Panel_Matrix_1D_LUT_GAMMA() {
	CA->Configure_one_d_calibration_attribute(10, 1024);

	xy_str.Format("Initial...");
	SetDlgItemText(IDC_STATIC_xy, xy_str);

	Update_MCU_Matrix(CA->matrix_raw, "raw");

	CA->Generate_Gamma_LUT(panel_gamma_dicom_LUT, 1.0);
	//Update_MCU_LUT(panel_gamma_dicom_LUT, SELECT_1D_LUT_ENGAMMA, 0, "raw");
	//Update_MCU_LUT(panel_gamma_dicom_LUT, SELECT_1D_LUT_DEGAMMA, 1, "raw");
	Update_MCU_LUT(panel_gamma_dicom_LUT, SELECT_1D_LUT_CCT, 2, "raw");

	//OnBnClickedButtonTrc();

	gamut_target = "Native";
	cct_target = "6500";
	tone_curve_target = "2.2";

	inited = true;
}

void CCa200SampleDlg::_colorMatrix_double_2_uint16(MATRIX matrix) {
	bool bStop_matrix_process;
	double fTmp, fData;
	int32_t nTmp32, nData32;
	int _row, _index;
	uint16_t wData16, _sign_msb = 0x0000;

	CString strTmp;

	bStop_matrix_process = false;

	for (_row = 0; _row < 3; _row++) {
		for (_index = 0; _index < 3; _index++) {
			//fTmp = _fColorMatrix[_row][_index];
			//fTmp = (fTmp * 10000.0);
			fTmp = (matrix[_row][_index] * 10000.0);
			fData = round(fTmp);
			nTmp32 = (int32_t)fData;


			if ((nTmp32 > -16384) && (nTmp32 < 16384)) {
				if (nTmp32 < 0) {
					nData32 = labs(nTmp32);
					_sign_msb = (1 << 15);
				}
				else {
					nData32 = labs(nTmp32);
					_sign_msb = 0x00;
				}

#if 1
				wData16 = (_sign_msb | (uint16_t)nData32);
#else
				wData16 = (uint16_t)nData32;
				wData16 = (_sign_msb | wData16);
#endif

				m_uint16ColorMatrix[_row][_index] = wData16;
			}
			else { /* Invalid matrix-xettings */

				strTmp.Format(TEXT("Invalid Matrix-Setting (%d) is detected @ [%d,%d]"), nTmp32, _row, _index);
				AfxMessageBox(strTmp);

				bStop_matrix_process = true;
				break;
			}
		}

		if (bStop_matrix_process == true) {
			break;
		}

	}
}

void CCa200SampleDlg::_colorMatrix_uint16_2_FPGA_Register() {
	uint32_t Tmp32_Low, Tmp32_High;

	/* Matrix-Row for RED */
	Tmp32_Low = (uint32_t)m_uint16ColorMatrix[0][0];
	Tmp32_High = (uint32_t)m_uint16ColorMatrix[0][1];
	m_Reg_01E0 = ((Tmp32_High << 16) | Tmp32_Low);

	//Tmp32_Low = (uint32_t) m_uint16ColorMatrix[0][2];
	//reg_value = Tmp32_Low;
	m_Reg_01E4 = (uint32_t)m_uint16ColorMatrix[0][2];


	/* Matrix-Row for GREEN */
	Tmp32_Low = (uint32_t)m_uint16ColorMatrix[1][0];
	Tmp32_High = (uint32_t)m_uint16ColorMatrix[1][1];
	m_Reg_01E8 = ((Tmp32_High << 16) | Tmp32_Low);

	//Tmp32_Low = (uint32_t) m_uint16ColorMatrix[1][2];
	//reg_value = Tmp32_Low;
	m_Reg_01EC = (uint32_t)m_uint16ColorMatrix[1][2];


	/* Matrix-Row for BLUE */
	Tmp32_Low = (uint32_t)m_uint16ColorMatrix[2][0];
	Tmp32_High = (uint32_t)m_uint16ColorMatrix[2][1];
	m_Reg_01F0 = ((Tmp32_High << 16) | Tmp32_Low);

	//Tmp32_Low = (uint32_t) m_uint16ColorMatrix[2][2];
	//reg_value = Tmp32_Low;
	m_Reg_01F4 = (uint32_t)m_uint16ColorMatrix[2][2];
}

void CCa200SampleDlg::Update_MCU_Matrix(MATRIX matrix, CString tag) {
	video_pip_state["matrix"] = tag;
	UpdateData(TRUE);

	_colorMatrix_double_2_uint16(matrix);
	_colorMatrix_uint16_2_FPGA_Register();

	UpdateMatrixRegisters();
}

void CCa200SampleDlg::UpdateMatrixRegisters()
{
	// TODO: Add your control notification handler code here
	uint32_t dwData;
	int16_t nError = ERROR_SCALER_SUCCESS;
	CString strTmp;

	/* Matrix-Row for RED */
	/******* Matrix-element Red x Red & Red x Green  *******/
	nError = SPI_Register_Write(LinkNo, OFFSET_COLORMATRIX_RedRed_RedGreen, m_Reg_01E0);
	if (nError != ERROR_SCALER_SUCCESS) {
		strTmp.Format(TEXT("SPI_Register_Write[0x%08X] failed, Error Code=%d"), OFFSET_COLORMATRIX_RedRed_RedGreen, nError);
		AfxMessageBox(strTmp);

		return;
	}


	nError = SPI_Register_Read(LinkNo, OFFSET_COLORMATRIX_RedRed_RedGreen, &dwData);
	if (nError != ERROR_SCALER_SUCCESS) {
		strTmp.Format(TEXT("SPI_Register_Read[0x%08X] failed, Error Code=%d"), OFFSET_COLORMATRIX_RedRed_RedGreen, nError);
		AfxMessageBox(strTmp);

		return;
	}

	if (m_Reg_01E0 != dwData) {
		strTmp.Format(TEXT("[0x%08X] Readback-checking failed, writeData=0x%08X, readData=0x%08X"), OFFSET_COLORMATRIX_RedRed_RedGreen, m_Reg_01E0, dwData);
		AfxMessageBox(strTmp);

		return;
	}

	/******* Matrix-element Red x Blue  *******/
	nError = SPI_Register_Write(LinkNo, OFFSET_COLORMATRIX_RedBlue, m_Reg_01E4);
	if (nError != ERROR_SCALER_SUCCESS) {
		strTmp.Format(TEXT("SPI_Register_Write[0x%08X] failed, Error Code=%d"), OFFSET_COLORMATRIX_RedBlue, nError);
		AfxMessageBox(strTmp);

		return;
	}

	nError = SPI_Register_Read(LinkNo, OFFSET_COLORMATRIX_RedBlue, &dwData);
	if (nError != ERROR_SCALER_SUCCESS) {
		strTmp.Format(TEXT("SPI_Register_Read[0x%08X] failed, Error Code=%d"), OFFSET_COLORMATRIX_RedBlue, nError);
		AfxMessageBox(strTmp);

		return;
	}

	if (m_Reg_01E4 != dwData) {
		strTmp.Format(TEXT("[0x%08X] Readback-checking failed, writeData=0x%08X, readData=0x%08X"), OFFSET_COLORMATRIX_RedBlue, m_Reg_01E4, dwData);
		AfxMessageBox(strTmp);

		return;
	}

	/* Matrix-Row for GREEN */
	/******* Matrix-element Green x Red & Green x Green  *******/
	nError = SPI_Register_Write(LinkNo, OFFSET_COLORMATRIX_GreenRed_GreenGreen, m_Reg_01E8);
	if (nError != ERROR_SCALER_SUCCESS) {
		strTmp.Format(TEXT("SPI_Register_Write[0x%08X] failed, Error Code=%d"), OFFSET_COLORMATRIX_GreenRed_GreenGreen, nError);
		AfxMessageBox(strTmp);

		return;
	}


	nError = SPI_Register_Read(LinkNo, OFFSET_COLORMATRIX_GreenRed_GreenGreen, &dwData);
	if (nError != ERROR_SCALER_SUCCESS) {
		strTmp.Format(TEXT("SPI_Register_Read[0x%08X] failed, Error Code=%d"), OFFSET_COLORMATRIX_GreenRed_GreenGreen, nError);
		AfxMessageBox(strTmp);

		return;
	}

	if (m_Reg_01E8 != dwData) {
		strTmp.Format(TEXT("[0x%08X] Readback-checking failed, writeData=0x%08X, readData=0x%08X"), OFFSET_COLORMATRIX_GreenRed_GreenGreen, m_Reg_01E8, dwData);
		AfxMessageBox(strTmp);

		return;
	}

	/******* Matrix-element Green x Blue  *******/
	nError = SPI_Register_Write(LinkNo, OFFSET_COLORMATRIX_GreenBlue, m_Reg_01EC);
	if (nError != ERROR_SCALER_SUCCESS) {
		strTmp.Format(TEXT("SPI_Register_Write[0x%08X] failed, Error Code=%d"), OFFSET_COLORMATRIX_GreenBlue, nError);
		AfxMessageBox(strTmp);

		return;
	}

	nError = SPI_Register_Read(LinkNo, OFFSET_COLORMATRIX_GreenBlue, &dwData);
	if (nError != ERROR_SCALER_SUCCESS) {
		strTmp.Format(TEXT("SPI_Register_Read[0x%08X] failed, Error Code=%d"), OFFSET_COLORMATRIX_GreenBlue, nError);
		AfxMessageBox(strTmp);

		return;
	}

	if (m_Reg_01EC != dwData) {
		strTmp.Format(TEXT("[0x%08X] Readback-checking failed, writeData=0x%08X, readData=0x%08X"), OFFSET_COLORMATRIX_GreenBlue, m_Reg_01EC, dwData);
		AfxMessageBox(strTmp);

		return;
	}

	/* Matrix-Row for BLUE */
	/******* Matrix-element Blue x Red & Blue x Green  *******/
	nError = SPI_Register_Write(LinkNo, OFFSET_COLORMATRIX_BlueRed_BlueGreen, m_Reg_01F0);
	if (nError != ERROR_SCALER_SUCCESS) {
		strTmp.Format(TEXT("SPI_Register_Write[0x%08X] failed, Error Code=%d"), OFFSET_COLORMATRIX_BlueRed_BlueGreen, nError);
		AfxMessageBox(strTmp);

		return;
	}


	nError = SPI_Register_Read(LinkNo, OFFSET_COLORMATRIX_BlueRed_BlueGreen, &dwData);
	if (nError != ERROR_SCALER_SUCCESS) {
		strTmp.Format(TEXT("SPI_Register_Read[0x%08X] failed, Error Code=%d"), OFFSET_COLORMATRIX_BlueRed_BlueGreen, nError);
		AfxMessageBox(strTmp);

		return;
	}

	if (m_Reg_01F0 != dwData) {
		strTmp.Format(TEXT("[0x%08X] Readback-checking failed, writeData=0x%08X, readData=0x%08X"), OFFSET_COLORMATRIX_BlueRed_BlueGreen, m_Reg_01F0, dwData);
		AfxMessageBox(strTmp);

		return;
	}

	/******* Matrix-element Blue x Blue  *******/
	nError = SPI_Register_Write(LinkNo, OFFSET_COLORMATRIX_BlueBlue, m_Reg_01F4);
	if (nError != ERROR_SCALER_SUCCESS) {
		strTmp.Format(TEXT("SPI_Register_Write[0x%08X] failed, Error Code=%d"), OFFSET_COLORMATRIX_BlueBlue, nError);
		AfxMessageBox(strTmp);

		return;
	}

	nError = SPI_Register_Read(LinkNo, OFFSET_COLORMATRIX_BlueBlue, &dwData);
	if (nError != ERROR_SCALER_SUCCESS) {
		strTmp.Format(TEXT("SPI_Register_Read[0x%08X] failed, Error Code=%d"), OFFSET_COLORMATRIX_BlueBlue, nError);
		AfxMessageBox(strTmp);

		return;
	}

	if (m_Reg_01F4 != dwData) {
		strTmp.Format(TEXT("[0x%08X] Readback-checking failed, writeData=0x%08X, readData=0x%08X"), OFFSET_COLORMATRIX_BlueBlue, m_Reg_01F4, dwData);
		AfxMessageBox(strTmp);

		return;
	}
}

void CCa200SampleDlg::PipelineLutCtlRefresh()
{
	// TODO: Add your control notification handler code here
	uint32_t _dwTxVideoPipelineCtrl;
	int16_t nError;
	CString strTemp;

	nError = SPI_Register_Read(LinkNo, OFFSET_TX_VIDEO_CONTROL_0, &_dwTxVideoPipelineCtrl);
	if (nError != ERROR_SCALER_SUCCESS) {
		strTemp.Format(TEXT("SPI_Register_Read() failed!, errno=%d"), nError);
		AfxMessageBox(strTemp);

		return;
	}
}


void CCa200SampleDlg::Update_MCU_LUT(const PRGBColor LUT, const uint32_t _m_dwFPGALUTSelect, const  uint32_t pip_position, const CString tag) {
	for (int i = 0; i < RGB_depth; i++) {
		mcu_temp_LUT[i] = LUT[i];
	}
	UpdateLutToMcu_FPGA(_m_dwFPGALUTSelect);

	switch (pip_position) {
	case 0:
		video_pip_state["encode_lut"] = tag;
		break;
	case 1:
		video_pip_state["decode_lut"] = tag;
		break;
	case 2:
		video_pip_state["base_lut"] = tag;
		break;
	default:
		return;
	}

}

void CCa200SampleDlg::UpdateLutToMcu_FPGA(const uint32_t _m_dwFPGALUTSelect) {
	XferLutToMcu();
	UpdateLutBufferToFPGA(_m_dwFPGALUTSelect);
}


void CCa200SampleDlg::XferLutToMcu() {
	// TODO: Add your control notification handler code here
	CString strError;
	int16_t nError = ERROR_SCALER_SUCCESS;
	//bool bDeGammaDataValidate;

	uint16_t LUT_index;
	int r;
	for (LUT_index = 0; LUT_index < RGB_depth; LUT_index++)
	{

		r = double(LUT_index) / double(RGB_max) * 100.0;
		xy_str.Format("Writing to MCU: %d%%", r);
		SetDlgItemText(IDC_STATIC_xy, xy_str);
		nError = Xfer_LUTable1D_Element(LinkNo, LUT_index, &(mcu_temp_LUT[LUT_index]));
		if (nError != ERROR_SCALER_SUCCESS) {
			break;
		}
	}

	if (nError != ERROR_SCALER_SUCCESS)
	{
		strError.Format(TEXT("Xfer_LUTable1D_Element() failed @ color-index, error code=%d"), LUT_index, nError);
		AfxMessageBox(strError);
	}
	else
	{
		//m_pcLUTUpdate.SetPos((int)0);
		//AfxMessageBox(TEXT("Transferring 1D_LUT RGB 1024-Entries is completed"));
	}
}

void CCa200SampleDlg::UpdateLutBufferToFPGA(const uint32_t m_dwFPGALUTSelect)
{
	// TODO: Add your control notification handler code here
#if defined(_KEEP_ENABLED_LUTS_after_TX_Video_Pipeline_LUT_Update_FPGA_) && (_KEEP_ENABLED_LUTS_after_TX_Video_Pipeline_LUT_Update_FPGA_ == 1)
	uint32_t _dwEnableLUTs, _dwMaskLUTs, _dwTemp;
	uint32_t _dwTxVideoPipelineCtrl;
#endif /* _KEEP_ENABLED_LUTS_after_TX_Video_Pipeline_LUT_Update_FPGA_ */
	int16_t nError;
	CString strTemp;


#if defined(_KEEP_ENABLED_LUTS_after_TX_Video_Pipeline_LUT_Update_FPGA_) && (_KEEP_ENABLED_LUTS_after_TX_Video_Pipeline_LUT_Update_FPGA_ == 1)
	switch (m_dwFPGALUTSelect)
	{
	case SELECT_3D_LUT:
		_dwMaskLUTs = ~(TX_VIDEO_PIPELINE_3D_LUT_BYPASS_MASK);
		_dwEnableLUTs = TX_VIDEO_PIPELINE_3D_LUT_ENABLE;
	case SELECT_1D_LUT_ENGAMMA:
	case SELECT_1D_LUT_DEGAMMA:
		_dwMaskLUTs = ~(TX_VIDEO_PIPELINE_1D_LUT_DEGAMMA_BYPASS_MASK);
		_dwEnableLUTs = TX_VIDEO_PIPELINE_1D_LUT_DEGAMMA_ENABLE;
		break;
	case SELECT_1D_LUT_CCT:
		_dwMaskLUTs = ~(TX_VIDEO_PIPELINE_1D_LUT_CCT_BYPASS_MASK);
		_dwEnableLUTs = TX_VIDEO_PIPELINE_1D_LUT_CCT_ENABLE;
		break;
	default:
		break;

	}

	nError = SPI_Register_Read(LinkNo, OFFSET_TX_VIDEO_CONTROL_0, &_dwTxVideoPipelineCtrl);
	if (nError != ERROR_SCALER_SUCCESS) {
		strTemp.Format(TEXT("SPI_Register_Read() failed!, errno=%d"), nError);
		AfxMessageBox(strTemp);

		return;
	}

	_dwTemp = (_dwTxVideoPipelineCtrl & _dwMaskLUTs);
	_dwTxVideoPipelineCtrl = (_dwTemp | _dwEnableLUTs);
#endif /* _KEEP_ENABLED_LUTS_after_TX_Video_Pipeline_LUT_Update_FPGA_ */


	nError = TX_Video_Pipeline_LUT_Update_FPGA(LinkNo, m_dwFPGALUTSelect, m_bLUTReadbackChecking);
	if (nError != ERROR_SCALER_SUCCESS) {
		strTemp.Format(TEXT("TX_Video_Pipeline_LUT_Update_FPGA() failed, error code=%d"), nError);
		AfxMessageBox(strTemp);

		return;
	}
	else {

#if defined(_KEEP_ENABLED_LUTS_after_TX_Video_Pipeline_LUT_Update_FPGA_) && (_KEEP_ENABLED_LUTS_after_TX_Video_Pipeline_LUT_Update_FPGA_ == 1)
		nError = SPI_Register_Write(LinkNo, OFFSET_TX_VIDEO_CONTROL_0, _dwTxVideoPipelineCtrl);
		if (nError != ERROR_SCALER_SUCCESS) {
			strTemp.Format(TEXT("SPI_Register_Write() failed!, errno=%d"), nError);
			AfxMessageBox(strTemp);
		}
#endif /* _KEEP_ENABLED_LUTS_after_TX_Video_Pipeline_LUT_Update_FPGA_ */

		/* Update LUT status */
		PipelineLutCtlRefresh();

		//AfxMessageBox(TEXT("Updating 1D_LUT-Buffer to FPFA is done"));
	}
}


void CCa200SampleDlg::Update_MCU_3D_LUT(const vector<vector<vector<RGBColor>>>& three_d_lut, const uint32_t _m_dwFPGALUTSelect, const uint32_t pip_position, const CString tag) {
	uint32_t red_color, green_color, blue_color;
	vector<uint32_t> m_3DLUT_Buffer;
	uint32_t _3DrgbColor;
	uint16_t index = 0;
	for (uint16_t r = 0; r < three_d_lut[0].size(); r++) {
		for (uint16_t g = 0; g < three_d_lut[0].size(); g++) {
			for (uint16_t b = 0; b < three_d_lut[0].size(); b++) {
				red_color = uint32_t(three_d_lut[r][g][b].Red);
				green_color = uint32_t(three_d_lut[r][g][b].Green);
				blue_color = uint32_t(three_d_lut[r][g][b].Blue);
				_3DrgbColor = ((red_color) | (green_color << 10) | (blue_color << 20));
				m_3DLUT_Buffer.push_back(_3DrgbColor);
				index++;
			}
		}
	}

	Update3DLutToMcu_FPGA(m_3DLUT_Buffer, _m_dwFPGALUTSelect);

	video_pip_state["3D_lut"] = tag;
}


void CCa200SampleDlg::Update3DLutToMcu_FPGA(const vector<uint32_t> m_3DLUT_Buffer, const uint32_t  _m_dwFPGALUTSelect) {
	XferLut3DLutToMcu(m_3DLUT_Buffer);
 	UpdateLutBufferToFPGA(_m_dwFPGALUTSelect);
}


void CCa200SampleDlg::XferLut3DLutToMcu(const vector<uint32_t> m_3DLUT_Buffer) {
	// TODO: Add your control notification handler code here
	CString strError;
	int16_t nError = ERROR_SCALER_SUCCESS;
	//bool bDeGammaDataValidate;

	uint16_t LUT_index = 0;

	for (LUT_index = 0; LUT_index < _3DLUT_array_size; LUT_index++)
	{
		nError = Xfer_LUTable3D_U32Data(LinkNo, LUT_index, m_3DLUT_Buffer[LUT_index]);
		if (nError != ERROR_SCALER_SUCCESS) {
			break;
		}
	}


	if (nError != ERROR_SCALER_SUCCESS)
	{
		strError.Format(TEXT("Xfer_LUTable3D_U32Data() failed @ color-index, error code=%d"), LUT_index, nError);
		AfxMessageBox(strError);
	}
}


void CCa200SampleDlg::Colour_Verify() {
	unordered_map <string, bool> select;
	CString cstr;
	string str;
	CString base_lut_path;
	RGBColor tone_lut[RGB_depth];
	for (int i = 0; i < gamut_list.size(); i++) {
		gamut_target = gamut_list[i];
		select = *select_list[i];
		for (int j = 0; j < cct_list.size(); j++) {
			cct_target = cct_list[j];
			for (int k = 0; k < trc_list.size(); k++) {
				tone_curve_target = trc_list[k];
				cstr.Format("%s_%s", cct_list[j], trc_list[k]);
				str = cstr;
				if (select[str]) {
					cstr.Format("%s_%s_%s", gamut_target, cct_list[j], trc_list[k]);
					((CListBox*)GetDlgItem(IDC_LIST_VF))->SetCurSel(((CListBox*)GetDlgItem(IDC_LIST_VF))->FindStringExact(0, cstr));
  
					RGBColor tone_encode_LUT[RGB_depth];
				
					CString tone_lut_path;
					CString three_d_lut_path;
					RGBColor tone_lut[RGB_depth];
					vector<vector <vector<RGBColor>>> three_d_lut;
					tone_lut_path.Format(".\\data\\lut\\tone_curve\\%s\\%s_%s.csv", cct_target, cct_target, tone_curve_target);
					//Load_Panel_1D_LUT(tone_lut_path, tone_lut);

					//three_d_lut_path.Format(".\\data\\lut\\three_d_lut\\%s\\%s_%s.csv", cct_target, gamut_target, cct_target);
					//Load_Panel_3D_LUT(three_d_lut_path, three_d_lut, 17);
					//Update_MCU_3D_LUT(three_d_lut, SELECT_3D_LUT, 0, gamut_target + "_" + cct_target + "_" + tone_curve_target);

					base_lut_path.Format(".\\data\\lut\\base\\base_%s.csv", cct_target);
					Load_Panel_1D_LUT(base_lut_path, tone_lut);
				
					if (video_pip_state["base_lut"].c_str() != base_lut_path) {
						Update_MCU_LUT(tone_lut, SELECT_1D_LUT_CCT, 2, base_lut_path);
					}

					Verify_Color(gamut_list[i], cct_list[j], trc_list[k]);
				}
			}
		}
	}
}


void CCa200SampleDlg::Verify_Color(CString gamut, CString cct, CString trc) {
	Check_State();
	send_index.clear();
	CString color_m_filename;
	CString str;
	CString cstr_matrix;

	xy_str.Format("Measuring %s, %sK, %s", gamut, cct, trc);
	SetDlgItemText(IDC_STATIC_xy, xy_str);

	str.Format(".\\data\\RGB_Own_LUT.csv");
	Load_Send_Index(str);

	Init_Data_Vector();
	Send_Get(send_index, nRet, LinkNo, data, true);

	color_m_filename.Format(".\\data\\calibration_data_report\\color_measure_data\\%s_%s_%s.csv", gamut, cct, trc);
	Write_Measure_Data(color_m_filename, data);
}

void CCa200SampleDlg:: Colour_Calibrate() {
	CString time;
	clock_t start_whole = clock();
	Check_State(); //check link state

	CString one_d_LUT_filename;
	CString gamut_cct_matrix_filename;

	vector<double> Lv_calibrated;
	vector<double> Lv_calibrated_std;

	double illuminance_amb = 0;
	double display_reflection = 0.000;
	double luminance_ratio = 0;
	double specific_lv_min = 1.0;
	double specific_lv_max = 0;
	int tone_interval = 1;
	vector<int> tone_index;
	vector<double> tone_lv_nor;

	MATRIX matrix_native_RGB_XYZ;
	MATRIX matrix_gamut_cct;

	CString three_d_lut_path;
	const int three_d_lut_size = 17;

	vector<CString> CCT_options = { "5000","5800","6500","7500","9300" };

	vector<double> lv_r;
	vector<double> lv_g;
	vector<double> lv_b;
	vector<double> lv_w;
	vector<MATRIX> panel_RGBW_maatrix;
	send_index.clear();
	send_index.push_back({ 0,0,0 });
	Send_Get(send_index, nRet, LinkNo, data, false);
	XYZ_0 = { data[0][3], data[0][4],data[0][5] };
	lv_r.push_back(data[0][4]);
	lv_g.push_back(data[0][4]);
	lv_b.push_back(data[0][4]);
	lv_w.push_back(data[0][4]);
	int v;
	for (int i = 0; i < 32; i++) {
		send_index.clear();
		v = (i + 1) * 32 - 1;
		send_index.push_back({ v,0,0 });
		send_index.push_back({ 0,v,0 });
		send_index.push_back({ 0,0,v });
		send_index.push_back({ v,v,v });
		Init_Data_Vector();
		Send_Get(send_index, nRet, LinkNo, data, false);
		XYZ_R = { data[0][3], data[0][4],data[0][5] };
		XYZ_G = { data[1][3], data[1][4],data[1][5] };
		XYZ_B = { data[2][3], data[2][4],data[2][5] };
		XYZ_W = { data[3][3], data[3][4],data[3][5] };
		lv_r.push_back(data[0][4]);
		lv_g.push_back(data[1][4]);
		lv_b.push_back(data[2][4]);
		lv_w.push_back(data[3][4]);
		panel_RGBW_maatrix.push_back(CA->Build_Matrix_RGB_to_XYZ(XYZ_R, XYZ_G, XYZ_B, XYZ_W, XYZ_0));
	}
	for (int i = 0; i < lv_r.size(); i++) {
		lv_r[i] /= lv_r[lv_r.size() - 1];
		lv_g[i] /= lv_g[lv_g.size() - 1];
		lv_b[i] /= lv_b[lv_b.size() - 1];
		lv_w[i] /= lv_w[lv_w.size() - 1];
	}
	panel_tone_curve.r = CA->calculate_trc(lv_r);
	panel_tone_curve.g = CA->calculate_trc(lv_g);
	panel_tone_curve.b = CA->calculate_trc(lv_b);
	panel_tone_curve.w = CA->calculate_trc(lv_w);

	for (int cct_index = 0; cct_index < CCT_options.size(); cct_index++) {
		clock_t start_cct = clock();
		inited = false;
		if (!inited) {
			Init_Panel_Matrix_1D_LUT_GAMMA();
		}
		cct_target = CCT_options[cct_index];
		panel_CCT_Calibration(panel_1d_LUT, atoi(cct_target), Lv_calibrated, panel_RGBW_maatrix);

		one_d_LUT_filename.Format(".\\data\\lut\\base\\base_%s.csv", cct_target);
		Write_1D_LUT(one_d_LUT_filename, panel_1d_LUT);
		Update_MCU_LUT(panel_1d_LUT, SELECT_1D_LUT_CCT, 2, one_d_LUT_filename);

		for (int i = 0; i < Lv_calibrated.size(); i++) {
			Lv_calibrated_std.push_back((Lv_calibrated[i] - Lv_calibrated[0]) / (Lv_calibrated[Lv_calibrated.size() - 1] - Lv_calibrated[0]));
		}
		CA->panel_tone_curve.w = CA->calculate_trc(Lv_calibrated_std);
		Lv_calibrated_std.clear();


		CA->DICOM_Mapping(panel_1d_LUT, panel_gamma_dicom_LUT, illuminance_amb, display_reflection, luminance_ratio, specific_lv_min, specific_lv_max, Lv_calibrated, tone_index, tone_lv_nor, tone_interval);
		for (int i = 0; i < RGB_depth; i++) {
			panel_gamma_dicom_LUT[i].Red = uint16_t(tone_index[i]);
			panel_gamma_dicom_LUT[i].Green = uint16_t(tone_index[i]);
			panel_gamma_dicom_LUT[i].Blue = uint16_t(tone_index[i]);
		}
		tone_index.clear();
		tone_lv_nor.clear();
		one_d_LUT_filename.Format(".\\data\\lut\\tone_curve\\%s\\%s_DICOMGSDF.csv", cct_target, cct_target);
		Write_1D_LUT(one_d_LUT_filename, panel_gamma_dicom_LUT);

		CA->PQ_Mapping(panel_1d_LUT, panel_gamma_dicom_LUT, Lv_calibrated, tone_index, tone_interval);
		for (int i = 0; i < RGB_depth; i++) {
			panel_gamma_dicom_LUT[i].Red = min(uint16_t(tone_index[i]), int(RGB_max));
			panel_gamma_dicom_LUT[i].Green = min(uint16_t(tone_index[i]), int(RGB_max));
			panel_gamma_dicom_LUT[i].Blue = min(uint16_t(tone_index[i]), int(RGB_max));
		}
		tone_index.clear();
		tone_lv_nor.clear();
		one_d_LUT_filename.Format(".\\data\\lut\\tone_curve\\%s\\%s_PQ.csv", cct_target, cct_target);
		Write_1D_LUT(one_d_LUT_filename, panel_gamma_dicom_LUT);

		CA->HLG_Mapping(panel_1d_LUT, panel_gamma_dicom_LUT, Lv_calibrated, tone_index, tone_interval);
		for (int i = 0; i < RGB_depth; i++) {
			panel_gamma_dicom_LUT[i].Red = uint16_t(tone_index[i]);
			panel_gamma_dicom_LUT[i].Green = uint16_t(tone_index[i]);
			panel_gamma_dicom_LUT[i].Blue = uint16_t(tone_index[i]);
		}
		tone_index.clear();
		tone_lv_nor.clear();
		one_d_LUT_filename.Format(".\\data\\lut\\tone_curve\\%s\\%s_HLG.csv", cct_target, cct_target);
		Write_1D_LUT(one_d_LUT_filename, panel_gamma_dicom_LUT);

		Measure_RGBW();

		for (int k = 0; k < trc_list.size(); k++) {
			tone_curve_target = trc_list[k];
			if (tone_curve_target == "PQ" || tone_curve_target == "HLG" || tone_curve_target == "DICOMGSDF") {
				continue;
			}
			else
			{
				CA->Gamma_Mapping(panel_1d_LUT, panel_gamma_dicom_LUT, Lv_calibrated, atof(tone_curve_target), tone_index, tone_interval);
				for (int i = 0; i < RGB_depth; i++) {
					panel_gamma_dicom_LUT[i].Red = uint16_t(tone_index[i]);
					panel_gamma_dicom_LUT[i].Green = uint16_t(tone_index[i]);
					panel_gamma_dicom_LUT[i].Blue = uint16_t(tone_index[i]);
				}
				tone_index.clear();
				one_d_LUT_filename.Format(".\\data\\lut\\tone_curve\\%s\\%s_%s.csv", cct_target, cct_target, tone_curve_target);
				Write_1D_LUT(one_d_LUT_filename, panel_gamma_dicom_LUT);
			}
		}

		matrix_native_RGB_XYZ = CA->Build_Matrix_RGB_to_XYZ(XYZ_R, XYZ_G, XYZ_B, XYZ_W, XYZ_0);
		matrix_gamut_cct;

		for (int i = 0; i < gamut_list.size(); i++) {
			gamut_target = gamut_list[i];
			if (gamut_target == "Native") {
				matrix_gamut_cct = CA->matrix_raw;
			}

			CA->Build_Matrix_Gamut_CCT_Trans(matrix_native_RGB_XYZ, matrix_gamut_cct, XYZ_W, XYZ_0, gamut_target, atoi(cct_target));
			gamut_cct_matrix_filename.Format(".\\data\\matrix\\matrix_%s_%s.csv", gamut_target, cct_target);
			Write_Matrix(gamut_cct_matrix_filename, matrix_gamut_cct);

			vector<vector <vector<RGBColor>>> three_d_lut;
			CA->Build_Three_D_LUT(three_d_lut, matrix_gamut_cct, tone_curve_target, three_d_lut_size);
			three_d_lut_path.Format(".\\data\\lut\\three_d_lut\\%s\\%s_%s.csv", cct_target, gamut_target, cct_target);
			Write_3D_LUT(three_d_lut_path, three_d_lut);
		}

		clock_t end_cct = clock();
		time.Format("%s K ----- %.2f sec, %.2f min\n", cct_target, ((double)(end_cct - start_cct)) / CLOCKS_PER_SEC, ((double)(end_cct - start_cct)) / CLOCKS_PER_SEC / 60.0);
		write_time_log(string(time));
	}
	//Export_Data_Image();
	clock_t end_whole = clock();
	time.Format("all of the calibration time ----- %.2f sec,  %.2f min\n", ((double)(end_whole - start_whole)) / CLOCKS_PER_SEC, ((double)(end_whole - start_whole)) / CLOCKS_PER_SEC / 60.0);
	write_time_log(string(time));
	OnBnClickedCheckImage();
}

// Initializes the port and sets the communication parameters
void initPort(HANDLE* hCom, char* COMPORT, int nComRate, int nComBits, COMMTIMEOUTS timeout) {
//void initPort(HANDLE * hCom, wchar_t* COMPORT, int nComRate, int nComBits, COMMTIMEOUTS timeout) {
		createPortFile(hCom, COMPORT);						// Initializes hCom to point to PORT#
	purgePort(hCom);									// Purges the COM port
	SetComParms(hCom, nComRate, nComBits, timeout);		// Uses the DCB structure to set up the COM port
	purgePort(hCom);
}

// Purge any outstanding requests on the serial port (initialize)
void purgePort(HANDLE* hCom) {
	PurgeComm(*hCom, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
}

// Output/Input messages to/from ports 
void outputToPort(HANDLE* hCom, LPCVOID buf, DWORD szBuf) {
	int i = 0;
	DWORD NumberofBytesTransmitted;
	LPDWORD lpErrors = 0;
	LPCOMSTAT lpStat = 0;

	i = WriteFile(
		*hCom,										// Write handle pointing to COM port
		buf,										// Buffer size
		szBuf,										// Size of buffer
		&NumberofBytesTransmitted,					// Written number of bytes
		NULL
	);
	// Handle the timeout error
	if (i == 0) {
		printf("\nWrite Error: 0x%x\n", GetLastError());
		ClearCommError(hCom, lpErrors, lpStat);		// Clears the device error flag to enable additional input and output operations. Retrieves information ofthe communications error.	
	}
	else
		printf("\nSuccessful transmission, there were %ld bytes transmitted\n", NumberofBytesTransmitted);
}

DWORD inputFromPort(HANDLE* hCom, LPVOID buf, DWORD szBuf) {
	int i = 0;
	DWORD NumberofBytesRead;
	LPDWORD lpErrors = 0;
	LPCOMSTAT lpStat = 0;

	i = ReadFile(
		*hCom,										// Read handle pointing to COM port
		buf,										// Buffer size
		szBuf,  									// Size of buffer - Maximum number of bytes to read
		&NumberofBytesRead,
		NULL
	);
	// Handle the timeout error
	if (i == 0) {
		printf("\nRead Error: 0x%x\n", GetLastError());
		ClearCommError(hCom, lpErrors, lpStat);		// Clears the device error flag to enable additional input and output operations. Retrieves information ofthe communications error.
	}
	else
		printf("\nSuccessful reception!, There were %ld bytes read\n", NumberofBytesRead);

	return(NumberofBytesRead);
}

// Sub functions called by above functions
/**************************************************************************************/
// Set the hCom HANDLE to point to a COM port, initialize for reading and writing, open the port and set securities
void createPortFile(HANDLE* hCom, char* COMPORT) {
//void createPortFile(HANDLE * hCom, wchar_t* COMPORT) {
		// Call the CreateFile() function 
	LPCSTR COMPORTset;
	COMPORTset = (LPCSTR)COMPORT;
	//COMPORTset = _T("\\\\.\\COM5");

	*hCom = CreateFile(
		COMPORTset,									// COM port number  --> If COM# is larger than 9 then use the following syntax--> "\\\\.\\COM10"
		GENERIC_READ | GENERIC_WRITE,				// Open for read and write
		NULL,										// No sharing allowed
		NULL,										// No security
		OPEN_EXISTING,								// Opens the existing com port
		FILE_ATTRIBUTE_NORMAL,						// Do not set any file attributes --> Use synchronous operation
		NULL										// No template
	);

	if (*hCom == INVALID_HANDLE_VALUE) {
		printf("\nFatal Error 0x%x: Unable to open\n", GetLastError());
	}
	else {
		printf("\nCOM is now open\n");
	}
}

static int SetComParms(HANDLE* hCom, int nComRate, int nComBits, COMMTIMEOUTS timeout) {
	DCB dcb;										// Windows device control block
	// Clear DCB to start out clean, then get current settings
	memset(&dcb, 0, sizeof(dcb));
	dcb.DCBlength = sizeof(dcb);
	if (!GetCommState(*hCom, &dcb))
		return(0);

	// Set our own parameters from Globals
	dcb.BaudRate = nComRate;						// Baud (bit) rate
	dcb.ByteSize = (BYTE)nComBits;					// Number of bits(8)
	dcb.Parity = 0;									// No parity	+
	dcb.StopBits = ONESTOPBIT;						// One stop bit
	if (!SetCommState(*hCom, &dcb))
		return(0);

	// Set communication timeouts (SEE COMMTIMEOUTS structure in MSDN) - want a fairly long timeout
	memset((void*)&timeout, 0, sizeof(timeout));
	timeout.ReadIntervalTimeout = 500;				// Maximum time allowed to elapse before arival of next byte in milliseconds. If the one_d_calibration_lut_size between the arrival of any two bytes exceeds this amount, the ReadFile operation is completed and buffered data is returned
	timeout.ReadTotalTimeoutMultiplier = 1;			// The multiplier used to calculate the total time-out period for read operations in milliseconds. For each read operation this value is multiplied by the requested number of bytes to be read
	timeout.ReadTotalTimeoutConstant = 5000;		// A constant added to the calculation of the total time-out period. This constant is added to the resulting product of the ReadTotalTimeoutMultiplier and the number of bytes (above).
	SetCommTimeouts(*hCom, &timeout);
	return(1);
}

/*
// Note: Comment out the Tx or Rx sections below to operate in single sided mode

// Declare constants, variables and communication parameters
const int BUFSIZE = 140;							// Buffer size
//wchar_t COMPORT_Rx[] = L"\\\\.\\COM5";						// COM port used for Rx (use L"COM6" for transmit program)
char COMPORT_Rx[] = "\\\\.\\COM5";						// COM port used for Rx (use L"COM6" for transmit program)
//_T("\\\\.\\COM%d")
//wchar_t COMPORT_Tx[] = L"\\\\.\\COM7";						// COM port used for Rx (use L"COM6" for transmit program)
char COMPORT_Tx[] = "\\\\.\\COM7";						// COM port used for Rx (use L"COM6" for transmit program)

// Communication variables and parameters
HANDLE hComRx = 0;										// Pointer to the selected COM port (Receiver)
HANDLE hComTx = 0;										// Pointer to the selected COM port (Transmitter)
int nComRate = 9600;								// Baud (Bit) rate in bits/second 
int nComBits = 8;									// Number of bits per frame
COMMTIMEOUTS timeout;								// A commtimeout struct variable

*/






/////////////////////////////////////////////////////////////////////////////
// CCa200SampleDlg Message handler

BOOL CCa200SampleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	
	SetIcon(m_hIcon, TRUE);			
	SetIcon(m_hIcon, FALSE);		
	
	// TODO: When do special initialization, please add to this site.
	// CA-SDK

	
	long lcan = 1;
	_bstr_t strcnfig(_T("1"));
	long lprt = PORT_USB;
	long lbr = 38400;
	_bstr_t strprbid(_T("P1"));
	_variant_t vprbid(_T("P1"));

	try{

		m_pCa200Obj = ICa200Ptr(__uuidof(Ca200));
		m_pCa200Obj->SetConfiguration(lcan, strcnfig, lprt, lbr);
	}
	catch(_com_error e){
		CString strerr;
		strerr.Format(_T("HR:0x%08x\nMSG:%s"), e.Error(), (LPCSTR)e.Description());
		AfxMessageBox((LPCSTR)strerr);
		return TRUE;
	}


	m_pCasObj = m_pCa200Obj->Cas;
	m_pCaObj = m_pCasObj ->ItemOfNumber[lcan];
	m_pOutputProbesObj = m_pCaObj ->OutputProbes;
	m_pOutputProbesObj ->RemoveAll();
	m_pOutputProbesObj ->Add(strprbid);
	m_pProbeObj = m_pOutputProbesObj ->Item[vprbid];
	m_pMemoryObj = m_pCaObj->Memory;

	m_pCaObj->SyncMode = SYNC_NTSC;
	m_pCaObj->AveragingMode = AVRG_FAST;
	m_pCaObj->SetAnalogRange(2.5, 2.5);
	m_pCaObj->DisplayMode = DSP_LXY;
	m_pCaObj->DisplayDigits = DIGT_4;
	m_pMemoryObj->ChannelNO = 0;


/*
	m_dwCk = 0;

	CCaEvent* pevntobj;

	if (NULL != (pevntobj = new CCaEvent)){
		m_pIDispatch = pevntobj ->GetIDispatch(FALSE);

		IConnectionPointContainerPtr pcpcobj;
		DWORD dwck;

		pcpcobj = m_pCaObj;
		pcpcobj -> FindConnectionPoint(IID_ICaEvent, &m_pIConnectionPointObj);
		m_pIConnectionPointObj ->Advise(m_pIDispatch, &dwck);
		m_dwCk = dwck;
	}
*/
	return TRUE;  
}

void CCa200SampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}



void CCa200SampleDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}


HCURSOR CCa200SampleDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CCa200SampleDlg::OnClose() 
{
	if (CanExit()){
		// CA-SDK
		if (m_dwCk != 0){
			m_pIConnectionPointObj ->Unadvise(m_dwCk);
		}
		m_pIDispatch ->Release();
		m_pCaObj ->RemoteMode = 0;
		CDialog::OnClose();
	}
}

void CCa200SampleDlg::OnOK() 
{
	Disconnect_ScalerBoard(LinkNo);
	if (CanExit()){
		// CA-SDK
		if (m_dwCk != 0){
			m_pIConnectionPointObj ->Unadvise(m_dwCk);
		}
		m_pIDispatch ->Release();
		m_pCaObj ->RemoteMode = 0;
		CDialog::OnOK();
	}
}

void CCa200SampleDlg::OnCancel() 
{
	if (CanExit()){
		CDialog::OnCancel();
	}
}

BOOL CCa200SampleDlg::CanExit()
{

	if (m_pAutoProxy != NULL)
	{
		ShowWindow(SW_HIDE);
		return FALSE;
	}

	return TRUE;
}

void CCa200SampleDlg::OnButtonCal0() 
{
	// CA-SDK
	try{
		m_pCaObj->CalZero();
	}
	catch(_com_error e){
		CString strerr;
		strerr.Format(_T("HR:0x%08x\nMSG:%s"), e.Error(), (LPCSTR)e.Description());
		AfxMessageBox((LPCSTR)strerr);
		return;
	}

	CButton* pb;

	pb = (CButton *)GetDlgItem(IDC_BUTTON_MSR);
	pb->EnableWindow(TRUE);
	
}

//Copyright (c) 2002-2013 KONICA MINOLTA, INC. 

void CCa200SampleDlg::OnBnClickedCancel()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CDialog::OnCancel();
}

void CCa200SampleDlg::OnBnClickedOk()
{
	CDialog::OnOK();
}

void CCa200SampleDlg::OnBnClickedButtonTrc()
{
	Check_State();
	Measure_Raw();
}

void CCa200SampleDlg::OnBnClickedButtonCalibrate()
{
	((CStatic*)GetDlgItem(IDC_STATIC_GP_CONDITION))->EnableWindow(0);
	CString mode;
	((CComboBox*)GetDlgItem(IDC_COMBO_MODE))->GetWindowText(mode);
	if (mode == "Calibrate") {
		Colour_Calibrate();
		xy_str.Format("Calibration Complete!");
		SetDlgItemText(IDC_STATIC_xy, xy_str);
	}
	else {
		Colour_Verify();
		xy_str.Format("Verification Complete!");
		SetDlgItemText(IDC_STATIC_xy, xy_str);
	}
	((CStatic*)GetDlgItem(IDC_STATIC_GP_CONDITION))->EnableWindow(1);
	inited = false;
}

void CCa200SampleDlg::OnBnClickedButtonGray()
{
	xy_str.Format("Measuring Grayscale...");
	SetDlgItemText(IDC_STATIC_xy, xy_str);
	Check_State();
	Init_Data_Vector();
	send_index.clear();

	CString color_m_filename;
	CString str;
	str.Format(".\\data\\RGB_Gray_LUT.csv");
	Load_Send_Index(str);
	
	Init_Data_Vector();
	Send_Get(send_index, nRet, LinkNo, data, false);
	
	color_m_filename.Format(".\\data\\Gray.csv");
	Write_Measure_Data(color_m_filename, data);
	xy_str.Format("Measure Complete!");
	SetDlgItemText(IDC_STATIC_xy, xy_str);
}

void CCa200SampleDlg::OnBnClickedButtonColor()
{
	xy_str.Format("Measuring Color...");
	SetDlgItemText(IDC_STATIC_xy, xy_str);
	Check_State();
	Init_Data_Vector();
	send_index.clear();

	CString color_m_filename;
	CString str;
	str.Format(".\\data\\RGB_Color_LUT.csv");
	Load_Send_Index(str);
	
	Init_Data_Vector();
	Send_Get(send_index, nRet, LinkNo, data, false);
	color_m_filename.Format(".\\data\\Color.csv");
	Write_Measure_Data(color_m_filename, data);
	xy_str.Format("Measure Complete!");
	SetDlgItemText(IDC_STATIC_xy, xy_str);
}

void CCa200SampleDlg::OnBnClickedButtonOwn()
{
	xy_str.Format("Measuring Own Color...");
	SetDlgItemText(IDC_STATIC_xy, xy_str);
	Check_State();
	Init_Data_Vector();
	send_index.clear();

	CString color_m_filename;
	CString str;
	str.Format(".\\data\\RGB_Own_LUT.csv");
	Load_Send_Index(str);

	Init_Data_Vector();
	Send_Get(send_index, nRet, LinkNo, data, true);
	color_m_filename.Format(".\\data\\Own.csv");
	Write_Measure_Data(color_m_filename, data);
	xy_str.Format("Measure Complete!");
	SetDlgItemText(IDC_STATIC_xy, xy_str);
}


#pragma region set calibrate parameter

void CCa200SampleDlg::Compare_THREE_D_LUT_PIP() {
	CString cstr_matrix;
	cstr_matrix.Format(".\\data\\matrix\\matrix_%s_%s.csv", "sRGB", "6500");
	MATRIX cct_gamut_matrix = Load_Matrix(cstr_matrix);
	vector<vector <vector<RGBColor>>> three_d_lut;
	//CA->Build_Three_D_LUT(three_d_lut, dicom_lv_nor_g, cct_gamut_matrix, "2.2", 17);

	RGBColor RGB_raw;
	RGBColor RGB_new;
	RGBColor result;

	vector<vector<double>> d;
	vector<double>dd;
	uint16_t r, g, b;
	for (int k = 0; k < 1024; k +=64) {
		r = max(0, k - 1);
		g = max(0, k - 1);
		b = max(0, k - 1);
		RGB_raw = { r, g, b };
		Virtual_Video_Pipeline_Proccess(RGB_raw, RGB_new, "sRGB", "6500", "2.2");

		CA->trilinearInterpolation(three_d_lut, r, g, b, result);

		dd = { double(RGB_new.Red), double(RGB_new.Green), double(RGB_new.Blue), double(result.Red), double(result.Green), double(result.Blue) };
		d.push_back(dd);
	}
	CString filename;
	filename.Format(".\\compare.csv");
	Write_Measure_Data(filename, d);
}
void CCa200SampleDlg::OnBnClickedButtonPattern()
{
	CA->Configure_one_d_calibration_attribute(10, 1024);
	CString cstr_matrix;
	cstr_matrix.Format(".\\data\\matrix\\matrix_%s_%s.csv", "sRGB", "6500");
	MATRIX cct_gamut_matrix = Load_Matrix(cstr_matrix);
	vector<vector <vector<RGBColor>>> three_d_lut;

	//CA->Build_Three_D_LUT(three_d_lut, dicom_lv_nor_g, cct_gamut_matrix, "2.2", 17);
	//Export_Data_Image();
	Check_State();
	Init_Data_Vector();
	
	CString sR;
	CString sG;
	CString sB;
	GetDlgItemText(IDC_EDIT_R_value, sR);
	GetDlgItemText(IDC_EDIT_G_value, sG);
	GetDlgItemText(IDC_EDIT_B_value, sB);

	RGBColor result;
	//CA->trilinearInterpolation(three_d_lut, _ttoi(sR), _ttoi(sG), _ttoi(sB), result);


	send_index.clear();
	send_index.push_back({ _ttoi(sR), _ttoi(sG), _ttoi(sB) });
	//send_index.push_back({ result.Red,result.Green, result.Blue });
	
	Init_Data_Vector();
	Send_Get(send_index, nRet, LinkNo, data, false);
	xy_str.Format("Pattern:%d, %d, %d", _ttoi(sR), _ttoi(sG), _ttoi(sB));
	SetDlgItemText(IDC_STATIC_xy, xy_str);
}


void CCa200SampleDlg::OnBnClickedButtonAddlistNative()
{
	Check_Selct(native_cct_cb, native_trc_cb, native_select, "Native", true);
}

void CCa200SampleDlg::OnBnClickedButtonAddlistSrgb()
{
	Check_Selct(srgb_cct_cb, srgb_trc_cb, srgb_select, "sRGB", true);
}

void CCa200SampleDlg::OnBnClickedButtonAddlistAdobe()
{
	Check_Selct(adobe_cct_cb, adobe_trc_cb, adobe_select, "ADOBE", true);
}

void CCa200SampleDlg::OnBnClickedButtonAddlistBt2020()
{
	Check_Selct(bt2020_cct_cb, bt2020_trc_cb, bt2020_select, "BT2020", true);
}

void CCa200SampleDlg::OnBnClickedButtonDeletelistNative()
{
	Check_Selct(native_cct_cb, native_trc_cb, native_select, "NATIVE", false);
}


void CCa200SampleDlg::OnBnClickedButtonDeletelistSrgb()
{
	Check_Selct(srgb_cct_cb, srgb_trc_cb, srgb_select, "SRGB", false);
}


void CCa200SampleDlg::OnBnClickedButtonDeletelistAdobe()
{
	Check_Selct(adobe_cct_cb, adobe_trc_cb, adobe_select, "ADOBE", false);
}


void CCa200SampleDlg::OnBnClickedButtonDeletelistBt2020()
{
	Check_Selct(bt2020_cct_cb, bt2020_trc_cb, bt2020_select, "BT2020", false);
}

void CCa200SampleDlg::OnCbnSelchangeComboPipGamut()
{

}

void CCa200SampleDlg::OnCbnSelchangeComboPipCct()
{

}

void CCa200SampleDlg::OnCbnSelchangeComboPipTrc() 
{
	CString mode;
	((CComboBox*)GetDlgItem(IDC_COMBO_PIP_TRC))->GetLBText(((CComboBox*)GetDlgItem(IDC_COMBO_PIP_TRC))->GetCurSel(), mode);
	if (mode == "DICOM") {
		((CComboBox*)GetDlgItem(IDC_COMBO_PIP_GAMUT))->SetCurSel(0);
		((CComboBox*)GetDlgItem(IDC_COMBO_PIP_GAMUT))->EnableWindow(0);
		((CComboBox*)GetDlgItem(IDC_COMBO_PIP_CCT))->SetCurSel(5);
		((CComboBox*)GetDlgItem(IDC_COMBO_PIP_CCT))->EnableWindow(0);
	}
	else {
		((CComboBox*)GetDlgItem(IDC_COMBO_PIP_GAMUT))->EnableWindow(1);
		((CComboBox*)GetDlgItem(IDC_COMBO_PIP_CCT))->EnableWindow(1);
	}
}

void CCa200SampleDlg::OnBnClickedButtonSetPipeline()
{
	CString encode_lut;
	CString matrix;
	CString decode_lut;
	CString base_lut;
	CString cstr_Error;

	try {
		((CComboBox*)GetDlgItem(IDC_COMBO_PIP_GAMUT))->GetLBText(((CComboBox*)GetDlgItem(IDC_COMBO_PIP_GAMUT))->GetCurSel(), gamut_target);
	}
	catch (exception) {
		cstr_Error.Format("Invalid GAMUT!");
		AfxMessageBox(cstr_Error);
		return;
	}
	try {
		((CComboBox*)GetDlgItem(IDC_COMBO_PIP_CCT))->GetLBText(((CComboBox*)GetDlgItem(IDC_COMBO_PIP_CCT))->GetCurSel(), cct_target);
	}
	catch (exception) {
		cstr_Error.Format("Invalid CCT!");
		AfxMessageBox(cstr_Error);
		return;
	}
	try {
		((CComboBox*)GetDlgItem(IDC_COMBO_PIP_TRC))->GetLBText(((CComboBox*)GetDlgItem(IDC_COMBO_PIP_TRC))->GetCurSel(), tone_curve_target);
	}
	catch (exception) {
		cstr_Error.Format("Invalid TRC!");
		AfxMessageBox(cstr_Error);
		return;
	}

	CString base_lut_path;
	//CString tone_lut_path;
	//CString three_d_lut_path;
	RGBColor tone_lut[RGB_depth];
	//vector<vector <vector<RGBColor>>> three_d_lut;
	//tone_lut_path.Format(".\\data\\lut\\tone_curve\\%s\\%s_%s.csv", cct_target, cct_target, tone_curve_target);

	base_lut_path.Format(".\\data\\lut\\base\\base_%s.csv", cct_target);

	Load_Panel_1D_LUT(base_lut_path, tone_lut);
	Update_MCU_LUT(tone_lut, SELECT_1D_LUT_CCT, 2, base_lut_path);

	//three_d_lut_path.Format(".\\data\\lut\\three_d_lut\\%s\\%s_%s.csv", cct_target, gamut_target, cct_target);
	//Load_Panel_3D_LUT(three_d_lut_path, three_d_lut, 17);
	//Update_MCU_3D_LUT(three_d_lut, SELECT_3D_LUT, 0, gamut_target + "_" + cct_target + "_" + tone_curve_target);

	//Measure_Raw();
	inited = false;
	xy_str.Format("Pipeline setting Complete!");
	SetDlgItemText(IDC_STATIC_xy, xy_str);

}


void CCa200SampleDlg::OnCbnSelchangeComboMode()
{
	CString mode;
	((CComboBox*)GetDlgItem(IDC_COMBO_MODE))->SetCurSel(((CComboBox*)GetDlgItem(IDC_COMBO_MODE))->GetCurSel());
	((CComboBox*)GetDlgItem(IDC_COMBO_MODE))->GetWindowText(mode);
	Generate_RGB_Pattern(LinkNo, RGB_max, RGB_max, RGB_max);
	if (mode == "Verify") {
		Verify_Btn_Enable(1);
	}
	else {
		Verify_Btn_Enable(0);
	}
}

void CCa200SampleDlg:: Check_Select_ALL(int c, vector<CButton*> button_select) {
	int n = (((CButton*)GetDlgItem(c))->GetCheck() == 0) ? 0 : 1;
	for (int i = 0; i < button_select.size(); i++) {
		button_select[i]->SetCheck(n);
	}
}

void CCa200SampleDlg::OnBnClickedCheckNativeCctAll()
{
	Check_Select_ALL(IDC_CHECK_NATIVE_CCT_ALL, native_cct_cb);
}


void CCa200SampleDlg::OnBnClickedCheckNativeTrcAll()
{
	Check_Select_ALL(IDC_CHECK_NATIVE_TRC_ALL, native_trc_cb);
}


void CCa200SampleDlg::OnBnClickedCheckSrgbCctAll()
{
	Check_Select_ALL(IDC_CHECK_SRGB_CCT_ALL, srgb_cct_cb);
}


void CCa200SampleDlg::OnBnClickedCheckSrgbTrcAll()
{
	Check_Select_ALL(IDC_CHECK_SRGB_TRC_ALL, srgb_trc_cb);
}


void CCa200SampleDlg::OnBnClickedCheckAdobeCctAll()
{
	Check_Select_ALL(IDC_CHECK_ADOBE_CCT_ALL, adobe_cct_cb);
}


void CCa200SampleDlg::OnBnClickedCheckAdobeTrcAll()
{
	Check_Select_ALL(IDC_CHECK_ADOBE_TRC_ALL, adobe_trc_cb);
}


void CCa200SampleDlg::OnBnClickedCheckBt2020CctAll()
{
	Check_Select_ALL(IDC_CHECK_BT2020_CCT_ALL, bt2020_cct_cb);
}


void CCa200SampleDlg::OnBnClickedCheckBt2020TrcAll()
{
	Check_Select_ALL(IDC_CHECK_BT2020_TRC_ALL, bt2020_trc_cb);
}
 
void CCa200SampleDlg:: Check_Verify_ALL(bool b) {
	CString cstr;
	string str;
	((CButton*)GetDlgItem(IDC_CHECK_NATIVE_CCT_ALL))->SetCheck(b);
	((CButton*)GetDlgItem(IDC_CHECK_NATIVE_TRC_ALL))->SetCheck(b);
	((CButton*)GetDlgItem(IDC_CHECK_SRGB_CCT_ALL))->SetCheck(b);
	((CButton*)GetDlgItem(IDC_CHECK_SRGB_TRC_ALL))->SetCheck(b);
	((CButton*)GetDlgItem(IDC_CHECK_ADOBE_CCT_ALL))->SetCheck(b);
	((CButton*)GetDlgItem(IDC_CHECK_ADOBE_TRC_ALL))->SetCheck(b);
	((CButton*)GetDlgItem(IDC_CHECK_BT2020_CCT_ALL))->SetCheck(b);
	((CButton*)GetDlgItem(IDC_CHECK_BT2020_TRC_ALL))->SetCheck(b);

	Check_Select_ALL(IDC_CHECK_NATIVE_CCT_ALL, native_cct_cb);
	Check_Select_ALL(IDC_CHECK_NATIVE_TRC_ALL, native_trc_cb);
	Check_Select_ALL(IDC_CHECK_SRGB_CCT_ALL, srgb_cct_cb);
	Check_Select_ALL(IDC_CHECK_SRGB_TRC_ALL, srgb_trc_cb);
	Check_Select_ALL(IDC_CHECK_ADOBE_CCT_ALL, adobe_cct_cb);
	Check_Select_ALL(IDC_CHECK_ADOBE_TRC_ALL, adobe_trc_cb);
	Check_Select_ALL(IDC_CHECK_BT2020_CCT_ALL, bt2020_cct_cb);
	Check_Select_ALL(IDC_CHECK_BT2020_TRC_ALL, bt2020_trc_cb);
	

	if (b) {
		Check_Selct(native_cct_cb, native_trc_cb, native_select, "NATIVE", true);
		Check_Selct(srgb_cct_cb, srgb_trc_cb, srgb_select, "SRGB", true);
		Check_Selct(adobe_cct_cb, adobe_trc_cb, adobe_select, "ADOBE", true);
		Check_Selct(bt2020_cct_cb, bt2020_trc_cb, bt2020_select, "BT2020", true);
	}
	else {
		Check_Selct(native_cct_cb, native_trc_cb, native_select, "NATIVE", false);
		Check_Selct(srgb_cct_cb, srgb_trc_cb, srgb_select, "SRGB", false);
		Check_Selct(adobe_cct_cb, adobe_trc_cb, adobe_select, "ADOBE", false);
		Check_Selct(bt2020_cct_cb, bt2020_trc_cb, bt2020_select, "BT2020", false);
		Init_Hash_Select(native_select);
		Init_Hash_Select(srgb_select);
		Init_Hash_Select(adobe_select);
		Init_Hash_Select(bt2020_select);
	}
}


void CCa200SampleDlg::OnBnClickedButtonVerifyAll()
{
	Check_Verify_ALL(true);
}


void CCa200SampleDlg::OnBnClickedButtonVerifyNone()
{
	Check_Verify_ALL(false);
	((CListBox*)GetDlgItem(IDC_LIST_VF))->ResetContent();
}


void CCa200SampleDlg::OnBnClickedButtonInit()
{
	Check_State(); //check link state
	
	Init_Panel_Matrix_1D_LUT_GAMMA();

	native_cct_cb = { (CButton*)GetDlgItem(IDC_CHECK_NATIVE_3500), (CButton*)GetDlgItem(IDC_CHECK_NATIVE_4000),
					  (CButton*)GetDlgItem(IDC_CHECK_NATIVE_4500), (CButton*)GetDlgItem(IDC_CHECK_NATIVE_5000),
					  (CButton*)GetDlgItem(IDC_CHECK_NATIVE_5800), (CButton*)GetDlgItem(IDC_CHECK_NATIVE_6500),
					  (CButton*)GetDlgItem(IDC_CHECK_NATIVE_7000), (CButton*)GetDlgItem(IDC_CHECK_NATIVE_7500), (CButton*)GetDlgItem(IDC_CHECK_NATIVE_9300) };
	native_trc_cb = { (CButton*)GetDlgItem(IDC_CHECK_NATIVE_18), (CButton*)GetDlgItem(IDC_CHECK_NATIVE_20),
					  (CButton*)GetDlgItem(IDC_CHECK_NATIVE_22), (CButton*)GetDlgItem(IDC_CHECK_NATIVE_23),
					  (CButton*)GetDlgItem(IDC_CHECK_NATIVE_24), (CButton*)GetDlgItem(IDC_CHECK_NATIVE_26),
					  (CButton*)GetDlgItem(IDC_CHECK_NATIVE_PQ), (CButton*)GetDlgItem(IDC_CHECK_NATIVE_HLG), (CButton*)GetDlgItem(IDC_CHECK_NATIVE_DICOMGSDF) };

	srgb_cct_cb = { (CButton*)GetDlgItem(IDC_CHECK_SRGB_3500), (CButton*)GetDlgItem(IDC_CHECK_SRGB_4000),
					(CButton*)GetDlgItem(IDC_CHECK_SRGB_4500), (CButton*)GetDlgItem(IDC_CHECK_SRGB_5000),
					(CButton*)GetDlgItem(IDC_CHECK_SRGB_5800), (CButton*)GetDlgItem(IDC_CHECK_SRGB_6500),
					(CButton*)GetDlgItem(IDC_CHECK_SRGB_7000), (CButton*)GetDlgItem(IDC_CHECK_SRGB_7500), (CButton*)GetDlgItem(IDC_CHECK_SRGB_9300) };
	srgb_trc_cb = { (CButton*)GetDlgItem(IDC_CHECK_SRGB_18), (CButton*)GetDlgItem(IDC_CHECK_SRGB_20),
					(CButton*)GetDlgItem(IDC_CHECK_SRGB_22), (CButton*)GetDlgItem(IDC_CHECK_SRGB_23),
					(CButton*)GetDlgItem(IDC_CHECK_SRGB_24), (CButton*)GetDlgItem(IDC_CHECK_SRGB_26),
					(CButton*)GetDlgItem(IDC_CHECK_SRGB_PQ), (CButton*)GetDlgItem(IDC_CHECK_SRGB_HLG), (CButton*)GetDlgItem(IDC_CHECK_SRGB_DICOMGSDF) };

	adobe_cct_cb = { (CButton*)GetDlgItem(IDC_CHECK_ADOBE_3500), (CButton*)GetDlgItem(IDC_CHECK_ADOBE_4000),
					(CButton*)GetDlgItem(IDC_CHECK_ADOBE_4500), (CButton*)GetDlgItem(IDC_CHECK_ADOBE_5000),
					(CButton*)GetDlgItem(IDC_CHECK_ADOBE_5800), (CButton*)GetDlgItem(IDC_CHECK_ADOBE_6500),
					(CButton*)GetDlgItem(IDC_CHECK_ADOBE_7000), (CButton*)GetDlgItem(IDC_CHECK_ADOBE_7500), (CButton*)GetDlgItem(IDC_CHECK_ADOBE_9300) };
	adobe_trc_cb = { (CButton*)GetDlgItem(IDC_CHECK_ADOBE_18), (CButton*)GetDlgItem(IDC_CHECK_ADOBE_20),
					(CButton*)GetDlgItem(IDC_CHECK_ADOBE_22), (CButton*)GetDlgItem(IDC_CHECK_ADOBE_23),
					(CButton*)GetDlgItem(IDC_CHECK_ADOBE_24), (CButton*)GetDlgItem(IDC_CHECK_ADOBE_26),
					(CButton*)GetDlgItem(IDC_CHECK_ADOBE_PQ), (CButton*)GetDlgItem(IDC_CHECK_ADOBE_HLG), (CButton*)GetDlgItem(IDC_CHECK_ADOBE_DICOMGSDF) };

	bt2020_cct_cb = { (CButton*)GetDlgItem(IDC_CHECK_BT2020_3500), (CButton*)GetDlgItem(IDC_CHECK_BT2020_4000),
					  (CButton*)GetDlgItem(IDC_CHECK_BT2020_4500), (CButton*)GetDlgItem(IDC_CHECK_BT2020_5000),
					  (CButton*)GetDlgItem(IDC_CHECK_BT2020_5800), (CButton*)GetDlgItem(IDC_CHECK_BT2020_6500),
					  (CButton*)GetDlgItem(IDC_CHECK_BT2020_7000), (CButton*)GetDlgItem(IDC_CHECK_BT2020_7500), (CButton*)GetDlgItem(IDC_CHECK_BT2020_9300) };
	bt2020_trc_cb = { (CButton*)GetDlgItem(IDC_CHECK_BT2020_18), (CButton*)GetDlgItem(IDC_CHECK_BT2020_20),
					  (CButton*)GetDlgItem(IDC_CHECK_BT2020_22), (CButton*)GetDlgItem(IDC_CHECK_BT2020_23),
					  (CButton*)GetDlgItem(IDC_CHECK_BT2020_24), (CButton*)GetDlgItem(IDC_CHECK_BT2020_26),
					  (CButton*)GetDlgItem(IDC_CHECK_BT2020_PQ), (CButton*)GetDlgItem(IDC_CHECK_BT2020_HLG), (CButton*)GetDlgItem(IDC_CHECK_BT2020_DICOMGSDF) };

	(CComboBox*)GetDlgItem(IDC_COMBO_MODE)->EnableWindow(1);
	((CComboBox*)GetDlgItem(IDC_COMBO_MODE))->SetCurSel(0);
	(CButton*)GetDlgItem(IDC_BUTTON_Calibrate)->EnableWindow(1);
	(CComboBox*)GetDlgItem(IDC_COMBO_PIP_GAMUT)->EnableWindow(1);
	(CComboBox*)GetDlgItem(IDC_COMBO_PIP_CCT)->EnableWindow(1);
	(CComboBox*)GetDlgItem(IDC_COMBO_PIP_TRC)->EnableWindow(1);
	(CButton*)GetDlgItem(IDC_BUTTON_SET_PIPELINE)->EnableWindow(1);
	(CButton*)GetDlgItem(IDC_BUTTON_Pattern)->EnableWindow(1);
	(CButton*)GetDlgItem(IDC_BUTTON_TRC)->EnableWindow(1);
	(CButton*)GetDlgItem(IDC_BUTTON_Gray)->EnableWindow(1);
	(CButton*)GetDlgItem(IDC_BUTTON_Color)->EnableWindow(1);
	(CButton*)GetDlgItem(IDC_BUTTON_OWN)->EnableWindow(1);

	xy_str.Format("Initial Complete!");
	SetDlgItemText(IDC_STATIC_xy, xy_str);
	
}

void CCa200SampleDlg::OnBnClickedCheckImage()
{
	Check_State();

	int mode;
	mode = ((CButton*)GetDlgItem(IDC_CHECK_Image))->GetCheck();
	
	if (mode) {
		Generate_RGB_Pattern(LinkNo, 0, 0, 0, ACTION_TX_TEST_PATTERN_INSERTION);
	}
	else {
		Generate_RGB_Pattern(LinkNo, RGB_max, RGB_max, RGB_max);
	}
}
