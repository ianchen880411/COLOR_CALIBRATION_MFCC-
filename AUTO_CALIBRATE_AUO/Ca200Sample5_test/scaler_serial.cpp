
#include "stdafx.h"
#include <stdio.h>
#include <windows.h>

#include "scaler_serial.h"
#include "SerialPort.h"

#include "scaler_internal.h"
#include "Version.h"

#pragma warning(disable : 4996) 

#define DELAY_BETWEEN_CHARACTER_SENDING       0
#define INTERVAL_BETWEEN_CHARACTER_SENDING    (500.0f) /*( 500 micro-seconds )*/

#define DELAY_BETWEEN_COMMAND_SENDING        0
#define SLEEP_BETWEEN_COMMAND_SENDING        500 /* 500 milliseconds */

#define _NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_            1
#define _NEW_ERROR_CODE_4_LUT_UPDATE_                      1

#define _ENABLE_1D_LUT_IN_TX_VIDEO_CONTROL_PIPELINE_       1

#define _IGNORE_TO_INITIAL_TX_DISPLAY_CONTROL_             1 /* the MCU firmware will initial the TX_DISPLAY_CONTROL */

/*
#include <vector>
using namespace std;

vector<CSerialPort> vPorts;
*/
/*

SERIAL_API int nserial=0;

SERIAL_API int fnserial(void)
{
	return 42;
}


CSerial::CSerial()
{
	return;
}
*/

#define MCU_LINK_SESSION_MAX   5

typedef struct _MCULinkSession {
	CSerialPort *pSerial;
	CRITICAL_SECTION CriticalSection;
} MCULinkSession, *PMCULinkSession;

static uint32_t attach_count = 0;

static HINSTANCE hInst;
static MCULinkSession LinkSession[MCU_LINK_SESSION_MAX];



static int16_t us_delay(double us_delay)
{
	LARGE_INTEGER frequency;
	LARGE_INTEGER start, current;
	double interval;

	if (::QueryPerformanceFrequency(&frequency) == FALSE)
		return -1;

	if (::QueryPerformanceCounter(&start) == FALSE)
		return -2;
    
	do {
		if (::QueryPerformanceCounter(&current) == FALSE)
			return -3;

		interval = static_cast<double>(current.QuadPart - start.QuadPart) * 1000000.0 / frequency.QuadPart;

	} while (interval < us_delay);

	return 0;
}

static int16_t _internal_error_Xfer(INT nIntenalError)
{
	int16_t nError = ERROR_SCALER_SUCCESS;

	switch(nIntenalError) {
	case err_internal_serial_read_timeout:
		nError = ERROR_SCALER_SERIAL_RESPONS_TIMEOUT;
		break;

	case err_internal_serial_read_wait_fail:
		nError = ERROR_SCALER_SERIAL_WAIT_FAILED;
		break;

	case err_internal_serial_send_fail:
		nError = ERROR_SCALER_SERIAL_SEND_CMD_FAILED;
		break;

	case err_internal_invalid_comm_handle:
		nError = ERROR_SCALER_INVALID_SERIAL_HANDLE;
		break;

	case err_internal_new_rcvbuf_fail:
		nError = ERROR_SCALER_SERIAL_RCVBUF_ALLOCATE_FAILED;
		break;

	case err_internal_invalid_rcvbuf_size:
		nError = ERROR_SCALER_SERIAL_INVALID_RCVBUF_SIZE;
		break;

	default:
		nError = ERROR_SCALER_UNDEFINED_INTERNAL_ERROR;
		break;

	}
	return nError;
}


#if 0
static int16_t _MCU_LPUART_Error_Xfer(int nMCUSerialError)
{
	int16_t nError = ERROR_SCALER_SUCCESS;
#else
static int16_t _MCU_LPUART_Error_Xfer(char *strError)
{
	int16_t nError = ERROR_SCALER_SUCCESS;
	int nMCUSerialError;

	sscanf(strError, "ERROR-%d", &nMCUSerialError);
#endif

	switch (nMCUSerialError) {
	case 0:
		nError = ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
		break;

	case 1:
		nError = ERROR_MCU_LPUART_TX_BUSY;
		break;

	case 2:
		nError = ERROR_MCU_LPUART_RX_BUSY;
		break;

	case 3:
		nError = ERROR_MCU_LPUART_TX_IDLE;
		break;

	case 4:
		nError = ERROR_MCU_LPUART_RX_IDLE;
		break;

	case 5:
		nError = ERROR_MCU_LPUART_TX_WATERMARK_TOO_LARGE; 
		break;

	case 6:
		nError = ERROR_MCU_LPUART_RX_WATERMARK_TOO_LARGE;
		break;

	case 7:
		nError = ERROR_MCU_LPUART_FLAG_CONNOT_CLEAR_MANUALLY;
		break;

	case 8:
		nError = ERROR_MCU_LPUART_RX_RING_BUFFER_OVERRUN;
		break;

	case 9:
		nError = ERROR_MCU_LPUART_RX_HARDWARE_OVERRUN;
		break;

	case 10:
		nError = ERROR_MCU_LPUART_NOISE_ERROR;
		break;

	case 11:
		nError = ERROR_MCU_LPUART_FRAMING_ERROR;
		break;

	case 12:
		nError = ERROR_MCU_LPUART_PARITY_ERROR;
		break;

	case 13:
		nError = ERROR_MCU_LPUART_BAUDRATE_NOT_SUPPORT;
		break;

	case 14:
		nError = ERROR_MCU_LPUART_IDLE_LINE_DETECTED;
		break;

	case 15:
		nError = ERROR_MCU_LPUART_TIMEOUT;
		break;

	case 90:
		nError = ERROR_MCU_LUT_INVALID_SELECT_LUT;
		break;
/*_NEW_ERROR_CODE_4_LUT_UPDATE_ */
	case 91:
		nError = ERROR_MCU_LUT_PIPELINE_STATE_IS_NOT_BYPASS;
		break;
	case 92:
		nError = ERROR_MCU_LUT_RED_READBACK_NOT_IDENTICAL;
		break;
	case 93:
		nError = ERROR_MCU_LUT_GREEN_READBACK_NOT_IDENTICAL;
		break;
	case 94:
		nError = ERROR_MCU_LUT_BLUE_READBACK_NOT_IDENTICAL;
		break;
/* _NEW_ERROR_CODE_4_LUT_FLASH_SAVE_LOAD_ */
	case 95:
		nError = ERORR_MCU_LUT_FLASH_ERASE_SECTOR_FAILED;
		break;
	case 96:
		nError = ERORR_MCU_LUT_SAVE_TO_FLASH_FAILED;
		break;
	case 97:
		nError = ERROR_MCU_LUT_FLASH_READBACK_NOT_IDENTICAL;
		break;

	case 20:
	default:
		nError = ERROR_MCU_LPUART_UNKNOWN;
		break;

	}
	return nError;
}

BOOL WINAPI DllMain(
	HINSTANCE hDLL,
	DWORD     dwReason,
	LPVOID    lpReserved
)
{

	USHORT vi;
	OSVERSIONINFO VerInfo;
	DWORD VerID;
	DWORD MajorVer;
	DWORD MinorVer;

	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&VerInfo);
		VerID = VerInfo.dwPlatformId;
		MajorVer = VerInfo.dwMajorVersion;
		MinorVer = VerInfo.dwMinorVersion;

		switch (VerID) {
		case 1: //Windows Me, Windows 98, or Windows 95.
			hInst = hDLL;
			MessageBox(NULL, "Windows98 is not supported.", "Error", MB_OK | MB_ICONEXCLAMATION);
			return FALSE;

		case 2: //Windows Server "Longhorn", Windows Server 2003, Windows XP, Windows 2000, or Windows NT.
			switch (MajorVer) {
			case 4: // Windows NT 4.0, Windows Me, Windows 98, or Windows 95.
				hInst = hDLL;
				MessageBox(NULL, "WindowsNT is not supported.", "Error", MB_OK | MB_ICONEXCLAMATION);
				return FALSE;

			default:
				hInst = hDLL;
				break;
			}

			break;
		}

		attach_count++;

		if (attach_count == 1) {
			for (vi = 0; vi < MCU_LINK_SESSION_MAX; vi++) {
				LinkSession[vi].pSerial = static_cast<CSerialPort *>INVALID_HANDLE_VALUE;
				InitializeCriticalSection(&LinkSession[vi].CriticalSection);
			}
		}

		break;

	case DLL_PROCESS_DETACH:

		attach_count--;

		if (attach_count == 0) {
			//UNLOAD all drivers that are open
			for (vi = 0; vi < MCU_LINK_SESSION_MAX; vi++) {
				if (LinkSession[vi].pSerial != static_cast<CSerialPort *>INVALID_HANDLE_VALUE) {
					CloseComm( static_cast<LPVOID>(LinkSession[vi].pSerial) );
					LinkSession[vi].pSerial = static_cast<CSerialPort *>INVALID_HANDLE_VALUE;
				}
				DeleteCriticalSection(&LinkSession[vi].CriticalSection);
			}
		}

		break;
	}

	return TRUE;
}

int16_t WINAPI Get_Library_Version(uint16_t wLibVersion[4])
{
	if (wLibVersion != NULL) {
		wLibVersion[0] = _SCALER_SERIAL_DLL_VERSION_MAJOR;
		wLibVersion[1] = _SCALER_SERIAL_DLL_VERSION_MINOR;
		wLibVersion[2] = _SCALER_SERIAL_DLL_VERSION_FLAG;
		wLibVersion[3] = _SCALER_SERIAL_DLL_VERSION_BUILD;
	}

	return ERROR_SCALER_SUCCESS;
}

int16_t WINAPI Connect_to_ScalerBoard(char* pStrCOM, int32_t* pLinkNo)
{
	LPVOID hSerial;
	int32_t LinkNo = -1;
	int vi;
	char strCOMDevName[20] = "\\\\.\\COMxx";



	if ((pStrCOM == NULL) || (pLinkNo == NULL))
		return ERROR_SCALER_INVALID_PARAMETER;

	LinkNo = *pLinkNo;

	if (LinkNo < 0) {
		for (vi = 0; vi < MCU_LINK_SESSION_MAX; vi++) {
			/* serach the 1st LinkSession /w INVALID_HANDLE_VALUE */
			if (LinkSession[vi].pSerial == static_cast<CSerialPort *>INVALID_HANDLE_VALUE) {
				LinkNo = vi;
				break;
			}
		}
	}

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	if (LinkSession[LinkNo].pSerial != static_cast<CSerialPort *>INVALID_HANDLE_VALUE)
		return ERROR_SCALER_LINK_NUM_DUPLICATED;

	/* Recommeded by Mark, 2022-Jan-06*/
	sprintf(strCOMDevName, "\\\\.\\%s", pStrCOM);

	hSerial = OpenComm((LPCSTR)strCOMDevName /*(LPCSTR)pStrCOM*/, CBR_115200, NOPARITY, ONESTOPBIT, 8);
	if (hSerial == INVALID_HANDLE_VALUE)
		return ERROR_SCALER_SERIAL_OPEN_DEVICE_FAILED;

	LinkSession[vi].pSerial = static_cast<CSerialPort *>(hSerial);
	*pLinkNo = LinkNo;

	return ERROR_SCALER_SUCCESS;
}

