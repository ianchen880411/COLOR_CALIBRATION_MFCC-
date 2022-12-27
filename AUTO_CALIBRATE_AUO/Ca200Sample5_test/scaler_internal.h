
#ifndef SCALER_SERIAL_INTERNAL_H
#define SCALER_SERIAL_INTERNAL_H

/* Internal Error */
#define err_internal_success                    0

#define err_internal_serial_ack_ok             -1001
#define err_internal_serial_ack_error          -1002
#define err_internal_serial_unknown            -1003

#define err_internal_invalid_rcvbuf_size       -1005

#define err_internal_invalid_comm_handle       -1011

#define err_internal_new_rcvbuf_fail           -1021

#define err_internal_serial_send_fail          -1031

#define err_internal_serial_read_timeout       -1041
#define err_internal_serial_read_wait_fail     -1042



#define SERIAL_CMD_ACK_OK         1
#define SERIAL_CMD_ACK_ERROR      2
#define SERIAL_CMD_UNKNOWN        3


/* MASK / SETTINGS between MCU and Middle-ware*/
#define SHIFT_BITS_SELECT_LUT                        12

#define LUT_ACTIVATE_FPGA_LUT_MASK                   0x1000
#define LUT_UPDATE_LUT_BUFFER_TO_FPGA_MASK           0x2000
#define LUT_LOAD_TO_BUFFER_MASK                      0x4000
#define LUT_READBACK_CHECK_MASK                      0x8000
#define LUT_LOAD_AND_ACTIVATE_OPERATION_MASK         (LUT_LOAD_TO_BUFFER_MASK | LUT_UPDATE_LUT_BUFFER_TO_FPGA_MASK | LUT_ACTIVATE_FPGA_LUT_MASK | LUT_READBACK_CHECK_MASK)


#define OP_ACTIVATE_FPGA_LUT                     LUT_ACTIVATE_FPGA_LUT_MASK
#define OP_UPDATE_LUT_BUFFER_TO_FPGA             LUT_UPDATE_LUT_BUFFER_TO_FPGA_MASK
#define OP_LOAD_FLASH_TO_LUT_BUFFER              LUT_LOAD_TO_BUFFER_MASK
#define OP_LUT_SET_AND_READBACK_CHECK            LUT_READBACK_CHECK_MASK

#define OP_LOAD_AND_ACTIVATE_LUT_BUFFER          (OP_LOAD_FLASH_TO_LUT_BUFFER | OP_UPDATE_LUT_BUFFER_TO_FPGA | OP_ACTIVATE_FPGA_LUT)

/* Register Offset */
/*
* private_fpga.h
*
*  Created on: 2020¦~10¤ë12¤é
*      Author: blackcat
*/

#ifndef PRIVATE_FPGA_H_
#define PRIVATE_FPGA_H_

#define VLGL_TICK_NUMBERS                                           5  /* 5 ticks => 5 millisecond */
// #define BYTES_PER_PIXEL                                             4  /* format in each pixel : WRGB one byte each for white, red, green, blue (MSB -- LSB) */
#define BYTES_PER_PIXEL                                             1  /* format in each pixel : 8bits for single-pixel(RGB332) */

#define BASE_ADDRESS_FPGA_LOCAL_REGISTERS                           0x06000000 /* changed in release-20210507, 0x80140000*/


#define OFFSET_FPGA_GLOBAL_CONTROL                                  0x00000000 /* Read-Write */
#define OFFSET_DDR_CALIBRATION_STATUS                               0x00000008 /* Read-Only */

//#define OFFSET_OSDCotroller_CONTROL_STATUS                          0x00000000

#define OFFSET_OSD_DETECT_DIMENSION                                 0x00000090 /* Read-Only */
#define OFFSET_OSD_DETECT_FRAME_RATE                                0x00000094 /* Read-Only */
#define OFFSET_OSD_DETECT_FRAME_CLOCK                               0x00000098 /* Read-Only */



