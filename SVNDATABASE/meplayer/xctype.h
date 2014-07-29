
//********************************************************************************** 
//
// Filename: xctype.h
// 
// ViXS Systems Confidential and Prioprietary 
//
//********************************************************************************* 

/***************************************************************************\
*                                                                           *
*   Copyright 2004 ViXS Systems Inc.  All Rights Reserved.                  *
*                                                                           *
*===========================================================================*
*                                                                           *
*   File Name: xctype.h                                                     *
*                                                                           *
*   Description:                                                            *
*       This file contains xcode type declaration                           *
*       Firmware.                                                           *
*                                                                           *
*===========================================================================*
*                                                                           *
*   $Source: /cvs2hg/rsync/XCSDK/XCSDK/XCMSDK/include/xctype.h,v $
*                                                                           *
*   $Log: not supported by cvs2svn $
*   Revision 1.174  2010/11/19 19:44:20  jwang
*   Added AUDIO_CH_ASSIGN_NONE.
*
*   Revision 1.173  2010/11/15 22:37:36  yfeng
*   Add charset for ISO-2022-JP
*
*   Revision 1.172  2010/11/12 15:08:48  elu
*   added more definition for author_charset_t.
*
*   Revision 1.171  2010/11/11 23:38:05  jwang
*   Fixed a bug of endian conversion.
*
*   Revision 1.170  2010/11/10 00:50:06  jwang
*   Added video resolution change callback.
*
*   Revision 1.169  2010/11/09 20:59:42  hxiao
*   enum_interrupt_edge
*
*   Revision 1.168  2010/11/04 19:22:51  hxiao
*   remove GPIO_CATEGORY_C1 etc
*
*   Revision 1.167  2010/11/03 19:36:48  hxiao
*   add more GPIO_CATEGORY
*
*   Revision 1.166  2010/11/01 21:51:14  hluo
*   modified comment
*
*   Revision 1.165  2010/10/18 22:54:33  yfeng
*   Add type author_charset_t
*
*   Revision 1.164  2010/10/12 16:23:18  yfeng
*   Add type cci_code_t for CCI information.
*
*   Revision 1.163  2010/10/08 12:59:30  hxiao
*   add XCODE_INPUT_TSI_SECURED
*
*   Revision 1.162  2010/10/07 21:38:38  jwang
*   Added DTS audio type.
*
*   Revision 1.161  2010/10/05 20:01:33  jwang
*   Added support for firmware new argument for MP4 lipsync algorithm.
*
*   Revision 1.160  2010/09/15 17:33:18  jwang
*   Added audio mixing support.
*
*   Revision 1.159  2010/09/09 16:19:55  jwang
*   Added player priority support.
*
*   Revision 1.158  2010/09/02 21:07:11  jwang
*   Separated AuthoringSDK from XCMSDK.
*
*   Revision 1.157  2010/08/31 18:28:37  jwang
*   Optimized code.
*
*   Revision 1.156  2010/08/27 23:06:29  jwang
*   Added interface for video filters.
*
*   Revision 1.155  2010/08/27 17:30:42  jwang
*   And support for multiple channels audio.
*
*   Revision 1.154  2010/08/26 22:16:54  jwang
*   Added support for HDMI audio output.
*
*   Revision 1.153  2010/08/13 23:52:19  jwang
*   Support user data extraction.
*
*   Revision 1.152  2010/08/09 15:06:40  jwang
*   Added PORT_656.
*
*   Revision 1.151  2010/08/06 15:48:24  hluo
*   added more play modes
*
*   Revision 1.150  2010/07/28 20:12:46  jwang
*   Added new audio format firmware supports.
*
*   Revision 1.149  2010/07/22 17:43:08  jwang
*   Renamed authoring profiles.
*
*   Revision 1.148  2010/07/15 02:17:12  jwang
*   Added DVD-VR author.
*
*   Revision 1.147  2010/07/08 19:48:26  jwang
*   Added MVC support.
*
*   Revision 1.146  2010/06/25 18:23:06  jwang
*   Added VC1 support.
*
*   Revision 1.145  2010/06/21 19:36:56  xchen
*   Add WMA/WMV PRO profile to DMS.
*
*   Revision 1.144  2010/06/18 22:00:06  xchen
*   Add WMV profile to DMS.
*
*   Revision 1.143  2010/06/16 21:22:18  jwang
*   Added more video/audio parameters.
*
*   Revision 1.142  2010/06/08 15:46:54  jwang
*   Added ASF/WMV/WMA format.
*
*   Revision 1.141  2010/05/31 20:07:00  jwang
*   Optimized private data detection algorithm.
*
*   Revision 1.140  2010/05/10 15:48:31  hluo
*   support front end program in XC4_FPGA_BOARD_VIPER_REV1
*
*   Revision 1.139  2010/04/27 22:16:43  jwang
*   Support Viper player.
*
*   Revision 1.138  2010/04/14 22:17:11  jwang
*   Added XC_INPUT_BUF and XC_OUTPUT_BUF.
*
*   Revision 1.137  2010/01/29 19:25:58  jwang
*   1) Added more MP4 type.
*   2) Added EAC3 audio.
*
*   Revision 1.136  2009/12/23 17:39:37  jwang
*   Added support for Matroska file.
*
*   Revision 1.135  2009/12/23 17:28:43  jwang
*   Added new MP4 type "XC_PS_MP4_3GP_6_PS" for Eagle.
*
*   Revision 1.134  2009/12/16 21:36:32  jwang
*   Added support for ADV7403 on XCode 4115 Developing Board Rev2.
*
*   Revision 1.133  2009/12/09 19:49:19  jwang
*   Support ISOM reader/player.
*
*   Revision 1.132  2009/11/20 23:24:54  jwang
*   Added AVI support.
*
*   Revision 1.131  2009/11/11 17:25:26  elu
*   Add BDAV_AUTHORING.
*
*   Revision 1.130  2009/11/02 23:16:20  jwang
*   Added TCP input.
*
*   Revision 1.129  2009/10/28 21:39:47  jwang
*   Added more MP4 sub format.
*
*   Revision 1.128  2009/10/06 19:38:47  yfeng
*   Move xc_auto_destroy_t into xctype.h
*
*   Revision 1.127  2009/09/25 15:24:07  jwang
*   Fixed a typo for GOP structure.
*
*   Revision 1.126  2009/09/24 21:23:40  jwang
*   Added TCP output.
*
*   Revision 1.125  2009/08/11 23:19:42  jwang
*   Added program number and program ID for ts_properties_t.
*
*   Revision 1.124  2009/08/04 16:47:25  jwang
*   Added 16 bit grayscale define.
*
*   Revision 1.123  2009/07/29 01:28:06  xcao
*   no message
*
*   Revision 1.121  2009/06/04 21:56:58  jwang
*   Added 100Hz refresh rate.
*
*   Revision 1.120  2009/05/28 15:21:09  xchen
*   Add query supporting video attributes (qualities) for analog inputs in web GUI.
*
*   Revision 1.119  2009/05/07 16:17:12  jwang
*   Added DSS mux format.
*
*   Revision 1.118  2009/05/06 22:56:15  elu
*   Added new type in audio_channel_assignment_t.
*
*   Revision 1.117  2009/04/08 19:25:26  hhua
*   - add #ifndef WIN32 around Linxu headers
*   - include "windows.h" in #ifdef WIN32 clause
*   - fix error and warnings in WIN32 part of get_time_ms()
*
*   Revision 1.116  2009/04/06 22:00:55  xchen
*   Fix a bug of overlap checking in timer recording.
*
*   Revision 1.115  2009/03/17 14:59:02  jwang
*   Added support for PSI filtering feature for ASI input.
*
*   Revision 1.114  2009/02/18 21:28:41  jwang
*   1) Added a new mux format for 204 byte TS.
*   2) Added a new field Progressive for video picture info.
*
*   Revision 1.113  2009/02/02 21:16:20  xchen
*   Fix Bugzilla Bug 4319.
*
*   Revision 1.112  2009/01/22 00:40:07  xchen
*   no message
*
*   Revision 1.111  2009/01/06 18:01:58  gxue
*    fix bug that not predefine PATH_MAX, which will cause unidentical size of PATH_MAX. This problem is introduced by gcc2.2_build3
*
*   Revision 1.109  2008/12/19 15:34:26  bsang
*   add messages of CAPTURE_VBI and INSERT_VBI_DATA_FLAGS
*
*   Revision 1.108  2008/12/16 08:21:33  jwang
*   Added the user_data ES type.
*
*   Revision 1.107  2008/12/15 14:21:51  bsang
*   Add mpeg4 profile and level messages
*
*   Revision 1.106  2008/12/11 08:41:26  jwang
*   Support 60Hz and 59.94Hz refresh rate.
*
*   Revision 1.105  2008/12/03 22:38:04  jwang
*   Used new interface to send compressed audio data.
*
*   Revision 1.104  2008/12/01 18:02:46  jwang
*   Added video render PTS callback.
*
*   Revision 1.103  2008/11/21 22:20:12  bsang
*   channel max to 2000
*
*   Revision 1.102  2008/11/12 22:01:43  jwang
*   Added interface to set EOS callback function.
*
*   Revision 1.101  2008/10/30 16:30:02  jwang
*   Added define SPDIF_OUTPUT_PCM16 and SPDIF_OUTPUT_PCM24.
*
*   Revision 1.100  2008/10/28 22:33:58  jwang
*   Support AVC picture information report for decoding.
*
*   Revision 1.99  2008/10/27 14:08:40  jwang
*   Optimized code.
*
*   Revision 1.98  2008/10/24 20:34:25  xchen
*   Add some TimeSeek function in DLNA server
*
*   Revision 1.97  2008/10/17 20:02:46  jwang
*   Added few more defines.
*
*   Revision 1.96  2008/10/16 23:09:37  xchen
*   Optimize DLNA server code.
*
*   Revision 1.95  2008/10/10 14:04:11  jwang
*   Fixed some typos.
*
*   Revision 1.94  2008/10/05 21:40:50  jwang
*   Added more video picture information.
*
*   Revision 1.93  2008/10/04 20:21:18  jwang
*   1) Added decoder callback defines.
*   2) Support C.
*
*   Revision 1.92  2008/09/22 19:37:47  yfeng
*   Add DTCP AKE client into it
*
*   Revision 1.91  2008/09/15 21:23:56  jwang
*   Support arbitrary speed setting for trick mode.
*
*   Revision 1.90  2008/09/09 21:26:13  jwang
*   Renamed tsd_serial_input_channel_mask to tsd_parallel_input_channel_mask.
*
*   Revision 1.89  2008/09/08 15:47:33  jwang
*   Support sub stream (mirror transcoding).
*
*   Revision 1.88  2008/09/05 15:29:45  hluo
*   modified struct ts_properties_t
*
*   Revision 1.87  2008/08/21 20:49:51  xchen
*   Add DLNA file tanscoding.
*
*   Revision 1.86  2008/08/11 21:10:23  jwang
*   1) Added more mux format.
*   2) Added more color space defines.
*
*   Revision 1.85  2008/07/18 18:40:56  jwang
*   Added more mux format.
*
*   Revision 1.84  2008/07/18 06:34:35  yfeng
*   Add more limitation for the dtcp::e_emi_code_t
*
*   Revision 1.83  2008/07/17 22:01:41  hluo
*   moved enum e_emi_code_t from dtcp_type.h to this file to fix a compile error
*
*   Revision 1.82  2008/07/15 20:06:05  jwang
*   Added BROADCAST macro define.
*
*   Revision 1.81  2008/07/03 20:41:54  jwang
*   Fixed a bug of page fault.
*
*   Revision 1.80  2008/07/02 20:10:08  jwang
*   Support more options for DVO config.
*
*   Revision 1.79  2008/06/25 15:54:03  jwang
*   Added picture structure define.
*
*   Revision 1.78  2008/06/13 20:07:09  xchen
*   1. Update timer recording feature.
*   2. Fix Bugzilla 3960
*
*   Revision 1.77  2008/05/13 21:28:04  jwang
*   Added a ASI passthrough demux interface.
*
*   Revision 1.76  2008/05/08 16:41:02  jwang
*   Modified video standard define.
*
*   Revision 1.75  2008/05/07 23:17:33  jwang
*   Added log utility.
*
*   Revision 1.74  2008/05/06 20:24:55  jwang
*   Added UDP input/output.
*
*   Revision 1.73  2008/05/02 02:27:20  xchen
*   add sample test program for file input to player output.
*
*   Revision 1.72  2008/04/25 22:35:20  jwang
*   1) Added refresh rate defines.
*   2) Modified authoring type defines.
*
*   Revision 1.71  2008/04/25 17:40:52  xchen
*   Add network setting feature.
*
*   Revision 1.70  2008/04/21 18:03:34  jwang
*   1) Fixed a compile warning.
*   2) Added a get_time_ms function.
*
*   Revision 1.69  2008/04/11 22:00:42  hluo
*   enum_analog_video_standard_t changed to enum_analog_digital_video_standard_t
*
*   Revision 1.68  2008/03/14 19:03:03  hluo
*   added dvo_configure_t
*
*   Revision 1.67  2008/03/06 16:33:28  jwang
*   Optimized code to support input video type and multiple audio.
*
*   Revision 1.66  2008/02/22 00:06:54  jwang
*   Optimized code to support AVC.
*
*   Revision 1.65  2008/02/08 22:06:12  jwang
*   Optimize code to support gcc 4.2.
*
*   Revision 1.64  2008/02/08 18:34:38  jwang
*   Optimized code.
*
*   Revision 1.63  2008/02/07 22:38:38  jwang
*   1) Added more GPIO category.
*   2) Moved file open mode define to here.
*
*   Revision 1.62  2008/02/04 22:29:32  hluo
*   no message
*
*   Revision 1.61  2008/02/01 21:49:22  jwang
*   Add GPIO defines.
*
*   Revision 1.60  2008/01/31 23:37:54  hluo
*   no message
*
*   Revision 1.59  2008/01/31 20:09:37  gxue
*   alhpa release
*
*   Revision 1.58  2008/01/30 20:46:39  jwang
*   Added digital video standard.
*
*   Revision 1.57  2008/01/28 17:14:40  hluo
*   changed include path for driver
*
*   Revision 1.56  2008/01/17 19:42:04  jwang
*   Added a workaround for ARC linux mutex bug.
*
*   Revision 1.55  2008/01/08 16:53:54  hluo
*   added a new enumerator for back end config.
*
*   Revision 1.54  2008/01/02 15:20:55  hluo
*   break analog standard into video standard and audio standard
*
*   Revision 1.53  2007/12/31 17:33:51  jwang
*   1) Added NULL output.
*   2) Added more PVR database opening mode.
*
*   Revision 1.52  2007/11/28 20:12:52  jwang
*   Added type define for YUV display.
*
*   Revision 1.51  2007/11/21 22:41:40  hluo
*   added photo_album_input_t
*
*   Revision 1.50  2007/11/20 00:05:55  jwang
*   Optimized xcoder_control_t.
*
*   Revision 1.49  2007/10/30 14:27:42  hluo
*   added class auto_lock_t
*
*   Revision 1.48  2007/10/19 21:51:18  hluo
*   added HTTP_input_t
*
*   Revision 1.47  2007/10/12 21:15:22  xchen
*   fix some bugs in timer recording stack.
*
*   Revision 1.46  2007/10/09 16:00:41  hluo
*   no message
*
*   Revision 1.45  2007/09/18 19:24:38  jwang
*   Support audio channel assignment.
*
*   Revision 1.44  2007/08/10 19:59:54  jwang
*   Added MPEG-4 video detection.
*
*   Revision 1.43  2007/07/31 22:44:55  jwang
*   Added YUV format define.
*
*   Revision 1.42  2007/07/30 22:08:57  xchen
*   Modify PVR feature.
*
*   Revision 1.41  2007/07/26 18:04:53  xchen
*   fix a bug.
*
*   Revision 1.40  2007/07/19 15:21:49  xchen
*   add web commands for PVR.
*
*   Revision 1.39  2007/07/10 16:35:05  hluo
*   no message
*
*   Revision 1.38  2007/06/28 18:34:04  jwang
*   Rename OUTPUT_TO_DVD_DISK to OUTPUT_TO_OPTICAL_DISK.
*
*   Revision 1.37  2007/06/27 20:06:59  sjin
*   Added PVR stack.
*
*   Revision 1.36  2007/06/07 18:29:53  hluo
*   no message
*
*   Revision 1.35  2007/06/05 20:09:56  xchen
*   Fix some bugs about timer recording.
*
*   Revision 1.34  2007/06/04 23:17:56  xchen
*   Optimize classes.
*
*   Revision 1.33  2007/05/29 23:32:47  jwang
*   Added AAC support.
*
*   Revision 1.32  2007/05/28 22:41:12  jwang
*   Added Bluray support.
*
*   Revision 1.31  2007/05/03 23:52:28  xchen
*   Added TS input for digital TV with GUI interface and fixed some bugs.
*
*   Revision 1.30  2007/05/03 02:43:45  xchen
*   Added tuner mode setting GUI interface and fixed some bugs.
*
*   Revision 1.29  2007/05/01 18:19:23  xchen
*   Optimize classes.
*
*   Revision 1.28  2007/04/27 02:32:18  xchen
*   Optimize classes.
*
*   Revision 1.27  2007/04/24 14:45:40  xchen
*   Optimize classes.
*
*   Revision 1.26  2007/04/23 23:03:34  xchen
*   Add user name and password for RTSP input.
*
*   Revision 1.25  2007/04/23 19:12:12  xchen
*   Optimize classes.
*
*   Revision 1.24  2007/04/20 21:46:54  xchen
*   Fixed some bugs and optimize classes.
*
*   Revision 1.23  2007/04/17 20:30:18  xchen
*   Optimize classes.
*
*   Revision 1.22  2007/04/13 02:56:15  xchen
*   Optimize classes.
*
*   Revision 1.21  2007/04/05 23:23:35  xchen
*   Add support of digital TV input.
*
*   Revision 1.20  2007/03/28 00:30:56  xchen
*   Optimized classes.
*
*   Revision 1.17  2007/03/20 00:12:36  xchen
*   Optimize classes.
*
*   Revision 1.16  2007/03/15 23:58:56  xchen
*   Changed video output type definition.
*
*   Revision 1.15  2007/03/12 21:41:56  xchen
*   Add support for low profile board.
*
*   Revision 1.14  2007/03/02 23:26:41  hluo
*   added new enum
*
*   Revision 1.13  2007/02/28 16:37:38  xchen
*   Optimize class.
*
*   Revision 1.12  2007/02/23 03:52:31  xchen
*   Optimize class
*
*   Revision 1.11  2007/02/15 21:28:50  xchen
*   Optimize class.
*
*   Revision 1.10  2007/02/09 01:30:31  xchen
*   Optimized xcode types definitions.
*
*   Revision 1.9  2007/01/02 21:18:10  xchen
*   Add timer recording reminder feature.
*
*   Revision 1.8  2006/12/05 21:53:05  xchen
*   Optimized classes.
*
*   Revision 1.7  2006/12/05 16:13:30  hluo
*   added a struct
*
*   Revision 1.6  2006/11/23 01:17:03  xchen
*   Added DVD/HDDVD authoring selection implement in recorder setup menu
*
*   Revision 1.5  2006/11/16 03:31:13  xchen
*   Added error check for timer recording.
*
*   Revision 1.4  2006/11/15 00:02:59  xchen
*   Added the stop recording feature in timer recording.
*
*   Revision 1.3  2006/11/09 02:44:17  xchen
*   Added timer recording feature.
*
*   Revision 1.2  2006/11/01 15:52:58  hluo
*   no message
*
*   Revision 1.1  2006/10/26 20:35:01  hluo
*   initial version
*
*  
\***************************************************************************/