int16_t WINAPI Disconnect_ScalerBoard(int32_t LinkNo)
{
	CSerialPort *pSerial;

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	if (LinkSession[LinkNo].pSerial == static_cast<CSerialPort *>INVALID_HANDLE_VALUE)
		return ERROR_SCALER_INVALID_SERIAL_HANDLE;

	pSerial = LinkSession[LinkNo].pSerial;
	CloseComm(static_cast<LPVOID>(pSerial));
	LinkSession[LinkNo].pSerial = static_cast<CSerialPort *>INVALID_HANDLE_VALUE;

	return ERROR_SCALER_SUCCESS;
}



#if 1
/* Combination Function*/
int16_t WINAPI Initial_TX_Control(int32_t LinkNo, uint32_t TXControl)
{
	return ERROR_SCALER_UNSUPPORTED_FUNCTION;
}

#else

int16_t WINAPI Initial_TX_Control(int32_t LinkNo, uint32_t TXControl)
{

	uint32_t dwData;

	int16_t nError = ERROR_SCALER_SUCCESS;

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	/* Get the PWM settngs to checl if eth MCU is active*/
	nError = SPI_Register_Read(LinkNo, OFFSET_V_BY_ONE_TX_CONTROL, &dwData);
	if (nError != ERROR_SCALER_SUCCESS)
		return nError;

#if 0
	nError = TX_Video_Pipeline_LUT_Reset_to_InitState(LinkNo);
	if (nError != ERROR_SCALER_SUCCESS)
		return nError;
#endif

	Sleep(10);
#if _IGNORE_TO_INITIAL_TX_DISPLAY_CONTROL_
	nError = SPI_Register_Write(LinkNo, OFFSET_TX_DISPLAY_CONTROL, TXControl);
	if (nError != ERROR_SCALER_SUCCESS)
		return nError;
#endif
	return nError; /* ERROR_SCALER_SUCCESS;*/
}

/* Combination Function*/
int16_t WINAPI Initial_TX_Control(int32_t LinkNo, uint32_t TXControl)
{

	uint32_t dwData, dwLUTCtrl;
	int LUT_reset_timeout;
	bool bLUTinitDone;

#if defined(_ENABLE_1D_LUT_IN_TX_VIDEO_CONTROL_PIPELINE_) && (_ENABLE_1D_LUT_IN_TX_VIDEO_CONTROL_PIPELINE_ == 1)
	uint32_t dwVideoCtrl;
#endif

	int16_t nError = ERROR_SCALER_SUCCESS;

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	/* Get the PWM settngs to checl if eth MCU is active*/
	nError = SPI_Register_Read(LinkNo, OFFSET_V_BY_ONE_TX_CONTROL, &dwData);
	if (nError != ERROR_SCALER_SUCCESS)
		return nError;

#if defined(_ENABLE_1D_LUT_IN_TX_VIDEO_CONTROL_PIPELINE_) && (_ENABLE_1D_LUT_IN_TX_VIDEO_CONTROL_PIPELINE_ == 1)
	/*
	* Disable TX_VIDEO_PIPELINE_1D_LUT function before reset the built-in 1D_LUT.
	*/
	nError = SPI_Register_Read(LinkNo, OFFSET_TX_VIDEO_CONTROL_0, &dwVideoCtrl);
	if (nError != ERROR_SCALER_SUCCESS)
		return nError;

	if ((dwVideoCtrl & TX_VIDEO_PIPELINE_1D_LUT_BYPASS_MASK) == TX_VIDEO_PIPELINE_1D_LUT_ENABLE)
	{
#if defined(DELAY_BETWEEN_COMMAND_SENDING) && DELAY_BETWEEN_COMMAND_SENDING
		Sleep(SLEEP_BETWEEN_COMMAND_SENDING);
#endif

		dwData = (dwVideoCtrl & ~(TX_VIDEO_PIPELINE_1D_LUT_BYPASS_MASK));
		nError = SPI_Register_Write(LinkNo, OFFSET_TX_VIDEO_CONTROL_0, dwData);
		if (nError != ERROR_SCALER_SUCCESS)
			return nError;
	}
#endif

	nError = SPI_Register_Read(LinkNo, OFFSET_VIDEO_LUT_CONTROL_0, &dwLUTCtrl);
	if (nError != ERROR_SCALER_SUCCESS)
		return nError;


#if defined(DELAY_BETWEEN_COMMAND_SENDING) && DELAY_BETWEEN_COMMAND_SENDING
	Sleep(SLEEP_BETWEEN_COMMAND_SENDING);
#endif

	dwLUTCtrl = dwLUTCtrl & ~(RESET_1D_LUT_INIT_SETTING);
	nError = SPI_Register_Write(LinkNo, OFFSET_VIDEO_LUT_CONTROL_0, dwLUTCtrl);
	if (nError != ERROR_SCALER_SUCCESS)
		return nError;

#if defined(DELAY_BETWEEN_COMMAND_SENDING) && DELAY_BETWEEN_COMMAND_SENDING
	Sleep(SLEEP_BETWEEN_COMMAND_SENDING);
#endif

	dwLUTCtrl = dwLUTCtrl | (RESET_1D_LUT_INIT_SETTING);
	nError = SPI_Register_Write(LinkNo, OFFSET_VIDEO_LUT_CONTROL_0, dwLUTCtrl);
	if (nError != ERROR_SCALER_SUCCESS)
		return nError;

	Sleep(20);

	dwLUTCtrl = dwLUTCtrl & ~(RESET_1D_LUT_INIT_SETTING);
	nError = SPI_Register_Write(LinkNo, OFFSET_VIDEO_LUT_CONTROL_0, dwLUTCtrl);
	if (nError != ERROR_SCALER_SUCCESS)
		return nError;

	LUT_reset_timeout = 3;
	bLUTinitDone = false; 
	do {
		Sleep(50);
		nError = SPI_Register_Read(LinkNo, OFFSET_VIDEO_LUT_CONTROL_0, &dwLUTCtrl);
		if (nError != ERROR_SCALER_SUCCESS)
			break;

		if ((dwLUTCtrl & RESETING_1D_LUT_INIT_MASK) == 0x00)
		{
			bLUTinitDone = true;
			break;
		}	

	} while (LUT_reset_timeout > 0);

	if (bLUTinitDone != true)
	{
		return ERROR_SCALER_LUT_RESET_TIMEOUT;
	}

#if defined(_ENABLE_1D_LUT_IN_TX_VIDEO_CONTROL_PIPELINE_) && (_ENABLE_1D_LUT_IN_TX_VIDEO_CONTROL_PIPELINE_ == 1)
	/*
	* dwControl keeps the original settings of OFFSET_TX_VIDEO_CONTROL_0
	* Enable TX_VIDEO_PIPELINE_1D_LUT function.
	*/
	dwData = (dwVideoCtrl | TX_VIDEO_PIPELINE_1D_LUT_ENABLE);
	nError = SPI_Register_Write(LinkNo, OFFSET_TX_VIDEO_CONTROL_0, dwData);
	if (nError != ERROR_SCALER_SUCCESS)
		return nError;
#endif

	Sleep(500);

	nError = SPI_Register_Write(LinkNo, OFFSET_TX_DISPLAY_CONTROL, TXControl);
	if (nError != ERROR_SCALER_SUCCESS)
		return nError;

	return nError; /* ERROR_SCALER_SUCCESS;*/
}

#endif

int16_t WINAPI Adjust_BackLight_PWM(int32_t LinkNo, uint32_t DutyCycle, uint8_t bPWMEn)
{
	CSerialPort *pSerial;
	LPVOID hSerial;

	char strCmd[25];
	uint32_t Value;
	size_t tmpLen;
	int16_t nError = ERROR_SCALER_SUCCESS;

	char rdBuf[256];
	UINT dwData = 0x00;
	DWORD rdBufLen, WrBufLen;
	UINT CmdAck = SERIAL_CMD_UNKNOWN; /* initial /w command-unknown*/
	INT nRet = 0;

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	if (LinkSession[LinkNo].pSerial == static_cast<CSerialPort *>INVALID_HANDLE_VALUE)
		return ERROR_SCALER_INVALID_SERIAL_HANDLE;

	if ( DutyCycle > 100 )
		return ERROR_SCALER_INVALID_PARAMETER;
	else {
		if(bPWMEn == 0)
			Value = 0x01; /* disable TX 12V */
		else
			Value = (DutyCycle << 8); /* bit14-8 */
	}

	pSerial = LinkSession[LinkNo].pSerial;
	hSerial = static_cast<CSerialPort *>(LinkSession[LinkNo].pSerial);

	sprintf(strCmd, "W %X %X\r", OFFSET_V_BY_ONE_TX_CONTROL, Value);

	tmpLen = strlen(strCmd);
	WrBufLen = static_cast<DWORD>(tmpLen);

	EnterCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	nRet = WriteCharComm(hSerial, ((LPCSTR)(LPCTSTR)strCmd), &WrBufLen);
	if (nRet == err_internal_success)
	{
		rdBufLen = 255;
		memset(rdBuf, 0x00, 256);
		nRet = ReadLineComm(hSerial, rdBuf, &rdBufLen, 500);
		if (nRet == 0) {
			rdBuf[rdBufLen - 1] = 0x00;
			if (strncmp(rdBuf, "W OK", 4) == 0) {
				CmdAck = SERIAL_CMD_ACK_OK;
			}
			else if (strncmp(rdBuf, "ERROR", 5) == 0) {
				CmdAck = SERIAL_CMD_ACK_ERROR;
#if defined(_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_) && (_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_)
				nError = _MCU_LPUART_Error_Xfer(rdBuf);
#else
				nError = ERROR_SCALER_SPI_REG_ACCESS_FAILED;
#endif
				goto _Adjust_BackLight_PWM_Exit;
			}
			else if (strncmp(rdBuf, "UNKNOWN", 7) == 0) {
				CmdAck = SERIAL_CMD_UNKNOWN;
				nError = ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
				goto _Adjust_BackLight_PWM_Exit;
				// return ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
			}
		}
		else {
			// return _internal_error_Xfer(nRet);
			nError = _internal_error_Xfer(nRet);
			goto _Adjust_BackLight_PWM_Exit;
		}
	}

_Adjust_BackLight_PWM_Exit:
	pSerial->Clear();
	LeaveCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	return nError; /* ERROR_SCALER_SUCCESS;*/
}