#define OFFSET_OSD_FRAMEBUF_BEGIN_COORDINATE                        0x00000004 /* horizontal-offset should be multiple of 64 */
#define OFFSET_OSD_FRAMEBUF_PIXELS                                  0x00000008 /* pixels in horizontal should be multiple of 64 */
#define OFFSET_FRAMEBUF_ADDRESS_OSDCotroller_READ_FROM              0x0000000C

#define OFFSET_V_BY_ONE_TX_CONTROL                                  0x000000C4  /* Read-Write */

#define OFFSET_TX_DISPLAY_CONTROL                                   0x000000D0  /* Read-Write */

#define OFFSET_TX_OSD_SCREEN_DIMENSION                              0x000000D8  /* Read-Write */
#define OFFSET_TX_OSD_SCREEN_LOCATION                               0x000000DC  /* Read-Write */

#define OFFSET_TX_MAIN_SCREEN_DIMENSION                             0x000000E0  /* Read-Write */
#define OFFSET_TX_MAIN_SCREEN_LOCATION                              0x000000E4  /* Read-Write */

#define OFFSET_TX_SUB_SCREEN_DIMENSION                              0x000000E8  /* Read-Write */
#define OFFSET_TX_SUB_SCREEN_LOCATION                               0x000000EC  /* Read-Write */

#define OFFSET_TX_VIDEO_CONTROL_0                                   0x00000100  /* Read-Write */

#define OFFSET_TX_TEST_PATTERN_CONTROL                              0x00000140  /* Read-Write */



#define OFFSET_VIDEO_LUT_CONTROL_0                                  0x00000180  /* Read-Only:bit-31/29/27, others are Read-Write */
#define OFFSET_VIDEO_LUT_CONTROL_1                                  0x00000184  /* Read-Write */
#define OFFSET_VIDEO_LUT_CONTROL_2                                  0x00000188  /* Read-Write */
#define OFFSET_VIDEO_LUT_STATUS_0                                   0x0000018C  /* Read-Only */

#define OFFSET_COLORMATRIX_RedRed_RedGreen                          0x000001E0  /* Read-Write */
#define OFFSET_COLORMATRIX_RedBlue                                  0x000001E4  /* Read-Write */

#define OFFSET_COLORMATRIX_GreenRed_GreenGreen                      0x000001E8  /* Read-Write */
#define OFFSET_COLORMATRIX_GreenBlue                                0x000001EC  /* Read-Write */

#define OFFSET_COLORMATRIX_BlueRed_BlueGreen                        0x000001F0  /* Read-Write */
#define OFFSET_COLORMATRIX_BlueBlue                                 0x000001F4  /* Read-Write */

#define OFFSET_OSDColor_LUT_CONTROL                                 0x00000210  /* Read-Write */
#define OFFSET_OSDColor_LUT_DATA_4_WRITE                            0x00000214  /* Read-Write */
#define OFFSET_OSDColor_LUT_DATA_READ                               0x00000218  /* Read-Only */

#define OFFSET_BACKLIGHT_DUTY_CYCLE                                 0x00000080  /* Read-Write */

#define OFFSET_SCRATCH_PAD0                                         0x000003E0  /* Read-Write */
#define OFFSET_SCRATCH_PAD1                                         0x000003E4  /* Read-Write */
#define OFFSET_SCRATCH_PAD2                                         0x000003E8  /* Read-Write */
#define OFFSET_SCRATCH_PAD3                                         0x000003EC  /* Read-Write */

#define OFFSET_FPGA_ID0                                             0x000003F0  /* Read-Only */
#define OFFSET_FPGA_ID1                                             0x000003F4  /* Read-Only */

/* ======================================================================================= */