#if !defined(XCTYPE_H__5FA901AE_F23E_4F89_9C02_3F9224A4F84A__INCLUDED_)
#define XCTYPE_H__5FA901AE_F23E_4F89_9C02_3F9224A4F84A__INCLUDED_

#ifdef __cplusplus

#include <algorithm>
#include <functional>
#include <list>
#include <deque>
#include <string>
#include <iostream>
#include <bitset>
#include <map>

#ifdef WIN32
#include "windows.h"
#endif

#ifndef WIN32
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>
// fix bug that not predefine PATH_MAX, which will cause unidentical size of PATH_MAX. This problem is introduced by gcc2.2_build3
#include <linux/limits.h>
#endif

//#include "xclog.h"

#endif // __cplusplus


#ifndef PATH_MAX
#define PATH_MAX    2048
#endif

#ifndef COUNTOF
#define COUNTOF(array)  (sizeof(array)/sizeof((array)[0]))
#endif

#define ALIGN_SIZE(x, y)    ((x + y - 1) & ~(y - 1))

#ifdef WIN32
#define delay_ms(ms)    Sleep(ms)
#else
#define delay_ms(ms)    usleep(ms * 1000)
#endif


typedef signed char         int8;
typedef unsigned char       uint8;
typedef short               int16;
typedef unsigned short      uint16;
typedef long                int32;
typedef unsigned long       uint32;
#ifdef WIN32
typedef _int64              int64;
typedef unsigned _int64     uint64;
#else
typedef long long           int64;
typedef unsigned long long  uint64;
#endif

#if (defined(_ISOC99_SOURCE) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || (defined (__WATCOMC__) && (defined (_STDINT_H_INCLUDED) || __WATCOMC__ >= 1250)) || (defined(__GNUC__) && (defined(_STDINT_H) || defined(_STDINT_H_)) )) && !defined (_PSTDINT_H_INCLUDED)
#include <stdint.h>
#else
#ifndef __int8_t_defined
typedef char                int8_t;
#endif
typedef short               int16_t;
typedef int                 int32_t;
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
#ifdef WIN32
typedef _int64              int64_t;
typedef unsigned _int64     uint64_t;
#else
typedef long long           int64_t;
typedef unsigned long long  uint64_t;
#endif
#endif


#ifdef WIN32
#define DIR_SLASH       "\\"
#else
#define DIR_SLASH       "/"
#endif


#define MAX_NUM_XCODE_CHANNELS          2
//#define MAX_NUM_INPUTS_PER_CHANNEL      4

#ifdef XCODE3_CHIP
#define MAX_NUM_TSI_PORT                2
#define MAX_NUM_DVO_PORT                1
#elif defined(XCODE4_EAGLE_CHIP)
#define MAX_NUM_TSI_PORT                2
#define MAX_NUM_DVO_PORT                2
#elif defined(XCODE4_VIPER_CHIP)
#define MAX_NUM_TSI_PORT                6
#define MAX_NUM_DVO_PORT                2
#endif

#define MAX_NUM_XCODE_AUDIOS            2   // May increace to 8 if firmware supports

#define MAX_NUM_PASSTHROUGH_PIDS        16  // Firmware support up to 16 pids passthrough
#define MAX_NUM_PASSTHROUGH_TABLE_IDS   16  // Firmware support up to 16 tables passthrough

#ifdef XCODE3_CHIP
#define MAX_PLAY_PCM_AUDIO_CHANNELS     2
#elif defined(XCODE4_VIPER_CHIP)
#define MAX_PLAY_PCM_AUDIO_CHANNELS     8
#endif


typedef enum
{
    GPIO_CATEGORY_INVALID       = 0,

    // GENERAL GPIOs
    GPIO_CATEGORY_DEDICATED     = 1,
    GPIO_CATEGORY_POD           = 2,

    // XCODE2 GPIOs
    GPIO_CATEGORY_RBM           = 3,

    // XCODE3 and above GPIOs
    GPIO_CATEGORY_A             = 4,
    GPIO_CATEGORY_B             = 5,
    GPIO_CATEGORY_C             = 6,
    GPIO_CATEGORY_D             = 7,
    GPIO_CATEGORY_E             = 8,
    GPIO_CATEGORY_F             = 9,
    GPIO_CATEGORY_G             = 10,
    GPIO_CATEGORY_H             = 11,
    GPIO_CATEGORY_I             = 12,
    GPIO_CATEGORY_J             = 13,

    // ENGINEERING BOARD REV1 GPIOs
    GPIO_CATEGORY_FPGA_GRP1     = 14,   // FPGA Register 0x03

    // XCODE 4115 DEVELOPMENT BOARD REV2 FPGA GPIOs
    GPIO_CATEGORY_X4E05_FPGA_MOD_GPIO   = 15,   // FPGA Register 0x03, modulator GPIOs
    GPIO_CATEGORY_X4E05_FPGA_MOD_RES    = 16,   // FPGA Register 0x09, modulator->XCode 656 routing
    GPIO_CATEGORY_X4210_FPGA_MOD_GPIO   = 17,   // FPGA Register 0x03, modulator GPIOs 
    GPIO_CATEGORY_X4210_FPGA_MOD_RES    = 18,   // FPGA Register 0x09, modulator->XCode 656 routing 

    GPIO_CATEGORY_L             = 19,   
    
} enum_gpio_category_t;

enum
{
    GPIO_BIT_MASK_0             = 0x00000001,
    GPIO_BIT_MASK_1             = 0x00000002,
    GPIO_BIT_MASK_2             = 0x00000004,
    GPIO_BIT_MASK_3             = 0x00000008,
    GPIO_BIT_MASK_4             = 0x00000010,
    GPIO_BIT_MASK_5             = 0x00000020,
    GPIO_BIT_MASK_6             = 0x00000040,
    GPIO_BIT_MASK_7             = 0x00000080,
    GPIO_BIT_MASK_8             = 0x00000100,
    GPIO_BIT_MASK_9             = 0x00000200,
    GPIO_BIT_MASK_10            = 0x00000400,
    GPIO_BIT_MASK_11            = 0x00000800,
    GPIO_BIT_MASK_12            = 0x00001000,
    GPIO_BIT_MASK_13            = 0x00002000,
    GPIO_BIT_MASK_14            = 0x00004000,
    GPIO_BIT_MASK_15            = 0x00008000,
    GPIO_BIT_MASK_16            = 0x00010000,
    GPIO_BIT_MASK_17            = 0x00020000,
    GPIO_BIT_MASK_18            = 0x00040000,
    GPIO_BIT_MASK_19            = 0x00080000,
    GPIO_BIT_MASK_20            = 0x00100000,
    GPIO_BIT_MASK_21            = 0x00200000,
    GPIO_BIT_MASK_22            = 0x00400000,
    GPIO_BIT_MASK_23            = 0x00800000,
    GPIO_BIT_MASK_24            = 0x01000000,
    GPIO_BIT_MASK_25            = 0x02000000,
    GPIO_BIT_MASK_26            = 0x04000000,
    GPIO_BIT_MASK_27            = 0x08000000,
    GPIO_BIT_MASK_28            = 0x10000000,
    GPIO_BIT_MASK_29            = 0x20000000,
    GPIO_BIT_MASK_30            = 0x40000000,
    GPIO_BIT_MASK_31            = 0x80000000,
};

typedef enum             
{
    level,
    edge,       
    
}enum_interrupt_edge;

typedef enum
{
    low,
    high,
}enum_interrupt_polarity;



typedef struct
{
    enum_gpio_category_t m_Category;
    unsigned int m_BitMask;
} gpio_t;

typedef enum
{
    PORT_SVIDEO                 = 0x00000001,
    PORT_COMPOSITE              = 0x00000002,
    PORT_COMPONENT              = 0x00000004,
    PORT_HDMI                   = 0x00000008,
    PORT_656                    = 0x00000010,
    PORT_JAPANESE_D_CONNECTOR   = 0x00000020,
} enum_video_output_port_t;

typedef enum
{
    SLOW_BLANKING_OFF           = 0x00000000,
    SLOW_BLANKING_16x9          = 0x00000001,
    SLOW_BLANKING_4x3           = 0x00000002,
    SLOW_BLANKING_FOLLOW        = 0x00000004,
    SLOW_BLANKING_MONITOR       = 0x00000008,
} enum_slow_blanking_mode_t;

typedef enum 
{
    XCODE_INPUT_HOST,
    XCODE_INPUT_YUV,
    XCODE_INPUT_TSI,
    XCODE_INPUT_TSI_PASSTHROUGH,
    XCODE_INPUT_TSI_SECURED,
} enum_xc_input_category_t;