int16_t WINAPI Generate_RGB_Pattern(int32_t LinkNo, uint16_t Red, uint16_t Green, uint16_t Blue, uint32_t LUT4TestPattern)
{
	CSerialPort *pSerial;
	LPVOID hSerial;

	char strCmd[25];
	uint32_t Value;
	size_t tmpLen;
	int16_t nError = ERROR_SCALER_SUCCESS;

	char rdBuf[256];
	UINT dwData = 0x00;
	DWORD rdBufLen, WrBufLen;
	UINT CmdAck = SERIAL_CMD_UNKNOWN; /* initial /w command-unknown*/
	INT nRet = 0;

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	if (LinkSession[LinkNo].pSerial == static_cast<CSerialPort *>INVALID_HANDLE_VALUE)
		return ERROR_SCALER_INVALID_SERIAL_HANDLE;

	if ((LUT4TestPattern & ACTION_TX_TEST_PATTERN_INSERTION) == ACTION_TX_TEST_PATTERN_INSERTION) {
		/* Undocumented Operation - Turn off the Test-Pattern */
		Value = 0x00;
	}
	else { /* Normal Operation */
		if ((LUT4TestPattern & ~(MASK_TX_TEST_PATTERN_1D_LUT_SELECT)) != 0x0)
			return ERROR_MCU_LUT_INVALID_SELECT_LUT;

		if ((Red > 0x3FF) || (Green > 0x3FF) || (Blue > 0x3FF))
			return ERROR_SCALER_INVALID_PARAMETER;
		else
			Value = (ACTION_TX_TEST_PATTERN_INSERTION | LUT4TestPattern | (Blue << 20) | (Green << 10) | Red);
	}

	pSerial = LinkSession[LinkNo].pSerial;
	hSerial = static_cast<CSerialPort *>(LinkSession[LinkNo].pSerial);

	sprintf(strCmd, "W %X %X\r", OFFSET_TX_TEST_PATTERN_CONTROL, Value);

	tmpLen = strlen(strCmd);
	WrBufLen = static_cast<DWORD>(tmpLen);

	EnterCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	nRet = WriteCharComm(hSerial, ((LPCSTR)(LPCTSTR)strCmd), &WrBufLen);
	if (nRet == err_internal_success)
	{
		rdBufLen = 255;
		memset(rdBuf, 0x00, 256);
		nRet = ReadLineComm(hSerial, rdBuf, &rdBufLen, 500);
		if (nRet == 0) {
			rdBuf[rdBufLen - 1] = 0x00;
			if (strncmp(rdBuf, "W OK", 4) == 0) {
				CmdAck = SERIAL_CMD_ACK_OK;
			}
			else if (strncmp(rdBuf, "ERROR", 5) == 0) {
				CmdAck = SERIAL_CMD_ACK_ERROR;
#if defined(_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_) && (_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_)
				nError = _MCU_LPUART_Error_Xfer(rdBuf);
#else
				nError = ERROR_SCALER_SPI_REG_ACCESS_FAILED;
#endif
				goto _Generate_RGB_Pattern_Exit;
			}
			else if (strncmp(rdBuf, "UNKNOWN", 7) == 0) {
				CmdAck = SERIAL_CMD_UNKNOWN;
				nError = ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
				goto _Generate_RGB_Pattern_Exit;
				// return ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
			}
		}
		else {
			// return _internal_error_Xfer(nRet);
			nError = _internal_error_Xfer(nRet);
			goto _Generate_RGB_Pattern_Exit;
		}
	}


_Generate_RGB_Pattern_Exit:
	pSerial->Clear();
	LeaveCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	return nError; /* ERROR_SCALER_SUCCESS;*/
}

int16_t WINAPI Generate_RGB_Pattern_Final_Stage(int32_t LinkNo, uint16_t Red, uint16_t Green, uint16_t Blue)
{
	CSerialPort *pSerial;
	LPVOID hSerial;

	char strCmd[25];
	uint32_t Value;
	size_t tmpLen;
	int16_t nError = ERROR_SCALER_SUCCESS;

	char rdBuf[256];
	UINT dwData = 0x00;
	DWORD rdBufLen, WrBufLen;
	UINT CmdAck = SERIAL_CMD_UNKNOWN; /* initial /w command-unknown*/
	INT nRet = 0;

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	if (LinkSession[LinkNo].pSerial == static_cast<CSerialPort *>INVALID_HANDLE_VALUE)
		return ERROR_SCALER_INVALID_SERIAL_HANDLE;

	if ((Red > 0x3FF) || (Green > 0x3FF) || (Blue > 0x3FF))
		return ERROR_SCALER_INVALID_PARAMETER;
	else
		Value = (ACTION_TX_TEST_PATTERN_INSERTION | (Blue << 20) | (Green << 10) | Red);


	pSerial = LinkSession[LinkNo].pSerial;
	hSerial = static_cast<CSerialPort *>(LinkSession[LinkNo].pSerial);

	sprintf(strCmd, "W D4 %X\r", Value);

	tmpLen = strlen(strCmd);
	WrBufLen = static_cast<DWORD>(tmpLen);

	EnterCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	nRet = WriteCharComm(hSerial, ((LPCSTR)(LPCTSTR)strCmd), &WrBufLen);
	if (nRet == err_internal_success)
	{
		rdBufLen = 255;
		memset(rdBuf, 0x00, 256);
		nRet = ReadLineComm(hSerial, rdBuf, &rdBufLen, 500);
		if (nRet == 0) {
			rdBuf[rdBufLen - 1] = 0x00;
			if (strncmp(rdBuf, "W OK", 4) == 0) {
				CmdAck = SERIAL_CMD_ACK_OK;
			}
			else if (strncmp(rdBuf, "ERROR", 5) == 0) {
				CmdAck = SERIAL_CMD_ACK_ERROR;
#if defined(_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_) && (_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_)
				nError = _MCU_LPUART_Error_Xfer(rdBuf);
#else
				nError = ERROR_SCALER_SPI_REG_ACCESS_FAILED;
#endif
				goto _Generate_RGB_Pattern_Exit;
			}
			else if (strncmp(rdBuf, "UNKNOWN", 7) == 0) {
				CmdAck = SERIAL_CMD_UNKNOWN;
				nError = ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
				goto _Generate_RGB_Pattern_Exit;
				// return ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
			}
		}
		else {
			// return _internal_error_Xfer(nRet);
			nError = _internal_error_Xfer(nRet);
			goto _Generate_RGB_Pattern_Exit;
		}
	}


_Generate_RGB_Pattern_Exit:
	pSerial->Clear();
	LeaveCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	return nError; /* ERROR_SCALER_SUCCESS;*/
}

int16_t WINAPI SPI_Register_Write(int32_t LinkNo, uint32_t Offset, uint32_t Value)
{
	CSerialPort *pSerial;
	LPVOID hSerial;

	char strCmd[25];
	size_t tmpLen;
	int16_t nError = ERROR_SCALER_SUCCESS;

	char rdBuf[256];
	UINT dwData = 0x00;
	DWORD rdBufLen, WrBufLen;
	UINT CmdAck = SERIAL_CMD_UNKNOWN; /* initial /w command-unknown*/
	INT nRet = 0;

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	if (LinkSession[LinkNo].pSerial == static_cast<CSerialPort *>INVALID_HANDLE_VALUE)
		return ERROR_SCALER_INVALID_SERIAL_HANDLE;

	pSerial = LinkSession[LinkNo].pSerial;
	hSerial = static_cast<CSerialPort *>(LinkSession[LinkNo].pSerial);

	sprintf(strCmd, "W %X %X\r", Offset, Value);

	tmpLen = strlen(strCmd);
	WrBufLen = static_cast<DWORD>(tmpLen);

	EnterCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	nRet = WriteCharComm(hSerial, ((LPCSTR)(LPCTSTR)strCmd), &WrBufLen);
	if (nRet == err_internal_success)
	{
		rdBufLen = 255;
		memset(rdBuf, 0x00, 256);
		nRet = ReadLineComm(hSerial, rdBuf, &rdBufLen, 500);
		if (nRet == 0) {
			rdBuf[rdBufLen - 1] = 0x00;
			if (strncmp(rdBuf, "W OK", 4) == 0) {
				CmdAck = SERIAL_CMD_ACK_OK;
			}
			else if (strncmp(rdBuf, "ERROR", 5) == 0) {
				CmdAck = SERIAL_CMD_ACK_ERROR;
#if defined(_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_) && (_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_)
				nError = _MCU_LPUART_Error_Xfer(rdBuf);
#else
				nError = ERROR_SCALER_SPI_REG_ACCESS_FAILED;
#endif
				goto _SPI_Register_Write_Exit;
			}
			else if (strncmp(rdBuf, "UNKNOWN", 7) == 0) {
				CmdAck = SERIAL_CMD_UNKNOWN;
				nError = ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
				goto _SPI_Register_Write_Exit;
				// return ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
			}
		}
		else {
			// return _internal_error_Xfer(nRet);
			nError = _internal_error_Xfer(nRet);
			goto _SPI_Register_Write_Exit;
		}
	}

_SPI_Register_Write_Exit:
	pSerial->Clear();
	LeaveCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	return nError; /* ERROR_SCALER_SUCCESS;*/
}

int16_t WINAPI SPI_Register_Read(int32_t LinkNo, uint32_t Offset, uint32_t* pValue)
{
	CSerialPort *pSerial;
	LPVOID hSerial;

	char strCmd[25];
	size_t tmpLen;
	int16_t nError = ERROR_SCALER_SUCCESS;

	char rdBuf[256];
	UINT dwData = 0x00;
	DWORD rdBufLen, WrBufLen;
	UINT CmdAck = SERIAL_CMD_UNKNOWN; /* initial /w command-unknown*/
	INT nRet = 0;

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	if (LinkSession[LinkNo].pSerial == static_cast<CSerialPort *>INVALID_HANDLE_VALUE)
		return ERROR_SCALER_INVALID_SERIAL_HANDLE;

	pSerial = LinkSession[LinkNo].pSerial;
	hSerial = static_cast<CSerialPort *>(LinkSession[LinkNo].pSerial);

	sprintf(strCmd, "R %x\r", Offset);

	tmpLen = strlen(strCmd);
	WrBufLen = static_cast<DWORD>(tmpLen);

	EnterCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	nRet = WriteCharComm(hSerial, ((LPCSTR)(LPCTSTR)strCmd), &WrBufLen);
	if (nRet == err_internal_success)
	{
		rdBufLen = 255;
		memset(rdBuf, 0x00, 256);
		nRet = ReadLineComm(hSerial, rdBuf, &rdBufLen, 500);
		if (nRet == 0) {

			rdBuf[rdBufLen - 1] = 0x00;
			if (strncmp(rdBuf, "UNKNOWN", 7) == 0) {
				CmdAck = SERIAL_CMD_UNKNOWN;
				nError = ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
				goto _SPI_Register_Read_Exit;
				// return ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
			}
			else if (strncmp(rdBuf, "ERROR", 5) == 0) {
				CmdAck = SERIAL_CMD_ACK_ERROR;
#if defined(_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_) && (_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_)
				nError = _MCU_LPUART_Error_Xfer(rdBuf);
#else
				nError = ERROR_SCALER_SPI_REG_ACCESS_FAILED;
#endif
				goto _SPI_Register_Read_Exit;
			}
			else {
				CmdAck = SERIAL_CMD_ACK_OK;
			}

			if (CmdAck == SERIAL_CMD_ACK_OK) {
				/* Datecode format: "R 0x%X\n" */
				sscanf_s(rdBuf, "R 0x%X", &dwData);

				if (pValue != NULL) {
					*pValue = static_cast<uint32_t>(dwData);
				}

				nError = ERROR_SCALER_SUCCESS;
				goto _SPI_Register_Read_Exit;

			}

		}
		else {
			// return _internal_error_Xfer(nRet);
			nError = _internal_error_Xfer(nRet);
			goto _SPI_Register_Read_Exit;
		}
	}

_SPI_Register_Read_Exit:
	pSerial->Clear();
	LeaveCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	return nError; /* ERROR_SCALER_SUCCESS;*/
}