#define OSD_FPGA_DATECODE0 0x20211124 /*0x20210513*/ /*0x20210506*/ /*0x20210428*/ /* built date */
#define OSD_FPGA_DATECODE1 0X11241410 /*0x05131328*/ /*0x05061800*/ /*0x04231425*/ /* built day & time */


//#define OSD_CONTENT_SOURCE_ADDRESS                                  0x14000000 /* 0x00000000 */
#define OSD_CONTENT_SOURCE_ADDRESS                                  0x20000000 /* for test only */                                                                               */

#define OSD_FRAMEBUF_WIDTH_DEFAULT                                  0x0300 /* width:768 */
#define OSD_FRAMEBUF_HEIGHT_DEFAULT                                 0x0396 /* height:918 */

#define OSD_FRAMEBUF_BEGIN_COORDINATE_DEFAULT                       0x026c0600 /* offset [y:620, x:1356] */
#define OSD_FRAMEBUF_PIXELS_DEFAULT                                 0x03960300 /* size [h:918, w:768] */
//#define OSD_FRAMEBUF_PIXELSIZE_DEFAULT                              0x000AC200 /* size in pixel [h:918, x w:768, single-byte per-pixel (B3-G3-R2) ] */
//#define OSD_FRAMEBUF_BANNER_OFFSET_DEFAULT                          0x00013200 /* size in pixel [h:102, x w:768, single-byte per-pixel (B3-G3-R2) ] */
//#define OSD_FRAMEBUF_ACTUAL_AREA_PIXELSIZE_DEFAULT                  0x00099000 /* size in pixel [h:816, x w:768, single-byte per-pixel (B3-G3-R2) ] */
//#define OSD_FRAMEBUF_ACTUAL_AREA_XFER_BLOCK_DEFAULT                 0x00002640 /* OSD_FRAMEBUF_ACTUAL_AREA_PIXELSIZE_DEFAULT / 64 */



#define TX_DISPLAY_SOURCE_HDMI_RX_0                            0x00000000
#define TX_DISPLAY_SOURCE_HDMI_RX_1                            0x00000001
#define TX_DISPLAY_SCREEN_SOURCE_DP                            0x00000002
#define TX_DISPLAY_SCREEN_SOURCE_SDI_RX_1                      0x00000003
#define TX_DISPLAY_SCREEN_SOURCE_QSD_SDI_RX                    0x00000004 /* Quad Square division SDI_RX1~4 */
#define TX_DISPLAY_SCREEN_SOURCE_Q2SI_SDI_RX                   0x00000005 /* Quad Square division Quad 2SI SDI_RX1~4 */



/* OFFSET_TX_DISPLAY_CONTROL : Bit/Mask definitions */
#define TX_DISPLAY_MAIN_SCREEN_SOURCE_MASK                          0x0000000F
#define TX_DISPLAY_SUB_SCREEN_SOURCE_MASK                           0x000000F0


#define TX_DISPLAY_MAIN_SCREEN_SOURCE_HDMI_RX_0                     0x00000000
#define TX_DISPLAY_MAIN_SCREEN_SOURCE_HDMI_RX_1                     0x00000001
#define TX_DISPLAY_MAIN_SCREEN_SOURCE_SD_RX_1                       0x00000002
#define TX_DISPLAY_MAIN_SCREEN_SOURCE_DP                            0x00000003
#define TX_DISPLAY_MAIN_SCREEN_SOURCE_QUAD_SQUARE_DIVISION          0x00000004
#define TX_DISPLAY_MAIN_SCREEN_SOURCE_QUAD_2SI                      0x00000005

#define TX_DISPLAY_SUB_SCREEN_SOURCE_HDMI_RX_0                      0x00000000
#define TX_DISPLAY_SUB_SCREEN_SOURCE_HDMI_RX_1                      0x00000010
#define TX_DISPLAY_SUB_SCREEN_SOURCE_SD_RX_1                        0x00000020
#define TX_DISPLAY_SUB_SCREEN_SOURCE_DP                             0x00000030
#define TX_DISPLAY_SUB_SCREEN_SOURCE_QUAD_SQUARE_DIVISION           0x00000040
#define TX_DISPLAY_SUB_SCREEN_SOURCE_QUAD_2SI                       0x00000050

