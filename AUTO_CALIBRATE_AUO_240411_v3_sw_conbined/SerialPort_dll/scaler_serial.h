
#ifndef SCALER_SERIAL_H
#define SCALER_SERIAL_H

#include <stdint.h>

#ifdef SERIAL_EXPORTS
#define SERIAL_API __declspec(dllexport)
#else
#define SERIAL_API __declspec(dllimport)
#endif

#include "predefine.h"

typedef struct _RGBColor {
	uint16_t  Red;
	uint16_t  Green;
	uint16_t  Blue;
} RGBColor, *PRGBColor;

typedef struct _LUTable1D {
	uint32_t  TableIndex;
	RGBColor rgbColor;
} LUTable1D, *PLUTable1D;

#if defined(_FPGA_SUPPORT_3D_LUT_) && (_FPGA_SUPPORT_3D_LUT_ == 1)

typedef struct _LUTable3D {
	uint32_t  _3DTableIndex;
	uint32_t  _3DrgbColor;
} LUTable3D, *PLUTable3D;

#endif /* _FPGA_SUPPORT_3D_LUT_ */

#define ERROR_RETURN_WITH_ON_OPERATION            1

#define ERROR_SCALER_SUCCESS                      0

#define ERROR_SCALER_INVALID_LINK_NUM            -1
#define ERROR_SCALER_LINK_NUM_DUPLICATED         -2
#define ERROR_SCALER_INVALID_SERIAL_HANDLE       -3

#define ERROR_SCALER_INVALID_PARAMETER           -11

#define ERROR_SCALER_SERIAL_OPEN_DEVICE_FAILED   -21
#define ERROR_SCALER_SERIAL_SEND_CMD_FAILED      -22
#define ERROR_SCALER_SERIAL_UNKNOWN_COMMAND      -23
#define ERROR_SCALER_SERIAL_RESPONS_TIMEOUT      -24
#define ERROR_SCALER_SERIAL_WAIT_FAILED          -25
#define ERROR_SCALER_SERIAL_INVALID_RCVBUF_SIZE  -26
#define ERROR_SCALER_SERIAL_RCVBUF_ALLOCATE_FAILED -27


#define ERROR_SCALER_SPI_REG_ACCESS_FAILED       -31

#define ERROR_SCALER_LUT_RESET_TIMEOUT           -51
#define ERROR_SCALER_LUT_INVALID_INDEX           -52
#define ERROR_SCALER_LUT_INVALID_FLASH_INDEX     -53
#define ERROR_SCALER_LUT_INVALID_SELECT_LUT      -54
#define ERROR_SCALER_LUT_INVALID_ARRAY_SIZE      -55
#define ERROR_SCALER_LUT_INVALID_OPERATION       -56
#define ERROR_SCALER_LUT_INVALID_DEGAMMA_DATA    -57

#define ERROR_SCALER_3DLUT_INVALID_DATA          -61

#define ERROR_SCALER_UNDEFINED_INTERNAL_ERROR    -101
#define ERROR_SCALER_UNSUPPORTED_FUNCTION        -102

#define ERROR_MCU_LPUART_TX_BUSY                 -201
#define ERROR_MCU_LPUART_RX_BUSY                 -202
#define ERROR_MCU_LPUART_TX_IDLE                 -203
#define ERROR_MCU_LPUART_RX_IDLE                 -204
#define ERROR_MCU_LPUART_TX_WATERMARK_TOO_LARGE  -205
#define ERROR_MCU_LPUART_RX_WATERMARK_TOO_LARGE  -206
#define ERROR_MCU_LPUART_FLAG_CONNOT_CLEAR_MANUALLY   -207
#define ERROR_MCU_LPUART_RX_RING_BUFFER_OVERRUN  -208
#define ERROR_MCU_LPUART_RX_HARDWARE_OVERRUN     -209
#define ERROR_MCU_LPUART_NOISE_ERROR             -210
#define ERROR_MCU_LPUART_FRAMING_ERROR           -211
#define ERROR_MCU_LPUART_PARITY_ERROR            -212
#define ERROR_MCU_LPUART_BAUDRATE_NOT_SUPPORT    -213
#define ERROR_MCU_LPUART_IDLE_LINE_DETECTED      -214
#define ERROR_MCU_LPUART_TIMEOUT                 -215
#define ERROR_MCU_LPUART_UNKNOWN                 -220

/* _NEW_ERROR_CODE_4_LUT_UPDATE_ */
#define ERROR_MCU_LUT_INVALID_SELECT_LUT             -260

