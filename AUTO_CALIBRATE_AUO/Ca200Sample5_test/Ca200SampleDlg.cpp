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
#include "Calibrate.h"

// CA-SDK
#include "Const.h"
#include "CaEvent.h"
#import "C:\Program Files (x86)\KONICAMINOLTA\CA-SDK\SDK\CA200Srvr.dll" no_namespace implementation_only  

#include "scaler_serial.h"
#include "scaler_internal.h"

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

extern MATRIX matrix_raw;

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
void  Write_Measure_Data(CString filename, vector<vector<double>>data) {
	CFile fout;
	fout.Open(filename, CFile::modeCreate | CFile::modeWrite);

	fout.Write("R,G,B,X,Y,Z,Lv,sx,sy,sz,lT,duv\n", 31);//

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

void Write_LUT(CString filename, RGBColor lut[]) {
	CFile fout;
	fout.Open(filename, CFile::modeCreate | CFile::modeWrite);

	fout.Write("i,R,G,B\n", 8);//

	for (int i = 0; i < 1024; i++) {

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


void CCa200SampleDlg::Load_Panel_1D_LUT(CString filename) {
	MATRIX m; 
	RGBColor RGB;
	string s;
	s = filename;
	m = Load_CSV(s);
	for (int i = 1; i < m.size(); i++) {
		RGB = { uint16_t(m[i][1]), uint16_t(m[i][2]), uint16_t(m[i][3]) };
		virtual_LUT[i-1] = RGB;
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

		/*
		nRet = Initial_TX_Control(LinkNo, TX_DISPLAY_MAIN_SCREEN_ENABLE);
		if (nRet != ERROR_SUCCESS)
		{
			CString strError;
			strError.Format("Initial_TX_Control() failed, error code=%d", nRet);
			AfxMessageBox(strError);
		}
		*/

		nRet = Adjust_BackLight_PWM(LinkNo, 100, 1);
		if (nRet != ERROR_SUCCESS)
		{
			CString strError;
			strError.Format("Adjust_BackLight_PWM() failed, error code=%d", nRet);
			AfxMessageBox(strError);
		}
	}
}

void CCa200SampleDlg::Init_Data_and_LUT() {
	vector<double> row;
	row.assign(12, 0);//配置一個 row 的大小  R,G,B,X,Y,Z,Lv,sx,sy,sz,lT,duv
	data.assign(0, row);//配置2維

	for (uint16_t i = 0; i < LUT_array_size; i++) {
 		virtual_LUT[i] = RGBColor{ i,i,i };
	}
}

void CCa200SampleDlg::Send_Get(const vector<vector<int>> send_index, const PRGBColor send_LUT, int16_t nRet, const int32_t LinkNo, vector<vector<double> >& data) {
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

	data.clear();

	for (int i = 0; i < send_index.size(); i++) {
		R = send_LUT[send_index[i][0]].Red;
		G = send_LUT[send_index[i][1]].Green;
		B = send_LUT[send_index[i][2]].Blue;

		m_str232.Format("Message send: %d,%d,%d", R, G, B);
		//SetDlgItemText(IDC_STATIC_232, m_str232);

		nRet = Generate_RGB_Pattern(LinkNo, R, G, B);//使用內建LUT
		Sleep(300);

		m_pCaObj->Measure(0);

		fX = m_pProbeObj->X;
		fY = m_pProbeObj->Y;
		fZ = m_pProbeObj->Z;

		fLv = m_pProbeObj->Lv;
		fx = m_pProbeObj->sx;
		fy = m_pProbeObj->sy;
		fz = 1.0 - fx - fy;
		lT = m_pProbeObj->T;
		//lT = xy_to_CCT(fx, fy); //儀器的色溫計算與現今廣泛使用的不太一樣
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
	send_index.push_back({ RGB.Red,  RGB.Green,  RGB.Blue });
	Send_Get(send_index, virtual_LUT, nRet, LinkNo, data);
	return vector<double> { data[0][3], data[0][4], data[0][5] };
}

void CCa200SampleDlg::Measure_RGBW() {
	send_index.clear();
	send_index.push_back({ 1023, 0, 0 });
	send_index.push_back({ 0, 1023, 0 });
	send_index.push_back({ 0, 0, 1023 });
	send_index.push_back({ 1023, 1023, 1023 });

	Send_Get(send_index, virtual_LUT, nRet, LinkNo, data);
	XYZ_R = { data[0][3], data[0][4],data[0][5] };
	XYZ_G = { data[1][3], data[1][4],data[1][5] };
	XYZ_B = { data[2][3], data[2][4],data[2][5] };
	XYZ_W = { data[3][3], data[3][4],data[3][5] };
}

void CCa200SampleDlg::panel_CCT_Calibration(PRGBColor result_LUT, const int cct_target) {
	vector<float>xy = CA->CCT_to_xy(cct_target);
	double targetx = xy[0];
	double targety = xy[1];
	xy_str.Format("Calibrating %s, %dK, x: %.3f, y: %.3f", gamut_target, cct_target, targetx, targety);
	SetDlgItemText(IDC_STATIC_xy, xy_str);
	
	CA->Init_LUT();
	int v;
	send_index.clear();
	for (int i = 0; i <16; i ++) {
		v = (i + 1) * 64 - 1;
		send_index.push_back({ v,v,v });
	}

	CA->Set_Calibration_Target(cct_target, panel_tone_curve);

	int n_limit = 0;
	int n = 0;
	while (n_limit <= n_limit) {
		Send_Get(send_index, virtual_LUT, nRet, LinkNo, data);
		CA->CCT_Calibration(virtual_LUT, data);
		n++;
	}
	CA->Build_CCT_LUT(result_LUT);

	Init_Data_and_LUT();

	xy_str.Format("Calibrating Complete!");
	SetDlgItemText(IDC_STATIC_xy, xy_str);
}

void CCa200SampleDlg::panel_LUT_Tone_Curve_Trans(PRGBColor result_LUT, const double tone_curve_target, const int interval) {
	xy_str.Format("Tone Curve Fitting...");
	SetDlgItemText(IDC_STATIC_xy, xy_str);
	vector<int> trc_correction_index;

	send_index.clear();
	for (int i = 0; i <= 1024; i += interval) {
		i = (i == 1024) ? 1023 : i;
		send_index.push_back({ i, i, i });
	}
	Send_Get(send_index, virtual_LUT, nRet, LinkNo, data);

	vector<double> Lv;
	for (int i = 0; i < data.size(); i ++) {
		Lv.push_back(data[i][4]);
	}
	CA->LUT_Tone_Curve_Trans(result_LUT, Lv, tone_curve_target, interval);

	xy_str.Format("Tone Curve Fitting Complete!");
	SetDlgItemText(IDC_STATIC_xy, xy_str);
}

void CCa200SampleDlg::Measure_Raw() {
	xy_str.Format("Measure R、G、B、W Scale...");
	SetDlgItemText(IDC_STATIC_xy, xy_str);
	
	send_index.clear();
	//vector<vector<int>> send_index;
	for (int i = 64; i <= 1024; i += 64) {
		i = min(i, 1023);
		send_index.push_back({ i, 0, 0 });
	}
	for (int i = 64; i <= 1024; i += 64) {
		i = min(i, 1023);
		send_index.push_back({ 0, i, 0 });
	}
	for (int i = 64; i <= 1024; i += 64) {
		i = min(i, 1023);
		send_index.push_back({ 0, 0, i });
	}
	for (int i = 0; i <= 1024; i += 64) {
		i = min(i, 1023);
		send_index.push_back({ i, i, i });
	}

	Send_Get(send_index, virtual_LUT, nRet, LinkNo, data);
	vector<double> lv;
	double lv_0 = data[48][4];
	XYZ_0 = { data[48][3], data[48][4],data[48][5] };

	vector<double> tempvec;

	XYZ_R = { data[15][3], data[15][4],data[15][5] };
	lv.push_back(lv_0 / data[15][4]);
	for (int i = 0; i < 16; i++) {
		tempvec = data[i];
		lv.push_back(tempvec[4] / data[15][4]);
	}
	panel_tone_curve.r = CA->calculate_trc(lv);
	lv.clear();

	XYZ_G = { data[31][3], data[31][4],data[31][5] };
	lv.push_back(lv_0 / data[31][4]);
	for (int i = 16; i < 32; i++) {
		tempvec = data[i];
		lv.push_back(tempvec[4] / data[31][4]);
	}
	panel_tone_curve.g = CA->calculate_trc(lv);
	lv.clear();

	XYZ_B = { data[47][3], data[47][4],data[47][5] };
	lv.push_back(lv_0 / data[47][4]);
	for (int i = 32; i < 48; i++) {
		tempvec = data[i];
		lv.push_back(tempvec[4] / data[47][4]);
	}
	panel_tone_curve.b = CA->calculate_trc(lv);
	lv.clear();

	XYZ_W = { data[64][3], data[64][4],data[64][5] };
	for (int i = 48; i < 65; i++) {
		tempvec = data[i];
		lv.push_back(tempvec[4] / data[64][4]);
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
			if (j == trc_list.size() - 1) {
				str.Format("%d_dicom", cct_list[i]);
			}
			else {
				str.Format("%d_%.2f",cct_list[i], trc_list[j]);
			}
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
				if (k == trc_list.size() - 1) {
					str.Format("%d_dicom", cct_list[j]);
					str_list.Format("%s_%d_dicom",gamut_list[i],  cct_list[j]);
				}
				else {
					str.Format("%d_%.2f", cct_list[j], trc_list[k]);
					str_list.Format("%s_%d_%.2f", gamut_list[i], cct_list[j], trc_list[k]);
				}
				s = str;
				if (select[s]) {
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
					if (j == trc_list.size() - 1) {
						str.Format("%d_dicom", cct_list[i]);
					}
					else {
						str.Format("%d_%.2f", cct_list[i], trc_list[j]);
					}
					s = str;
					if(select[s] != mode)
						select[s] = mode;
				}
			}
		}
	}
	Display_Verifiy_List();
}

void CCa200SampleDlg::Update_MCU_LUT(PRGBColor mcu_LUT, PRGBColor LUT) {
	for (int i = 0; i < LUT_array_size; i++) {
		mcu_LUT[i] = LUT[i];
	}
}
void CCa200SampleDlg::Init_Panel_Matrix_1D_LUT_GAMMA() {
	xy_str.Format("Initial...");
	SetDlgItemText(IDC_STATIC_xy, xy_str);
	gamut_target = "Native";
	cct_target = 0;
	Update_MCU_Matrix(CA->matrix_raw);
	UpdateMatrixRegisters();
	CA->Generate_Gamma_LUT(panel_gamma_dicom_LUT, 1.0);
	Update_MCU_LUT(mcu_temp_LUT, panel_gamma_dicom_LUT);
	UpdateLut2Mcu_FPGA(SELECT_1D_LUT_GAMMA);
	UpdateLut2Mcu_FPGA(SELECT_1D_LUT_DEGAMMA_TEST);
	OnBnClickedButtonTrc();
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

void CCa200SampleDlg::Update_MCU_Matrix(MATRIX matrix) {
	UpdateData(TRUE);

	_colorMatrix_double_2_uint16(matrix);
	_colorMatrix_uint16_2_FPGA_Register();
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


void CCa200SampleDlg::XferLut2Mcu() {
	// TODO: Add your control notification handler code here
	CString strError;
	int16_t nError = ERROR_SCALER_SUCCESS;
	//bool bDeGammaDataValidate;

	uint16_t LUT_index;
	int r;
	for (LUT_index = 0; LUT_index < LUT_array_size; LUT_index++)
	{
		
		r = double(LUT_index) / double(LUT_array_size -1)*100.0;
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

void CCa200SampleDlg::UpdateLut2Mcu_FPGA(uint32_t _m_dwFPGALUTSelect) {
	m_dwFPGALUTSelect = _m_dwFPGALUTSelect;
	XferLut2Mcu();
	UpdateLutBuffer2FPGA();
}

void CCa200SampleDlg::UpdateLutBuffer2FPGA()
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
	case SELECT_1D_LUT_GAMMA:
	case SELECT_1D_LUT_DICOM:
		_dwMaskLUTs = ~(TX_VIDEO_PIPELINE_1D_LUT_BYPASS_MASK);
		_dwEnableLUTs = TX_VIDEO_PIPELINE_1D_LUT_ENABLE;
		break;
	case SELECT_1D_LUT_DEGAMMA:
		_dwMaskLUTs = ~(TX_VIDEO_PIPELINE_1D_LUT_DEGAMMA_BYPASS_MASK);
		_dwEnableLUTs = TX_VIDEO_PIPELINE_1D_LUT_DEGAMMA_ENABLE;
		break;
	case SELECT_1D_LUT_DEGAMMA_TEST:
		_dwMaskLUTs = ~(TX_VIDEO_PIPELINE_1D_LUT_DEGAMMA_TEST_BYPASS_MASK);
		_dwEnableLUTs = TX_VIDEO_PIPELINE_1D_LUT_DEGAMMA_TEST_ENABLE;
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

void CCa200SampleDlg::PipelineLutCtlUpdate()
{
	// TODO: Add your control notification handler code here
	int BTState;
	uint32_t _dwEnableLUTs, _dwMaskLUTs, _dwTemp;
	uint32_t _dwTxVideoPipelineCtrl;
	int16_t nError;
	CString strTemp;

	_dwEnableLUTs = 0x00;

	_dwMaskLUTs = ~(TX_VIDEO_PIPELINE_1D_LUT_DEGAMMA_BYPASS_MASK | TX_VIDEO_PIPELINE_1D_LUT_BYPASS_MASK | TX_VIDEO_PIPELINE_1D_LUT_DEGAMMA_TEST_BYPASS_MASK);

	nError = SPI_Register_Read(LinkNo, OFFSET_TX_VIDEO_CONTROL_0, &_dwTxVideoPipelineCtrl);
	if (nError != ERROR_SCALER_SUCCESS) {
		strTemp.Format(TEXT("SPI_Register_Read() failed!, errno=%d"), nError);
		AfxMessageBox(strTemp);

		return;
	}

	_dwTemp = (_dwTxVideoPipelineCtrl & _dwMaskLUTs);
	_dwTxVideoPipelineCtrl = (_dwTemp | _dwEnableLUTs);

	nError = SPI_Register_Write(LinkNo, OFFSET_TX_VIDEO_CONTROL_0, _dwTxVideoPipelineCtrl);
	if (nError != ERROR_SCALER_SUCCESS) {
		strTemp.Format(TEXT("SPI_Register_Write() failed!, errno=%d"), nError);
		AfxMessageBox(strTemp);

		return;
	}

	/* Update LUT status */
	PipelineLutCtlRefresh();

}

void CCa200SampleDlg::Verify_Color(CString gamut, int cct, double trc) {
	Check_State();
	send_index.clear();
	CString color_m_filename;
	CString str;
	CString cstr_matrix;
	
	cstr_matrix.Format(".\\data\\matrix\\%s\\matrix_%s_%d.csv", gamut, gamut, cct);
	Update_MCU_Matrix(Load_Matrix(cstr_matrix));
	UpdateMatrixRegisters();

	if (trc == 0.0) {
		str.Format(".\\data\\lut\\trc\\dicom\\lut_%s_%d_dicom.csv", gamut, cct);
	}
	else {
		str.Format(".\\data\\lut\\trc\\gamma\\lut_%.2f.csv", trc);
	}

	Load_Panel_1D_LUT(str);
	Update_MCU_LUT (mcu_temp_LUT, virtual_LUT);
	UpdateLut2Mcu_FPGA(SELECT_1D_LUT_GAMMA);

	if (trc == 0.0) {
		xy_str.Format("Measuring %s, %dK, DICOM", gamut, cct);
	}
	else {
		xy_str.Format("Measuring %s, %dK, %.2f", gamut, cct, trc);
	}
	SetDlgItemText(IDC_STATIC_xy, xy_str);

	str.Format(".\\data\\RGB_Color_LUT.csv");
	Load_Send_Index(str);
	Init_Data_and_LUT();
	Send_Get(send_index, virtual_LUT, nRet, LinkNo, data);

	if (trc == 0.0) {
		color_m_filename.Format(".\\data\\color\\%s\\%d_dicom.csv", gamut, cct);
	}
	else {
		color_m_filename.Format(".\\data\\color\\%s\\%d_%.2f.csv", gamut, cct, trc);
	}
	Write_Measure_Data(color_m_filename, data);
}

void CCa200SampleDlg:: Colour_Calibrate() {
	Check_State(); //check link state
	Init_Data_and_LUT();

	if (!inited) {
		Init_Panel_Matrix_1D_LUT_GAMMA();
	}

	CString one_d_LUT_filename;
	CString gamut_cct_matrix_filename;
	CString gamma_dicom_LUT_filename;
	CString max_Lv_filename;

	vector<double> max_Lv;
	vector<double> min_Lv;

	MATRIX matrix_native_RGB_XYZ;
	MATRIX matrix_gamut_cct;

	OnBnClickedButtonTrc();
	CFile fout;
	CString filename;
	filename.Format(".\\data\\panel_attribute\\gamma_raw.csv");
	fout.Open(filename, CFile::modeCreate | CFile::modeWrite);
	fout.Write("W,R,G,B\n", 8);//
	char str[255];
	sprintf_s(str, "%f,", panel_tone_curve.w);
	fout.Write(str, strlen(str));
	sprintf_s(str, "%f,", panel_tone_curve.r);
	fout.Write(str, strlen(str));
	sprintf_s(str, "%f,", panel_tone_curve.g);
	fout.Write(str, strlen(str));
	sprintf_s(str, "%f\n", panel_tone_curve.b);
	fout.Write(str, strlen(str));
	fout.Close();

	gamut_target = "Native";
	cct_target = 6500;

	panel_CCT_Calibration(panel_1d_LUT, cct_target);
	Update_MCU_LUT(mcu_temp_LUT, panel_1d_LUT);
	UpdateLut2Mcu_FPGA(SELECT_1D_LUT_DEGAMMA_TEST);
	one_d_LUT_filename.Format(".\\data\\lut\\1d_lut_raw.csv");
	Write_LUT(one_d_LUT_filename, panel_1d_LUT);

	trc_target = 1.0;
	panel_LUT_Tone_Curve_Trans(panel_1d_LUT, trc_target, 4);
	Update_MCU_LUT(mcu_temp_LUT, panel_1d_LUT);
	UpdateLut2Mcu_FPGA(SELECT_1D_LUT_DEGAMMA_TEST);
	one_d_LUT_filename.Format(".\\data\\lut\\1d_lut.csv");
	Write_LUT(one_d_LUT_filename, panel_1d_LUT);
	
	//OnBnClickedButtonTrc();
 
	min_Lv = Measure_MaxLv(RGBColor{ 0,0,0 });
	max_Lv_filename.Format(".\\data\\max_lv\\min_lv.csv");
	Write_Max_Lv(max_Lv_filename, min_Lv);

	xy_str.Format("Generate Gamut and CCT Transform Matrix and Gamma/DICOM LUT ...");
	SetDlgItemText(IDC_STATIC_xy, xy_str);

	Measure_RGBW();
	matrix_native_RGB_XYZ = CA->Build_Matrix_RGB_to_XYZ(XYZ_R, XYZ_G, XYZ_B, XYZ_W);

	for (int i = 0; i < gamut_list.size(); i++) {
		gamut_target = gamut_list[i];
		for (int j = 0; j < cct_list.size(); j++) {
			cct_target = cct_list[j];
			CA->Build_Matrix_Gamut_CCT_Trans(matrix_native_RGB_XYZ, matrix_gamut_cct, gamut_target, cct_target);
			Update_MCU_Matrix(matrix_gamut_cct);
			UpdateMatrixRegisters();
			gamut_cct_matrix_filename.Format(".\\data\\matrix\\%s\\matrix_%s_%d.csv", gamut_target, gamut_target, cct_target);
			Write_Matrix(gamut_cct_matrix_filename, matrix_gamut_cct);

			max_Lv = Measure_MaxLv(RGBColor{ 1023,1023,1023 });
			max_Lv_filename.Format(".\\data\\max_lv\\max_lv_%s_%d.csv", gamut_target, cct_target);
			Write_Max_Lv(max_Lv_filename, max_Lv);

			CA->Cal_DICOM_LUT(panel_gamma_dicom_LUT, min_Lv[1], max_Lv[1]);
			gamma_dicom_LUT_filename.Format(".\\data\\lut\\trc\\dicom\\lut_%s_%d_dicom.csv", gamut_target, cct_target);
			Write_LUT(gamma_dicom_LUT_filename, panel_gamma_dicom_LUT);

			/*
			if (gamut_target == "Native" && cct_target == 6500) {
				Update_MCU_LUT(mcu_temp_LUT, panel_gamma_dicom_LUT);
				UpdateLut2Mcu_FPGA(SELECT_1D_LUT_GAMMA);
				CString ii;
				ii.Format(".\\data\\RGB_Own_LUT.csv");
				Load_Send_Index(ii);
				Init_Data_and_LUT();
				Send_Get(send_index, virtual_LUT, nRet, LinkNo, data);
				CString color_m_filename;
				color_m_filename.Format(".\\data\\color\\%s_%d_dicom.csv", gamut_target, cct_target);
				Write_Measure_Data(color_m_filename, data);
				CA->Generate_Gamma_LUT(panel_gamma_dicom_LUT, 1.0);
				Update_MCU_LUT(mcu_temp_LUT, panel_gamma_dicom_LUT);
				UpdateLut2Mcu_FPGA(SELECT_1D_LUT_GAMMA);
			}

			if (gamut_target == "sRGB" && cct_target == 5000) {
				Update_MCU_LUT(mcu_temp_LUT, panel_gamma_dicom_LUT);
				UpdateLut2Mcu_FPGA(SELECT_1D_LUT_GAMMA);
				CString ii;
				ii.Format(".\\data\\RGB_Own_LUT.csv");
				Load_Send_Index(ii);
				Init_Data_and_LUT();
				Send_Get(send_index, virtual_LUT, nRet, LinkNo, data);
				CString color_m_filename;
				color_m_filename.Format(".\\data\\color\\%s_%d_dicom.csv", gamut_target, cct_target);
				Write_Measure_Data(color_m_filename, data);
				CA->Generate_Gamma_LUT(panel_gamma_dicom_LUT, 1.0);
				Update_MCU_LUT(mcu_temp_LUT, panel_gamma_dicom_LUT);
				UpdateLut2Mcu_FPGA(SELECT_1D_LUT_GAMMA);
			}

			if (gamut_target == "ADOBE" && cct_target == 9300) {
				Update_MCU_LUT(mcu_temp_LUT, panel_gamma_dicom_LUT);
				UpdateLut2Mcu_FPGA(SELECT_1D_LUT_GAMMA);
				CString ii;
				ii.Format(".\\data\\RGB_Own_LUT.csv");
				Load_Send_Index(ii);
				Init_Data_and_LUT();
				Send_Get(send_index, virtual_LUT, nRet, LinkNo, data);
				CString color_m_filename;
				color_m_filename.Format(".\\data\\color\\%s_%d_dicom.csv", gamut_target, cct_target);
				Write_Measure_Data(color_m_filename, data);
				CA->Generate_Gamma_LUT(panel_gamma_dicom_LUT, 1.0);
				Update_MCU_LUT(mcu_temp_LUT, panel_gamma_dicom_LUT);
				UpdateLut2Mcu_FPGA(SELECT_1D_LUT_GAMMA);
			}

			if (gamut_target == "BT2020" && cct_target == 3500) {
				Update_MCU_LUT(mcu_temp_LUT, panel_gamma_dicom_LUT);
				UpdateLut2Mcu_FPGA(SELECT_1D_LUT_GAMMA);
				CString ii;
				ii.Format(".\\data\\RGB_Own_LUT.csv");
				Load_Send_Index(ii);
				Init_Data_and_LUT();
				Send_Get(send_index, virtual_LUT, nRet, LinkNo, data);
				CString color_m_filename;
				color_m_filename.Format(".\\data\\color\\%s_%d_dicom.csv", gamut_target, cct_target);
				Write_Measure_Data(color_m_filename, data);
				CA->Generate_Gamma_LUT(panel_gamma_dicom_LUT, 1.0);
				Update_MCU_LUT(mcu_temp_LUT, panel_gamma_dicom_LUT);
				UpdateLut2Mcu_FPGA(SELECT_1D_LUT_GAMMA);
			}
			*/
		}
	}
	
	for (int k = 0; k < trc_list.size()-1; k++) {
		trc_target = trc_list[k];
		for (uint16_t g = 0; g < LUT_array_size; g++) {
			panel_gamma_dicom_LUT[g].Red = pow(g / 1023.0, trc_target) * 1023.0;
			panel_gamma_dicom_LUT[g].Green = pow(g / 1023.0, trc_target) * 1023.0;
			panel_gamma_dicom_LUT[g].Blue = pow(g / 1023.0, trc_target) * 1023.0;
		}
		gamma_dicom_LUT_filename.Format(".\\data\\lut\\trc\\gamma\\lut_%.2f.csv", trc_target);
		Write_LUT(gamma_dicom_LUT_filename, panel_gamma_dicom_LUT);
	}
}

void CCa200SampleDlg:: Colour_Verify() {
	unordered_map <string, bool> select;
	CString cstr;
	CString cstr_matrix;
	string str;

	if (!pipeline_set["one_d_lut"]) {
		cstr.Format(".\\data\\lut\\1d_lut.csv");
		Load_Panel_1D_LUT(cstr);
		Update_MCU_LUT(mcu_temp_LUT, virtual_LUT);
		UpdateLut2Mcu_FPGA(SELECT_1D_LUT_DEGAMMA_TEST);
	}

	for (int i = 0; i < gamut_list.size(); i++) {
		gamut_target = gamut_list[i];
		select = *select_list[i];
		for (int j = 0; j < cct_list.size(); j++) {
			cct_target = cct_list[j];
			for (int k = 0; k < trc_list.size(); k++) {
				if (k == trc_list.size() - 1) {
					cstr.Format("%d_dicom", cct_list[j]);
				}
				else {
					cstr.Format("%d_%.2f", cct_list[j], trc_list[k]);
				}
				str = cstr;
				if (select[str]) {
					if (k == trc_list.size() - 1) {
						cstr.Format("%s_%d_dicom", gamut_target, cct_list[j]);
					}
					else {
						cstr.Format("%s_%d_%.2f", gamut_target, cct_list[j], trc_list[k]);
					}
					((CListBox*)GetDlgItem(IDC_LIST_VF))->SetCurSel(((CListBox*)GetDlgItem(IDC_LIST_VF))->FindStringExact(0, cstr));
					Verify_Color(gamut_list[i], cct_list[j], trc_list[k]);
				}
			}
		}
	}
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
	timeout.ReadIntervalTimeout = 500;				// Maximum time allowed to elapse before arival of next byte in milliseconds. If the interval between the arrival of any two bytes exceeds this amount, the ReadFile operation is completed and buffered data is returned
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
	Init_Data_and_LUT();
	Measure_Raw();
}

void CCa200SampleDlg::OnBnClickedButtonCalibrate()
{
	((CStatic*)GetDlgItem(IDC_STATIC_GP_CONDITION))->EnableWindow(0);
	CString mode;
	((CComboBox*)GetDlgItem(IDC_COMBO_MODE))->GetWindowText(mode);
	if (mode == "Calibrate") {
		Colour_Calibrate();
		pipeline_set["trc_lut"] = false;
		pipeline_set["matrix"] = false;
		pipeline_set["one_d_lut"] = true;
		xy_str.Format("Calibration Complete!");
		SetDlgItemText(IDC_STATIC_xy, xy_str);
	}
	else {
		Colour_Verify();
		pipeline_set["trc_lut"] = false;
		pipeline_set["matrix"] = false;
		pipeline_set["one_d_lut"] = true;
		inited = false;
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
	Init_Data_and_LUT();
	send_index.clear();
	CString color_m_filename;
	CString str;

	str.Format(".\\data\\RGB_Gray_LUT.csv");
	Load_Send_Index(str);
	Send_Get(send_index, virtual_LUT, nRet, LinkNo, data);
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
	Init_Data_and_LUT();
	send_index.clear();
	CString color_m_filename;
	CString str;

	str.Format(".\\data\\RGB_Color_LUT.csv");
	Load_Send_Index(str);
	Send_Get(send_index, virtual_LUT, nRet, LinkNo, data);
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
	Init_Data_and_LUT();
	send_index.clear();
	CString color_m_filename;
	CString str;

	str.Format(".\\data\\RGB_Own_LUT.csv");
	Load_Send_Index(str);
	Send_Get(send_index, virtual_LUT, nRet, LinkNo, data);
	color_m_filename.Format(".\\data\\Own.csv");
	Write_Measure_Data(color_m_filename, data);
	xy_str.Format("Measure Complete!");
	SetDlgItemText(IDC_STATIC_xy, xy_str);
}


#pragma region set calibrate parameter

void CCa200SampleDlg::OnBnClickedButtonPattern()
{
	Check_State();
	Init_Data_and_LUT();
	CString sR;
	CString sG;
	CString sB;
	GetDlgItemText(IDC_EDIT_R_value, sR);
	GetDlgItemText(IDC_EDIT_G_value, sG);
	GetDlgItemText(IDC_EDIT_B_value, sB);
	send_index.clear();
	send_index.push_back({ _ttoi(sR), _ttoi(sG), _ttoi(sB) });
	Send_Get(send_index, virtual_LUT, nRet, LinkNo, data);
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
	CString cstr_Error;
	CString cstr_trc;
	try {
		((CComboBox*)GetDlgItem(IDC_COMBO_PIP_TRC))->GetLBText(((CComboBox*)GetDlgItem(IDC_COMBO_PIP_TRC))->GetCurSel(), cstr_trc);

	}
	catch (exception) {
		cstr_Error.Format("Invalid TRC!");
		AfxMessageBox(cstr_Error);
		return;
	}
	if (cstr_trc == "DICOM") {
		pipeline_set["trc_lut"] = false;
	}
	pipeline_set["matrix"] = false;
}

void CCa200SampleDlg::OnCbnSelchangeComboPipCct()
{
	CString cstr_Error;
	CString cstr_trc;
	try {
		((CComboBox*)GetDlgItem(IDC_COMBO_PIP_TRC))->GetLBText(((CComboBox*)GetDlgItem(IDC_COMBO_PIP_TRC))->GetCurSel(), cstr_trc);

	}
	catch (exception) {
		cstr_Error.Format("Invalid TRC!");
		AfxMessageBox(cstr_Error);
		return;
	}
	if (cstr_trc == "DICOM") {
		pipeline_set["trc_lut"] = false;
	}
	pipeline_set["matrix"] = false;
}

void CCa200SampleDlg::OnCbnSelchangeComboPipTrc()
{
	pipeline_set["trc_lut"] = false;
}

void CCa200SampleDlg::OnBnClickedButtonSetPipeline()
{
	CString cstr_gamut;
	CString cstr_cct;
	CString cstr_trc;
	CString cstr_matrix;
	CString cstr_lut;
	CString cstr_Error;

	try {
		((CComboBox*)GetDlgItem(IDC_COMBO_PIP_GAMUT))->GetLBText(((CComboBox*)GetDlgItem(IDC_COMBO_PIP_GAMUT))->GetCurSel(), cstr_gamut);
	}
	catch (exception) {
		cstr_Error.Format("Invalid GAMUT!");
		AfxMessageBox(cstr_Error);
		return;
	}
	try {
		((CComboBox*)GetDlgItem(IDC_COMBO_PIP_CCT))->GetLBText(((CComboBox*)GetDlgItem(IDC_COMBO_PIP_CCT))->GetCurSel(), cstr_cct);
		cct_target = atoi(cstr_cct);
	}
	catch (exception) {
		cstr_Error.Format("Invalid CCT!");
		AfxMessageBox(cstr_Error);
		return;
	}
	try {
		((CComboBox*)GetDlgItem(IDC_COMBO_PIP_TRC))->GetLBText(((CComboBox*)GetDlgItem(IDC_COMBO_PIP_TRC))->GetCurSel(), cstr_trc);

	}
	catch (exception) {
		cstr_Error.Format("Invalid TRC!");
		AfxMessageBox(cstr_Error);
		return;
	}
	if (!pipeline_set["trc_lut"]) {
		if (cstr_trc == "DICOM") {
			cstr_lut.Format(".\\data\\lut\\trc\\dicom\\lut_%s_%d_dicom.csv", cstr_gamut, atoi(cstr_cct));
		}
		else {
			cstr_lut.Format(".\\data\\lut\\trc\\gamma\\lut_%.2f.csv", atof(cstr_trc));
		}
		Load_Panel_1D_LUT(cstr_lut);
		Update_MCU_LUT(mcu_temp_LUT, virtual_LUT);
		UpdateLut2Mcu_FPGA(SELECT_1D_LUT_GAMMA);
		pipeline_set["trc_lut"] = true;
	}

	if (!pipeline_set["matrix"]) {
		cstr_matrix.Format(".\\data\\matrix\\%s\\matrix_%s_%d.csv", cstr_gamut, cstr_gamut, atoi(cstr_cct));
		Update_MCU_Matrix(Load_Matrix(cstr_matrix));
		UpdateMatrixRegisters();
		pipeline_set["matrix"] = true;
	}

	if (!pipeline_set["one_d_lut"]) {
		cstr_lut.Format(".\\data\\lut\\1d_lut.csv");
		Load_Panel_1D_LUT(cstr_lut);
		Update_MCU_LUT(mcu_temp_LUT, virtual_LUT);
		UpdateLut2Mcu_FPGA(SELECT_1D_LUT_DEGAMMA_TEST);
		pipeline_set["one_d_lut"] = true;
	}

	if (cstr_trc == "DICOM") {
		xy_str.Format("Measuring %s, %dK, DICOM", cstr_gamut, atoi(cstr_cct));
	}
	else {
		xy_str.Format("Measuring %s, %dK, %.2f", cstr_gamut, atoi(cstr_cct), atof(cstr_trc));
	}

	Init_Data_and_LUT();
	Measure_Raw();
	inited = false;
	xy_str.Format("Pipeline setting Complete!");
	SetDlgItemText(IDC_STATIC_xy, xy_str);
}

void CCa200SampleDlg::OnCbnSelchangeComboMode()
{
	CString mode;
	((CComboBox*)GetDlgItem(IDC_COMBO_MODE))->SetCurSel(((CComboBox*)GetDlgItem(IDC_COMBO_MODE))->GetCurSel());
	((CComboBox*)GetDlgItem(IDC_COMBO_MODE))->GetWindowText(mode);
	int index = LUT_array_size - 1;
	Generate_RGB_Pattern(LinkNo, virtual_LUT[index].Red, virtual_LUT[index].Green, virtual_LUT[index].Blue);
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
					  (CButton*)GetDlgItem(IDC_CHECK_NATIVE_7500), (CButton*)GetDlgItem(IDC_CHECK_NATIVE_9300) };
	native_trc_cb = { (CButton*)GetDlgItem(IDC_CHECK_NATIVE_18), (CButton*)GetDlgItem(IDC_CHECK_NATIVE_20),
					  (CButton*)GetDlgItem(IDC_CHECK_NATIVE_22), (CButton*)GetDlgItem(IDC_CHECK_NATIVE_23),
					  (CButton*)GetDlgItem(IDC_CHECK_NATIVE_24), (CButton*)GetDlgItem(IDC_CHECK_NATIVE_26),
					  (CButton*)GetDlgItem(IDC_CHECK_NATIVE_DICOM) };

	srgb_cct_cb = { (CButton*)GetDlgItem(IDC_CHECK_SRGB_3500), (CButton*)GetDlgItem(IDC_CHECK_SRGB_4000),
					(CButton*)GetDlgItem(IDC_CHECK_SRGB_4500), (CButton*)GetDlgItem(IDC_CHECK_SRGB_5000),
					(CButton*)GetDlgItem(IDC_CHECK_SRGB_5800), (CButton*)GetDlgItem(IDC_CHECK_SRGB_6500),
					(CButton*)GetDlgItem(IDC_CHECK_SRGB_7500), (CButton*)GetDlgItem(IDC_CHECK_SRGB_9300) };
	srgb_trc_cb = { (CButton*)GetDlgItem(IDC_CHECK_SRGB_18), (CButton*)GetDlgItem(IDC_CHECK_SRGB_20),
					(CButton*)GetDlgItem(IDC_CHECK_SRGB_22), (CButton*)GetDlgItem(IDC_CHECK_SRGB_23),
					(CButton*)GetDlgItem(IDC_CHECK_SRGB_24), (CButton*)GetDlgItem(IDC_CHECK_SRGB_26),
					(CButton*)GetDlgItem(IDC_CHECK_SRGB_DICOM) };

	adobe_cct_cb = { (CButton*)GetDlgItem(IDC_CHECK_ADOBE_3500), (CButton*)GetDlgItem(IDC_CHECK_ADOBE_4000),
					(CButton*)GetDlgItem(IDC_CHECK_ADOBE_4500), (CButton*)GetDlgItem(IDC_CHECK_ADOBE_5000),
					(CButton*)GetDlgItem(IDC_CHECK_ADOBE_5800), (CButton*)GetDlgItem(IDC_CHECK_ADOBE_6500),
					(CButton*)GetDlgItem(IDC_CHECK_ADOBE_7500), (CButton*)GetDlgItem(IDC_CHECK_ADOBE_9300) };
	adobe_trc_cb = { (CButton*)GetDlgItem(IDC_CHECK_ADOBE_18), (CButton*)GetDlgItem(IDC_CHECK_ADOBE_20),
					(CButton*)GetDlgItem(IDC_CHECK_ADOBE_22), (CButton*)GetDlgItem(IDC_CHECK_ADOBE_23),
					(CButton*)GetDlgItem(IDC_CHECK_ADOBE_24), (CButton*)GetDlgItem(IDC_CHECK_ADOBE_26),
					(CButton*)GetDlgItem(IDC_CHECK_ADOBE_DICOM) };

	bt2020_cct_cb = { (CButton*)GetDlgItem(IDC_CHECK_BT2020_3500), (CButton*)GetDlgItem(IDC_CHECK_BT2020_4000),
					  (CButton*)GetDlgItem(IDC_CHECK_BT2020_4500), (CButton*)GetDlgItem(IDC_CHECK_BT2020_5000),
					  (CButton*)GetDlgItem(IDC_CHECK_BT2020_5800), (CButton*)GetDlgItem(IDC_CHECK_BT2020_6500),
					  (CButton*)GetDlgItem(IDC_CHECK_BT2020_7500), (CButton*)GetDlgItem(IDC_CHECK_BT2020_9300) };
	bt2020_trc_cb = { (CButton*)GetDlgItem(IDC_CHECK_BT2020_18), (CButton*)GetDlgItem(IDC_CHECK_BT2020_20),
					  (CButton*)GetDlgItem(IDC_CHECK_BT2020_22), (CButton*)GetDlgItem(IDC_CHECK_BT2020_23),
					  (CButton*)GetDlgItem(IDC_CHECK_BT2020_24), (CButton*)GetDlgItem(IDC_CHECK_BT2020_26),
					  (CButton*)GetDlgItem(IDC_CHECK_BT2020_DICOM) };

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

	pipeline_set["trc_lut"] = false;
	pipeline_set["matrix"] = false;
	pipeline_set["one_d_lut"] = false;

	xy_str.Format("Initial Complete!");
	SetDlgItemText(IDC_STATIC_xy, xy_str);
	
}

void CCa200SampleDlg::OnBnClickedCheckImage()
{
	Check_State();
	Init_Data_and_LUT();

	int mode;
	mode = ((CButton*)GetDlgItem(IDC_CHECK_Image))->GetCheck();
	
	if (mode) {
		Generate_RGB_Pattern(LinkNo, 0, 0, 0, ACTION_TX_TEST_PATTERN_INSERTION);
	}
	else {
		int index = LUT_array_size - 1;
		Generate_RGB_Pattern(LinkNo, 1023, 1023, 1023);
	}
}