typedef enum 
{
    XCODE_OUTPUT_HOST,
    XCODE_OUTPUT_DVO,
} enum_xc_output_category_t;


typedef enum 
{
    XC_TUNER_ANALOG,
    XC_TUNER_DIGITAL,
} enum_xc_tuner_type_t;

typedef enum 
{
    XC_INPUT_SVIDEO,
    XC_INPUT_COMPONENT,
    XC_INPUT_COMPOSITE,
    XC_INPUT_SCART,
    XC_INPUT_HDMI,
    XC_INPUT_TV_TUNER,

    XC_INPUT_TS,

    XC_INPUT_FILE,
    XC_INPUT_RTSP,
    XC_INPUT_HTTP,
    XC_INPUT_PHOTO_ALBUM,
    XC_INPUT_UDP,
    XC_INPUT_TCP,
    XC_INPUT_BUF,
} enum_xc_input_type_t;

typedef enum
{
    XC_OUTPUT_NULL,
    XC_OUTPUT_AUTHORING,
    XC_OUTPUT_TS,
    XC_OUTPUT_FILE,
    XC_OUTPUT_RTSP,
    XC_OUTPUT_PLAYER,
    XC_OUTPUT_UDP,
    XC_OUTPUT_TCP,
    XC_OUTPUT_BUF,
} enum_xc_output_type_t;

typedef enum
{
    XC_ASPECT_RATIO_DEFAULT = 0,
    XC_ASPECT_RATIO_1_TO_1,
    XC_ASPECT_RATIO_4_TO_3,
    XC_ASPECT_RATIO_16_TO_9,
    XC_ASPECT_RATIO_2_POINT_1_TO_1,

    XC_MPEG4_ASPECT_RATIO_DEFAULT,
    XC_MPEG4_ASPECT_RATIO_1_TO_1,
    XC_MPEG4_ASPECT_RATIO_12_TO_11,
    XC_MPEG4_ASPECT_RATIO_10_TO_11,
    XC_MPEG4_ASPECT_RATIO_16_TO_11,
    XC_MPEG4_ASPECT_RATIO_40_TO_33, 
} enum_xc_disp_aspect_ratio_t;

typedef enum
{
    XC_SHARPNESS_DEFAULT = 0,
    XC_SHARPNESS_SMOOTH,
    XC_SHARPNESS_MEDIUM,
    XC_SHARPNESS_SHARP,
} enum_xc_video_sharpness_t;

typedef enum
{
    XC_BLACKOUT_DEFAULT = 0,
    XC_BLACKOUT_BLACK,
    XC_BLACKOUT_GRAY,
    XC_BLACKOUT_BLUE,
    XC_BLACKOUT_GREEN,
    XC_BLACKOUT_RED
} enum_xc_blackout_color_t;

typedef enum 
{
    XC_PS_DEFAULT_PS = 0,
    XC_PS_ISO_PS,
    XC_PS_DVD_PS,
    XC_PS_DVD_VR_PS,
    XC_PS_CUSTOM_1_PS,
    XC_PS_CUSTOM_2_PS,
    XC_PS_CUSTOM_3_PS,
    XC_PS_MCE_PS,
    // MP4 types
    XC_PS_MP4_GEN_PS,
    XC_PS_MP4_MSV_PS,
    XC_PS_MP4_PSP_PS,
	XC_PS_MP4_PT_PS,
	XC_PS_MP4_3GP_PS,
	XC_PS_MP4_3GP_6_PS,
	XC_PS_MP4_3GP_7_PS,
	XC_PS_MP4_3GP_ITL_PS,
	XC_PS_MP4_3GP_ITL_RPL_PS,
	XC_PS_MP4_3GP_7_RPL_PS,
	XC_PS_MP4_GEN_RPL_PS,
} enum_xc_PS_TYPE_t;

typedef enum
{
    XC_MP4_LSYNC_EDITLIST = 0,
    XC_MP4_LSYNC_BASIC,
} enum_xc_mp4_lipsync_algo_t;

typedef enum
{
    XC_VFR_THRES_ZERO,
    XC_VFR_THRES_ONE,
    XC_VFR_THRES_TWO,
    XC_VFR_THRES_THREE,
    XC_VFR_THRES_FOUR
} enum_xc_VFRTHRESHOLD_t;

typedef enum
{
    XC_PROFILE_DEFAULT = 0,
    XC_PROFILE_SIMPLE,
    XC_PROFILE_ADVANCED_SIMPLE,
	XC_PROFILE_CUSTOM_0 = 256,
	// adding for ms format
	XC_EAVENC_MPV_PROFILE_UNKNOWN = 0,
	XC_EAVENC_MPV_PROFILE_SIMPLE = 1,
	XC_EAVENC_MPV_PROFILE_BASE = 1,
	XC_EAVENC_MPV_PROFILE_MAIN, //2
	XC_EAVENC_MPV_PROFILE_HIGH, //3
	XC_EAVENC_MPV_PROFILE_422,  //4
	XC_EAVENC_MPV_PROFILE_HIGH10, //5
	XC_EAVENC_MPV_PROFILE_444 // 6
} enum_xc_mpeg4_profile_t;

typedef enum
{
    XC_LEVEL_DEFAULT = 0,
    XC_LEVEL_0,
    XC_LEVEL_1,
    XC_LEVEL_2,
    XC_LEVEL_3,
    XC_LEVEL_4,
    XC_LEVEL_5, // mpeg4 ASP can have level5 max
	// adding for ms format
	XC_EAVENC_MPV_LEVEL_LOW         = 1,
	XC_EAVENC_MPV_LEVEL_MAIN 	    = 2,
	XC_EAVENC_MPV_LEVEL_HIGH1440    = 3,
	XC_EAVENC_MPV_LEVEL_HIGH 	    = 4,
	XC_EAVENC_MPV_LEVEL1            = 1,
	XC_EAVENC_MPV_LEVEL1_B          = 10,
	XC_EAVENC_MPV_LEVEL1_1 	        = 11,
	XC_EAVENC_MPV_LEVEL1_2 	        = 12,
	XC_EAVENC_MPV_LEVEL1_3 	        = 13,
	XC_EAVENC_MPV_LEVEL2 		    = 2,
	XC_EAVENC_MPV_LEVEL2_1 	        = 21,
	XC_EAVENC_MPV_LEVEL2_2 	        = 22,
	XC_EAVENC_MPV_LEVEL3 		    = 3,
	XC_EAVENC_MPV_LEVEL3_1 	        = 31,
	XC_EAVENC_MPV_LEVEL3_2 	        = 32,
	XC_EAVENC_MPV_LEVEL4 		    = 4,
	XC_EAVENC_MPV_LEVEL4_1 	        = 41,
	XC_EAVENC_MPV_LEVEL4_2 	        = 42,
	XC_EAVENC_MPV_LEVEL5 		    = 5,
	XC_EAVENC_MPV_LEVEL5_1		    = 51
} enum_xc_mpeg4_level_t;

typedef enum
{
    XC_QUANTTYPE_DEFAULT = 0,
    XC_QUANTTYPE_0,             // H.263
    XC_QUANTTYPE_1,             // MPEG
} enum_xc_mpeg4_quant_type_t;

typedef enum
{
    XC_GOP_B_DEFAULT   = 0x00,
    XC_GOP_0_B         = 0x01,
    XC_GOP_1_B         = 0x02,
    XC_GOP_2_B         = 0x03,
    XC_GOP_3_B         = 0x04,
    XC_GOP_B_MASK      = 0xFF,
    XC_GOP_B_SHIFT     = 0,
    
    XC_GOP_P_DEFAULT   = 0x0000,
    XC_GOP_0_P         = 0x0100,
    XC_GOP_1_P         = 0x0200,
    XC_GOP_2_P         = 0x0300,
    XC_GOP_3_P         = 0x0400,
    XC_GOP_4_P         = 0x0500,
    XC_GOP_5_P         = 0x0600,
    XC_GOP_9_P         = 0x0A00,
    XC_GOP_14_P        = 0x0F00,
    XC_GOP_29_P        = 0x1E00,
    XC_GOP_P_MASK      = 0xFF00,
    XC_GOP_P_SHIFT     = 8,

    XC_GOP_DEFAULT     = (XC_GOP_2_B | XC_GOP_4_P),
} enum_xc_GOP_TYPE_t;

typedef enum
{
    XC_UNKNOWN_AUDIO    = 0,
    XC_MP1              = 1,
    XC_MP2,
    XC_MP3,
    XC_AC3,
    XC_PCM,
    XC_D_V_IEC,
    XC_D_V_SMPTE,
    XC_A_A_C,
    XC_G726,
    XC_AMR,
    XC_MPEG4_AAC,
    XC_AACPLUS,
    XC_WMA,
    XC_OGG,
    XC_EAC3,
    XC_DOLBY_TRUEHD,
    XC_LPCM,
    XC_DTS_CORE,
} enum_xc_audio_type_t;

typedef enum
{
    XC_UNKNOWN_VIDEO    = 0,
    XC_MPEG1_STREAM     = 1,
    XC_MPEG2_VBR,
    XC_MPEG4_VBR,
    XC_MPEG2_CBR,
    XC_MPEG4_CBR,
    XC_MPEG4_AVC,
    XC_YUV_4_2_0,
    XC_YUV_4_2_2,
    XC_YUV_4_4_4,
    XC_RGB,
    XC_AVC_CBR,
    XC_VC1,
    XC_MVC,
} enum_xc_video_type_t;


typedef enum
{
    XC_CAPTURE_VBI_DISABLE = 0,
    XC_CAPTURE_VBI_ENABLE,
} enum_xc_capture_vbi_t;


#define XC_VBI_FLAG_INS_LINE21_IN_PIC                       0x00000001

#define XC_VBI_FLAG_INS_LINE21_IN_GOP                       0x00000002

#define XC_VBI_FLAG_DO_NOT_INS_LN21                         0x00000004

#define XC_VBI_FLAG_INS_LINE21_IN_PIC_ADDITIONAL_EIA608		0x00000008

#define XC_VBI_FLAG_LN21_MASK                  (XC_VBI_FLAG_INS_LINE21_IN_PIC | XC_VBI_FLAG_INS_LINE21_IN_GOP | XC_VBI_FLAG_DO_NOT_INS_LN21)

#define	XC_VBI_FLAG_INS_CCI_IN_RDI                          0x00000100

#define	XC_VBI_FLAG_INS_CCI_IN_PIC                          0x00000200

#define XC_VBI_FLAG_INS_PRV_DATA_IN_SEQ                     0x00001000

#define XC_VBI_FLAG_INS_PRV_DATA_IN_GOP                     0x00002000

#define XC_VBI_FLAG_DO_NOT_INS_PRV_DATA                     0x00004000

#define XC_VBI_FLAG_PRV_DATA_MASK              (XC_VBI_FLAG_INS_PRV_DATA_IN_SEQ | XC_VBI_FLAG_INS_PRV_DATA_IN_GOP | XC_VBI_FLAG_DO_NOT_INS_PRV_DATA)

#define XC_VBI_FLAG_INS_PRV_VBI_DATA_IN_PIC                 0x00000010

#define XC_VBI_FLAG_GOP_USER_DATA_PASSTHRU                  0x00008000

#define XC_VBI_FLAG_PIC_USER_DATA_PASSTHRU                  0x00010000


typedef enum
{
    STREAMING   = 0,
    PVR         = 1,
} enum_video_rc_mode_t;

typedef enum
{
    NO_FILTER   = 0,
    NOISE_FILTER_ONLY,
    COMB_FILTER_ONLY,
    NOISE_AND_COMB_FILTERS,
} enum_filter_mode_t;

typedef enum
{
    NORMAL_AUDIO_ENCODE = 0,
    AC3_BYPASS_MODE,
} enum_AC3_bypass_mode_t;

typedef enum 
{
    TBC_ENABLE,
    TBC_DISABLE,
} enum_xcode_tbc_enable_t;

typedef enum 
{
    TELECINE_ENABLE,
    TELECINE_DISABLE,
} enum_xcode_telecine_enable;

typedef enum
{
    MUX_FORMAT_UNKNOWN  = 0,

    // Raw Format, for video is YUV data, for audio is PCM data
    MUX_FORMAT_RAW      = 1,
    MUX_FORMAT_RAW_YUV  = MUX_FORMAT_RAW,
    MUX_FORMAT_RAW_PCM  = MUX_FORMAT_RAW,

    // AV Mux Format
    MUX_FORMAT_ES       = 2,
    MUX_FORMAT_PS       = 3,
    MUX_FORMAT_TS       = 4,
    MUX_FORMAT_TTS      = 5,
    MUX_FORMAT_MP4      = 6,
    MUX_FORMAT_PES      = 7,
    MUX_FORMAT_TS204    = 8,
    MUX_FORMAT_DSS      = 9,	// 130-byte packet DirectTV stream
    MUX_FORMAT_AVI      = 10,
    MUX_FORMAT_MKV      = 11,
    MUX_FORMAT_MOV      = 12,
    MUX_FORMAT_3GP      = 13,
    MUX_FORMAT_F4V      = 14,
    MUX_FORMAT_ISOM     = 15,   // Generic ISOM format, including MP4, 3GP, F4V, MOV ...
    MUX_FORMAT_FLV      = 16,
    MUX_FORMAT_ASF      = 17,   // ASF, WMV, WMA format

    // Audio Only Format
    MUX_FORMAT_WAV      = 20,

    // Image Format
    MUX_FORMAT_JPEG     = 30,
    MUX_FORMAT_BITMAP   = 31,
    MUX_FORMAT_TIFF_RGB = 32,
    MUX_FORMAT_TIFF_YUV = 33,
    MUX_FORMAT_PNG      = 34,
    MUX_FORMAT_GIF      = 35,
} enum_xc_mux_format_t;