#define ERROR_MCU_LUT_PIPELINE_STATE_IS_NOT_BYPASS   -261
#define ERROR_MCU_LUT_RED_READBACK_NOT_IDENTICAL     -262
#define ERROR_MCU_LUT_GREEN_READBACK_NOT_IDENTICAL   -263
#define ERROR_MCU_LUT_BLUE_READBACK_NOT_IDENTICAL    -264

/* _NEW_ERROR_CODE_4_LUT_FLASH_SAVE_LOAD_ */
#define ERORR_MCU_LUT_FLASH_ERASE_SECTOR_FAILED      -265
#define ERORR_MCU_LUT_SAVE_TO_FLASH_FAILED           -266
#define ERROR_MCU_LUT_FLASH_READBACK_NOT_IDENTICAL   -267

/* _NEW_ERROR_CODE_4_3DLUT_FLASH_SAVE_LOAD_ */
#define ERROR_MCU_3D_LUT_INVALID_ENTRY_INDEX         -271

#define ERROR_MCU_3D_LUT_READBACK_NOT_IDENTICAL      -279

#define VIDEO_PIPELINE_LUT_BYPASS                    0x0
#define VIDEO_PIPELINE_LUT_ENABLE                    0x1


#if defined(_FPGA_SUPPORT_1D_DEGAMMA_LUT_) && (_FPGA_SUPPORT_1D_DEGAMMA_LUT_ == 1)

#if defined(_SUPPORT_FPGA_DATECODE_1206_ABOVE_) && ( _SUPPORT_FPGA_DATECODE_1206_ABOVE_== 1)

#define SELECT_1D_LUT_ENGAMMA                        0x0
#define SELECT_1D_LUT_DEGAMMA                        0x1
#define SELECT_1D_LUT_DICOM                          0x2
#if defined(_FPGA_SUPPORT_3D_LUT_) && (_FPGA_SUPPORT_3D_LUT_ == 1)
#define SELECT_3D_LUT                                0x3
#endif /* _FPGA_SUPPORT_3D_LUT_ */

#if defined(_FPGA_SUPPORT_1D_CCT_LUT_) && (_FPGA_SUPPORT_1D_CCT_LUT_ == 1)
#define SELECT_1D_LUT_CCT							 0x4
#endif /* _FPGA_SUPPORT_1D_CCT_LUT_ */

#if defined(_FPGA_SUPPORT_1D_EXTRA_LUT_) && (_FPGA_SUPPORT_1D_EXTRA_LUT_ == 1)
#define SELECT_1D_LUT_EXTRA							 0x5
#endif /* _FPGA_SUPPORT_1D_EXTRA_LUT_ */

#else /* ! _SUPPORT_FPGA_DATECODE_1206_ABOVE_ */
#define SELECT_1D_LUT_GAMMA                          0x0
#define SELECT_1D_LUT_DEGAMMA                        0x1
#define SELECT_1D_LUT_DICOM                          0x2

#if defined(_FPGA_SUPPORT_3D_LUT_) && (_FPGA_SUPPORT_3D_LUT_ == 1)
#define SELECT_3D_LUT                                0x3
#endif /* _FPGA_SUPPORT_3D_LUT_ */

#if defined(_FPGA_SUPPORT_1D_DEGAMMA_TEST_LUT_) && (_FPGA_SUPPORT_1D_DEGAMMA_TEST_LUT_ == 1)
#define SELECT_1D_LUT_DEGAMMA_TEST                   0x4
#endif /* _FPGA_SUPPORT_1D_DEGAMMA_TEST_LUT_ */

#endif /* _SUPPORT_FPGA_DATECODE_1206_ABOVE_ */

#endif /* _FPGA_SUPPORT_1D_DEGAMMA_LUT_ */

#define SELECT_1D_LUT_TO_BE_RESET                    0x0
#if defined(_FPGA_SUPPORT_3D_LUT_) && (_FPGA_SUPPORT_3D_LUT_ == 1)
#define SELECT_3D_LUT_TO_BE_RESET                    0x3
#endif


/* Individual API for LUT_LOAD_TO_BUFFER_MASK */