int16_t WINAPI FPGA_Datecode_Get(int32_t LinkNo, uint32_t* pDatecode1, uint32_t* pDatecode2)
{
	CSerialPort *pSerial;
	LPVOID hSerial;

	char strCmd[25] = "C\r";
	size_t tmpLen;
	int16_t nError = ERROR_SCALER_SUCCESS;

	char rdBuf[256];
	UINT dwDatecode1 = 0x00, dwDatecode2 = 0x00;
	DWORD rdBufLen, WrBufLen;
	UINT CmdAck = SERIAL_CMD_UNKNOWN; /* initial /w command-unknown*/
	INT nRet = 0;

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	if (LinkSession[LinkNo].pSerial == static_cast<CSerialPort *>INVALID_HANDLE_VALUE)
		return ERROR_SCALER_INVALID_SERIAL_HANDLE;

	pSerial = LinkSession[LinkNo].pSerial;
	hSerial = static_cast<CSerialPort *>(LinkSession[LinkNo].pSerial);

	tmpLen = strlen(strCmd);
	WrBufLen = static_cast<DWORD>(tmpLen);

	EnterCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	nRet = WriteCharComm(hSerial, ((LPCSTR)(LPCTSTR)strCmd), &WrBufLen);
	if (nRet == err_internal_success)
	{
		rdBufLen = 255;
		memset(rdBuf, 0x00, 256);
		nRet = ReadLineComm(hSerial, rdBuf, &rdBufLen, 500);
		if (nRet == 0) {

			rdBuf[rdBufLen - 1] = 0x00;
			if (strncmp(rdBuf, "UNKNOWN", 7) == 0) {
				CmdAck = SERIAL_CMD_UNKNOWN;
				nError = ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
				goto _FPGA_Datecode_Get_Exit;
				// return ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
			}
			else if (strncmp(rdBuf, "ERROR", 5) == 0) {
				CmdAck = SERIAL_CMD_ACK_ERROR;
#if defined(_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_) && (_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_)
				nError = _MCU_LPUART_Error_Xfer(rdBuf);
#else
				nError = ERROR_SCALER_SPI_REG_ACCESS_FAILED;
#endif
				goto _FPGA_Datecode_Get_Exit;
			}
			else {
				CmdAck = SERIAL_CMD_ACK_OK;
			}

			if (CmdAck == SERIAL_CMD_ACK_OK) {
				/* Datecode format: "R 0x%X\n" */
				sscanf_s(rdBuf, "C 0x%X 0x%X", &dwDatecode1, &dwDatecode2);

				if (pDatecode1 != NULL) {
					*pDatecode1 = static_cast<uint32_t>(dwDatecode1);
				}

				if (pDatecode2 != NULL) {
					*pDatecode2 = static_cast<uint32_t>(dwDatecode2);
				}

				nError = ERROR_SCALER_SUCCESS;
				goto _FPGA_Datecode_Get_Exit;

			}

		}
		else {
			// return _internal_error_Xfer(nRet);
			nError = _internal_error_Xfer(nRet);
			goto _FPGA_Datecode_Get_Exit;
		}
	}

_FPGA_Datecode_Get_Exit:
	pSerial->Clear();
	LeaveCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	return nError; /* ERROR_SCALER_SUCCESS;*/
}

int16_t WINAPI MCU_SysTick_Get(int32_t LinkNo, uint32_t* pSysTick)
{
	CSerialPort *pSerial;
	LPVOID hSerial;

	char strCmd[25] = "T\r";
	size_t tmpLen;
	int16_t nError = ERROR_SCALER_SUCCESS;

	char rdBuf[256];
	UINT dwSysTick = 0x00;
	DWORD rdBufLen, WrBufLen;
	UINT CmdAck = SERIAL_CMD_UNKNOWN; /* initial /w command-unknown*/
	INT nRet = 0;

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	if (LinkSession[LinkNo].pSerial == static_cast<CSerialPort *>INVALID_HANDLE_VALUE)
		return ERROR_SCALER_INVALID_SERIAL_HANDLE;

	pSerial = LinkSession[LinkNo].pSerial;
	hSerial = static_cast<CSerialPort *>(LinkSession[LinkNo].pSerial);

	tmpLen = strlen(strCmd);
	WrBufLen = static_cast<DWORD>(tmpLen);

	EnterCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	nRet = WriteCharComm(hSerial, ((LPCSTR)(LPCTSTR)strCmd), &WrBufLen);
	if (nRet == err_internal_success)
	{
		rdBufLen = 255;
		memset(rdBuf, 0x00, 256);
		nRet = ReadLineComm(hSerial, rdBuf, &rdBufLen, 500);
		if (nRet == 0) {

			rdBuf[rdBufLen - 1] = 0x00;
			if (strncmp(rdBuf, "UNKNOWN", 7) == 0) {
				CmdAck = SERIAL_CMD_UNKNOWN;
				nError = ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
				goto _MCU_SysTick_Get_Exit;
				// return ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
			}
			else if (strncmp(rdBuf, "ERROR", 5) == 0) {
				CmdAck = SERIAL_CMD_ACK_ERROR;
#if defined(_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_) && (_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_)
				nError = _MCU_LPUART_Error_Xfer(rdBuf);
#else
				nError = ERROR_SCALER_SPI_REG_ACCESS_FAILED;
#endif
				goto _MCU_SysTick_Get_Exit;
			}
			else {
				CmdAck = SERIAL_CMD_ACK_OK;
			}

			if (CmdAck == SERIAL_CMD_ACK_OK) {
				/* Datecode format: "R 0x%X\n" */
				sscanf_s(rdBuf, "T %ld", &dwSysTick);

				if (pSysTick != NULL) {
					*pSysTick = static_cast<uint32_t>(dwSysTick);
				}

				nError = ERROR_SCALER_SUCCESS;
				goto _MCU_SysTick_Get_Exit;

			}

		}
		else {
			// return _internal_error_Xfer(nRet);
			nError = _internal_error_Xfer(nRet);
			goto _MCU_SysTick_Get_Exit;
		}
	}

 
_MCU_SysTick_Get_Exit:
	pSerial->Clear();
	LeaveCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	return nError; /* ERROR_SCALER_SUCCESS;*/
}

/* Combination Function*/

int16_t WINAPI Xfer_LUTable1D_Element(int32_t LinkNo, uint32_t dwLUTIndex, RGBColor *pColorElement)
{

	CSerialPort *pSerial;
	LPVOID hSerial;

	char strCmd[25];
	size_t tmpLen;
	int16_t nError = ERROR_SCALER_SUCCESS;

	char rdBuf[256];
	UINT dwData = 0x00;
	DWORD rdBufLen, WrBufLen;
	UINT CmdAck = SERIAL_CMD_UNKNOWN; /* initial /w command-unknown*/
	INT nRet = 0;

	uint32_t dwColorTemp;
	uint32_t dwRGBData;

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	if (LinkSession[LinkNo].pSerial == static_cast<CSerialPort *>INVALID_HANDLE_VALUE)
		return ERROR_SCALER_INVALID_SERIAL_HANDLE;

	pSerial = LinkSession[LinkNo].pSerial;
	hSerial = static_cast<CSerialPort *>(LinkSession[LinkNo].pSerial);

	if (dwLUTIndex > 0x3FF)
		return ERROR_SCALER_LUT_INVALID_INDEX;

	/* Red Color */
	dwColorTemp = pColorElement->Red;
	dwRGBData = (dwColorTemp & 0x3FF);
	/* Green Color */
	dwColorTemp = pColorElement->Green;
	dwRGBData = (dwRGBData | ((dwColorTemp & 0x3FF) << 10));
	/* Blue Color */
	dwColorTemp = pColorElement->Blue;
	dwRGBData = (dwRGBData | ((dwColorTemp & 0x3FF) << 20));


	sprintf(strCmd, "P %X %X\r", dwLUTIndex, dwRGBData);

	tmpLen = strlen(strCmd);
	WrBufLen = static_cast<DWORD>(tmpLen);

	EnterCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	nRet = WriteCharComm(hSerial, ((LPCSTR)(LPCTSTR)strCmd), &WrBufLen);
	if (nRet == err_internal_success)
	{
		rdBufLen = 255;
		memset(rdBuf, 0x00, 256);
		nRet = ReadLineComm(hSerial, rdBuf, &rdBufLen, 500);
		if (nRet == 0) {
			rdBuf[rdBufLen - 1] = 0x00;
			if (strncmp(rdBuf, "P OK", 4) == 0) {
				CmdAck = SERIAL_CMD_ACK_OK;
			}
			else if (strncmp(rdBuf, "ERROR", 5) == 0) {
				CmdAck = SERIAL_CMD_ACK_ERROR;
#if defined(_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_) && (_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_)
				nError = _MCU_LPUART_Error_Xfer(rdBuf);
#else
				nError = ERROR_SCALER_SPI_REG_ACCESS_FAILED;
#endif
				goto _Xfer_LUTable1D_Element_Exit;
			}
			else if (strncmp(rdBuf, "UNKNOWN", 7) == 0) {
				CmdAck = SERIAL_CMD_UNKNOWN;
				nError = ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
				goto _Xfer_LUTable1D_Element_Exit;
				// return ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
			}
		}
		else {
			// return _internal_error_Xfer(nRet);
			nError = _internal_error_Xfer(nRet);
			goto _Xfer_LUTable1D_Element_Exit;
		}
	}

_Xfer_LUTable1D_Element_Exit:
	pSerial->Clear();
	LeaveCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	return nError;

}