typedef enum
{
    VIDEO_REFRESH_RATE_NONE     = 0x00000000,
    VIDEO_REFRESH_RATE_60HZ     = 0x01000000,   // 60 field (30 frame) per second
    VIDEO_REFRESH_RATE_50HZ     = 0x02000000,   // 50 field (25 frame) per second
    VIDEO_REFRESH_RATE_48HZ     = 0x04000000,   // 48 field (24 frame) per second
    VIDEO_REFRESH_RATE_120HZ    = 0x08000000,   // 120 field (60 frame) per second
    VIDEO_REFRESH_RATE_5994HZ   = 0x10000000,   // 59.94 field (29.97 frame) per second
    VIDEO_REFRESH_RATE_11988HZ  = 0x20000000,   // 119.88 field (59.94 frame) per second
    VIDEO_REFRESH_RATE_100HZ    = 0x40000000,   // 100 field (50 frame) per second
} enum_video_refresh_rate_t;


typedef enum
{
    ANALOG_DIGITAL_VIDEO_STANDARD_NONE          = 0x00000000,

    ANALOG_VIDEO_STANDARD_NTSC                  = 0x00000001,
    ANALOG_VIDEO_STANDARD_NTSC_J                = 0x00000002,
    ANALOG_VIDEO_STANDARD_NTSC_443_50           = 0x00000004,
    ANALOG_VIDEO_STANDARD_NTSC_443_60           = 0x00000008,
    ANALOG_VIDEO_STANDARD_PAL                   = 0x00000010,
    ANALOG_VIDEO_STANDARD_PAL_358               = 0x00000020,
    ANALOG_VIDEO_STANDARD_PAL_443_60            = 0x00000040,
    ANALOG_VIDEO_STANDARD_SECAM                 = 0x00000080,

    DIGITAL_VIDEO_STANDARD_480i                 = 0x00000001,
    DIGITAL_VIDEO_STANDARD_480p                 = 0x00000200,
    DIGITAL_VIDEO_STANDARD_576i                 = 0x00000010,
    DIGITAL_VIDEO_STANDARD_576p                 = 0x00000800,
    DIGITAL_VIDEO_STANDARD_720p                 = 0x00001000,
    DIGITAL_VIDEO_STANDARD_1080i                = 0x00002000,
    DIGITAL_VIDEO_STANDARD_1080p                = 0x00004000,
} enum_analog_digital_video_standard_t;


#define VIDEO_REFRESH_RATE_SHIFT        24
#define VIDEO_REFRESH_RATE_MASK         ((uint32)0xFF << VIDEO_REFRESH_RATE_SHIFT)
#define VIDEO_STANDARD_SHIFT            0
#define VIDEO_STANDARD_MASK             ((uint32)0xFFFFFF << VIDEO_STANDARD_SHIFT)


typedef enum
{
    ANALOG_AUDIO_STANDARD_NONE          = 0x00000000,
    ANALOG_AUDIO_STANDARD_BASEBAND      = 0x00000001,
    ANALOG_AUDIO_STANDARD_M             = 0x00000002,
    ANALOG_AUDIO_STANDARD_M_KOREA       = 0x00000004,
    ANALOG_AUDIO_STANDARD_M_BTSC        = 0x00000008,
    ANALOG_AUDIO_STANDARD_M_EIAJ        = 0x00000010,
    ANALOG_AUDIO_STANDARD_443           = 0x00000020,
    ANALOG_AUDIO_STANDARD_BG            = 0x00000040,
    ANALOG_AUDIO_STANDARD_BG_A2         = 0x00000080,
    ANALOG_AUDIO_STANDARD_BG_NICAM      = 0x00000100,
    ANALOG_AUDIO_STANDARD_DK            = 0x00000200,
    ANALOG_AUDIO_STANDARD_DK_A2_1       = 0x00000400,
    ANALOG_AUDIO_STANDARD_DK_A2_2       = 0x00000800,
    ANALOG_AUDIO_STANDARD_DK_A2_3       = 0x00001000,
    ANALOG_AUDIO_STANDARD_DK_NICAM      = 0x00002000,
    ANALOG_AUDIO_STANDARD_L_NICAM       = 0x00004000,
    ANALOG_AUDIO_STANDARD_L1_NICAM      = 0x00008000,
    ANALOG_AUDIO_STANDARD_I_NICAM       = 0x00010000,
    ANALOG_AUDIO_STANDARD_FM_107_50     = 0x00020000,
    ANALOG_AUDIO_STANDARD_FM_107_75     = 0x00040000,
    ANALOG_AUDIO_STANDARD_FM_50         = 0x00080000,
    ANALOG_AUDIO_STANDARD_FM_75         = 0x00100000,
} enum_analog_audio_standard_t;

typedef uint64 enum_analog_standard_t;

#define ANALOG_VIDEO_REFRESH_RATE_SHIFT 56
#define ANALOG_VIDEO_REFRESH_RATE_MASK  ((uint64)0xFF << ANALOG_VIDEO_REFRESH_RATE_SHIFT)
#define ANALOG_VIDEO_STANDARD_SHIFT     32
#define ANALOG_VIDEO_STANDARD_MASK      ((uint64)0xFFFFFF << ANALOG_VIDEO_STANDARD_SHIFT)
#define ANALOG_AUDIO_STANDARD_SHIFT     0
#define ANALOG_AUDIO_STANDARD_MASK      ((uint64)(0xFFFFFFFF) << ANALOG_AUDIO_STANDARD_SHIFT)

#define ANALOG_STANDARD_NONE          (((enum_analog_standard_t)ANALOG_DIGITAL_VIDEO_STANDARD_NONE << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_NONE << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_NONE)

#define ANALOG_STANDARD_NTSC_M        (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_NTSC << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_5994HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_M)
#define ANALOG_STANDARD_NTSC_M_KOREA  (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_NTSC << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_5994HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_M_KOREA)
#define ANALOG_STANDARD_NTSC_M_BTSC   (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_NTSC << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_5994HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_M_BTSC)
#define ANALOG_STANDARD_NTSC_M_EIAJ   (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_NTSC << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_5994HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_M_EIAJ)
#define ANALOG_STANDARD_NTSC_443      (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_NTSC << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_5994HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_443)

#define ANALOG_STANDARD_PAL_M         (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_PAL << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_M)
#define ANALOG_STANDARD_PAL_BG        (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_PAL << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_BG)
#define ANALOG_STANDARD_PAL_BG_A2     (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_PAL << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_BG_A2)
#define ANALOG_STANDARD_PAL_BG_NICAM  (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_PAL << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_BG_NICAM)
#define ANALOG_STANDARD_PAL_DK        (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_PAL << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_DK)
#define ANALOG_STANDARD_PAL_DK_A2_1   (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_PAL << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_DK_A2_1)
#define ANALOG_STANDARD_PAL_DK_A2_2   (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_PAL << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_DK_A2_2)
#define ANALOG_STANDARD_PAL_DK_A2_3   (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_PAL << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_DK_A2_3)
#define ANALOG_STANDARD_PAL_DK_NICAM  (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_PAL << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_DK_NICAM)
#define ANALOG_STANDARD_PAL_I_NICAM   (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_PAL << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_I_NICAM)

#define ANALOG_STANDARD_SECAM_BG      (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_SECAM << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_BG)
#define ANALOG_STANDARD_SECAM_BG_A2   (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_SECAM << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_BG_A2)
#define ANALOG_STANDARD_SECAM_BG_NICAM (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_SECAM << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_BG_NICAM)
#define ANALOG_STANDARD_SECAM_DK      (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_SECAM << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_DK)
#define ANALOG_STANDARD_SECAM_DK_A2_1 (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_SECAM << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_DK_A2_1)
#define ANALOG_STANDARD_SECAM_DK_A2_2 (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_SECAM << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_DK_A2_2)
#define ANALOG_STANDARD_SECAM_DK_A2_3 (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_SECAM << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_DK_A2_3)
#define ANALOG_STANDARD_SECAM_NICAM   (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_SECAM << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_DK_NICAM)
#define ANALOG_STANDARD_SECAM_L_NICAM  (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_SECAM << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_L_NICAM)
#define ANALOG_STANDARD_SECAM_L1_NICAM (((enum_analog_standard_t)ANALOG_VIDEO_STANDARD_SECAM << ANALOG_VIDEO_STANDARD_SHIFT) | ((enum_analog_standard_t)VIDEO_REFRESH_RATE_50HZ << ANALOG_VIDEO_REFRESH_RATE_SHIFT) | (enum_analog_standard_t)ANALOG_AUDIO_STANDARD_L1_NICAM)


struct analog_standard_t
{
    enum_analog_standard_t m_Standard;
    const char *m_StandardStr;
};

struct input_type_t
{
    enum_xc_input_type_t m_InputType;
    const char *m_InputTypeStr;
};

struct output_type_t
{
    enum_xc_output_type_t m_OutputType;
    const char *m_OutputTypeStr;
};
typedef enum
{
    HD_VIDEO_480I   = 0x0,
    HD_VIDEO_480P,
    HD_VIDEO_576I,
    HD_VIDEO_576P,
    HD_VIDEO_720P,
    HD_VIDEO_1080I,
} enum_hd_format_t;

typedef enum
{
    ANALOG_AUDIO_STATUS_NONE            = 0x00000000,
    ANALOG_AUDIO_STATUS_MONO            = 0x00000001,
    ANALOG_AUDIO_STATUS_DUAL            = 0x00000002,
    ANALOG_AUDIO_STATUS_STEREO          = 0x00000004,
} enum_analog_audio_status_t;

typedef enum
{
    AUDIO_MIX_NONE                  = 0,
    AUDIO_MIX_MONO                  = 1,
    AUDIO_MIX_STEREO                = 2,
    AUDIO_MIX_LANGUAGE_A            = 3,
    AUDIO_MIX_LANGUAGE_B            = 4,
    AUDIO_MIX_STEREO_SWAP           = 5,
    AUDIO_MIX_FM_AM_MONO            = 6,
} enum_audio_mix_t;

typedef enum
{
    VIDEO_STATUS_READY_MASK         = 0x00000001,
    VIDEO_STATUS_NOT_READY          = 0x00000000,
    VIDEO_STATUS_READY              = 0x00000001,

    VIDEO_STATUS_HVSYNC_LOCKED_MASK = 0x00000002,
    VIDEO_STATUS_HVSYNC_UNLOCKED    = 0x00000000,
    VIDEO_STATUS_HVSYNC_LOCKED      = 0x00000002,

    VIDEO_STATUS_INTERLACED_MASK    = 0x00000004,
    VIDEO_STATUS_NON_INTERLACED     = 0x00000000,
    VIDEO_STATUS_INTERLACED         = 0x00000004,

    VIDEO_STATUS_FILED_RATE_MASK    = 0x00000008,
    VIDEO_STATUS_50HZ_FILED_RATE    = 0x00000000,
    VIDEO_STATUS_60HZ_FILED_RATE    = 0x00000008,

    VIDEO_STATUS_MICROVISION_MASK   = 0x00000030,
    VIDEO_STATUS_NO_MICROVISION     = 0x00000000,
    VIDEO_STATUS_NORMAL_MV          = 0x00000010,
    VIDEO_STATUS_COLOR_STRIPE_MV    = 0x00000020,
    VIDEO_STATUS_CSTYPE3_MV         = 0x00000030,

    VIDEO_STATUS_STANDARD_MASK      = 0x000000C0,
    VIDEO_STATUS_NO_STANDARD        = 0x00000000,
    VIDEO_STATUS_NTSC_STANDARD      = 0x00000040,
    VIDEO_STATUS_PAL_STANDARD       = 0x00000080,
    VIDEO_STATUS_SECAM_STANDARD     = 0x000000C0,
} enum_video_status_t;

typedef enum
{
    TUNING_MODE_CABLE,
    TUNING_MODE_AIR,
} enum_tuning_mode_t;

typedef enum
{
    TUNER_MODE_AIR_TV   = 1,
    TUNER_MODE_CABLE_TV = 2,
    TUNER_MODE_FM_RADIO = 4,
    TUNER_MODE_AM_RADIO = 8,
    TUNER_MODE_DSS      = 16,
    TUNER_MODE_ATSC     = 32,
    TUNER_MODE_AIR_DTV  = 64,
    TUNER_MODE_CABLE_DTV= 128,
    TUNER_MODE_DVB_T    = 256,
} enum_tuner_mode_t;

typedef enum
{
    OPEN_MODE_NONE              = 0x00000000,
    OPEN_MODE_READ              = 0x00000001,
    OPEN_MODE_APPEND_RECORD     = 0x00000002,
    OPEN_MODE_TRUNCATE_RECORD   = 0x00000004,
    OPEN_MODE_EDIT              = 0x00000008,
    OPEN_MODE_OFFLINE_PLAYBACK  = 0x00000010,
    OPEN_MODE_REALTIME_PLAYBACK = 0x00000020,
    OPEN_MODE_GET_INFO          = 0x00000001,
    OPEN_MODE_DLNA_GET_INFO     = 0x00000001,
    OPEN_MODE_DLNA_READ         = 0x00000040,
} open_mode_t;

typedef enum
{
    PVR_ITEM_CATEGORY,
    PVR_ITEM_TITLE,
} pvr_item_type_t;

struct tuner_mode_caps_t
{
    enum_analog_standard_t m_StandardsSupported;
    unsigned int m_MinFrequency;
    unsigned int m_MaxFrequency;
    unsigned int m_MinStepFrequency;
    unsigned int m_FineStepFrequency;
    unsigned int m_CoarseStepFrequency;
    int m_AGCLevelMax;
    int m_AGCLevelMin;
};

struct analog_input_picture_attribute_t
{
    int m_VideoContrast;
    int m_VideoLuminance;
    int m_VideoSaturation;
    int m_VideoHue;
    int m_VideoSharpness;
    
#ifdef __cplusplus
    analog_input_picture_attribute_t(void)
        : m_VideoContrast (-1),
          m_VideoLuminance (-1),
          m_VideoSaturation (-1),
          m_VideoHue (-1),
          m_VideoSharpness (-1)
    {
    }

    analog_input_picture_attribute_t( const analog_input_picture_attribute_t &Defaults)
        : m_VideoContrast (Defaults.m_VideoContrast),
          m_VideoLuminance (Defaults.m_VideoLuminance),
          m_VideoSaturation (Defaults.m_VideoSaturation),
          m_VideoHue (Defaults.m_VideoHue),
          m_VideoSharpness (Defaults.m_VideoSharpness)
    {
    }

