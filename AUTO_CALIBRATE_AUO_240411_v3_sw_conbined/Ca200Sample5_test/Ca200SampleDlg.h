#include "resource.h"
#include "calibrate.h"
#include "scaler_serial.h"
#include "scaler_internal.h"
#include <unordered_map>
#include <fstream>
#include <sstream>

#ifndef _3DLUT_array_size
#define _3DLUT_array_size    4913    /* 17x17x17 cube */
#endif

#if !defined(AFX_CA200SAMPLEDLG_H__85F30F2C_42D9_48BB_8323_005EF636DB07__INCLUDED_)
#define AFX_CA200SAMPLEDLG_H__85F30F2C_42D9_48BB_8323_005EF636DB07__INCLUDED_

// CA-SDK
#import "C:\Program Files (x86)\KONICAMINOLTA\CA-SDK\SDK\CA200Srvr.dll" no_namespace no_implementation 

#if _MSC_VER > 1000
#pragma on
#endif // _MSC_VER > 1000

#define MASK_SINGLE_COLOR   0x3FF /* 10-bits */
#define MASK_INVALID_SINGLE_COLOR_RANGE  0xFFFFFC00 /* (~MASK_SINGLE_COLOR) */
#define BITS_SHIFT_4_RED    0
#define BITS_SHIFT_4_GREEN  10
#define BITS_SHIFT_4_BLUE   20




class CCa200SampleDlgAutoProxy;

/////////////////////////////////////////////////////////////////////////////
// CCa200SampleDlg Dialog 

const uint32_t RGB_depth = 1024;
const float RGB_max = 1023.0;

class CCa200SampleDlg : public CDialog
{
	DECLARE_DYNAMIC(CCa200SampleDlg);
	friend class CCa200SampleDlgAutoProxy;

// Construction 
public:
	void Virtual_Video_Pipeline_Proccess(RGBColor RGB_raw, RGBColor& RGB_new, CString gamut, CString cct, CString tone_curve);
	void Send_Get(const vector<vector<int>> send_index, int16_t nRet, const int32_t LinkNo, vector<vector<double> >& data, bool virtual_video_pip);
	void Set_FPGA_Video_Pipeline(CString gamut, CString cct, CString tone_curve);
	IConnectionPointPtr m_pIConnectionPointObj;
	DWORD m_dwCk;
	IDispatch* m_pIDispatch;
	IOutputProbesPtr m_pOutputProbesObj;
	IProbesPtr m_pProbesObj;
	ICasPtr m_pCasObj;
	IMemoryPtr m_pMemoryObj;
	IProbePtr m_pProbeObj;
    ICaPtr m_pCaObj;
	ICa200Ptr m_pCa200Obj;
	CCa200SampleDlg(CWnd* pParent = NULL);	
	CCa200SampleDlg(int i);
	virtual ~CCa200SampleDlg();

// Dialog Data
	//{{AFX_DATA(CCa200SampleDlg)
	enum { IDD = IDD_CA200SAMPLE_DIALOG };
	CString	m_strLv;
	CString	m_strT;
	CString	m_strx;
	CString	m_stry;
	CString	m_strduv;
	CString	m_str232;
	CString	m_raw_trc_r;
	CString	m_raw_trc_g;
	CString	m_raw_trc_b;
	CString	m_raw_trc_w;
	CString	m_raw_cct;
	CString	m_raw_Lv;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CCa200SampleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV Support
	//}}AFX_VIRTUAL


protected:

	CCa200SampleDlgAutoProxy* m_pAutoProxy;
	HICON m_hIcon; 
	BOOL CanExit();
	int32_t LinkNo = -1;
	int16_t nRet;
	LUTable1D m_LUTArray[RGB_depth];
	vector<vector<int>> send_index;
	CProgressCtrl m_pcLUTUpdate;
	uint32_t m_dwFPGALUTSelect;
	bool m_bLUTReadbackChecking = true;
	bool cal_zero = false; //for 410
	//bool cal_zero = true; // for 310