int16_t WINAPI Set_LUTable1D_Element(int32_t LinkNo, uint32_t dwLUTSelect, LUTable1D* pLUTElement)
{
//	uint32_t dwMaskLUTBypass, dwLUTEnable;

	CSerialPort *pSerial;
	LPVOID hSerial;

	char strCmd[25];
	size_t tmpLen;
	int16_t nError = ERROR_SCALER_SUCCESS;

	char rdBuf[256];
	UINT dwData = 0x00;
	DWORD rdBufLen, WrBufLen;
	UINT CmdAck = SERIAL_CMD_UNKNOWN; /* initial /w command-unknown*/
	INT nRet = 0;

	uint32_t dwColorTemp, dwLUTTmep;
	uint32_t dwRGBData, dwLUTIndex;

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	if (LinkSession[LinkNo].pSerial == static_cast<CSerialPort *>INVALID_HANDLE_VALUE)
		return ERROR_SCALER_INVALID_SERIAL_HANDLE;

	pSerial = LinkSession[LinkNo].pSerial;
	hSerial = static_cast<CSerialPort *>(LinkSession[LinkNo].pSerial);

	if (pLUTElement->TableIndex > 0x3FF)
		return ERROR_SCALER_LUT_INVALID_INDEX;


	switch (dwLUTSelect) {
#if defined(_FPGA_SUPPORT_1D_DEGAMMA_LUT_) && (_FPGA_SUPPORT_1D_DEGAMMA_LUT_ == 1)
	case SELECT_1D_LUT_GAMMA:
		dwLUTTmep = (SELECT_1D_LUT_GAMMA << SHIFT_BITS_SELECT_LUT);
		break;
	case SELECT_1D_LUT_DEGAMMA:
		dwLUTTmep = (SELECT_1D_LUT_DEGAMMA << SHIFT_BITS_SELECT_LUT);
		break;
	case SELECT_1D_LUT_DICOM:
		dwLUTTmep = (SELECT_1D_LUT_DICOM << SHIFT_BITS_SELECT_LUT);
		break;
#if defined(_FPGA_SUPPORT_3D_LUT_) && (_FPGA_SUPPORT_3D_LUT_ == 1)
	case SELECT_3D_LUT:
		dwLUTTmep = (SELECT_3D_LUT << SHIFT_BITS_SELECT_LUT);
		break;
#endif /* _FPGA_SUPPORT_3D_LUT_ */
#if defined(_FPGA_SUPPORT_1D_DEGAMMA_TEST_LUT_) && (_FPGA_SUPPORT_1D_DEGAMMA_TEST_LUT_ == 1)
	case SELECT_1D_LUT_DEGAMMA_TEST:
		dwLUTTmep = (SELECT_1D_LUT_DEGAMMA_TEST << SHIFT_BITS_SELECT_LUT);
		break;
#endif /* _FPGA_SUPPORT_1D_DEGAMMA_TEST_LUT_ */

#else /* ! _FPGA_SUPPORT_1D_DEGAMMA_LUT_ */
	case SELECT_1D_LUT_GAMMA:
		dwLUTTmep = (SELECT_1D_LUT_GAMMA << SHIFT_BITS_SELECT_LUT);
		break;
	case SELECT_1D_LUT_GAMMA_ADJUST:
		dwLUTTmep = (SELECT_1D_LUT_GAMMA_ADJUST << SHIFT_BITS_SELECT_LUT);
		break;
	case SELECT_1D_LUT_DICOM:
		dwLUTTmep = (SELECT_1D_LUT_DICOM << SHIFT_BITS_SELECT_LUT);
		break;
#if defined(_FPGA_SUPPORT_3D_LUT_) && (_FPGA_SUPPORT_3D_LUT_ == 1)
	case SELECT_3D_LUT:
		dwLUTTmep = (SELECT_3D_LUT << SHIFT_BITS_SELECT_LUT);
		break;
#endif /* _FPGA_SUPPORT_3D_LUT_ */
#endif /* _FPGA_SUPPORT_1D_DEGAMMA_LUT_ */
	default:
		nError = ERROR_SCALER_LUT_INVALID_SELECT_LUT;
		return nError;
	}

	dwLUTIndex = (dwLUTTmep | (pLUTElement->TableIndex & 0x3FF)); /* valid LUT Index: 0 ~ 1023 */

																  /* Red Color */
	dwColorTemp = pLUTElement->rgbColor.Red;
	dwRGBData = (dwColorTemp & 0x3FF);
	/* Green Color */
	dwColorTemp = pLUTElement->rgbColor.Green;
	dwRGBData = (dwRGBData | ((dwColorTemp & 0x3FF) << 10));
	/* Blue Color */
	dwColorTemp = pLUTElement->rgbColor.Blue;
	dwRGBData = (dwRGBData | ((dwColorTemp & 0x3FF) << 20));


	sprintf(strCmd, "U %X %X\r", dwLUTIndex, dwRGBData);

	tmpLen = strlen(strCmd);
	WrBufLen = static_cast<DWORD>(tmpLen);

	EnterCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	nRet = WriteCharComm(hSerial, ((LPCSTR)(LPCTSTR)strCmd), &WrBufLen);
	if (nRet == err_internal_success)
	{
		rdBufLen = 255;
		memset(rdBuf, 0x00, 256);
		nRet = ReadLineComm(hSerial, rdBuf, &rdBufLen, 500);
		if (nRet == 0) {
			rdBuf[rdBufLen - 1] = 0x00;
			if (strncmp(rdBuf, "U OK", 4) == 0) {
				CmdAck = SERIAL_CMD_ACK_OK;
			}
			else if (strncmp(rdBuf, "ERROR", 5) == 0) {
				CmdAck = SERIAL_CMD_ACK_ERROR;
#if defined(_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_) && (_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_)
				nError = _MCU_LPUART_Error_Xfer(rdBuf);
#else
				nError = ERROR_SCALER_SPI_REG_ACCESS_FAILED;
#endif
				goto _Set_LUTable1D_Element_Exit;
			}
			else if (strncmp(rdBuf, "UNKNOWN", 7) == 0) {
				CmdAck = SERIAL_CMD_UNKNOWN;
				nError = ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
				goto _Set_LUTable1D_Element_Exit;
				// return ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
			}
		}
		else {
			// return _internal_error_Xfer(nRet);
			nError = _internal_error_Xfer(nRet);
			goto _Set_LUTable1D_Element_Exit;
		}
	}

_Set_LUTable1D_Element_Exit:
	pSerial->Clear();
	LeaveCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	return nError;

}

int16_t WINAPI TX_Video_Pipeline_LUT_Actiate_FPGA(int32_t LinkNo, uint32_t dwFPGALUTSelect)
{
	return TX_Video_Pipeline_LUT_Activate_FPGA(LinkNo, dwFPGALUTSelect);
}

int16_t WINAPI TX_Video_Pipeline_LUT_Activate_FPGA(int32_t LinkNo, uint32_t dwFPGALUTSelect)
{
	CSerialPort *pSerial;
	LPVOID hSerial;

	char strCmd[25];
	size_t tmpLen;
	int16_t nError = ERROR_SCALER_SUCCESS;

	char rdBuf[256];
	UINT dwData = 0x00;
	DWORD rdBufLen, WrBufLen;
	UINT CmdAck = SERIAL_CMD_UNKNOWN; /* initial /w command-unknown*/
	INT nRet = 0;

	uint16_t wLUT;

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	if (LinkSession[LinkNo].pSerial == static_cast<CSerialPort *>INVALID_HANDLE_VALUE)
		return ERROR_SCALER_INVALID_SERIAL_HANDLE;

	pSerial = LinkSession[LinkNo].pSerial;
	hSerial = static_cast<CSerialPort *>(LinkSession[LinkNo].pSerial);


	switch (dwFPGALUTSelect) {

	case SELECT_1D_LUT_DICOM:
		wLUT = (SELECT_1D_LUT_DICOM);
		break;

	case SELECT_1D_LUT_GAMMA:
		wLUT = (SELECT_1D_LUT_GAMMA);
		break;

	default:
		nError = ERROR_SCALER_LUT_INVALID_SELECT_LUT;
		goto _TX_Video_Pipeline_LUT_Actiate_FPGA_Exit;
	}

	wLUT = (wLUT | OP_ACTIVATE_FPGA_LUT);
	sprintf(strCmd, "L %X\r", wLUT);
	// sprintf(strCmd, "L %X %X\r", wLUT, OP_ACTIVATE_FPGA_LUT);

	tmpLen = strlen(strCmd);
	WrBufLen = static_cast<DWORD>(tmpLen);

	EnterCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	nRet = WriteCharComm(hSerial, ((LPCSTR)(LPCTSTR)strCmd), &WrBufLen);
	if (nRet == err_internal_success)
	{
		rdBufLen = 255;
		memset(rdBuf, 0x00, 256);
		nRet = ReadLineComm(hSerial, rdBuf, &rdBufLen, 500);
		if (nRet == 0) {
			rdBuf[rdBufLen - 1] = 0x00;
			if (strncmp(rdBuf, "L OK", 4) == 0) {
				CmdAck = SERIAL_CMD_ACK_OK;
			}
			else if (strncmp(rdBuf, "ERROR", 5) == 0) {
				CmdAck = SERIAL_CMD_ACK_ERROR;
#if defined(_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_) && (_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_)
				nError = _MCU_LPUART_Error_Xfer(rdBuf);
#else
				nError = ERROR_SCALER_SPI_REG_ACCESS_FAILED;
#endif
				goto _TX_Video_Pipeline_LUT_Actiate_FPGA_Exit;
			}
			else if (strncmp(rdBuf, "UNKNOWN", 7) == 0) {
				CmdAck = SERIAL_CMD_UNKNOWN;
				nError = ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
				goto _TX_Video_Pipeline_LUT_Actiate_FPGA_Exit;
				// return ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
			}
		}
		else {
			// return _internal_error_Xfer(nRet);
			nError = _internal_error_Xfer(nRet);
			goto _TX_Video_Pipeline_LUT_Actiate_FPGA_Exit;
		}
	}

_TX_Video_Pipeline_LUT_Actiate_FPGA_Exit:
	pSerial->Clear();
	LeaveCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	return nError;
}