    analog_input_picture_attribute_t( int VideoContrast, int VideoLuminance, int VideoSaturation, int VideoHue, int VideoSharpness )
        : m_VideoContrast (VideoContrast),
          m_VideoLuminance (VideoLuminance),
          m_VideoSaturation (VideoSaturation),
          m_VideoHue (VideoHue),
          m_VideoSharpness (VideoSharpness)
    {
    }
#endif
};

//data type below is not listed on our design document
typedef enum
{
    AUDIO_INPUT_INDEX_LR            = 0,
    AUDIO_INPUT_INDEX_TV_TUNER      = 1,
    AUDIO_INPUT_INDEX_FM_TUNER      = 2,
    AUDIO_INPUT_INDEX_MUTE          = 100,
} enum_xc_audio_input_index_t;


typedef enum
{
    ES_AU_TYPE_UNKNOWN              = 0,

    ES_AU_TYPE_I_FRAME              = 1,
    ES_AU_TYPE_P_FRAME              = 2,
    ES_AU_TYPE_B_FRAME              = 3,
    ES_AU_TYPE_D_FRAME              = 4,
    ES_AU_TYPE_BI_FRAME             = 5,
    ES_AU_TYPE_SKIPPED_FRAME        = 6,

    ES_AU_TYPE_MPGA_AUDIO_FRAME     = 10,
    ES_AU_TYPE_LPCM_AUDIO_FRAME     = 11,
    ES_AU_TYPE_AC3_AUDIO_FRAME      = 12,
    ES_AU_TYPE_DDPLUS_AUDIO_FRAME   = 13,
    ES_AU_TYPE_DD_LOSSLESS_AUDIO_FRAME  = 14,
    ES_AU_TYPE_DTS_AUDIO_FRAME      = 15,
    ES_AU_TYPE_DTS_HD_AUDIO_FRAME   = 16,
    ES_AU_TYPE_AAC_AUDIO_FRAME      = 17,
    ES_AU_TYPE_MLP_AUDIO_FRAME      = 18,
    ES_AU_TYPE_MUTE_AUDIO_FRAME     = 29,   // A special internal dummy type

    ES_AU_TYPE_SUB_PICTURE_FRAME    = 30,
    ES_AU_TYPE_HIGHLIGHT_FRAME      = 31,

    ES_AU_TYPE_TEXT_SUBTITLE_FRAME  = 45,
    ES_AU_TYPE_GRAPHICS_FRAME       = 46,

    ES_AU_TYPE_OTHER_FRAME          = 50,

    ES_AU_TYPE_EOS                  = 100,
} au_type_t;


enum
{
    UNKNOWN_PROFILE             = 0,

    MPEG2_STANDARD_PROFILE      = 1,
    MPEG2_ENHANCED_PROFILE      = 2,

    MPEG4_STANDARD_PROFILE      = 3,
    MPEG4_ENHANCED_PROFILE      = 4,

    AVC_STANDARD_PROFILE        = 5,
    AVC_ENHANCED_PROFILE        = 6,

    VC1_STANDARD_PROFILE        = 7,
    VC1_ENHANCED_PROFILE        = 8,
};


typedef enum
{
    PIC_STRUCT_UNKNOWN              = 0,
    PIC_STRUCT_FRAME                = 1,
    PIC_STRUCT_TOP_FIELD            = 2,
    PIC_STRUCT_BOTTOM_FIELD         = 3,
    PIC_STRUCT_TOP_BOTTOM           = 4,
    PIC_STRUCT_BOTTOM_TOP           = 5,
    PIC_STRUCT_TOP_BOTTOM_TOP       = 6,
    PIC_STRUCT_BOTTOM_TOP_BOTTOM    = 7,
    PIC_STRUCT_FRAME_DOUBLE         = 8,
    PIC_STRUCT_FRAME_TRIPLE         = 9,
} picture_structure_t;


enum
{
    YUV_FORMAT_MASK1    = 0x0000000F,
    YUV_FORMAT_UYVY     = 0x00000001,
    YUV_FORMAT_IYUV     = 0x00000002,
    YUV_FORMAT_YV12     = 0x00000003,
    YUV_FORMAT_YV16     = 0x00000004,
    YUV_FORMAT_YUY2     = 0x00000005,

    YUV_FORMAT_MASK2    = 0x000000F0,
    YUV_FORMAT_444      = 0x00000010,
    YUV_FORMAT_422      = 0x00000020,
    YUV_FORMAT_420      = 0x00000030,

    GRAYSCALE_FORMAT_MASK = 0x00000F00,
    GRAYSCALE_FORMAT_1  = 0x00000100,
    GRAYSCALE_FORMAT_2  = 0x00000200,
    GRAYSCALE_FORMAT_4  = 0x00000300,
    GRAYSCALE_FORMAT_8  = 0x00000400,
    GRAYSCALE_FORMAT_16 = 0x00000500,

    RGB_FORMAT_MASK     = 0x0000F000,
    RGB_FORMAT_RGB1     = 0x00001000,
    RGB_FORMAT_RGB2     = 0x00002000,
    RGB_FORMAT_RGB4     = 0x00003000,
    RGB_FORMAT_RGB8     = 0x00004000,
    RGB_FORMAT_RGB15    = 0x00005000,
    RGB_FORMAT_RGB16    = 0x00006000,
    RGB_FORMAT_RGB24    = 0x00007000,
    RGB_FORMAT_RGB32    = 0x00008000,
    RGB_FORMAT_ARGB32   = 0x00009000,
};


typedef struct
{
    void *Y;
    void *Cr;
    void *Cb;

    unsigned int width;
    unsigned int height;
    unsigned int chroma_width;
    unsigned int chroma_height;
    
    unsigned int pitch_width;
    unsigned int pitch_height;
    unsigned int pitch_chroma_width;
    unsigned int pitch_chroma_height;
    
    unsigned int index;
} yuv_display_buffer_t;


typedef enum
{
    ES_STREAM_TYPE_UNKNOWN              = 0,
    ES_STREAM_TYPE_USER_DATA            = 1,
    ES_STREAM_TYPE_USER_PES_DATA        = 2,

    ES_STREAM_TYPE_MPEG1_VIDEO          = 10,
    ES_STREAM_TYPE_MPEG2_VIDEO          = 11,
    ES_STREAM_TYPE_MPEG4_VIDEO          = 12,
    ES_STREAM_TYPE_AVC_VIDEO            = 13,
    ES_STREAM_TYPE_MVC_VIDEO            = 14,
    ES_STREAM_TYPE_H263_VIDEO           = 15,
    ES_STREAM_TYPE_WMV_VIDEO            = 16,
    ES_STREAM_TYPE_VC1_VIDEO            = 17,

    ES_STREAM_TYPE_MPEG1_AUDIO          = 20,
    ES_STREAM_TYPE_MPEG2_AUDIO          = 21,
    ES_STREAM_TYPE_LPCM_AUDIO           = 22,
    ES_STREAM_TYPE_AC3_AUDIO            = 23,
    ES_STREAM_TYPE_DD_LOSSLESS_AUDIO    = 24,
    ES_STREAM_TYPE_DDPLUS_AUDIO         = 25,
    ES_STREAM_TYPE_DDPLUS2_AUDIO        = 26,
    ES_STREAM_TYPE_DTS_AUDIO            = 27,
    ES_STREAM_TYPE_DTS_HD_AUDIO         = 28,
    ES_STREAM_TYPE_DTS_HD_XLL_AUDIO     = 29,
    ES_STREAM_TYPE_DTS_HD_NONXLL_AUDIO  = 30,
    ES_STREAM_TYPE_DTS_HD_LBR_AUDIO     = 31,
    ES_STREAM_TYPE_AAC_AUDIO            = 32,
    ES_STREAM_TYPE_AMR_AUDIO            = 33,
    ES_STREAM_TYPE_WMA_AUDIO            = 34,
    ES_STREAM_TYPE_MLP_AUDIO            = 35,
    // Following audio formats are not implemented, but only reversed the poistions
    ES_STREAM_TYPE_MP3_AUDIO            = 36,   // MPEG1 layer 3
    ES_STREAM_TYPE_ATRAC_AUDIO          = 37,
    ES_STREAM_TYPE_SACD_AUDIO           = 38,
    ES_STREAM_TYPE_DST_AUDIO            = 39,

    ES_STREAM_TYPE_SUB_PICTURE          = 40,
    ES_STREAM_TYPE_HIGHLIGHT            = 41,

    ES_STREAM_TYPE_PRESENT_GRAPH        = 50,
    ES_STREAM_TYPE_INTERACTIVE_GRAPH    = 51,

    ES_STREAM_TYPE_TEXT_SUBTITLE        = 52,
} es_stream_type_t;


typedef enum
{
    AUDIO_CH_ASSIGN_NONE    = 0x00000000,

    AUDIO_CH_ASSIGN_L       = 0x00000001,
    AUDIO_CH_ASSIGN_R       = 0x00000002,
    AUDIO_CH_ASSIGN_LF      = 0x00000001,
    AUDIO_CH_ASSIGN_RF      = 0x00000002,
    AUDIO_CH_ASSIGN_LB      = 0x00000004,
    AUDIO_CH_ASSIGN_RB      = 0x00000008,
    AUDIO_CH_ASSIGN_LT      = 0x00000010,
    AUDIO_CH_ASSIGN_RT      = 0x00000020,
    AUDIO_CH_ASSIGN_LR_SUM  = 0x00000040,
    AUDIO_CH_ASSIGN_LR_DIF  = 0x00000080,

    AUDIO_CH_ASSIGN_C       = 0x00000100,
    AUDIO_CH_ASSIGN_CF      = 0x00000100,
    AUDIO_CH_ASSIGN_CB      = 0x00000200,
    AUDIO_CH_ASSIGN_CL      = 0x00000400,
    AUDIO_CH_ASSIGN_CR      = 0x00000800,

    AUDIO_CH_ASSIGN_S       = 0x00001000,
    AUDIO_CH_ASSIGN_SL      = 0x00002000,
    AUDIO_CH_ASSIGN_SR      = 0x00004000,
    AUDIO_CH_ASSIGN_SL1     = 0x00002000,
    AUDIO_CH_ASSIGN_SR1     = 0x00004000,
    AUDIO_CH_ASSIGN_SL2     = 0x00008000,
    AUDIO_CH_ASSIGN_SR2     = 0x00010000,
    AUDIO_CH_ASSIGN_SB      = 0x00020000,
    AUDIO_CH_ASSIGN_SLB     = 0x00040000,
    AUDIO_CH_ASSIGN_SRB     = 0x00080000,

    AUDIO_CH_ASSIGN_CH1     = 0x00100000,
    AUDIO_CH_ASSIGN_CH2     = 0x00200000,

    AUDIO_CH_ASSIGN_OV      = 0x01000000,

    AUDIO_CH_ASSIGN_LFE     = 0x40000000,
    AUDIO_CH_ASSIGN_LFE1    = 0x40000000,
    AUDIO_CH_ASSIGN_LFE2    = 0x80000000,
} audio_channel_assignment_t;
// L = left, R = right, C = center, S = surround, F = front, B = rear, T = total, OV = overhead.


typedef enum
{
    ALL_FILTER_DISABLED         = 0x00000000,
    DENOISER_ENABLED            = 0x00000001,
    DEINTERLACER_ENABLED        = 0x00000002,
    MOSQUITO_DENOISER_ENABLED   = 0x00000004,
    DEBLOCKER_ENABLED           = 0x00000008,
} video_filter_mode_t;


typedef struct
{
  int n;  /* numerator   */
  int d;  /* denominator */
} ratio_t;


typedef struct
{
    int strm_id;
    es_stream_type_t strm_type;
    int h_size;
    int v_size;
    int bit_rate;
    int interlaced;
    ratio_t aspect_ratio;
    ratio_t frame_rate;
    int pulldown_32;
    int profile;
    int digital_cc;
    int line21;
    int still_pic;
    unsigned int fourcc;
} video_parameter_t;

typedef struct
{
    int strm_id;
    es_stream_type_t strm_type;
    int channels;
    int quant_word_len;
    int sample_freq;
    int bitrate;
    int empasis;
    int down_mix_code;
    int mute;
    int dynamic_range;
    unsigned int channel_assign;
    unsigned char lang_code[2];
    int lang_ext;
    unsigned int fourcc;
    unsigned short format_tag;
} audio_parameter_t;


typedef enum
{
    SUBPIC_UNKNOWN_MODE     = 0,
    SUBPIC_RLE2_MODE1       = 1,    // 2 bit color depth, up to 53220 bytes
    SUBPIC_RLE2_MODE2       = 2,    // 2 bit color depth, up to 393216 bytes
    SUBPIC_RLE8_MODE        = 3,    // 8 bit color depth, up to 1572864 bytes
    BD_GRAPHICS_MODE        = 4,    // 8 bit color depth, Bluray graphics
    SUBTITLE_SRT_SSA_MODE   = 5,
    SUBTITLE_SRT_ASS_MODE   = 6,
    SUBTITLE_UTF8_TEXT_MODE = 7,
    SUBTITLE_VOBSUB_MODE    = 8,
    SUBTITLE_USF_MODE       = 9,
    SUBTITLE_KATE_MODE      = 10,
    SUBTITLE_BITMAP_MODE    = 11,
} subpic_mode_t;


typedef struct
{
    int strm_id;
    es_stream_type_t strm_type;
    uint8 lang_code[2];
    int lang_ext;
    subpic_mode_t mode;
} subpic_parameter_t;


typedef struct
{
    int strm_id;
    es_stream_type_t strm_type;
} highlight_parameter_t;


typedef struct
{
    int strm_id;
    es_stream_type_t strm_type;
} graphics_parameter_t;

typedef struct
{
    unsigned char y;
    unsigned char u;
    unsigned char v;
} yuv_t;


typedef struct
{
    unsigned char y;
    unsigned char cb;
    unsigned char cr;
} ycbcr_t;


typedef struct
{
    int x;
    int y;
} point_t;


typedef struct
{
    int x;
    int y;
    int w;
    int h;
} rectangle_t;


typedef struct
{
    unsigned char a;
    unsigned char r;
    unsigned char g;
    unsigned char b;
} rgb_t;