#define TX_DISPLAY_MAIN_SCREEN_ENABLE                               0x00000100 /*Bit-8, 1/0 : enable/disable the Main window*/
#define TX_DISPLAY_SUB_SCREEN_ENABLE                                0x00000200 /*Bit-9, 1/0 : enable/disable the Main window*/
#define TX_DISPLAY_OSD_SCREEN_ENABLE                                0x00000400 /*Bit-10, 1/0 : enable/disable the OSD window*/

#define TX_DISPLAY_TX_TIMING_MODE_SYNC_TO_INPUT                     0x00001000 /*Bit-12, 0: free-run, 1: synchronize to incoming video (default: 0) */
#define TX_DISPLAY_TX_DATA_MAPPING_MODE_AUO                         0x00004000 /*Bit-14, 0: 4-start(INNOLUX), 1:half(AUO)" */
#define TX_DISPLAY_BLACK_SCREEN_INSERTION_ENABLE                    0x00008000 /*Bit-15, 1/0 : enable/disable the "Insert Black-Scrren" */

#define TX_DISPLAY_MAIN_SCREEN_FLIP_X_ENABLE                        0x00010000 /*Bit-16, TX Main Screen Screen X-Flip Enable Control
*        1 = flip the TX main screen in horizontal. Default is 0
*/
#define TX_DISPLAY_MAIN_SCREEN_FLIP_Y_ENABLE                        0x00020000 /*Bit-17, TX Main Screen Screen Y-Flip Enable Control
*        1 = flip the TX main screen in vertical. Default is 0
*/

#define TX_DISPLAY_SUB_SCREEN_FLIP_X_ENABLE                         0x00040000 /*Bit-18, TX Sub Screen Screen X-Flip Enable Control
*        1 = flip the TX main screen in horizontal. Default is 0
*/
#define TX_DISPLAY_SUB_SCREEN_FLIP_Y_ENABLE                         0x00080000 /*Bit-19, TX Sub Screen Screen Y-Flip Enable Control
*        1 = flip the TX main screen in vertical. Default is 0
*/


/* OFFSET_TX_VIDEO_CONTROL_0 : Bit/Mask definitions */
#if defined(_FPGA_SUPPORT_1D_DEGAMMA_LUT_) && (_FPGA_SUPPORT_1D_DEGAMMA_LUT_ == 1)
#define TX_VIDEO_PIPELINE_ALL_LUT_MASK                              0x0000001F /*[4:0], 0 : bypass incoming raw data(default), 1: ednable the video pipeline function
                                                                                * Bit-0 : 1D_LUT for De-Gamma-Test
                                                                                * Bit-2 : 3D_LUT
																				* Bit-3 : 1D_LUT for Gamma & DICOM
																				* Bit-4 : 1D_LUT for De-Gamma
																				*/
#if defined(_FPGA_SUPPORT_1D_DEGAMMA_TEST_LUT_) && (_FPGA_SUPPORT_1D_DEGAMMA_TEST_LUT_ == 1)
#define TX_VIDEO_PIPELINE_1D_LUT_DEGAMMA_TEST_BYPASS_MASK           0x00000001 /* Bit-0 : 0: bypass incoming raw data(default), 1: ednable the video pipeline function */
#endif

#define TX_VIDEO_PIPELINE_3D_LUT_BYPASS_MASK                        0x00000004 /* Bit-2 : 0: bypass incoming raw data(default), 1: ednable the video pipeline function */
#define TX_VIDEO_PIPELINE_1D_LUT_BYPASS_MASK                        0x00000008 /* Bit-3 : 0: bypass incoming raw data(default), 1: ednable the video pipeline function */
#define TX_VIDEO_PIPELINE_1D_LUT_DEGAMMA_BYPASS_MASK                0x00000010 /* Bit-4 : 0: bypass incoming raw data(default), 1: ednable the video pipeline function */