#ifdef __cplusplus
extern "C" {
#endif

	LPVOID WINAPI OpenComm(LPCSTR lpszPortNum,
		DWORD  dwBaudRate = CBR_115200,
		BYTE   byParity = NOPARITY,
		BYTE   byStopBits = ONESTOPBIT,
		BYTE   byByteSize = 8);

	void WINAPI CloseComm(LPVOID lpComm);

	DWORD WINAPI WriteComm(LPVOID lpComm, LPCSTR lpData, DWORD dwLen);
	DWORD WINAPI ReadComm(LPVOID lpComm, LPSTR lpDest, DWORD dwLen, DWORD dwMaxWait);
	INT WINAPI ReadLineComm(LPVOID lpComm, LPSTR lpDest, DWORD* pdwLen, DWORD dwMaxWait);
	INT WINAPI WriteCharComm(LPVOID lpComm, LPCSTR lpData, DWORD *pdwLen);

  int16_t WINAPI Get_Library_Version(uint16_t wLibVersion[4]);
  
	int16_t WINAPI Connect_to_ScalerBoard(char * pStrCOM, int32_t* pLinkNo);
	int16_t WINAPI Disconnect_ScalerBoard(int32_t LinkNo);

	int16_t WINAPI Adjust_BackLight_PWM(int32_t LinkNo, uint32_t DutyCycle, uint8_t bPWMEn = 1);

	int16_t WINAPI Generate_RGB_Pattern(int32_t LinkNo, uint16_t Red, uint16_t Green, uint16_t Blue, uint32_t LUT4TestPattern = 0x00000000 );

	int16_t WINAPI TX_Video_Pipeline_LUT_Reset_to_InitState(int32_t LinkNo, uint32_t dwLUTSelect);

	int16_t WINAPI Set_LUTable1D_Element(int32_t LinkNo, uint32_t dwLUTSelect, LUTable1D* pLUTElement);
	int16_t WINAPI Set_LUTable1D_Array(int32_t LinkNo, uint32_t dwLUTSelect, LUTable1D LUTArray[], uint32_t LUTArraySize);

	int16_t WINAPI Xfer_LUTable1D_U32Data(int32_t LinkNo, uint32_t dwLUTIndex, uint32_t dwRGBData);
	int16_t WINAPI Xfer_LUTable1D_Element(int32_t LinkNo, uint32_t dwLUTIndex, RGBColor *pColorElement);

	int16_t WINAPI Xfer_LUTable3D_U32Data(int32_t LinkNo, uint32_t dwLUTIndex, uint32_t dwRGBData);

	int16_t WINAPI TX_Video_Pipeline_LUT_Actiate_FPGA(int32_t LinkNo, uint32_t dwFPGALUTSelect);
	int16_t WINAPI TX_Video_Pipeline_LUT_Activate_FPGA(int32_t LinkNo, uint32_t dwFPGALUTSelect);
	
	int16_t WINAPI TX_Video_Pipeline_LUT_Update_FPGA(int32_t LinkNo, uint32_t dwFPGALUTSelect, bool bReadback_checking);
	int16_t WINAPI TX_Video_Pipeline_LUT_Load_from_Flash(int32_t LinkNo, uint32_t dwFlashLUTIndex);

	int16_t WINAPI TX_Video_Pipeline_LUT_Store_to_Flash(int32_t LinkNo, uint32_t dwFlashLUTIndex);

	int16_t WINAPI TX_Video_Pipeline_LUT_Enable(int32_t LinkNo, uint32_t dwLUTSelect, bool bLUTEanble);

	int16_t WINAPI Load_LUTBuffer_from_Flash_Calibrate_Section(int32_t LinkNo, uint32_t dwFlashLUTIndex);
	int16_t WINAPI Save_LUTBuffer_to_Flash_Calibrate_Section(int32_t LinkNo, uint32_t dwFlashLUTIndex);

	int16_t WINAPI FPGA_Datecode_Get(int32_t LinkNo, uint32_t* pDatecode1, uint32_t* pDatecode2);
	int16_t WINAPI MCU_SysTick_Get(int32_t LinkNo, uint32_t* pSysTick);

	int16_t WINAPI SPI_Register_Write(int32_t LinkNo, uint32_t Offset, uint32_t Value);
	int16_t WINAPI SPI_Register_Read(int32_t LinkNo, uint32_t Offset, uint32_t* pValue);

	int16_t WINAPI Initial_TX_Control(int32_t LinkNo, uint32_t TXControl = 0x0100);

	int16_t WINAPI Generate_RGB_Pattern_Final_Stage(int32_t LinkNo, uint16_t Red, uint16_t Green, uint16_t Blue);
#ifdef __cplusplus
}
#endif


#endif /* SCALER_SERIAL_H */