typedef enum
{
    CONTENT_TYPE_UNKNOWN,
    CONTENT_TYPE_IMAGE,
    CONTENT_TYPE_AUDIO,
    CONTENT_TYPE_VIDEO,
} enum_content_type_t;

struct content_attributes_t
{
    enum_content_type_t content_type;
    int is_SD_in;
    enum_xc_mux_format_t content_format;
    es_stream_type_t video_type;
    es_stream_type_t audio_type;
    int h_size;
    int v_size;
    int bit_rate;
    int audio_bit_rate;
    int video_bit_rate;
    unsigned int width;
    unsigned int height;
    unsigned int timestamp;
    int is_Support_RangeSeek;
    int is_Support_TimeSeek;
    int is_Support_CI_Param;
    unsigned short audio_format_tag;
};

typedef enum
{
    TSI_DISABLE         = 0,
    TSI_ENABLE          = 1,
} tsi_port_enable_t;

typedef enum
{
    TSI_SERIAL_MODE     = 0,
    TSI_PARALLEL_MODE   = 1,
} tsi_port_data_mode_t;

typedef enum
{
    TSI_PACKET_SIZE_188         = 0,
    TSI_PACKET_SIZE_192         = 1,
    TSI_PACKET_SIZE_204         = 2,
    TSI_PACKET_SIZE_DIRECTTV    = 3,
} tsi_port_packet_size_t;

typedef enum
{
    TSI_SYNC_BYTE_OFF   = 0,
    TSI_SYNC_BYTE_ON    = 1,
} tsi_port_sync_mode_t;

typedef enum
{
    TSI_DATA_LOCK_POSITIVE_EDGE = 0,
    TSI_DATA_LOCK_NEGATIVE_EDGE = 1,
} tsi_port_data_lock_polarity_t;

typedef enum
{
    TSI_VALID_LOW       = 0,
    TSI_VALID_HIGH      = 1,
} tsi_port_valid_polarity_t;

typedef enum
{
    TSI_KEEP_NULL_PACKET    = 0,
    TSI_DROP_NULL_PACKET    = 1,
} tsi_port_null_packet_t;

struct tsd_configure_t
{
    tsi_port_enable_t               enable;
    tsi_port_data_mode_t            data_mode;
    tsi_port_packet_size_t          packet_size;
    tsi_port_sync_mode_t            sync_mode;
    tsi_port_data_lock_polarity_t   data_lock_polarity;
    tsi_port_valid_polarity_t       valid_polarity;
    tsi_port_null_packet_t          null_packet;
};

typedef enum
{
    DVO_SERIAL_MODE     = 0,
    DVO_PARALLEL_MODE   = 1,
} dvo_port_data_mode_t;

typedef enum
{
    DVO_PACKET_SIZE_188     = 0,
    DVO_PACKET_SIZE_192     = 1,
} dvo_port_packet_size_t;

typedef enum
{
    DVO_CLOCK_6_75MHZ   = 0,
    DVO_CLOCK_13_5MHZ   = 1,
    DVO_CLOCK_27MHZ     = 2,
    DVO_CLOCK_54MHZ     = 3,
    DVO_CLOCK_108MHZ    = 4,
} dvo_port_clock_speed_t;

typedef enum
{
    DVO_CLOCK_NOT_INVERTED  = 0,
    DVO_CLOCK_INVERTED      = 1,
} dvo_port_clock_inversion_t;

typedef enum
{
    DVO_VALID_LOW       = 0,
    DVO_VALID_HIGH      = 1,
} dvo_port_valid_polarity_t;

typedef enum
{
    DVO_NULL_STUFFING_DISABLE   = 0,
    DVO_NULL_STUFFING_ENABLE    = 1,
} dvo_port_null_stuffing_t;

typedef enum
{
    DVO_1_CYCLE_SYNC_DURATION   = 0,
    DVO_8_CYCLE_SYNC_DURATION   = 1,
} dvo_port_serial_sync_duration_t;

typedef enum
{
    DVO_PCR_INCREMENT_DISABLE   = 0,
    DVO_PCR_INCREMENT_ENABLE    = 1,
} dvo_port_pcr_increment_t;

typedef enum
{
    DVO_NO_PACKT_SAPCING        = 0,
    DVO_SINGLE_PACKT_SAPCING    = 1,
    DVO_DOUBLE_PACKT_SAPCING    = 2,
} dvo_port_packet_spacing_t;

struct dvo_configure_t
{
    dvo_port_data_mode_t                data_mode;
    dvo_port_packet_size_t              packet_size;
    dvo_port_clock_speed_t              clock_speed;
    dvo_port_clock_inversion_t          clock_polarity;
    dvo_port_valid_polarity_t           valid_polarity;
    dvo_port_null_stuffing_t            null_stuffing;
    dvo_port_serial_sync_duration_t     sync_duration;
    dvo_port_pcr_increment_t            pcr_inc;
    dvo_port_packet_spacing_t           spacing;
};

struct ts_properties_t
{
    int video_pid;
    enum_xc_video_type_t video_format;
    video_parameter_t video_parameter;
    int num_audio;
    int audio_pid[MAX_NUM_XCODE_AUDIOS];
    enum_xc_audio_type_t audio_format[MAX_NUM_XCODE_AUDIOS];
    audio_parameter_t audio_parameter[MAX_NUM_XCODE_AUDIOS];
    int pcr_pid;
    int pmt_pid;
    int program_number;
    int program_ID;
    int is_SD_in;
};

struct photo_album_properties_t
{
    char m_Root[PATH_MAX];
    char m_Path[PATH_MAX];
    char m_ExtIncluded[88];
    int m_FrameRepeatNum;
    int m_SubDirIncluded;
    int m_Sort;
    int m_Effect;
    int m_Repeat;
};

struct udp_properties_t
{
    char m_Host[PATH_MAX];
    unsigned int m_Port;
};

struct tcp_properties_t
{
    char m_Host[PATH_MAX];
    unsigned int m_Port;
};

struct tv_properties_t
{
    int m_TvChannelIndex;
    struct ts_properties_t m_ts_properties;
};

typedef enum
{
    NO_PROPERTY,
    FILE_PATH,
    RTSP_PROPERTY,
    TS_PROPERTY,
    ATV_PROPERTY,
    DTV_PROPERTY,
    UDP_PROPERTY,
    HTTP_PROPERTY,
    PHOTO_PROPERTY,
} m_properties_type_t;


struct rtsp_properties_t
{
    char m_URL[PATH_MAX];
    char m_Username[PATH_MAX];
    char m_Password[PATH_MAX];
};


#ifdef __cplusplus

struct timer_recording_record_t
{
    mutable int m_RecordId;
    char m_Name[64];
    int m_XcodeChannel;
    int m_InputIndex;
    int m_OutputIndex;
    time_t m_StartTime;
    time_t m_EndTime;
    m_properties_type_t m_InputPropertiesType;
    m_properties_type_t m_OutputPropertiesType;
    int m_Pvr;
    char m_InResolution[64];
    char m_OutResolution[64];
    enum_xc_mux_format_t m_Format;
    enum_xc_video_type_t m_VideoType;
    enum_xc_audio_type_t m_AudioType;
    int m_BitRate;
    tsd_configure_t m_TSDConfig;
    union
    {
        char m_InputFilePath[PATH_MAX];
        rtsp_properties_t m_rtsp_properties;
        tv_properties_t m_tv_properties;
        ts_properties_t m_ts_properties;
		photo_album_properties_t m_photo_album_properties;
	    char m_http_url[PATH_MAX];
	    unsigned int m_udp_port;
    } m_InputProperties;
    union
    {
	    char m_OutputFilePath[PATH_MAX];
	    udp_properties_t m_udp_properties;
        dvo_configure_t m_DVOConfig;
    } m_OutputProperties;
    bool m_Started;

    timer_recording_record_t(void) : m_Started(false)
    {
    }

    struct is_ovrelap : std::binary_function<timer_recording_record_t*, timer_recording_record_t*, bool>
    {
        bool operator()(const timer_recording_record_t *pRecordA, const timer_recording_record_t *pRecordB) const
        {
            //A in B
            if ( ((pRecordA->m_StartTime >= pRecordB->m_StartTime) &&
                  (pRecordA->m_StartTime <= pRecordB->m_EndTime)) || 
                 ((pRecordA->m_EndTime >= pRecordB->m_StartTime) &&
                  (pRecordA->m_EndTime <= pRecordB->m_EndTime)) )
            {
                return true;
            }
            //B in A
            else if ( ((pRecordB->m_StartTime >= pRecordA->m_StartTime) &&
                  (pRecordB->m_StartTime <= pRecordA->m_EndTime)) || 
                 ((pRecordB->m_EndTime >= pRecordA->m_StartTime) &&
                  (pRecordB->m_EndTime <= pRecordA->m_EndTime)) )
            {
                return true;
            }
            return false;
        }
    };

    struct has_id : std::binary_function<timer_recording_record_t*, int, bool>
    {
        bool operator()(const timer_recording_record_t *pRecord, int RecordId) const
        {
            return pRecord->m_RecordId == RecordId;
        }
    };

    struct start_before : std::binary_function<timer_recording_record_t*, timer_recording_record_t*, bool>
    {
        bool operator()(const timer_recording_record_t *pRecordA, const timer_recording_record_t *pRecordB) const
        {
            return pRecordA->m_StartTime < pRecordB->m_StartTime;
        }
    };

    struct end_at : std::binary_function<timer_recording_record_t*, time_t, bool>
    {
        bool operator()(const timer_recording_record_t *pRecord, time_t EndTime) const
        {
            if ( pRecord->m_Started )
            {
                return pRecord->m_EndTime <= EndTime;
            }
            return false;
        }
    };

    struct before : std::binary_function<timer_recording_record_t*, time_t, bool>
    {
        bool operator()(const timer_recording_record_t *pRecord, time_t EndTime) const
        {
            return pRecord->m_EndTime <= EndTime;
        }
    };

    struct start_at : std::binary_function<timer_recording_record_t*, time_t, bool>
    {
        bool operator()(const timer_recording_record_t *pRecord, time_t StartTime) const
        {
            if ( !pRecord->m_Started )
            {
                return pRecord->m_StartTime <= StartTime;
            }
            return false;
        }
    };

    struct start_or_end_before : std::binary_function<timer_recording_record_t*, timer_recording_record_t*, bool>
    {
        bool operator()(const timer_recording_record_t *pRecordA, const timer_recording_record_t *pRecordB) const
        {
            if ( pRecordA->m_Started )
            {
                if ( pRecordB->m_Started )
                {
                    return pRecordA->m_EndTime < pRecordB->m_EndTime;
                }
                else
                {
                    return pRecordA->m_EndTime < pRecordB->m_StartTime;
                }
            }
            else
            {
                if ( pRecordB->m_Started )
                {
                    return pRecordA->m_StartTime < pRecordB->m_EndTime;
                }
                else
                {
                    return pRecordA->m_StartTime < pRecordB->m_StartTime;
                }
            }
        }
    };

    struct start_or_end_before_time : std::binary_function<timer_recording_record_t*, time_t, bool>
    {
        bool operator()(const timer_recording_record_t *pRecord, const time_t CheckingTime) const
        {
            if ( pRecord->m_Started )
            {
                return pRecord->m_EndTime < CheckingTime;
            }
            else
            {
                return pRecord->m_StartTime < CheckingTime;
            }
        }
    };
};

#endif // __cplusplus


typedef enum
{
    DVD_VIDEO_AUTHORING,
    DVD_VR_AUTHORING,
    HDDVD_AUTHORING,
    BDMV_AUTHORING,
    BDAV_AUTHORING,
    AUTHORING_TYPE_NOT_DEFINED,
} enum_authoring_profile_t;


struct file_info_t
{
  /** The length of the file. A length less than 0 indicates the size 
   *  is unknown, and data will be sent until 0 bytes are returned from
   *  a read call. */
  int file_length;

  /** The time at which the contents of the file was modified;
   *  The time system is always local (not GMT). */
  time_t last_modified;

  /** If the file is a directory, {\bf is_directory} contains
   * a non-zero value. For a regular file, it should be 0. */
  int is_directory;

  /** If the file or directory is readable, this contains 
   * a non-zero value. If unreadable, it should be set to 0. */
  int is_readable;

  /** The content type of the file. This string needs to be allocated 
   *  by the caller using {\bf ixmlCloneDOMString}.  When finished 
   *  with it, the SDK frees the {\bf DOMString}. */
   
  char content_type[80];

};

#ifdef __cplusplus

struct event_info_t
{
    enum event_type_t
    {
        STARTTIMEREDRECORDING,
        STOPTIMEREDRECORDING,
    };

    int m_XcodeChannel;
    time_t m_StartTime;
    event_type_t m_EventType;

    event_info_t(int XcodeChannel, time_t StartTime, event_type_t EventType) : m_XcodeChannel(XcodeChannel), m_StartTime(StartTime), m_EventType(EventType)
    {
    }
};

typedef std::list<event_info_t> event_info_list_t;

#endif

typedef enum
{
    DISK_TYPE_NONE              = 0x0000,

    DISK_TYPE_CD_ROM            = 0x0008,
    DISK_TYPE_CD_R              = 0x0009,
    DISK_TYPE_CD_RW             = 0x000A,

    DISK_TYPE_DVD_ROM           = 0x0010,

    DISK_TYPE_DVD_R             = 0x0011,
    DISK_TYPE_DVD_RAM           = 0x0012,
    DISK_TYPE_DVD_RW            = 0x0013,
    DISK_TYPE_DVD_R_DL          = 0x0015,
    DISK_TYPE_DVD_RW_DL         = 0x0017,

    DISK_TYPE_DVD_PLUS_RW       = 0x001A,
    DISK_TYPE_DVD_PLUS_R        = 0x001B,
    DISK_TYPE_DVD_PLUS_RW_DL    = 0x002A,
    DISK_TYPE_DVD_PLUS_R_DL     = 0x002B,

    DISK_TYPE_DVD_BD_ROM        = 0x0040,
    DISK_TYPE_DVD_BD_R          = 0x0041,
    DISK_TYPE_DVD_BD_RE         = 0x0043,

    DISK_TYPE_DVD_HD_DVD_ROM    = 0x0050,
    DISK_TYPE_DVD_HD_DVD_R      = 0x0051,
    DISK_TYPE_DVD_HD_DVD_RAM    = 0x0052,

    DISK_TYPE_NO_SUPPORTED      = 0xFFFF,
} disk_type_t;