#if defined(_FPGA_SUPPORT_1D_DEGAMMA_TEST_LUT_) && (_FPGA_SUPPORT_1D_DEGAMMA_TEST_LUT_ == 1)
#define TX_VIDEO_PIPELINE_1D_LUT_DEGAMMA_TEST_BYPASS                0x00000000 /* Bit-0, bypass incoming raw data */  
#define TX_VIDEO_PIPELINE_1D_LUT_DEGAMMA_TEST_ENABLE                0x00000001 /* Bit-0, ednable the video pipeline function */
#endif /* _FPGA_SUPPORT_1D_DEGAMMA_TEST_LUT_ */

#define TX_VIDEO_PIPELINE_3D_LUT_BYPASS                             0x00000000 /* Bit-2, bypass incoming raw data */  
#define TX_VIDEO_PIPELINE_3D_LUT_ENABLE                             0x00000004 /* Bit-2, ednable the video pipeline function */

#define TX_VIDEO_PIPELINE_1D_LUT_BYPASS                             0x00000000 /* Bit-3, bypass incoming raw data */  
#define TX_VIDEO_PIPELINE_1D_LUT_ENABLE                             0x00000008 /* Bit-3, ednable the video pipeline function */ 

#define TX_VIDEO_PIPELINE_1D_LUT_DEGAMMA_BYPASS                     0x00000000 /* Bit-4, bypass incoming raw data */  
#define TX_VIDEO_PIPELINE_1D_LUT_DEGAMMA_ENABLE                     0x00000010 /* Bit-4, ednable the video pipeline function */

#else /* ! _FPGA_SUPPORT_1D_DEGAMMA_LUT_ */

#define TX_VIDEO_PIPELINE_ALL_LUT_MASK                              0x0000001F /*[4:0], 0: bypass incoming raw data(default), 1: ednable the video pipeline function
                                                                                * Bit-2 : 3D_LUT
                                                                                * Bit-3 : 1D_LUT for Gamma & DICOM 
                                                                                * Bit-4 : 1D_LUT for Gamma Adjustment
                                                                                */
#define TX_VIDEO_PIPELINE_3D_LUT_BYPASS_MASK                        0x00000004 /* Bit-2 : 0: bypass incoming raw data(default), 1: ednable the video pipeline function */
#define TX_VIDEO_PIPELINE_1D_LUT_BYPASS_MASK                        0x00000008 /* Bit-3 : 0: bypass incoming raw data(default), 1: ednable the video pipeline function */
#define TX_VIDEO_PIPELINE_1D_LUT_GAMMA_ADJ_BYPASS_MASK              0x00000010 /* Bit-4 : 0: bypass incoming raw data(default), 1: ednable the video pipeline function */

#define TX_VIDEO_PIPELINE_3D_LUT_BYPASS                             0x00000000 /* Bit-2, bypass incoming raw data */  
#define TX_VIDEO_PIPELINE_3D_LUT_ENABLE                             0x00000004 /* Bit-2, ednable the video pipeline function */

#define TX_VIDEO_PIPELINE_1D_LUT_BYPASS                             0x00000000 /* Bit-3, bypass incoming raw data */  
#define TX_VIDEO_PIPELINE_1D_LUT_ENABLE                             0x00000008 /* Bit-3, ednable the video pipeline function */ 

#define TX_VIDEO_PIPELINE_1D_LUT_GAMMA_ADJ_BYPASS                   0x00000000 /* Bit-4, bypass incoming raw data */  
#define TX_VIDEO_PIPELINE_1D_LUT_GAMMA_ADJ_ENABLE                   0x00000010 /* Bit-4, ednable the video pipeline function */