	unordered_map <string, string > video_pip_state;
	vector<CString> gamut_list{ "Native", "sRGB", "ADOBE", "BT2020" };
	vector<CString> cct_list{ "3500", "4000", "4500", "5000", "5800", "6500", "7000", "7500", "9300" };
	vector<CString> trc_list{ "1.8", "2.0", "2.2", "2.35", "2.4", "2.6", "PQ", "HLG", "DICOMGSDF" };

	CString gamut_target;
	CString cct_target;
	CString tone_curve_target;

	TRC panel_tone_curve;
	vector<vector <vector<PRGBColor>>> three_lut;
	uint32_t m_3DLUTArray[_3DLUT_array_size];

	RGBColor mcu_temp_LUT[RGB_depth]; //1D LUT for MCU upadate
	RGBColor panel_1d_LUT[RGB_depth]; //1D LUT in video pipeline
	RGBColor panel_gamma_dicom_LUT[RGB_depth]; //gamma/dicom LUT in video pipeline
	MATRIX panel_gamut_cct_matrix; //gamut and cct transform matrix in video pipeline
	
	vector<double> XYZ_R;
	vector<double> XYZ_G;
	vector<double> XYZ_B;
	vector<double> XYZ_W;
	vector<double> XYZ_0;
	vector< vector<double>> data; //Àx¦sÅª¨ìªºRGBXYZ

	bool inited = false;
	void CA_CAL_ZERO();
	void Check_State();
	void Init_Panel_Matrix_1D_LUT_GAMMA();
	void Init_Data_Vector();

	MATRIX Load_CSV(string filename);
	MATRIX Load_Matrix(CString filename);
	MATRIX Load_LUT_RGB(CString filename);
	void Load_Panel_1D_LUT(CString filename, PRGBColor LUT);
	void Load_Panel_3D_LUT(CString filename, vector<vector<vector<RGBColor>>>& three_d_lut, int lut_size);
	void Load_Send_Index(CString filename);

	void _Loas_All_File(char* lpPath, vector<CString>& fileList);
	void _Load_ALL_Matrix(CString file_directory, vector< MATRIX>& Matrix_List);
	void _Load_GamutMatrix_File(uint32_t* m_PageBuffer, const CString file_directory, int& buf_index);
	void _Load_LUT_File(uint32_t* m_PageBuffer, int& buf_index);
	void Export_Data_Image();

	unordered_map <string, bool> native_select;
	unordered_map <string, bool> srgb_select;
	unordered_map <string, bool> adobe_select;
	unordered_map <string, bool> bt2020_select;
	unordered_map <string, bool> dicom_select;
	vector<unordered_map <string, bool>*> select_list{ &native_select, &srgb_select, &adobe_select, &bt2020_select };
	vector<CButton*> native_cct_cb;
	vector<CButton*> native_trc_cb;
	vector<CButton*> srgb_cct_cb;
	vector<CButton*> srgb_trc_cb;
	vector<CButton*> adobe_cct_cb;
	vector<CButton*> adobe_trc_cb;
	vector<CButton*> bt2020_cct_cb;
	vector<CButton*> bt2020_trc_cb;

	void Colour_Verify();
	void Verify_Color(CString gamut, CString cct, CString trc);
	void Init_Hash_Select(unordered_map<string, bool>& select);
	void Verify_Btn_Enable(bool b);
	void Check_Selct(vector<CButton*> button_cct_select, vector<CButton*> button_trc_select, unordered_map<string, bool>& select, CString g, bool mode);
	void Check_Select_ALL(int c, vector<CButton*> button_select);
	void Check_Verify_ALL(bool b);
	void Display_Verifiy_List();

	void Colour_Calibrate();
	void panel_CCT_Calibration(PRGBColor result_LUT, const int cct_target, vector<double>& Lv, vector<MATRIX> panel_RGBW_maatrix);
	vector<double> Measure_MaxLv(RGBColor RGB);
	void Measure_RGBW();
	void Measure_Raw();