typedef struct 
{
    unsigned int m_Frequency;
    int m_ChannelNum;
    char Name[64];
    enum_analog_standard_t m_Standard;
} program_table_item_t;

typedef enum
{
    UNKNOWN_LANGUAGE    = 0,
    ENGLISH,
    FRENCH,
    ITALIAN,
    SPANISH,
    CHINESE,
    JAPANESE,
    TURKISH,
} enum_authoring_language_t;

//refer to http://forum.doom9.org/showthread.php?s=&threadid=83539 for DVD language code

typedef enum
{
    OUTPUT_TO_ISO_FILE       = 0,
    OUTPUT_TO_OPTICAL_DISK   = 1,
    OUTPUT_TO_DIRECTORY      = 2,
} disk_output_mode_t;


typedef enum
{
    // General playback control
    STREAM_STOP,
    STREAM_PAUSE,
    STREAM_PLAY,
    STREAM_REPLAY,

    // Special trick modes with hard coded speed
    STREAM_SFWx2,
    STREAM_SFWx8,
    STREAM_SFWx16,
    STREAM_FFWx1_5,
    STREAM_FFWx2,
    STREAM_FFWx4,
    STREAM_FFWx8,
    STREAM_FFWx16,
    STREAM_FFWx30,
    STREAM_FFWx40,
    STREAM_FFWx100,
    STREAM_SRWx2,
    STREAM_SRWx8,
    STREAM_SRWx16,
    STREAM_RWD,
    STREAM_FRWx2,
    STREAM_FRWx4,
    STREAM_FRWx8,
    STREAM_FRWx16,
    STREAM_FRWx30,
    STREAM_FRWx40,
    STREAM_FRWx100,

    // Special trick modes
    STREAM_STEP_ONCE            = 80,

    // Generic trick modes
    STREAM_FWD_IPB_GENERIC      = 100,
    STREAM_FWD_IP_GENERIC,
    STREAM_FWD_I_GENERIC,
    STREAM_RWD_IPB_GENERIC,             // Not support yet
    STREAM_RWD_IP_GENERIC,              // Not support yet
    STREAM_RWD_I_GENERIC,

    // Flags can or-ed to the normal trick modes
    STREAM_FLAG_MASK            = 0xFF000000,
    STREAM_FLAG_NO_AUDIO        = 0x01000000,
} stream_mode_t;


#ifndef WIN32
typedef enum 
{
    STREAM_SEEK_SET,
    STREAM_SEEK_CUR,
} stream_seek_origin_t;
#endif


typedef enum
{
    AUDIO_OUTPUT_PORT_I2S       = 0,    // PCM
    AUDIO_OUTPUT_PORT_SPDIF,
#ifdef XCODE4_VIPER_CHIP
    AUDIO_OUTPUT_PORT_HDMI,
#endif
    AUDIO_OUTPUT_PORTS_MAX,
} audio_output_port_type_t;


typedef enum
{
    SPDIF_OUTPUT_NONE,
    SPDIF_OUTPUT_PCM16,
    SPDIF_OUTPUT_PCM24,
    SPDIF_OUTPUT_RAW,
    SPDIF_OUTPUT_AAC,
    SPDIF_OUTPUT_AC3,
    SPDIF_OUTPUT_MPGA,
    SPDIF_OUTPUT_DTS,
    SPDIF_OUTPUT_DDPLUS,
    SPDIF_OUTPUT_DTS_MA,
    SPDIF_OUTPUT_DOLBY_TRUEHD,
    SPDIF_OUTPUT_AUTO,
} spdif_output_format_t;


typedef enum
{
    HDMI_OUTPUT_NONE,
    HDMI_OUTPUT_PCM_2CH,
    HDMI_OUTPUT_PCM_MULTICH,
    HDMI_OUTPUT_BYPASS,
    HDMI_OUTPUT_AAC,
    HDMI_OUTPUT_AC3,
    HDMI_OUTPUT_MPGA,
    HDMI_OUTPUT_DTS,
    HDMI_OUTPUT_DDPLUS,
    HDMI_OUTPUT_DTS_MA,
    HDMI_OUTPUT_DOLBY_TRUEHD,
    HDMI_OUTPUT_WMA,
    HDMI_OUTPUT_AUTO,
} hdmi_output_format_t;


typedef enum_authoring_language_t enum_language_t;

#ifdef __cplusplus
typedef std::deque<program_table_item_t> program_list_t;
#endif


struct rtsp_server_open_properties_t
{
    int m_InputIndex;
    m_properties_type_t m_PropertiesType;
    tsd_configure_t m_TSDConfig;
    union
    {
        char m_InputFilePath[PATH_MAX];
        struct rtsp_properties_t m_rtsp_properties;
        struct tv_properties_t m_tv_properties;
        struct ts_properties_t m_ts_properties;
    } m_InputProperties;
};


#define NEW1(type)          (type*)malloc(sizeof(type))
#define RENEW1(p,type)      (type*)realloc((p), sizeof(type))
#define NEW(type,num)       (type*)malloc(sizeof(type) * (num))
#define RENEW(p,type,num)   (type*)realloc((p), sizeof(type) * (num))


#ifdef WIN32

#define thread_t    HANDLE
#define mutex_t     HANDLE
#define signal_t    HANDLE

#define CREATE_THREAD(thread, func, stack_size, para, detached)     \
                                    ((thread) = CreateThread(NULL, (stack_size), (func), (para), 0, NULL))
#define EXIT_THREAD(err)            ExitThread(err)

#define CREATE_MUTEX(mutex)         (((mutex) = CreateMutex(NULL, FALSE, NULL)) == NULL)
#define DESTROY_MUTEX(mutex)        (CloseHandle(mutex) != TRUE)
#define TRY_LOCK_MUTEX(mutex)       (WaitForSingleObject((mutex), 0L) != WAIT_OBJECT_0)
#define LOCK_MUTEX(mutex)           (WaitForSingleObject((mutex), INFINITE) != WAIT_OBJECT_0)
#define UNLOCK_MUTEX(mutex)         (ReleaseMutex((mutex)) != TRUE)

#define CREATE_SIGNAL(signal, max)  (((signal) = CreateSemaphore(NULL, 0, (max), NULL)) == NULL)
#define DESTROY_SIGNAL(signal)      (CloseHandle(signal) != TRUE)
#define WAIT_SIGNAL(signal, mutex)  (WaitForSingleObject((signal), INFINITE) != WAIT_OBJECT_0)
#define WAIT_SIGNAL_TIMEOUT(signal, mutex, timeout) \
                                    (WaitForSingleObject((signal), timeout) != WAIT_OBJECT_0)
#define SIGNAL(signal)              (ReleaseSemaphore((signal), 1, NULL) != TRUE)
#define BROADCAST(signal)           (ReleaseSemaphore((signal), 1, NULL) != TRUE)

#else   // LINUX

#define thread_t    pthread_t
#if !(USE_INTERLOCK_REGISTER)
#define mutex_t     pthread_mutex_t
#define signal_t    pthread_cond_t
#else
#define mutex_t     volatile int *
#define signal_t    volatile int *
#endif

#define CREATE_THREAD(thread, func, stack_size, para, detached)                                 \
                                    {                                                           \
                                        pthread_attr_t thread_attr;                             \
                                        pthread_attr_init(&thread_attr);                        \
                                        pthread_attr_setstacksize(&thread_attr, (stack_size));  \
                                        if (detached)                                           \
                                        {                                                       \
                                            pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED); \
                                        }                                                       \
                                        pthread_create(&(thread), &thread_attr, (func), (para));\
                                        pthread_attr_destroy(&thread_attr);                     \
                                    }
#define EXIT_THREAD(err)            pthread_exit(err)

#if !(USE_INTERLOCK_REGISTER)

#define CREATE_MUTEX(mutex)         pthread_mutex_init(&(mutex), NULL)  //((mutex) = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER)
#define DESTROY_MUTEX(mutex)        pthread_mutex_destroy(&(mutex))
#define TRY_LOCK_MUTEX(mutex)       pthread_mutex_trylock(&(mutex))
#define LOCK_MUTEX(mutex)           pthread_mutex_lock(&(mutex))
#define UNLOCK_MUTEX(mutex)         pthread_mutex_unlock(&(mutex))

#define CREATE_SIGNAL(signal, max)  pthread_cond_init(&(signal), NULL)  //((signal) = (pthread_cond_t)PTHREAD_COND_INITIALIZER)
#define DESTROY_SIGNAL(signal)      pthread_cond_destroy(&(signal))

#ifdef STABLE_KERNEL
#define WAIT_SIGNAL(signal, mutex)  pthread_cond_wait(&(signal), &(mutex))
#else
#define WAIT_SIGNAL(signal, mutex)                          \
                                    {                       \
                                        UNLOCK_MUTEX(mutex);\
                                        usleep(10000);      \
                                        LOCK_MUTEX(mutex);  \
                                    }
#endif

#define WAIT_SIGNAL_TIMEOUT(signal, mutex, timeout) \
                                    pthread_cond_timedwait(&(signal), &(mutex), &(timeout))
#define SIGNAL(signal)              pthread_cond_signal(&(signal))
#define BROADCAST(signal)           pthread_cond_broadcast(&(signal))

#else // USE_INTERLOCK_REGISTER

#include "xcodeio.h"
#define XC_MIPS_INTERLOCK15_REG     0x04CC
static xcode_io_t *mutexxcodeio = xcode_io_t::get_io();

#define CREATE_MUTEX(mutex)         { (mutex) = new int; *(mutex) = 0; }
#define DESTROY_MUTEX(mutex)        delete (mutex);
#define TRY_LOCK_MUTEX(mutex, result)                                                                       \
                                    {                                                                       \
                                        while(1) /* Try to lock mutex_of_mutex */                           \
                                        {                                                                   \
                                            mutexxcodeio->mmreg_write(XC_MIPS_INTERLOCK15_REG, 0x80000055); \
                                            if ((mutexxcodeio->mmreg_read(XC_MIPS_INTERLOCK15_REG) & 0xFF) == 0x55)   \
                                            {                                                               \
                                                break;                                                      \
                                            }                                                               \
                                            usleep(10000);                                                  \
                                        }                                                                   \
                                        if (*(mutex) == 0)                                                  \
                                        {                                                                   \
                                            (result) = 0;                                                   \
                                            *(mutex) = 1;                                                   \
                                        }                                                                   \
                                        else                                                                \
                                        {                                                                   \
                                            (result) = EBUSY;                                               \
                                        }                                                                   \
                                        mutexxcodeio->mmreg_write(XC_MIPS_INTERLOCK15_REG, 0x55);           \
                                    }
#define LOCK_MUTEX(mutex)                                                                                       \
                                    {                                                                           \
                                        while(1)                                                                \
                                        {                                                                       \
                                            while(1) /* Try to lock mutex_of_mutex */                           \
                                            {                                                                   \
                                                mutexxcodeio->mmreg_write(XC_MIPS_INTERLOCK15_REG, 0x80000055); \
                                                if ((mutexxcodeio->mmreg_read(XC_MIPS_INTERLOCK15_REG) & 0xFF) == 0x55)   \
                                                {                                                               \
                                                    break;                                                      \
                                                }                                                               \
                                                usleep(10000);                                                  \
                                            }                                                                   \
                                            if (*(mutex) == 0)                                                  \
                                            {                                                                   \
                                                break;                                                          \
                                            }                                                                   \
                                            mutexxcodeio->mmreg_write(XC_MIPS_INTERLOCK15_REG, 0x55);           \
                                            usleep(10000);                                                      \
                                        }                                                                       \
                                        *(mutex) = 1;                                                           \
                                        mutexxcodeio->mmreg_write(XC_MIPS_INTERLOCK15_REG, 0x55);               \
                                    }
#define UNLOCK_MUTEX(mutex)                                                                                 \
                                    {                                                                       \
                                        while(1) /* Try to lock mutex_of_mutex */                           \
                                        {                                                                   \
                                            mutexxcodeio->mmreg_write(XC_MIPS_INTERLOCK15_REG, 0x80000055); \
                                            if ((mutexxcodeio->mmreg_read(XC_MIPS_INTERLOCK15_REG) & 0xFF) == 0x55)   \
                                            {                                                               \
                                                break;                                                      \
                                            }                                                               \
                                            usleep(10000);                                                  \
                                        }                                                                   \
                                        *(mutex) = 0;                                                       \
                                        mutexxcodeio->mmreg_write(XC_MIPS_INTERLOCK15_REG, 0x55);           \
                                    }

#define CREATE_SIGNAL(signal, max)  { (signal) = new int; *(signal) = 0; }
#define DESTROY_SIGNAL(signal)      delete (signal);
#if 1
#define WAIT_SIGNAL(signal, mutex)                          \
                                    {                       \
                                        UNLOCK_MUTEX(mutex);\
                                        usleep(10000);      \
                                        LOCK_MUTEX(mutex);  \
                                    }
#else
#define WAIT_SIGNAL(signal, mutex)                              \
                                    {                           \
                                        UNLOCK_MUTEX(mutex);    \
                                        while(!(*(signal)))     \
                                        {                       \
                                            usleep(10000);      \
                                        }                       \
                                        LOCK_MUTEX(mutex);      \
                                        *(signal) = 0;          \
                                    }
#endif
#define WAIT_SIGNAL_TIMEOUT(signal, mutex, timeout)             \
                                    {                           \
                                        UNLOCK_MUTEX(mutex);    \
                                        while(!(*(signal)) && ((timeout.tv_sec) > 0))    \
                                        {                       \
                                            usleep(10000);      \
                                            timeout.tv_sec --;  \
                                        }                       \
                                        LOCK_MUTEX(mutex);      \
                                        if ((timeout.tv_sec) > 0)      \
                                        {                       \
                                            *(signal) = 0;      \
                                        }                       \
                                    }
#define SIGNAL(signal)              *(signal) = 1;
#define BROADCAST(signal)           SIGNAL(signal)

#endif // USE_INTERLOCK_REGISTER

#endif