#endif /* _FPGA_SUPPORT_1D_DEGAMMA_LUT_ */


#define TX_VIDEO_PIPELINE_MAIN_SCREEN_MASK                          0x00001000  /* Bit-12: 0: Gamma LUT(default), 1: DICOM LUT */
#define TX_VIDEO_PIPELINE_SUB_SCREEN_MASK                           0x00002000  /* Bit-13: 0/1 = Gamma LUT(default) / DICOM LUT */

#define TX_VIDEO_PIPELINE_RERANGE_MAPPING_MASK                      0x007F0000  /* [22-16]  RX Video Pipeline Re-range mapping function enable control
																				 * 0: bypass incoming raw data (default), 1: enable the re-range mapping 
																				 * Bit-16 : enable HDMI RX_1
																				 * Bit-17 : enable HDMI RX_2
																				 * Bit-18 : enable DP RX
																				 * Bit-19 : enable SDI RX_1
																				 * Bit-20 : enable SDI RX_2
																				 * Bit-21 : enable SDI RX_3
																				 * Bit-22 : enable SDI RX_4
                                                                                 */


#define TX_VIDEO_PIPELINE_RERANGE_MAPPING_HDMI_RX_1                 0x00010000  /* enable the re-range mapping ( HDMI RX_1 -> TX )*/
#define TX_VIDEO_PIPELINE_RERANGE_MAPPING_HDMI_RX_2                 0x00020000  /* enable the re-range mapping ( HDMI RX_2 -> TX )*/
#define TX_VIDEO_PIPELINE_RERANGE_MAPPING_DP_RX                     0x00040000  /* enable the re-range mapping ( DP RX -> TX )*/
#define TX_VIDEO_PIPELINE_RERANGE_MAPPING_SDI_RX_1                  0x00080000  /* enable the re-range mapping ( SDI RX_1 -> TX )*/
#define TX_VIDEO_PIPELINE_RERANGE_MAPPING_SDI_RX_2                  0x00100000  /* enable the re-range mapping ( SDI RX_2 -> TX )*/
#define TX_VIDEO_PIPELINE_RERANGE_MAPPING_SDI_RX_3                  0x00200000  /* enable the re-range mapping ( SDI RX_3 -> TX )*/
#define TX_VIDEO_PIPELINE_RERANGE_MAPPING_SDI_RX_4                  0x00400000  /* enable the re-range mapping ( SDI RX_4 -> TX )*/



#define TX_VIDEO_PIPELINE_MAIN_SCREEN_GAMMA_LUT                     0x00000000 /*  TX Video Pipeline Main Screen Gamma LUT Function enable */
#define TX_VIDEO_PIPELINE_MAIN_SCREEN_DICOM_LUT                     0x00001000 /*  TX Video Pipeline Main Screen DICOM LUT Function enable */

#define TX_VIDEO_PIPELINE_SUB_SCREEN_GAMMA_LUT                      0x00000000 /*  TX Video Pipeline Sub Screen Gamma LUT Function enable */
#define TX_VIDEO_PIPELINE_SUB_SCREEN_DICOM_LUT                      0x00002000 /*  TX Video Pipeline Sub Screen DICOM LUT Function enable */

/* OFFSET_TX_TEST_PATTERN_CONTROL: Bit/Mask definitions */
#define ACTION_TX_TEST_PATTERN_INSERTION                            0x80000000 /*Bit-31, 1: insert TX test pattern */
// #define ACTION_TX_TEST_PATTERN_LOOKUP_TABLE                         0x40000000 /*Bit-30, 0: Gamma 1D_LUT, 1: DICOM 1D_LUT */
#define MASK_TX_TEST_PATTERN_1D_LUT_SELECT                          0x40000000 /*Bit-30, 0: Gamma 1D_LUT, 1: DICOM 1D_LUT */
#define TX_TEST_PATTERN_4_1D_LUT_GAMMA                              0x00000000 /*Bit-30, 0: 1D_LUT Gamma */
#define TX_TEST_PATTERN_4_1D_LUT_DICOM                              0x40000000 /*Bit-30, 1: 1D_LUT DICO M*/
#define MASK_TX_TEST_PATTERN_RAW_DATA                               0x3FFFFFFF /* [9:0] Red, [19:10] Green, [29:20] Blue