int16_t WINAPI TX_Video_Pipeline_LUT_Update_FPGA(int32_t LinkNo, uint32_t dwFPGALUTSelect, bool bReadback_checking)
{
	CSerialPort *pSerial;
	LPVOID hSerial;

	char strCmd[25];
	size_t tmpLen;
	int16_t nError = ERROR_SCALER_SUCCESS;

	char rdBuf[256];
	UINT dwData = 0x00;
	DWORD rdBufLen, WrBufLen;
	UINT CmdAck = SERIAL_CMD_UNKNOWN; /* initial /w command-unknown*/
	INT nRet = 0;

	uint16_t wLUT;

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	if (LinkSession[LinkNo].pSerial == static_cast<CSerialPort *>INVALID_HANDLE_VALUE)
		return ERROR_SCALER_INVALID_SERIAL_HANDLE;

	pSerial = LinkSession[LinkNo].pSerial;
	hSerial = static_cast<CSerialPort *>(LinkSession[LinkNo].pSerial);


	switch (dwFPGALUTSelect) {
#if defined(_FPGA_SUPPORT_1D_DEGAMMA_LUT_) && (_FPGA_SUPPORT_1D_DEGAMMA_LUT_ == 1)
	case SELECT_1D_LUT_GAMMA:
		wLUT = (SELECT_1D_LUT_GAMMA);
		break;
	case SELECT_1D_LUT_DEGAMMA:
		wLUT = (SELECT_1D_LUT_DEGAMMA);
		break;
	case SELECT_1D_LUT_DICOM:
		wLUT = (SELECT_1D_LUT_DICOM);
		break;
#if defined(_FPGA_SUPPORT_3D_LUT_) && (_FPGA_SUPPORT_3D_LUT_ == 1)
	case SELECT_3D_LUT:
		dwLUTTmep = (SELECT_3D_LUT << SHIFT_BITS_SELECT_LUT);
		break;
#endif /* _FPGA_SUPPORT_3D_LUT_ */
#if defined(_FPGA_SUPPORT_1D_DEGAMMA_TEST_LUT_) && (_FPGA_SUPPORT_1D_DEGAMMA_TEST_LUT_ == 1)
	case SELECT_1D_LUT_DEGAMMA_TEST:
		wLUT = (SELECT_1D_LUT_DEGAMMA_TEST);
		break;
#endif /* _FPGA_SUPPORT_1D_DEGAMMA_TEST_LUT_ */

#else /* ! _FPGA_SUPPORT_1D_DEGAMMA_LUT_ */
	case SELECT_1D_LUT_GAMMA:
		wLUT = (SELECT_1D_LUT_GAMMA);
		break;

	case SELECT_1D_LUT_GAMMA_ADJUST:
		wLUT = (SELECT_1D_LUT_GAMMA_ADJUST);
		break;

	case SELECT_1D_LUT_DICOM:
		wLUT = (SELECT_1D_LUT_DICOM);
		break;

#if defined(_FPGA_SUPPORT_3D_LUT_) && (_FPGA_SUPPORT_3D_LUT_ == 1)
	case SELECT_3D_LUT:
		wLUT = (SELECT_3D_LUT);
		break;
#endif

#endif /* _FPGA_SUPPORT_1D_DEGAMMA_LUT_ */

	default:
		nError = ERROR_SCALER_LUT_INVALID_SELECT_LUT;
		goto _TTX_Video_Pipeline_LUT_Update_FPGA_Exit;
	}

	wLUT = (wLUT | OP_UPDATE_LUT_BUFFER_TO_FPGA);
	if (bReadback_checking == true) {
		wLUT = (wLUT | OP_LUT_SET_AND_READBACK_CHECK);
	}

	sprintf(strCmd, "L %X\r", wLUT);
	// sprintf(strCmd, "L %X %X\r", wLUT, OP_UPDATE_LUT_BUFFER_TO_FPGA);

	tmpLen = strlen(strCmd);
	WrBufLen = static_cast<DWORD>(tmpLen);

	EnterCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	nRet = WriteCharComm(hSerial, ((LPCSTR)(LPCTSTR)strCmd), &WrBufLen);
	if (nRet == err_internal_success)
	{
		rdBufLen = 255;
		memset(rdBuf, 0x00, 256);
		nRet = ReadLineComm(hSerial, rdBuf, &rdBufLen, 500);
		if (nRet == 0) {
			rdBuf[rdBufLen - 1] = 0x00;
			if (strncmp(rdBuf, "L OK", 4) == 0) {
				CmdAck = SERIAL_CMD_ACK_OK;
			}
			else if (strncmp(rdBuf, "ERROR", 5) == 0) {
				CmdAck = SERIAL_CMD_ACK_ERROR;
#if defined(_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_) && (_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_)
				nError = _MCU_LPUART_Error_Xfer(rdBuf);
#else
				nError = ERROR_SCALER_SPI_REG_ACCESS_FAILED;
#endif
				goto _TTX_Video_Pipeline_LUT_Update_FPGA_Exit;
			}
			else if (strncmp(rdBuf, "UNKNOWN", 7) == 0) {
				CmdAck = SERIAL_CMD_UNKNOWN;
				nError = ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
				goto _TTX_Video_Pipeline_LUT_Update_FPGA_Exit;
				// return ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
			}
		}
		else {
			// return _internal_error_Xfer(nRet);
			nError = _internal_error_Xfer(nRet);
			goto _TTX_Video_Pipeline_LUT_Update_FPGA_Exit;
		}
	}

_TTX_Video_Pipeline_LUT_Update_FPGA_Exit:
	pSerial->Clear();
	LeaveCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	return nError;
}

int16_t WINAPI TX_Video_Pipeline_LUT_Load_from_Flash(int32_t LinkNo, uint32_t dwFlashLUTIndex)
{
	CSerialPort *pSerial;
	LPVOID hSerial;

	char strCmd[25];
	size_t tmpLen;
	int16_t nError = ERROR_SCALER_SUCCESS;

	char rdBuf[256];
	UINT dwData = 0x00;
	DWORD rdBufLen, WrBufLen;
	UINT CmdAck = SERIAL_CMD_UNKNOWN; /* initial /w command-unknown*/
	INT nRet = 0;

	uint16_t wLUT;

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	if (LinkSession[LinkNo].pSerial == static_cast<CSerialPort *>INVALID_HANDLE_VALUE)
		return ERROR_SCALER_INVALID_SERIAL_HANDLE;

	if (dwFlashLUTIndex >= FLASH_LUT_INDEX_MAX)
		return ERROR_SCALER_LUT_INVALID_FLASH_INDEX;

	pSerial = LinkSession[LinkNo].pSerial;
	hSerial = static_cast<CSerialPort *>(LinkSession[LinkNo].pSerial);

	wLUT = (dwFlashLUTIndex | OP_LOAD_FLASH_TO_LUT_BUFFER);
	sprintf(strCmd, "L %X\r", wLUT);
	// wLUT = (dwFlashLUTIndex);
	// sprintf(strCmd, "L %X %X\r", wLUT, OP_LOAD_FLASH_TO_LUT_BUFFER);

	tmpLen = strlen(strCmd);
	WrBufLen = static_cast<DWORD>(tmpLen);

	EnterCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	nRet = WriteCharComm(hSerial, ((LPCSTR)(LPCTSTR)strCmd), &WrBufLen);
	if (nRet == err_internal_success)
	{
		rdBufLen = 255;
		memset(rdBuf, 0x00, 256);
		nRet = ReadLineComm(hSerial, rdBuf, &rdBufLen, 500);
		if (nRet == 0) {
			rdBuf[rdBufLen - 1] = 0x00;
			if (strncmp(rdBuf, "L OK", 4) == 0) {
				CmdAck = SERIAL_CMD_ACK_OK;
			}
			else if (strncmp(rdBuf, "ERROR", 5) == 0) {
				CmdAck = SERIAL_CMD_ACK_ERROR;
#if defined(_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_) && (_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_)
				nError = _MCU_LPUART_Error_Xfer(rdBuf);
#else
				nError = ERROR_SCALER_SPI_REG_ACCESS_FAILED;
#endif
				goto _TX_Video_Pipeline_LUT_Load_from_Flash_Exit;
			}
			else if (strncmp(rdBuf, "UNKNOWN", 7) == 0) {
				CmdAck = SERIAL_CMD_UNKNOWN;
				nError = ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
				goto _TX_Video_Pipeline_LUT_Load_from_Flash_Exit;
				// return ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
			}
		}
		else {
			// return _internal_error_Xfer(nRet);
			nError = _internal_error_Xfer(nRet);
			goto _TX_Video_Pipeline_LUT_Load_from_Flash_Exit;
		}
	}

_TX_Video_Pipeline_LUT_Load_from_Flash_Exit:
	pSerial->Clear();
	LeaveCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	return nError;
}

int16_t WINAPI TX_Video_Pipeline_LUT_Store_to_Flash(int32_t LinkNo, uint32_t dwFlashLUTIndex)
{

	CSerialPort *pSerial;
	LPVOID hSerial;

	char strCmd[25];
	size_t tmpLen;
	int16_t nError = ERROR_SCALER_SUCCESS;

	char rdBuf[256];
	UINT dwData = 0x00;
	DWORD rdBufLen, WrBufLen;
	UINT CmdAck = SERIAL_CMD_UNKNOWN; /* initial /w command-unknown*/
	INT nRet = 0;

	uint16_t wLUT;

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	if (LinkSession[LinkNo].pSerial == static_cast<CSerialPort *>INVALID_HANDLE_VALUE)
		return ERROR_SCALER_INVALID_SERIAL_HANDLE;

	if (dwFlashLUTIndex >= FLASH_LUT_INDEX_MAX)
		return ERROR_SCALER_LUT_INVALID_FLASH_INDEX;

	pSerial = LinkSession[LinkNo].pSerial;
	hSerial = static_cast<CSerialPort *>(LinkSession[LinkNo].pSerial);

	wLUT = dwFlashLUTIndex;
	sprintf(strCmd, "S %X\r", wLUT);

	tmpLen = strlen(strCmd);
	WrBufLen = static_cast<DWORD>(tmpLen);

	EnterCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	nRet = WriteCharComm(hSerial, ((LPCSTR)(LPCTSTR)strCmd), &WrBufLen);
	if (nRet == err_internal_success)
	{
		rdBufLen = 255;
		memset(rdBuf, 0x00, 256);
		nRet = ReadLineComm(hSerial, rdBuf, &rdBufLen, 500);
		if (nRet == 0) {
			rdBuf[rdBufLen - 1] = 0x00;
			if (strncmp(rdBuf, "S OK", 4) == 0) {
				CmdAck = SERIAL_CMD_ACK_OK;
			}
			else if (strncmp(rdBuf, "ERROR", 5) == 0) {
				CmdAck = SERIAL_CMD_ACK_ERROR;
#if defined(_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_) && (_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_)
				nError = _MCU_LPUART_Error_Xfer(rdBuf);
#else
				nError = ERROR_SCALER_SPI_REG_ACCESS_FAILED;
#endif
				goto _TX_Video_Pipeline_LUT_Store_to_Flash_Exit;
			}
			else if (strncmp(rdBuf, "UNKNOWN", 7) == 0) {
				CmdAck = SERIAL_CMD_UNKNOWN;
				nError = ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
				goto _TX_Video_Pipeline_LUT_Store_to_Flash_Exit;
				// return ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
			}
		}
		else {
			// return _internal_error_Xfer(nRet);
			nError = _internal_error_Xfer(nRet);
			goto _TX_Video_Pipeline_LUT_Store_to_Flash_Exit;
		}
	}

_TX_Video_Pipeline_LUT_Store_to_Flash_Exit:
	pSerial->Clear();
	LeaveCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	return nError;
}