	void PipelineLutCtlRefresh();

	void Update_MCU_Matrix(MATRIX matrix, CString tag);
	void _colorMatrix_double_2_uint16(MATRIX matrix);
	void _colorMatrix_uint16_2_FPGA_Register();
	void UpdateMatrixRegisters();

	void Update_MCU_LUT(const PRGBColor LUT, const uint32_t _m_dwFPGALUTSelect, const uint32_t pip_position, const CString tag);
	void UpdateLutToMcu_FPGA(const uint32_t m_dwFPGALUTSelect);
	void XferLutToMcu();
	void Update_MCU_3D_LUT(const vector<vector <vector<RGBColor>>>& three_d_lut, const uint32_t _m_dwFPGALUTSelect, const uint32_t pip_position, const CString tag);
	void Update3DLutToMcu_FPGA(const vector<uint32_t> m_3DLUT_Buffer, const uint32_t  _m_dwFPGALUTSelect);
	void XferLut3DLutToMcu(const vector<uint32_t> m_3DLUT_Buffer);
	void UpdateLutBufferToFPGA(const uint32_t m_dwFPGALUTSelect);


	//{{AFX_MSG(CCa200SampleDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnButtonCal0();
	//}}AFX_MSG

	UINT32 m_PageBuffer[9216];

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonColor();
	afx_msg void OnBnClickedButtonTrc();
	afx_msg void OnBnClickedButtonCalibrate();
	afx_msg void OnBnClickedButtonGray();
	CString xy_str;
	uint16_t m_uint16ColorMatrix[3][3];
	uint32_t m_Reg_01E0;
	uint32_t m_Reg_01E4;
	uint32_t m_Reg_01E8;
	uint32_t m_Reg_01EC;
	uint32_t m_Reg_01F0;
	uint32_t m_Reg_01F4;
	void Compare_THREE_D_LUT_PIP();
	afx_msg void OnBnClickedButtonPattern();
	afx_msg void OnBnClickedButtonAddlistNative();
	afx_msg void OnBnClickedButtonAddlistSrgb();
	afx_msg void OnBnClickedButtonAddlistAdobe();
	afx_msg void OnBnClickedButtonAddlistBt2020();
	CListBox m_list_vf;
	afx_msg void OnBnClickedButtonSetPipeline();
	afx_msg void OnCbnSelchangeComboMode();
	afx_msg void OnBnClickedCheckNativeCctAll();
	afx_msg void OnBnClickedCheckNativeTrcAll();
	afx_msg void OnBnClickedCheckSrgbCctAll();
	afx_msg void OnBnClickedCheckSrgbTrcAll();
	afx_msg void OnBnClickedCheckAdobeCctAll();
	afx_msg void OnBnClickedCheckAdobeTrcAll();
	afx_msg void OnBnClickedCheckBt2020CctAll();
	afx_msg void OnBnClickedCheckBt2020TrcAll();
	afx_msg void OnBnClickedButtonVerifyAll();
	afx_msg void OnBnClickedButtonVerifyNone();
	afx_msg void OnBnClickedButtonDeletelistNative();
	afx_msg void OnBnClickedButtonDeletelistSrgb();
	afx_msg void OnBnClickedButtonDeletelistAdobe();
	afx_msg void OnBnClickedButtonDeletelistBt2020();
	afx_msg void OnBnClickedButtonInit();
	afx_msg void OnBnClickedButtonOwn();
	afx_msg void OnBnClickedCheckImage();
	afx_msg void OnCbnSelchangeComboPipTrc();
	afx_msg void OnCbnSelchangeComboPipGamut();
	afx_msg void OnCbnSelchangeComboPipCct();
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CA200SAMPLEDLG_H__85F30F2C_42D9_48BB_8323_005EF636DB07__INCLUDED_)

//Copyright (c) 2002-2013 KONICA MINOLTA, INC. 