/* OFFSET_VIDEO_LUT_CONTROL_0 : Bit/Mask definitions */
#define ACTION_LUT_ACCESS_TRIGGER                                   0x00000001 /*Bit-0, Force this bit from 0 --> 1 to trigger the LUT access action */
#define TYPE_LUT_ACCESS                                             0x00000010 /*Bit-4, 0/1 = read/write */
#define TARGET_LUT_ACCESS_MASK                                      0x00000F00 /*[11:8], 0/1/2/3 = 1D_LUT Gamma/1D-LUT Gamma Adjustment/1D-LUT DICOM/3D-LUT */
#define RESET_1D_LUT_INIT_SETTING                                   0x01000000 /*Bit-24, 0 -> 1 transition reset 1D-LUT to the initial one-to-one settings, this bit is need to de-assert by MCU */
#define RESETING_1D_LUT_INIT_MASK                                   0x02000000 /*Bit-25, 1D LUT Initial Reset State (Read-Only), 1 = 1D-LUT reset action is in-progress */
#define RESET_3D_LUT_SETTING                                        0x10000000 /*Bit-28, 0 -> 1 transition reset 3D-LUT to the initial one-to-one settings, this bit is need to de-assert by MCU */
#define RESETTING_3D_LUT_MASK                                       0x20000000 /*Bit-29, 1D LUT Initial Reset State (Read-Only), 1 = 3D-LUT reset action is in-progress */
#define REARCH_3D_LUT_SETTING                                       0x40000000 /*Bit-30, 0 -> 1 transition re-arch. 3D-LUT, this bit is need to de-assert by MCU */
#define REARCHING_3D_LUT_MASK                                       0x80000000 /*Bit-31, 3D-LUT Re-arch. Accessing State (Read-Only), 1 = access action is busy */

/* definition for TARGET_LUT_ACCESS_XXX */
#define TARGET_LUT_ACCESS_GAMMA_1D                                  0x00000000 /*[11:8], 0/1/2/3 = 1D_LUT Gamma/1D-LUT Gamma Adjustment/1D-LUT DICOM/3D-LUT */
#define TARGET_LUT_ACCESS_GAMMA_ADJUST_1D                           0x00000100 /*[11:8], 0/1/2/3 = 1D_LUT Gamma/1D-LUT Gamma Adjustment/1D-LUT DICOM/3D-LUT */
#define TARGET_LUT_ACCESS_DICOM_1D                                  0x00000200 /*[11:8], 0/1/2/3 = 1D_LUT Gamma/1D-LUT Gamma Adjustment/1D-LUT DICOM/3D-LUT */
#define TARGET_LUT_ACCESS_3D                                        0x00000300 /*[11:8], 0/1/2/3 = 1D_LUT Gamma/1D-LUT Gamma Adjustment/1D-LUT DICOM/3D-LUT */


/* OFFSET_VIDEO_LUT_CONTROL_1 : Bit/Mask definitions */
#define FLASH_LUT_INDEX_MAX                                         112        /* (128 - 16) x 4K Byte */
#define INDEX_LUT_MASK                                              0x000003FF /*[9:0], Index/Address for LUT */
#define SELECTION_LUT_MASK                                          0x00000C00 /*[11:10], 0/1/2 is for R/G/B LUT selection */

/* defnition for SELECTION_LUT_XXX*/
#define SELCTION_LUT_RED                                            0x00000000 /*[11:10], 0/1/2 is for R/G/B LUT selection */
#define SELCTION_LUT_GREEN                                          0x00000400 /*[11:10], 0/1/2 is for R/G/B LUT selection */
#define SELCTION_LUT_BLUE                                           0x00000800 /*[11:10], 0/1/2 is for R/G/B LUT selection */