int16_t WINAPI Set_LUTable1D_Array(int32_t LinkNo, uint32_t dwLUTSelect, LUTable1D LUTArray[], uint32_t LUTArraySize)
{
	return ERROR_SCALER_UNSUPPORTED_FUNCTION;
}

 
/* Send the command "I LUTSelect" to MCU */
int16_t WINAPI TX_Video_Pipeline_LUT_Reset_to_InitState(int32_t LinkNo, uint32_t dwLUTSelect)
{

	CSerialPort *pSerial;
	LPVOID hSerial;

	char strCmd[25];
	size_t tmpLen;
	int16_t nError = ERROR_SCALER_SUCCESS;

	char rdBuf[256];
	UINT dwData = 0x00;
	DWORD rdBufLen, WrBufLen;
	UINT CmdAck = SERIAL_CMD_UNKNOWN; /* initial /w command-unknown*/
	INT nRet = 0;

	uint16_t wLUT;

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	if (LinkSession[LinkNo].pSerial == static_cast<CSerialPort *>INVALID_HANDLE_VALUE)
		return ERROR_SCALER_INVALID_SERIAL_HANDLE;

	pSerial = LinkSession[LinkNo].pSerial;
	hSerial = static_cast<CSerialPort *>(LinkSession[LinkNo].pSerial);

	wLUT = dwLUTSelect;
	sprintf(strCmd, "I %X\r", wLUT);

	tmpLen = strlen(strCmd);
	WrBufLen = static_cast<DWORD>(tmpLen);

	EnterCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	nRet = WriteCharComm(hSerial, ((LPCSTR)(LPCTSTR)strCmd), &WrBufLen);
	if (nRet == err_internal_success)
	{
		rdBufLen = 255;
		memset(rdBuf, 0x00, 256);
		nRet = ReadLineComm(hSerial, rdBuf, &rdBufLen, 500);
		if (nRet == 0) {
			rdBuf[rdBufLen - 1] = 0x00;
			if (strncmp(rdBuf, "I OK", 4) == 0) {
				CmdAck = SERIAL_CMD_ACK_OK;
			}
			else if (strncmp(rdBuf, "ERROR", 5) == 0) {
				CmdAck = SERIAL_CMD_ACK_ERROR;
#if defined(_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_) && (_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_)
				nError = _MCU_LPUART_Error_Xfer(rdBuf);
#else
				nError = ERROR_SCALER_SPI_REG_ACCESS_FAILED;
#endif
				goto _TX_Video_Pipeline_LUT_Reset_to_InitState_Exit;
			}
			else if (strncmp(rdBuf, "UNKNOWN", 7) == 0) {
				CmdAck = SERIAL_CMD_UNKNOWN;
				nError = ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
				goto _TX_Video_Pipeline_LUT_Reset_to_InitState_Exit;
				// return ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
			}
		}
		else {
			// return _internal_error_Xfer(nRet);
			nError = _internal_error_Xfer(nRet);
			goto _TX_Video_Pipeline_LUT_Reset_to_InitState_Exit;
		}
	}

_TX_Video_Pipeline_LUT_Reset_to_InitState_Exit:

	pSerial->Clear();
	LeaveCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	return nError;
}


int16_t WINAPI TX_Video_Pipeline_LUT_Enable(int32_t LinkNo, uint32_t dwLUTSelect, bool bLUTEanble)
{

	uint32_t dwData, dwControl;
	uint32_t dwMaskLUTBypass, dwLUTEnable;
	int16_t nError = ERROR_SCALER_SUCCESS;

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	nError = SPI_Register_Read(LinkNo, OFFSET_TX_VIDEO_CONTROL_0, &dwControl);
	if (nError != ERROR_SCALER_SUCCESS)
		return nError;

	switch (dwLUTSelect)
	{
#if defined(_FPGA_SUPPORT_1D_DEGAMMA_LUT_) && (_FPGA_SUPPORT_1D_DEGAMMA_LUT_ == 1)

#if defined(_FPGA_SUPPORT_1D_DEGAMMA_TEST_LUT_) && (_FPGA_SUPPORT_1D_DEGAMMA_TEST_LUT_ == 1)
	case SELECT_1D_LUT_DEGAMMA_TEST:
		dwMaskLUTBypass = TX_VIDEO_PIPELINE_1D_LUT_DEGAMMA_TEST_BYPASS_MASK;
		dwLUTEnable = TX_VIDEO_PIPELINE_1D_LUT_DEGAMMA_TEST_ENABLE;
		break;

#endif /* _FPGA_SUPPORT_1D_DEGAMMA_TEST_LUT_ */

	case SELECT_1D_LUT_GAMMA:
		dwMaskLUTBypass = TX_VIDEO_PIPELINE_1D_LUT_BYPASS_MASK;
		dwLUTEnable = TX_VIDEO_PIPELINE_1D_LUT_ENABLE;

		if ((dwControl & TX_VIDEO_PIPELINE_MAIN_SCREEN_MASK) != TX_VIDEO_PIPELINE_MAIN_SCREEN_GAMMA_LUT) {
			dwMaskLUTBypass = (dwMaskLUTBypass | TX_VIDEO_PIPELINE_MAIN_SCREEN_MASK);
			dwLUTEnable = (dwLUTEnable | TX_VIDEO_PIPELINE_MAIN_SCREEN_GAMMA_LUT);
		}

		break;

	case SELECT_1D_LUT_DICOM:
		dwMaskLUTBypass = TX_VIDEO_PIPELINE_1D_LUT_BYPASS_MASK;
		dwLUTEnable = TX_VIDEO_PIPELINE_1D_LUT_ENABLE;

		if ((dwControl & TX_VIDEO_PIPELINE_MAIN_SCREEN_MASK) != TX_VIDEO_PIPELINE_MAIN_SCREEN_DICOM_LUT) {
			dwMaskLUTBypass = (dwMaskLUTBypass | TX_VIDEO_PIPELINE_MAIN_SCREEN_MASK);
			dwLUTEnable = (dwLUTEnable | TX_VIDEO_PIPELINE_MAIN_SCREEN_DICOM_LUT);
		}

		break;
#if defined(_FPGA_SUPPORT_3D_LUT_) && (_FPGA_SUPPORT_3D_LUT_ == 1)
	case SELECT_3D_LUT:
		dwMaskLUTBypass = TX_VIDEO_PIPELINE_3D_LUT_BYPASS_MASK;
		dwLUTEnable = TX_VIDEO_PIPELINE_3D_LUT_ENABLE;
		break;
#endif

	case SELECT_1D_LUT_DEGAMMA:
		dwMaskLUTBypass = TX_VIDEO_PIPELINE_1D_LUT_DEGAMMA_BYPASS_MASK;
		dwLUTEnable = TX_VIDEO_PIPELINE_1D_LUT_DEGAMMA_ENABLE;
		break;

#else /* ! _FPGA_SUPPORT_1D_DEGAMMA_LUT_ */
	case SELECT_1D_LUT_GAMMA:
		dwMaskLUTBypass = TX_VIDEO_PIPELINE_1D_LUT_BYPASS_MASK;
		dwLUTEnable = TX_VIDEO_PIPELINE_1D_LUT_ENABLE;

		if ((dwControl & TX_VIDEO_PIPELINE_MAIN_SCREEN_MASK) != TX_VIDEO_PIPELINE_MAIN_SCREEN_GAMMA_LUT) {
			dwMaskLUTBypass = (dwMaskLUTBypass | TX_VIDEO_PIPELINE_MAIN_SCREEN_MASK);
			dwLUTEnable = (dwLUTEnable | TX_VIDEO_PIPELINE_MAIN_SCREEN_GAMMA_LUT);
		}

		break;

	case SELECT_1D_LUT_DICOM:
		dwMaskLUTBypass = TX_VIDEO_PIPELINE_1D_LUT_BYPASS_MASK;
		dwLUTEnable = TX_VIDEO_PIPELINE_1D_LUT_ENABLE;

		if ((dwControl & TX_VIDEO_PIPELINE_MAIN_SCREEN_MASK) != TX_VIDEO_PIPELINE_MAIN_SCREEN_DICOM_LUT) {
			dwMaskLUTBypass = (dwMaskLUTBypass | TX_VIDEO_PIPELINE_MAIN_SCREEN_MASK);
			dwLUTEnable = (dwLUTEnable | TX_VIDEO_PIPELINE_MAIN_SCREEN_DICOM_LUT);
		}

		break;
#if defined(_FPGA_SUPPORT_3D_LUT_) && (_FPGA_SUPPORT_3D_LUT_ == 1)
	case SELECT_3D_LUT:
		dwMaskLUTBypass = TX_VIDEO_PIPELINE_3D_LUT_BYPASS_MASK;
		dwLUTEnable = TX_VIDEO_PIPELINE_3D_LUT_ENABLE;
		break;
#endif
	case SELECT_1D_LUT_GAMMA_ADJUST:
		dwMaskLUTBypass = TX_VIDEO_PIPELINE_1D_LUT_GAMMA_ADJ_BYPASS_MASK;
		dwLUTEnable = TX_VIDEO_PIPELINE_1D_LUT_GAMMA_ADJ_ENABLE;
		break;
#endif /* _FPGA_SUPPORT_1D_DEGAMMA_LUT_ */

	default:
		nError = ERROR_SCALER_LUT_INVALID_SELECT_LUT;
		return nError;

	}

	if (bLUTEanble == false) { /* bypass the incoming signal*/

#if defined(DELAY_BETWEEN_COMMAND_SENDING) && DELAY_BETWEEN_COMMAND_SENDING
		Sleep(SLEEP_BETWEEN_COMMAND_SENDING);
#endif
		dwData = (dwControl & ~(TX_VIDEO_PIPELINE_ALL_LUT_MASK));
		nError = SPI_Register_Write(LinkNo, OFFSET_TX_VIDEO_CONTROL_0, dwData);
		if (nError != ERROR_SCALER_SUCCESS)
			return nError;


	}
	else if (bLUTEanble == true) { /* enable the LUT in TX Video-Pipeline */

		dwData = (dwControl & ~(dwMaskLUTBypass) );
		dwData = (dwData | dwLUTEnable);
		nError = SPI_Register_Write(LinkNo, OFFSET_TX_VIDEO_CONTROL_0, dwData);
		if (nError != ERROR_SCALER_SUCCESS)
			return nError;

	}

	return nError; /* ERROR_SCALER_SUCCESS;*/
}

LPVOID WINAPI OpenComm(
	LPCSTR lpszPortNum, 
	DWORD  dwBaudRate,
	BYTE   byParity,
	BYTE   byStopBits,
	BYTE   byByteSize)
{
	CSerialPort *pSerial = new CSerialPort();
	if (pSerial->Open(lpszPortNum, dwBaudRate, byParity, byStopBits, byByteSize)){
		return pSerial;
	} else {
		delete pSerial;
		return INVALID_HANDLE_VALUE;
	}
}

void WINAPI CloseComm(LPVOID lpComm)
{
	if (lpComm != INVALID_HANDLE_VALUE){
		//CSerialPort *pSerial = (CSerialPort*)lpComm;
		CSerialPort *pSerial = static_cast<CSerialPort *>(lpComm);
		if (NULL != pSerial) {

			pSerial->Flush();
			pSerial->Clear();

			pSerial->Close();
			delete pSerial;
		}
	}
}