#define SET_1BE(x, y)   { *(uint8 *)&(x) = (uint8)(y); }
#define SET_2BE(x, y)   { ((uint8 *)&(x))[0] = (uint8)((y) >> 8);  ((uint8 *)&(x))[1] = (uint8)(y); }
#define SET_3BE(x, y)   { ((uint8 *)&(x))[0] = (uint8)((y) >> 16); ((uint8 *)&(x))[1] = (uint8)((y) >> 8);  ((uint8 *)&(x))[2] = (uint8)(y); }
#define SET_4BE(x, y)   { ((uint8 *)&(x))[0] = (uint8)((y) >> 24); ((uint8 *)&(x))[1] = (uint8)((y) >> 16); ((uint8 *)&(x))[2] = (uint8)((y) >> 8); ((uint8 *)&(x))[3] = (uint8)(y); }
#define SET_8BE(x, y)   { SET_4BE(((uint8 *)&(x))[4], (uint32)(y)); SET_4BE(x, (uint64)(y) >> 32); }

#define GET_1BE(x)      (*(uint8 *)&(x))
#define GET_2BE(x)      (uint16)(((uint8 *)&(x))[0] << 8 | ((uint8 *)&(x))[1])
#define GET_3BE(x)      (uint32)(((uint8 *)&(x))[0] << 16 | ((uint8 *)&(x))[1] << 8 | ((uint8 *)&(x))[2])
#define GET_4BE(x)      (uint32)(((uint8 *)&(x))[0] << 24 | ((uint8 *)&(x))[1] << 16 | ((uint8 *)&(x))[2] << 8 | ((uint8 *)&(x))[3])
#define GET_8BE(x)      (uint64)(((uint64)GET_4BE(x) << 32) | (uint64)GET_4BE(((uint8 *)&(x))[4]))

#if 1
#define SET_1LE(x, y)   { *(uint8 *)&(x) = (uint8)(y); }
#define SET_2LE(x, y)   { ((uint8 *)&(x))[1] = (uint8)((y) >> 8);  ((uint8 *)&(x))[0] = (uint8)(y); }
#define SET_3LE(x, y)   { ((uint8 *)&(x))[2] = (uint8)((y) >> 16); ((uint8 *)&(x))[1] = (uint8)((y) >> 8);  ((uint8 *)&(x))[0] = (uint8)(y); }
#define SET_4LE(x, y)   { ((uint8 *)&(x))[3] = (uint8)((y) >> 24); ((uint8 *)&(x))[2] = (uint8)((y) >> 16); ((uint8 *)&(x))[1] = (uint8)((y) >> 8); ((uint8 *)&(x))[0] = (uint8)(y); }
#define SET_8LE(x, y)   { SET_4BE(x, (uint32)(y)); SET_4BE(((uint8 *)&(x))[4], (uint64)(y) >> 32); }

#define GET_1LE(x)      (*(uint8 *)&(x))
#define GET_2LE(x)      (uint16)(((uint8 *)&(x))[1] << 8 | ((uint8 *)&(x))[0])
#define GET_3LE(x)      (uint32)(((uint8 *)&(x))[2] << 16 | ((uint8 *)&(x))[1] << 8 | ((uint8 *)&(x))[0])
#define GET_4LE(x)      (uint32)(((uint8 *)&(x))[3] << 24 | ((uint8 *)&(x))[2] << 16 | ((uint8 *)&(x))[1] << 8 | ((uint8 *)&(x))[0])
#define GET_8LE(x)      (uint64)((uint64)GET_4LE(x) | ((uint64)GET_4LE(((uint8 *)&(x))[4]) << 32))
#else
// For Little-Endian CPU, it can just use this one for better performance
#define SET_1LE(x, y)   { *(uint8 *)&(x) = (uint8)(y); }
#define SET_2LE(x, y)   { *(uint16 *)&(x) = (uint16)(y); }
#define SET_3LE(x, y)   { ((uint8 *)&(x))[2] = (uint8)((y) >> 16); ((uint8 *)&(x))[1] = (uint8)((y) >> 8);  ((uint8 *)&(x))[0] = (uint8)(y); }
#define SET_4LE(x, y)   { *(uint32 *)&(x) = (uint32)(y); }
#define SET_8LE(x, y)   { *(uint64 *)&(x) = (uint64)(y); }

#define GET_1LE(x)      (*(uint8 *)&(x))
#define GET_2LE(x)      (*(uint16 *)&(x))
#define GET_3LE(x)      (((uint8 *)&(x))[2] << 16 | ((uint8 *)&(x))[1] << 8 | ((uint8 *)&(x))[0])
#define GET_4LE(x)      (*(uint32 *)&(x))
#define GET_8LE(x)      (*(uint64 *)&(x))
#endif


#ifdef __cplusplus

class auto_lock_t
{
public:
    auto_lock_t(mutex_t *pMutex)
        : m_pMutex(pMutex)
    {
        LOCK_MUTEX(*m_pMutex);
    }

    ~auto_lock_t(void)
    {
        UNLOCK_MUTEX(*m_pMutex);
    }

private:
    mutex_t *m_pMutex;
};

template<typename __TYPE__>
class xc_auto_destroy_t
{
public:
    xc_auto_destroy_t(__TYPE__ * pIns, bool IsArray = false ):m_pIns(pIns),m_IsArray(IsArray) {}
    ~xc_auto_destroy_t()
    {
        if (NULL != m_pIns)
        {
            if (m_IsArray)
            {
                delete []m_pIns;
            }
            else
            {
                delete m_pIns;
            }
        }
    }
    void reset(void) { m_pIns = NULL; m_IsArray = false; }
private:
    __TYPE__ * m_pIns;
    bool m_IsArray;
};

#endif // __cplusplus


enum open_file_mode_t
{
    FILE_READ, FILE_WRITE
};


inline int get_time_ms()
{
    int now;
#ifdef WIN32
    LARGE_INTEGER frequency;
    LARGE_INTEGER current;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&current);
	current.QuadPart /= (frequency.QuadPart / 1000);
    now = current.LowPart;
#else
    struct timeval current;
    gettimeofday(&current, NULL);
    now = current.tv_sec * 1000 + current.tv_usec / 1000;
#endif

    return now;
}

struct network_info_t
{
    char m_Device[18];
    char m_Method[18];
    int m_IPAddr[4];
    int m_Subnet[4];
};

#ifdef __cplusplus
namespace dtcp
{
#endif
#ifndef __E_EMI_CODE_T__
#define __E_EMI_CODE_T__
	enum e_emi_code_t
	{
		E_EMI_COPY_NEVER = 12,                
		E_EMI_COPY_ONE_GEN_FORMAT_COG = 10,		
		E_EMI_COPY_ONE_GEN_FORMAT_NON_COG = 8,	
		E_EMI_MOVE = 6,                      
		E_EMI_NO_MORE_COPIES = 4,             
		E_EMI_COPY_FREE_EPN_ASSERTED = 2,		
		E_EMI_COPY_FREE = 0,					
		E_EMI_UNKNOWN = -1                   
	};
#endif
#ifdef __cplusplus
}
#endif



typedef struct
{
    au_type_t Format;
    int SampleRate;
    int BitPerSample;
    int NumOfChannel;
    unsigned int ChannelAssign;
    int64 PTS;
    int FrameSize;
} audio_frame_info_t;

typedef struct
{
    au_type_t PictureType;
    picture_structure_t PictureStructure;
    int64 PTS;
    int64 DTS;
    int Width;
    int Height;
    int AspectRatio;
    int FrameRateCode;
    int FrameRateCodeAVCExtension;
    int BitRate;
    int VbvBufferSize;
    int Progressive;
    uint64 DisplayExtention;
    int DisplayHorizontalSize;
    int DisplayVerticalSize;
    int LowDelay;
    int FrameCropTopOffset;
    int FrameCropBottomOffset;
    int FrameCropLeftOffset;
    int FrameCropRightOffset;
    int RepeatFirstField;
    struct
    {
	    int pan_scan_rect_id;
	    int pan_scan_rect_cancel_flag;
	    int pan_scan_cnt_minus1;
	    int pan_scan_rect_left_offset[3];
	    int pan_scan_rect_right_offset[3];
	    int pan_scan_rect_top_offset[3];
	    int pan_scan_rect_bottom_offset[3];
	    int pan_scan_rect_repetition_period;
    } PanscanRectangleSei;
    int prevGOPSizeInBytes;
} video_picture_info_t;

typedef struct
{
    int Type;
    int64 PTS;
    unsigned int Size;
    void *Buf;
} user_data_info_t;

typedef void (*audio_frame_decoded_callback_fn_t)(void *pArg, const audio_frame_info_t *pAudioFrameInfo);

typedef void (*video_picture_decoded_callback_fn_t)(void *pArg, const video_picture_info_t *pVideoPictureInfo);

typedef void (*user_data_extract_callback_fn_t)(void *pArg, const user_data_info_t *pUserDataInfo);

typedef void (*video_resolution_change_callback_fn_t)(void *pArg, const video_picture_info_t *pVideoPictureInfo);

typedef void (*decode_render_eos_callback_fn_t)(void *pArg);

typedef void (*render_video_pts_callback_t)(void *pArg, int64 PTS);

typedef void (*video_picture_marker_callback_fn_t)(void *pArg, int Markers);

typedef struct
{
    int ContrastSupport;
    int LuminanceSupport;
    int SaturationSupport;
    int HueSupport;
    int SharpnessSupport;
} video_attributes_support_info_t;


typedef size_t (* riff_read_callback_t)(void *handle, void *buf, size_t size, int full_read);
typedef int64_t (* riff_seek_callback_t)(void *handle, int64_t offset, int origin);
typedef int64_t (* riff_tell_callback_t)(void *handle);

typedef struct
{
    riff_read_callback_t m_Read;
    riff_seek_callback_t m_Seek;
    riff_tell_callback_t m_Tell;
    void *m_pHandle;
    void *m_pArg;
    uint64 m_Offset;
} source_access_callbacks_t;


typedef struct 
{
    int m_IsLastReceive; // 0--no 1--yes
    int m_Type; // 0-- video 1--audio
    uint32 mp4_mdat_size;
    uint32 mp4_mdat_size_offset;
    uint32 mp4_mdat_size_hi;
    uint32 mp4_current_framerate;
} receive_end_t;


typedef enum
{
    PLAYER_PRIORITY_MASK            = 0x000000FF,
    INDEPENDENT_PLAYER              = 0x00000000,   // Players are absolutely independent from each other
    PRIMARY_PLAYER                  = 0x00000001,   // Player has connection to each other, but may or may not sync to each other
    SECONDARY_PLAYER                = 0x00000002,   // Player has connection to each other, but may or may not sync to each other
    TERTIARY_PLAYER                 = 0x00000003,   // Player has connection to each other, but may or may not sync to each other

    STC_MASTER_FLAG                 = 0x80000000,   // This player drives STC, other players sync to this STC
    STC_SLAVE_FLAG                  = 0x00000000,   // This player need to sync to STC, but cannot drive STC or change STC status
} player_priority_t;


// Defines used for audio mixing
#define GAIN_SCALE_UP_POWER_2   13
// Convert float coefficiency to integer coefficient (scaled by 8192 to 16bit unsigned integer)
#define INT_GAIN(x)             (unsigned int)((x) * (1 << GAIN_SCALE_UP_POWER_2) + 0.5)


enum cci_code_t
{
    CCI_COPY_FREE = 0,
    CCI_NO_MORE_COPIES = 1,
    CCI_COPY_ONE_GENERATION = 2,
    CCI_COPY_NEVER = 3,
    
    //OTHERS for extension.
};

enum author_charset_t
{
    AUTHOR_CHARSET_ASCII = 0,   // english
    AUTHOR_CHARSET_ISO8859_1 = 1, 
    AUTHOR_CHARSET_SHIFT_JIS = 2, // shift-jis
    AUTHOR_CHARSET_KS_X_1001 = 3,
    AUTHOR_CHARSET_KS_X_1005 = 4,
    AUTHOR_CHARSET_ISO8859_2 = 5,
    AUTHOR_CHARSET_ISO8859_3 = 6,
    AUTHOR_CHARSET_ISO8859_4 = 7,    
    AUTHOR_CHARSET_ISO8859_5 = 8,
    AUTHOR_CHARSET_ISO8859_6 = 9,
    AUTHOR_CHARSET_ISO8859_7 = 10,
    AUTHOR_CHARSET_ISO8859_8 = 11,
    AUTHOR_CHARSET_ISO8859_9 = 12,
    AUTHOR_CHARSET_ISO8859_15 = 13,
    AUTHOR_CHARSET_IEC10646_UNICODE = 14,
    AUTHOR_CHARSET_GB18030 = 15,
    AUTHOR_CHARSET_BIG5 = 16,
    AUTHOR_CHARSET_UCS2 = 17,
    AUTHOR_CHARSET_UCS4 = 18,
    AUTHOR_CHARSET_UTF8 = 19,
    AUTHOR_CHARSET_UTF16 = 20,
    AUTHOR_CHARSET_ISO_2022_JP = 21, //ISO-2022-JP.
};

/**
* @brief The encryption flags in authoring.
*/
enum author_encrypto_flags_t
{
    AUTHOR_ENCRYPTO_AUTODETECT = 0,         ///< Detect CCI in contents
    AUTHOR_ENCRYPTO_FORCE_ENABLE = 1,    ///< Force to use encryption.
    AUTHOR_ENCRYPTO_FORCE_DISABLE = 2   ///< Force to not use encryption.
};


// Following function will return a static string contains the Middleware build name and build time
extern const char *query_mw_version(void);


struct firmware_status_t
{
    unsigned int status;
    unsigned int flags;
    unsigned int buffer_overwrite;
    unsigned int video_error_frames_dropped;
    unsigned int video_error;
    unsigned int video_timestamp_error;
    unsigned int discontinuous_timestamp;
    unsigned int last_video_error_frame_num;
    uint64 last_video_error_pts;
    uint64 last_video_error_dts;
    unsigned int last_timestamp_error_frame_num;
    uint64 last_timestamp_error_pts;
    uint64 last_timestamp_error_dts;
};


#endif  /* XCTYPE_H__5FA901AE_F23E_4F89_9C02_3F9224A4F84A__INCLUDED_ */