/* OFFSET_VIDEO_LUT_CONTROL_2 : Mask definitions */
#define LUT_DATA_WRITE_MASK                                         0x000003FF /*[9:0], LUT Write Data [9:0] */

/* OFFSET_VIDEO_LUT_STATUS_0 : Mask definitions */
#define LUT_DATA_READ_MASK                                          0x000003FF /*[9:0], LUT Read Data [9:0] */



#define MASK_OSD_SCALE_MODE                                         0x000F0000 /* Bit19~16, scaled-mode not supported yet */
#define BITS_OSD_SCALE_MODE_SHIFT                                   16


#define MASK_DDR_CALIBARTION_FAIL                                   0x00000000
#define MASK_DDR_CALIBARTION_OK                                     0x00000001
#define MASK_DDR_IS_READY                                           MASK_DDR_CALIBARTION_OK

#define MASK_DDR_MEMORY_STATUS                                      0x00000001
#define BITS_DDR_MEMORY_STATUS_SHIFT                                0


#define OSD_DMA_BYTES_IN_BLOCK                                      64
#define BITS_OSD_BYTES_IN_BLOCK_SHIFT                               6

#define PERIPHERAL_FPGA_IRQ_BRIDGE_0_IRQ                            1 /*2*/
#define PERIPHERAL_FPGA_IRQ_BRIDGE_0_IRQ_INTERRUPT_CONTROLLER_ID    0


/* Look-Up-Table controlling definition : OFFSET_OSDColor_LUT_CONTROL */
#define LUT_INDEX_MAX                                               256

#define MASK_OSDColor_LUT_TRIGGER_CONTROL                           0x00000001
#define MASK_OSDColor_LUT_ACCESS_TYPE_READ                          0x00000000
#define MASK_OSDColor_LUT_ACCESS_TYPE_WRITE                         0x00000010
#define BITS_OSDColor_LUT_COLOR_INDEXES_SHIFT                       8
#define MASK_OSDColor_LUT_COLOR_INDEXES                             0x0000FF00
#define MASK_OSDColor_LUT_COLOR_VALUE                               0x00FFFFFF  /* RGB color */

/* Keypad definitions : OFFSET_KEYPAD_BUTTON_STATUS */
#define MASK_KEYPAD_STATUS_DOWN                                     0x00000008
#define MASK_KEYPAD_STATUS_UP                                       0x00000002
#define MASK_KEYPAD_STATUS_ENTER                                    0x00000001
#define MASK_KEYPAD_STATUS_ESC                                      0x00000040

// #define MASK_KEYPAD_VLGL_KEYS                                       (MASK_KEYPAD_STATUS_ESC | MASK_KEYPAD_STATUS_DOWN | MASK_KEYPAD_STATUS_UP | MASK_KEYPAD_STATUS_ENTER)
#define MASK_KEYPAD_VLGL_KEYS                                       0x0000004B

/* Unknown/Unused Buttons */
#define MASK_KEYPAD_STATUS_POWER                                    0x00000080
#define MASK_KEYPAD_STATUS_SOURCE1                                  0x00000020
#define MASK_KEYPAD_STATUS_SOURCE2                                  0x00000040

#define OSD_BANNER_WIDTH_IN_PIXEL                                   OSD_FRAMEBUF_WIDTH_DEFAULT
#define OSD_BANNER_HEIGHT_IN_PIXEL                                  102 /* 102 pixels */
#define OSD_BANNER_PIXELSIZE_DEFAULT                                OSD_FRAMEBUF_BANNER_OFFSET_DEFAULT

#endif /* PRIVATE_FPGA_H_ */


#endif /* SCALER_SERIAL_INTERNAL_H */