DWORD WINAPI WriteComm(LPVOID lpComm, LPCSTR lpData, DWORD dwLen)
{
	if (lpComm != INVALID_HANDLE_VALUE){
		//CSerialPort *pSerial = (CSerialPort*)lpComm;
		CSerialPort *pSerial = static_cast<CSerialPort *>(lpComm);
		DWORD dwResult = pSerial->WriteData(lpData, dwLen);
		if (dwResult > 0 && dwResult < (DWORD)-1){
			pSerial->Flush();
			pSerial->Clear();
		}
		return dwResult;
	}
	return (0);
}

DWORD WINAPI ReadComm(LPVOID lpComm, LPSTR lpDest, DWORD dwLen, DWORD dwMaxWait)
{
	if (lpComm != INVALID_HANDLE_VALUE){
		//CSerialPort *pSerial = (CSerialPort*)lpComm;
		char *buffer = new char[dwLen];
		CSerialPort *pSerial = static_cast<CSerialPort *>(lpComm);
		DWORD dwResult = pSerial->ReadData(buffer, dwLen, dwMaxWait);
		if (dwResult > 0 && dwResult <= dwLen){
			//strncpy(lpDest, buffer, dwResult);
			strncpy_s(lpDest, dwLen, buffer, dwResult);
		}

		if (buffer != NULL)
			delete buffer;

		return dwResult;
	}
	return (0);
}

INT WINAPI WriteCharComm(LPVOID lpComm, LPCSTR lpData, DWORD *pdwLen)
{
	CSerialPort *pSerial = NULL;

	CHAR bChar;
	DWORD dwResult;
	DWORD iter;
	DWORD dwLen = 0;

	INT nRet = err_internal_success;

	if (lpComm != INVALID_HANDLE_VALUE) {
		//CSerialPort *pSerial = (CSerialPort*)lpComm;
		pSerial = static_cast<CSerialPort *>(lpComm);

		if (pdwLen != NULL) {
			dwLen = *pdwLen;
		}

		for (iter = 0; iter < dwLen; iter++) {
			bChar = lpData[iter];

			dwResult = pSerial->WriteData(&bChar, (DWORD)1);
			if (dwResult == (DWORD)-1) { /* failed to transfer single-byte */
				nRet = err_internal_serial_send_fail;
				break;
			}

#if defined(DELAY_BETWEEN_CHARACTER_SENDING) && DELAY_BETWEEN_CHARACTER_SENDING
			/* no delay is required for latest-character */
			if(iter < (dwLen-1) )
				us_delay(INTERVAL_BETWEEN_CHARACTER_SENDING); /* busy-loop for 500 micro-second delay */
#endif
		}
	}

	pSerial->Flush();
	/* Clear() may destory the TX/RX buffer, remove it */
	//	pSerial->Clear(); 

	if (pdwLen != NULL) {
		*pdwLen = iter;
	}

	if (iter == dwLen) {
		nRet = err_internal_success;
	}


	return nRet;
}

INT WINAPI ReadLineComm(LPVOID lpComm, LPSTR lpDest, DWORD* pdwLen, DWORD dwMaxWait)
{
	char *rdBuf = NULL;
	INT nRet = err_internal_success;

	DWORD dwResult, rdBufLen, RcvLen;
	BOOL bReadLine;
	DWORD dwLen = 0;
	char *tmpBuf;

	if (lpComm != INVALID_HANDLE_VALUE) {

		//CSerialPort *pSerial = (CSerialPort*)lpComm;
		CSerialPort *pSerial = static_cast<CSerialPort *>(lpComm);

		if (pdwLen != NULL) {
			dwLen = *pdwLen;
		}

		if (dwLen > 0) {
			rdBuf = new char[dwLen];

			if (rdBuf == NULL) {
				nRet = err_internal_new_rcvbuf_fail;
				goto _ReadLineComm_Exit;
			}
		}
		else {
			nRet = err_internal_invalid_rcvbuf_size;
			goto _ReadLineComm_Exit;
		}

		memset(rdBuf, 0x00, dwLen);
		rdBufLen = 255;

		bReadLine = FALSE;
		RcvLen = 0;

		nRet = err_internal_success;
		do {
			tmpBuf = &(rdBuf[RcvLen]);
			dwResult = pSerial->ReadData(tmpBuf, rdBufLen, dwMaxWait);
			if ( (dwResult > 0) && (dwResult <= dwLen) ) {
				RcvLen += dwResult;
				rdBufLen -= dwResult;

				if (rdBuf[RcvLen - 1] == '\n') {
					bReadLine = TRUE;
					rdBuf[RcvLen - 1] = 0x00;
				}
			}
			else if (dwResult > rdBufLen) {
				switch (dwResult) {
				case WAIT_FAILED:
					nRet = err_internal_serial_read_wait_fail;
					break;

				case WAIT_TIMEOUT:
					nRet = err_internal_serial_read_timeout;
					break;
				}

				break; /* error */
			}

		} while ( (bReadLine == FALSE) && (nRet == err_internal_success) );

		if (bReadLine == TRUE)
		{
			strncpy_s(lpDest, dwLen, rdBuf, RcvLen);
			if (pdwLen != NULL) {
				*pdwLen = RcvLen;
			}

		}

	}
	else {
		return err_internal_invalid_comm_handle;
	}

_ReadLineComm_Exit:
	if (rdBuf != NULL)
		delete rdBuf;

	return nRet;
}

#if 0
int16_t WINAPI TX_Video_Pipeline_LUT_Load_and_Activate(int32_t LinkNo, uint32_t dwLUTSelect, uint16_t wLoad_Activate_Readback)
{
	CSerialPort *pSerial;
	LPVOID hSerial;

	char strCmd[25];
	size_t tmpLen;
	int16_t nError = ERROR_SCALER_SUCCESS;

	char rdBuf[256];
	UINT dwData = 0x00;
	DWORD rdBufLen, WrBufLen;
	UINT CmdAck = SERIAL_CMD_UNKNOWN; /* initial /w command-unknown*/
	INT nRet = 0;

	uint16_t wLUT;

	if (LinkNo >= MCU_LINK_SESSION_MAX)
		return ERROR_SCALER_INVALID_LINK_NUM;

	if (LinkSession[LinkNo].pSerial == static_cast<CSerialPort *>INVALID_HANDLE_VALUE)
		return ERROR_SCALER_INVALID_SERIAL_HANDLE;

	if (wLoad_Activate_Readback & ~(LUT_LOAD_AND_ACTIVATE_OPERATION_MASK))
		return ERROR_SCALER_INVALID_PARAMETER;

	pSerial = LinkSession[LinkNo].pSerial;
	hSerial = static_cast<CSerialPort *>(LinkSession[LinkNo].pSerial);


	switch (dwLUTSelect) {
	case SELECT_1D_LUT_GAMMA_ADJUST:
		wLUT = (SELECT_1D_LUT_GAMMA_ADJUST);
		break;

	case SELECT_1D_LUT_DICOM:
		wLUT = (SELECT_1D_LUT_DICOM);
		break;
#if defined(_FPGA_SUPPORT_3D_LUT_) && (_FPGA_SUPPORT_3D_LUT_ == 1)
	case SELECT_3D_LUT:
		wLUT = (SELECT_3D_LUT);
		break;
#endif
	case SELECT_1D_LUT_GAMMA:
	default:
		wLUT = (SELECT_1D_LUT_GAMMA);
		break;
	}

	wLUT = (wLUT | wLoad_Activate_Readback);
	sprintf(strCmd, "L %X\r", wLUT);
	// sprintf(strCmd, "L %X %X\r", wLUT, wLoad_Activate_Readback);

	tmpLen = strlen(strCmd);
	WrBufLen = static_cast<DWORD>(tmpLen);

	EnterCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	nRet = WriteCharComm(hSerial, ((LPCSTR)(LPCTSTR)strCmd), &WrBufLen);
	if (nRet == err_internal_success)
	{
		rdBufLen = 255;
		memset(rdBuf, 0x00, 256);
		nRet = ReadLineComm(hSerial, rdBuf, &rdBufLen, 500);
		if (nRet == 0) {
			rdBuf[rdBufLen - 1] = 0x00;
			if (strncmp(rdBuf, "L OK", 4) == 0) {
				CmdAck = SERIAL_CMD_ACK_OK;
			}
			else if (strncmp(rdBuf, "ERROR", 5) == 0) {
				CmdAck = SERIAL_CMD_ACK_ERROR;
#if defined(_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_) && (_NEW_FIRMWARE_SUPPORT_MCU_LPUART_ERROR_)
				nError = _MCU_LPUART_Error_Xfer(rdBuf);
#else
				nError = ERROR_SCALER_SPI_REG_ACCESS_FAILED;
#endif
				goto _TX_Video_Pipeline_LUT_Load_and_Activate_Exit;
			}
			else if (strncmp(rdBuf, "UNKNOWN", 7) == 0) {
				CmdAck = SERIAL_CMD_UNKNOWN;
				nError = ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
				goto _TX_Video_Pipeline_LUT_Load_and_Activate_Exit;
				// return ERROR_SCALER_SERIAL_UNKNOWN_COMMAND;
			}
		}
		else {
			// return _internal_error_Xfer(nRet);
			nError = _internal_error_Xfer(nRet);
			goto _TX_Video_Pipeline_LUT_Load_and_Activate_Exit;
		}
	}

_TX_Video_Pipeline_LUT_Load_and_Activate_Exit:
	pSerial->Clear();
	LeaveCriticalSection(&(LinkSession[LinkNo].CriticalSection));
	return nError;
}

int16_t WINAPI Set_LUTable1D_Array(int32_t LinkNo, uint32_t dwLUTSelect, LUTable1D LUTArray[], uint32_t LUTArraySize)
{
	int16_t nError = ERROR_SCALER_SUCCESS;
	uint32_t iter;

	if ((LUTArraySize == 0) || (LUTArraySize > 0x400))
		return ERROR_SCALER_LUT_INVALID_ARRAY_SIZE;

	/* Bypass 1D_LUT in TX Pipeline */
	nError = TX_Video_Pipeline_LUT_Enable(LinkNo, dwLUTSelect, false);
	if (nError != ERROR_SCALER_SUCCESS)
		return nError;

	for (iter = 0; iter < LUTArraySize; iter++) {
		nError = Set_LUTable1D_Element(LinkNo, dwLUTSelect, &(LUTArray[iter]));

		if (nError != ERROR_SCALER_SUCCESS)
			break;
	}

	if (nError != ERROR_SCALER_SUCCESS)
		goto _Set_LUTable1D_Array_Exit;

	/* Enbale 1D_LUT in TX Pipeline*/
	nError = TX_Video_Pipeline_LUT_Enable(LinkNo, dwLUTSelect, true);
	if (nError != ERROR_SCALER_SUCCESS)
		return nError;

_Set_LUTable1D_Array_Exit:
	return nError;
}

#endif