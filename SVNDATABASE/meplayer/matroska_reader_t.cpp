/***************************************************************************\
 *                                                                           *
 *   Copyright 2006 ViXS Systems Inc.  All Rights Reserved.                  *
 *                                                                           *
 *===========================================================================*
 *                                                                           *
 *   File Name: matroska.cpp                                                 *
 *                                                                           *
 *   Description:                                                            *
 *       This file contains the implementation of the MATROSKA file reader.  *
 *                                                                           *
 *===========================================================================*

\***************************************************************************/

/*********************************************************************************/
/*********************************************************************************/
/**                                                                             **/
/**  KNOWN ISSUE:                                                               **/
/**                                                                             **/
/**  1. Doesn't support multiple Segments                                       **/
/**  2. Not all video/audio/sub-title codec are supported                       **/
/**  3. Control/button codec are not supported                                  **/
/**  4. Stream must contain Cues for seeking                                    **/
/**  5. Compression is not supported (except header stripping)                  **/
/**  6. Encryption is not supported                                             **/
/**                                                                             **/
/*********************************************************************************/
/*********************************************************************************/

//#ifdef ENABLE_MKV_SUPPORT
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "matroska_reader_t.h"
#include "ebml.h"
//#include "common.h"
namespace DMP{

//#define printf(...)        xclog::printf(27, __VA_ARGS__);     // Borrow log #27
//#define DEBUG_READER_PRINT(...) xclog::printf(27, __VA_ARGS__);     // Borrow log #27

#ifndef array_size
#define array_size(x) (sizeof(x))/(sizeof((x)[0]))
#endif
#ifndef AV_RB32
#define AV_RB32(x)                           \
		((((const uint8_t*)(x))[0] << 24) |         \
				(((const uint8_t*)(x))[1] << 16) |         \
				(((const uint8_t*)(x))[2] <<  8) |         \
				((const uint8_t*)(x))[3])
#endif
#ifndef AV_RL32
#define AV_RL32(x)                           \
		((((const uint8_t*)(x))[3] << 24) |         \
				(((const uint8_t*)(x))[2] << 16) |         \
				(((const uint8_t*)(x))[1] <<  8) |         \
				((const uint8_t*)(x))[0])
#endif
#ifndef AV_RB16
#define AV_RB16(x)                           \
		((((const uint8_t*)(x))[0] << 8) |          \
				((const uint8_t*)(x))[1])
#endif
#ifndef offsetof
#define offsetof(s,m) ((size_t)&(((s*)0)->m))
#endif
#ifndef VXMIN
#define VXMIN(a,b) (a)>(b)?(b):(a)
#endif
#ifndef VXMAX
#define VXMAX(a,b) (a)>(b)?(a):(b)
#endif
#ifndef MKTAG
#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))
#endif
#define URL_RDONLY 				0
#define URL_WRONLY 				1
#define URL_RDWR   				2
#define IO_BUFFER_SIZE 			32768
#define AVSEEK_FORCE 			0x20000
#define SHORT_SEEK_THRESHOLD 	4096
#define AV_PKT_FLAG_KEY   		0x0001
#define ME_BUFFER_SIZE			0x800000
#define FF_INPUT_BUFFER_PADDING_SIZE 8
#define AUDIO_AVC_HEADER_SIZE	7
#define AV_LZO_INPUT_DEPLETED 1
//! decoded data did not fit into output buffer
#define AV_LZO_OUTPUT_FULL 2
//! a reference to previously decoded data was wrong
#define AV_LZO_INVALID_BACKPTR 4
//! a non-specific error in the compressed bitstream
#define AV_LZO_ERROR 8
/** \} */

#define AV_LZO_INPUT_PADDING 8
#define AV_LZO_OUTPUT_PADDING 12
//#define UNALIGNED_LOADSTORE
#define BUILTIN_MEMCPY
#ifdef UNALIGNED_LOADSTORE
#define COPY2(d, s) *(uint16_t *)(d) = *(uint16_t *)(s);
#define COPY4(d, s) *(uint32_t *)(d) = *(uint32_t *)(s);
#elif defined(BUILTIN_MEMCPY)
#define COPY2(d, s) memcpy(d, s, 2);
#define COPY4(d, s) memcpy(d, s, 4);
#else
#define COPY2(d, s) (d)[0] = (s)[0]; (d)[1] = (s)[1];
#define COPY4(d, s) (d)[0] = (s)[0]; (d)[1] = (s)[1]; (d)[2] = (s)[2]; (d)[3] = (s)[3];
#endif


static unsigned char avc_start_code[] = { 0, 0, 0, 1 };

const CodecTags ff_mkv_codec_tags[]={
		{"A_AAC"            , CODEC_ID_AAC			,ES_STREAM_TYPE_AAC_AUDIO},
		{"A_AC3"            , CODEC_ID_AC3			,ES_STREAM_TYPE_AC3_AUDIO},
		{"A_DTS"            , CODEC_ID_DTS			,ES_STREAM_TYPE_DTS_AUDIO},
		{"A_EAC3"           , CODEC_ID_EAC3			,ES_STREAM_TYPE_AC3_AUDIO},
		{"A_FLAC"           , CODEC_ID_FLAC			,ES_STREAM_TYPE_AAC_AUDIO},
		{"A_MLP"            , CODEC_ID_MLP			,ES_STREAM_TYPE_AAC_AUDIO},
		{"A_MPEG/L2"        , CODEC_ID_MP2			,ES_STREAM_TYPE_MPEG1_AUDIO},
		{"A_MPEG/L1"        , CODEC_ID_MP2			,ES_STREAM_TYPE_MPEG1_AUDIO},
		{"A_MPEG/L3"        , CODEC_ID_MP3			,ES_STREAM_TYPE_LPCM_AUDIO},
		{"A_PCM/FLOAT/IEEE" , CODEC_ID_PCM_F32LE	,ES_STREAM_TYPE_AAC_AUDIO},
		{"A_PCM/FLOAT/IEEE" , CODEC_ID_PCM_F64LE	,ES_STREAM_TYPE_LPCM_AUDIO},
		{"A_PCM/INT/BIG"    , CODEC_ID_PCM_S16BE	,ES_STREAM_TYPE_LPCM_AUDIO},
		{"A_PCM/INT/BIG"    , CODEC_ID_PCM_S24BE	,ES_STREAM_TYPE_LPCM_AUDIO},
		{"A_PCM/INT/BIG"    , CODEC_ID_PCM_S32BE	,ES_STREAM_TYPE_LPCM_AUDIO},
		{"A_PCM/INT/LIT"    , CODEC_ID_PCM_S16LE	,ES_STREAM_TYPE_LPCM_AUDIO},
		{"A_PCM/INT/LIT"    , CODEC_ID_PCM_S24LE	,ES_STREAM_TYPE_LPCM_AUDIO},
		{"A_PCM/INT/LIT"    , CODEC_ID_PCM_S32LE	,ES_STREAM_TYPE_LPCM_AUDIO},
		{"A_PCM/INT/LIT"    , CODEC_ID_PCM_U8		,ES_STREAM_TYPE_LPCM_AUDIO},
		{"A_QUICKTIME/QDM2" , CODEC_ID_QDM2			,ES_STREAM_TYPE_UNKNOWN},
		{"A_REAL/14_4"      , CODEC_ID_RA_144		,ES_STREAM_TYPE_UNKNOWN},
		{"A_REAL/28_8"      , CODEC_ID_RA_288		,ES_STREAM_TYPE_UNKNOWN},
		{"A_REAL/ATRC"      , CODEC_ID_ATRAC3		,ES_STREAM_TYPE_UNKNOWN},
		{"A_REAL/COOK"      , CODEC_ID_COOK			,ES_STREAM_TYPE_UNKNOWN},
		{"A_REAL/SIPR"      , CODEC_ID_SIPR			,ES_STREAM_TYPE_UNKNOWN},
		{"A_TRUEHD"         , CODEC_ID_TRUEHD		,ES_STREAM_TYPE_UNKNOWN},
		{"A_TTA1"           , CODEC_ID_TTA			,ES_STREAM_TYPE_UNKNOWN},
		{"A_VORBIS"         , CODEC_ID_VORBIS		,ES_STREAM_TYPE_UNKNOWN},
		{"A_WAVPACK4"       , CODEC_ID_WAVPACK		,ES_STREAM_TYPE_UNKNOWN},

		{"S_TEXT/UTF8"      , CODEC_ID_TEXT			,ES_STREAM_TYPE_SUB_PICTURE},
		{"S_TEXT/UTF8"      , CODEC_ID_SRT			,ES_STREAM_TYPE_SUB_PICTURE},
		{"S_TEXT/ASCII"     , CODEC_ID_TEXT			,ES_STREAM_TYPE_SUB_PICTURE},
		{"S_TEXT/ASS"       , CODEC_ID_SSA			,ES_STREAM_TYPE_SUB_PICTURE},
		{"S_TEXT/SSA"       , CODEC_ID_SSA			,ES_STREAM_TYPE_SUB_PICTURE},
		{"S_ASS"            , CODEC_ID_SSA			,ES_STREAM_TYPE_SUB_PICTURE},
		{"S_SSA"            , CODEC_ID_SSA			,ES_STREAM_TYPE_SUB_PICTURE},
		{"S_VOBSUB"         , CODEC_ID_DVD_SUBTITLE	,ES_STREAM_TYPE_SUB_PICTURE},
		{"S_HDMV/PGS"       , CODEC_ID_HDMV_PGS_SUBTITLE,ES_STREAM_TYPE_SUB_PICTURE},

		{"V_DIRAC"          , CODEC_ID_DIRAC		,ES_STREAM_TYPE_UNKNOWN},
		{"V_MJPEG"          , CODEC_ID_MJPEG		,},
		{"V_MPEG1"          , CODEC_ID_MPEG1VIDEO	,ES_STREAM_TYPE_MPEG1_VIDEO},
		{"V_MPEG2"          , CODEC_ID_MPEG2VIDEO	,ES_STREAM_TYPE_MPEG2_VIDEO},
		{"V_MPEG4/ISO/ASP"  , CODEC_ID_MPEG4		,ES_STREAM_TYPE_MPEG4_VIDEO},
		{"V_MPEG4/ISO/AP"   , CODEC_ID_MPEG4		,ES_STREAM_TYPE_MPEG4_VIDEO},
		{"V_MPEG4/ISO/SP"   , CODEC_ID_MPEG4		,ES_STREAM_TYPE_MPEG4_VIDEO},
		{"V_MPEG4/ISO/AVC"  , CODEC_ID_H264			,ES_STREAM_TYPE_AVC_VIDEO},
		{"V_MPEG4/MS/V3"    , CODEC_ID_MSMPEG4V3	,ES_STREAM_TYPE_MPEG4_VIDEO},
		{"V_REAL/RV10"      , CODEC_ID_RV10			,ES_STREAM_TYPE_UNKNOWN},
		{"V_REAL/RV20"      , CODEC_ID_RV20			,ES_STREAM_TYPE_UNKNOWN},
		{"V_REAL/RV30"      , CODEC_ID_RV30			,ES_STREAM_TYPE_UNKNOWN},
		{"V_REAL/RV40"      , CODEC_ID_RV40			,ES_STREAM_TYPE_UNKNOWN},
		{"V_SNOW"           , CODEC_ID_SNOW			,ES_STREAM_TYPE_UNKNOWN},
		{"V_THEORA"         , CODEC_ID_THEORA		,ES_STREAM_TYPE_UNKNOWN},
		{"V_UNCOMPRESSED"   , CODEC_ID_RAWVIDEO		,ES_STREAM_TYPE_UNKNOWN},
		{"V_VP8"            , CODEC_ID_VP8			,ES_STREAM_TYPE_UNKNOWN},

		{""                 , CODEC_ID_NONE			,ES_STREAM_TYPE_UNKNOWN}
};

const AVCodecTag ff_codec_bmp_tags[] = {
		{ CODEC_ID_H264,         MKTAG('H', '2', '6', '4') },
		{ CODEC_ID_H264,         MKTAG('h', '2', '6', '4') },
		{ CODEC_ID_H264,         MKTAG('X', '2', '6', '4') },
		{ CODEC_ID_H264,         MKTAG('x', '2', '6', '4') },
		{ CODEC_ID_H264,         MKTAG('a', 'v', 'c', '1') },
		{ CODEC_ID_H264,         MKTAG('V', 'S', 'S', 'H') },
		{ CODEC_ID_H263,         MKTAG('H', '2', '6', '3') },
		{ CODEC_ID_H263,         MKTAG('X', '2', '6', '3') },
		{ CODEC_ID_H263,         MKTAG('T', '2', '6', '3') },
		{ CODEC_ID_H263,         MKTAG('L', '2', '6', '3') },
		{ CODEC_ID_H263,         MKTAG('V', 'X', '1', 'K') },
		{ CODEC_ID_H263,         MKTAG('Z', 'y', 'G', 'o') },
		{ CODEC_ID_H263P,        MKTAG('H', '2', '6', '3') },
		{ CODEC_ID_H263I,        MKTAG('I', '2', '6', '3') }, /* intel h263 */
		{ CODEC_ID_H261,         MKTAG('H', '2', '6', '1') },
		{ CODEC_ID_H263P,        MKTAG('U', '2', '6', '3') },
		{ CODEC_ID_H263P,        MKTAG('v', 'i', 'v', '1') },
		{ CODEC_ID_MPEG4,        MKTAG('F', 'M', 'P', '4') },
		{ CODEC_ID_MPEG4,        MKTAG('D', 'I', 'V', 'X') },
		{ CODEC_ID_MPEG4,        MKTAG('D', 'X', '5', '0') },
		{ CODEC_ID_MPEG4,        MKTAG('X', 'V', 'I', 'D') },
		{ CODEC_ID_MPEG4,        MKTAG('M', 'P', '4', 'S') },
		{ CODEC_ID_MPEG4,        MKTAG('M', '4', 'S', '2') },
		{ CODEC_ID_MPEG4,        MKTAG( 4 ,  0 ,  0 ,  0 ) }, /* some broken avi use this */
		{ CODEC_ID_MPEG4,        MKTAG('D', 'I', 'V', '1') },
		{ CODEC_ID_MPEG4,        MKTAG('B', 'L', 'Z', '0') },
		{ CODEC_ID_MPEG4,        MKTAG('m', 'p', '4', 'v') },
		{ CODEC_ID_MPEG4,        MKTAG('U', 'M', 'P', '4') },
		{ CODEC_ID_MPEG4,        MKTAG('W', 'V', '1', 'F') },
		{ CODEC_ID_MPEG4,        MKTAG('S', 'E', 'D', 'G') },
		{ CODEC_ID_MPEG4,        MKTAG('R', 'M', 'P', '4') },
		{ CODEC_ID_MPEG4,        MKTAG('3', 'I', 'V', '2') },
		{ CODEC_ID_MPEG4,        MKTAG('W', 'A', 'W', 'V') }, /* WaWv MPEG-4 Video Codec */
		{ CODEC_ID_MPEG4,        MKTAG('F', 'F', 'D', 'S') },
		{ CODEC_ID_MPEG4,        MKTAG('F', 'V', 'F', 'W') },
		{ CODEC_ID_MPEG4,        MKTAG('D', 'C', 'O', 'D') },
		{ CODEC_ID_MPEG4,        MKTAG('M', 'V', 'X', 'M') },
		{ CODEC_ID_MPEG4,        MKTAG('P', 'M', '4', 'V') },
		{ CODEC_ID_MPEG4,        MKTAG('S', 'M', 'P', '4') },
		{ CODEC_ID_MPEG4,        MKTAG('D', 'X', 'G', 'M') },
		{ CODEC_ID_MPEG4,        MKTAG('V', 'I', 'D', 'M') },
		{ CODEC_ID_MPEG4,        MKTAG('M', '4', 'T', '3') },
		{ CODEC_ID_MPEG4,        MKTAG('G', 'E', 'O', 'X') },
		{ CODEC_ID_MPEG4,        MKTAG('H', 'D', 'X', '4') }, /* flipped video */
		{ CODEC_ID_MPEG4,        MKTAG('D', 'M', 'K', '2') },
		{ CODEC_ID_MPEG4,        MKTAG('D', 'I', 'G', 'I') },
		{ CODEC_ID_MPEG4,        MKTAG('I', 'N', 'M', 'C') },
		{ CODEC_ID_MPEG4,        MKTAG('E', 'P', 'H', 'V') }, /* Ephv MPEG-4 */
		{ CODEC_ID_MPEG4,        MKTAG('E', 'M', '4', 'A') },
		{ CODEC_ID_MPEG4,        MKTAG('M', '4', 'C', 'C') }, /* Divio MPEG-4 */
		{ CODEC_ID_MPEG4,        MKTAG('S', 'N', '4', '0') },
		{ CODEC_ID_MPEG4,        MKTAG('V', 'S', 'P', 'X') },
		{ CODEC_ID_MPEG4,        MKTAG('U', 'L', 'D', 'X') },
		{ CODEC_ID_MPEG4,        MKTAG('G', 'E', 'O', 'V') },
		{ CODEC_ID_MPEG4,        MKTAG('S', 'I', 'P', 'P') }, /* Samsung SHR-6040 */
		{ CODEC_ID_MSMPEG4V3,    MKTAG('M', 'P', '4', '3') },
		{ CODEC_ID_MSMPEG4V3,    MKTAG('D', 'I', 'V', '3') },
		{ CODEC_ID_MSMPEG4V3,    MKTAG('M', 'P', 'G', '3') },
		{ CODEC_ID_MSMPEG4V3,    MKTAG('D', 'I', 'V', '5') },
		{ CODEC_ID_MSMPEG4V3,    MKTAG('D', 'I', 'V', '6') },
		{ CODEC_ID_MSMPEG4V3,    MKTAG('D', 'I', 'V', '4') },
		{ CODEC_ID_MSMPEG4V3,    MKTAG('D', 'V', 'X', '3') },
		{ CODEC_ID_MSMPEG4V3,    MKTAG('A', 'P', '4', '1') },
		{ CODEC_ID_MSMPEG4V3,    MKTAG('C', 'O', 'L', '1') },
		{ CODEC_ID_MSMPEG4V3,    MKTAG('C', 'O', 'L', '0') },
		{ CODEC_ID_MSMPEG4V2,    MKTAG('M', 'P', '4', '2') },
		{ CODEC_ID_MSMPEG4V2,    MKTAG('D', 'I', 'V', '2') },
		{ CODEC_ID_MSMPEG4V1,    MKTAG('M', 'P', 'G', '4') },
		{ CODEC_ID_MSMPEG4V1,    MKTAG('M', 'P', '4', '1') },
		{ CODEC_ID_WMV1,         MKTAG('W', 'M', 'V', '1') },
		{ CODEC_ID_WMV2,         MKTAG('W', 'M', 'V', '2') },
		{ CODEC_ID_DVVIDEO,      MKTAG('d', 'v', 's', 'd') },
		{ CODEC_ID_DVVIDEO,      MKTAG('d', 'v', 'h', 'd') },
		{ CODEC_ID_DVVIDEO,      MKTAG('d', 'v', 'h', '1') },
		{ CODEC_ID_DVVIDEO,      MKTAG('d', 'v', 's', 'l') },
		{ CODEC_ID_DVVIDEO,      MKTAG('d', 'v', '2', '5') },
		{ CODEC_ID_DVVIDEO,      MKTAG('d', 'v', '5', '0') },
		{ CODEC_ID_DVVIDEO,      MKTAG('c', 'd', 'v', 'c') }, /* Canopus DV */
		{ CODEC_ID_DVVIDEO,      MKTAG('C', 'D', 'V', 'H') }, /* Canopus DV */
		{ CODEC_ID_DVVIDEO,      MKTAG('d', 'v', 'c', ' ') },
		{ CODEC_ID_DVVIDEO,      MKTAG('d', 'v', 'c', 's') },
		{ CODEC_ID_DVVIDEO,      MKTAG('d', 'v', 'h', '1') },
		{ CODEC_ID_MPEG1VIDEO,   MKTAG('m', 'p', 'g', '1') },
		{ CODEC_ID_MPEG1VIDEO,   MKTAG('m', 'p', 'g', '2') },
		{ CODEC_ID_MPEG2VIDEO,   MKTAG('m', 'p', 'g', '2') },
		{ CODEC_ID_MPEG2VIDEO,   MKTAG('M', 'P', 'E', 'G') },
		{ CODEC_ID_MPEG1VIDEO,   MKTAG('P', 'I', 'M', '1') },
		{ CODEC_ID_MPEG2VIDEO,   MKTAG('P', 'I', 'M', '2') },
		{ CODEC_ID_MPEG1VIDEO,   MKTAG('V', 'C', 'R', '2') },
		{ CODEC_ID_MPEG1VIDEO,   MKTAG( 1 ,  0 ,  0 ,  16) },
		{ CODEC_ID_MPEG2VIDEO,   MKTAG( 2 ,  0 ,  0 ,  16) },
		{ CODEC_ID_MPEG4,        MKTAG( 4 ,  0 ,  0 ,  16) },
		{ CODEC_ID_MPEG2VIDEO,   MKTAG('D', 'V', 'R', ' ') },
		{ CODEC_ID_MPEG2VIDEO,   MKTAG('M', 'M', 'E', 'S') },
		{ CODEC_ID_MPEG2VIDEO,   MKTAG('L', 'M', 'P', '2') }, /* Lead MPEG2 in avi */
		{ CODEC_ID_MPEG2VIDEO,   MKTAG('s', 'l', 'i', 'f') },
		{ CODEC_ID_MPEG2VIDEO,   MKTAG('E', 'M', '2', 'V') },
		{ CODEC_ID_MPEG2VIDEO,   MKTAG('M', '7', '0', '1') }, /* Matrox MPEG2 intra-only */
		{ CODEC_ID_MJPEG,        MKTAG('M', 'J', 'P', 'G') },
		{ CODEC_ID_MJPEG,        MKTAG('L', 'J', 'P', 'G') },
		{ CODEC_ID_MJPEG,        MKTAG('d', 'm', 'b', '1') },
		{ CODEC_ID_MJPEG,        MKTAG('m', 'j', 'p', 'a') },
		{ CODEC_ID_LJPEG,        MKTAG('L', 'J', 'P', 'G') },
		{ CODEC_ID_MJPEG,        MKTAG('J', 'P', 'G', 'L') }, /* Pegasus lossless JPEG */
		{ CODEC_ID_JPEGLS,       MKTAG('M', 'J', 'L', 'S') }, /* JPEG-LS custom FOURCC for avi - encoder */
		{ CODEC_ID_JPEGLS,       MKTAG('M', 'J', 'P', 'G') },
		{ CODEC_ID_MJPEG,        MKTAG('M', 'J', 'L', 'S') }, /* JPEG-LS custom FOURCC for avi - decoder */
		{ CODEC_ID_MJPEG,        MKTAG('j', 'p', 'e', 'g') },
		{ CODEC_ID_MJPEG,        MKTAG('I', 'J', 'P', 'G') },
		{ CODEC_ID_MJPEG,        MKTAG('A', 'V', 'R', 'n') },
		{ CODEC_ID_MJPEG,        MKTAG('A', 'C', 'D', 'V') },
		{ CODEC_ID_MJPEG,        MKTAG('Q', 'I', 'V', 'G') },
		{ CODEC_ID_MJPEG,        MKTAG('S', 'L', 'M', 'J') }, /* SL M-JPEG */
		{ CODEC_ID_MJPEG,        MKTAG('C', 'J', 'P', 'G') }, /* Creative Webcam JPEG */
		{ CODEC_ID_MJPEG,        MKTAG('I', 'J', 'L', 'V') }, /* Intel JPEG Library Video Codec */
		{ CODEC_ID_MJPEG,        MKTAG('M', 'V', 'J', 'P') }, /* Midvid JPEG Video Codec */
		{ CODEC_ID_MJPEG,        MKTAG('A', 'V', 'I', '1') },
		{ CODEC_ID_MJPEG,        MKTAG('A', 'V', 'I', '2') },
		{ CODEC_ID_MJPEG,        MKTAG('M', 'T', 'S', 'J') },
		{ CODEC_ID_MJPEG,        MKTAG('Z', 'J', 'P', 'G') }, /* Paradigm Matrix M-JPEG Codec */
		{ CODEC_ID_HUFFYUV,      MKTAG('H', 'F', 'Y', 'U') },
		{ CODEC_ID_FFVHUFF,      MKTAG('F', 'F', 'V', 'H') },
		{ CODEC_ID_CYUV,         MKTAG('C', 'Y', 'U', 'V') },
		{ CODEC_ID_RAWVIDEO,     MKTAG( 0 ,  0 ,  0 ,  0 ) },
		{ CODEC_ID_RAWVIDEO,     MKTAG( 3 ,  0 ,  0 ,  0 ) },
		{ CODEC_ID_RAWVIDEO,     MKTAG('I', '4', '2', '0') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('Y', 'U', 'Y', '2') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('Y', '4', '2', '2') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('V', '4', '2', '2') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('Y', 'U', 'N', 'V') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('U', 'Y', 'N', 'V') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('U', 'Y', 'N', 'Y') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('u', 'y', 'v', '1') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('2', 'V', 'u', '1') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('2', 'v', 'u', 'y') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('y', 'u', 'v', 's') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('P', '4', '2', '2') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('Y', 'V', '1', '2') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('U', 'Y', 'V', 'Y') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('V', 'Y', 'U', 'Y') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('I', 'Y', 'U', 'V') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('Y', '8', '0', '0') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('H', 'D', 'Y', 'C') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('Y', 'V', 'U', '9') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('V', 'D', 'T', 'Z') }, /* SoftLab-NSK VideoTizer */
		{ CODEC_ID_RAWVIDEO,     MKTAG('Y', '4', '1', '1') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('N', 'V', '1', '2') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('N', 'V', '2', '1') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('Y', '4', '1', 'B') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('Y', '4', '2', 'B') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('Y', 'U', 'V', '9') },
		{ CODEC_ID_RAWVIDEO,     MKTAG('Y', 'V', 'U', '9') },
		{ CODEC_ID_FRWU,         MKTAG('F', 'R', 'W', 'U') },
		{ CODEC_ID_R10K,         MKTAG('R', '1', '0', 'k') },
		{ CODEC_ID_R210,         MKTAG('r', '2', '1', '0') },
		{ CODEC_ID_V210,         MKTAG('v', '2', '1', '0') },
		{ CODEC_ID_INDEO3,       MKTAG('I', 'V', '3', '1') },
		{ CODEC_ID_INDEO3,       MKTAG('I', 'V', '3', '2') },
		{ CODEC_ID_INDEO4,       MKTAG('I', 'V', '4', '1') },
		{ CODEC_ID_INDEO5,       MKTAG('I', 'V', '5', '0') },
		{ CODEC_ID_VP3,          MKTAG('V', 'P', '3', '1') },
		{ CODEC_ID_VP3,          MKTAG('V', 'P', '3', '0') },
		{ CODEC_ID_VP5,          MKTAG('V', 'P', '5', '0') },
		{ CODEC_ID_VP6,          MKTAG('V', 'P', '6', '0') },
		{ CODEC_ID_VP6,          MKTAG('V', 'P', '6', '1') },
		{ CODEC_ID_VP6,          MKTAG('V', 'P', '6', '2') },
		{ CODEC_ID_VP6F,         MKTAG('V', 'P', '6', 'F') },
		{ CODEC_ID_VP6F,         MKTAG('F', 'L', 'V', '4') },
		{ CODEC_ID_VP8,          MKTAG('V', 'P', '8', '0') },
		{ CODEC_ID_ASV1,         MKTAG('A', 'S', 'V', '1') },
		{ CODEC_ID_ASV2,         MKTAG('A', 'S', 'V', '2') },
		{ CODEC_ID_VCR1,         MKTAG('V', 'C', 'R', '1') },
		{ CODEC_ID_FFV1,         MKTAG('F', 'F', 'V', '1') },
		{ CODEC_ID_XAN_WC4,      MKTAG('X', 'x', 'a', 'n') },
		{ CODEC_ID_MIMIC,        MKTAG('L', 'M', '2', '0') },
		{ CODEC_ID_MSRLE,        MKTAG('m', 'r', 'l', 'e') },
		{ CODEC_ID_MSRLE,        MKTAG( 1 ,  0 ,  0 ,  0 ) },
		{ CODEC_ID_MSRLE,        MKTAG( 2 ,  0 ,  0 ,  0 ) },
		{ CODEC_ID_MSVIDEO1,     MKTAG('M', 'S', 'V', 'C') },
		{ CODEC_ID_MSVIDEO1,     MKTAG('m', 's', 'v', 'c') },
		{ CODEC_ID_MSVIDEO1,     MKTAG('C', 'R', 'A', 'M') },
		{ CODEC_ID_MSVIDEO1,     MKTAG('c', 'r', 'a', 'm') },
		{ CODEC_ID_MSVIDEO1,     MKTAG('W', 'H', 'A', 'M') },
		{ CODEC_ID_MSVIDEO1,     MKTAG('w', 'h', 'a', 'm') },
		{ CODEC_ID_CINEPAK,      MKTAG('c', 'v', 'i', 'd') },
		{ CODEC_ID_TRUEMOTION1,  MKTAG('D', 'U', 'C', 'K') },
		{ CODEC_ID_TRUEMOTION1,  MKTAG('P', 'V', 'E', 'Z') },
		{ CODEC_ID_MSZH,         MKTAG('M', 'S', 'Z', 'H') },
		{ CODEC_ID_ZLIB,         MKTAG('Z', 'L', 'I', 'B') },
		{ CODEC_ID_SNOW,         MKTAG('S', 'N', 'O', 'W') },
		{ CODEC_ID_4XM,          MKTAG('4', 'X', 'M', 'V') },
		{ CODEC_ID_FLV1,         MKTAG('F', 'L', 'V', '1') },
		{ CODEC_ID_FLASHSV,      MKTAG('F', 'S', 'V', '1') },
		{ CODEC_ID_SVQ1,         MKTAG('s', 'v', 'q', '1') },
		{ CODEC_ID_TSCC,         MKTAG('t', 's', 'c', 'c') },
		{ CODEC_ID_ULTI,         MKTAG('U', 'L', 'T', 'I') },
		{ CODEC_ID_VIXL,         MKTAG('V', 'I', 'X', 'L') },
		{ CODEC_ID_QPEG,         MKTAG('Q', 'P', 'E', 'G') },
		{ CODEC_ID_QPEG,         MKTAG('Q', '1', '.', '0') },
		{ CODEC_ID_QPEG,         MKTAG('Q', '1', '.', '1') },
		{ CODEC_ID_WMV3,         MKTAG('W', 'M', 'V', '3') },
		{ CODEC_ID_WMV3,         MKTAG('W', 'M', 'V', 'P') },
		{ CODEC_ID_VC1,          MKTAG('W', 'V', 'C', '1') },
		{ CODEC_ID_VC1,          MKTAG('W', 'M', 'V', 'A') },
		{ CODEC_ID_LOCO,         MKTAG('L', 'O', 'C', 'O') },
		{ CODEC_ID_WNV1,         MKTAG('W', 'N', 'V', '1') },
		{ CODEC_ID_AASC,         MKTAG('A', 'A', 'S', 'C') },
		{ CODEC_ID_INDEO2,       MKTAG('R', 'T', '2', '1') },
		{ CODEC_ID_FRAPS,        MKTAG('F', 'P', 'S', '1') },
		{ CODEC_ID_THEORA,       MKTAG('t', 'h', 'e', 'o') },
		{ CODEC_ID_TRUEMOTION2,  MKTAG('T', 'M', '2', '0') },
		{ CODEC_ID_CSCD,         MKTAG('C', 'S', 'C', 'D') },
		{ CODEC_ID_ZMBV,         MKTAG('Z', 'M', 'B', 'V') },
		{ CODEC_ID_KMVC,         MKTAG('K', 'M', 'V', 'C') },
		{ CODEC_ID_CAVS,         MKTAG('C', 'A', 'V', 'S') },
		{ CODEC_ID_JPEG2000,     MKTAG('M', 'J', '2', 'C') },
		{ CODEC_ID_VMNC,         MKTAG('V', 'M', 'n', 'c') },
		{ CODEC_ID_TARGA,        MKTAG('t', 'g', 'a', ' ') },
		{ CODEC_ID_PNG,          MKTAG('M', 'P', 'N', 'G') },
		{ CODEC_ID_PNG,          MKTAG('P', 'N', 'G', '1') },
		{ CODEC_ID_CLJR,         MKTAG('c', 'l', 'j', 'r') },
		{ CODEC_ID_DIRAC,        MKTAG('d', 'r', 'a', 'c') },
		{ CODEC_ID_RPZA,         MKTAG('a', 'z', 'p', 'r') },
		{ CODEC_ID_RPZA,         MKTAG('R', 'P', 'Z', 'A') },
		{ CODEC_ID_RPZA,         MKTAG('r', 'p', 'z', 'a') },
		{ CODEC_ID_SP5X,         MKTAG('S', 'P', '5', '4') },
		{ CODEC_ID_AURA,         MKTAG('A', 'U', 'R', 'A') },
		{ CODEC_ID_AURA2,        MKTAG('A', 'U', 'R', '2') },
		{ CODEC_ID_DPX,          MKTAG('d', 'p', 'x', ' ') },
		{ CODEC_ID_KGV1,         MKTAG('K', 'G', 'V', '1') },
		{ CODEC_ID_LAGARITH,     MKTAG('L', 'A', 'G', 'S') },
		{ CODEC_ID_NONE,         0 }
};

const AVCodecTag ff_codec_wav_tags[] = {
		{ CODEC_ID_PCM_S16LE,       0x0001 },
		{ CODEC_ID_PCM_U8,          0x0001 }, /* must come after s16le in this list */
		{ CODEC_ID_PCM_S24LE,       0x0001 },
		{ CODEC_ID_PCM_S32LE,       0x0001 },
		{ CODEC_ID_ADPCM_MS,        0x0002 },
		{ CODEC_ID_PCM_F32LE,       0x0003 },
		{ CODEC_ID_PCM_F64LE,       0x0003 }, /* must come after f32le in this list */
		{ CODEC_ID_PCM_ALAW,        0x0006 },
		{ CODEC_ID_PCM_MULAW,       0x0007 },
		{ CODEC_ID_WMAVOICE,        0x000A },
		{ CODEC_ID_ADPCM_IMA_WAV,   0x0011 },
		{ CODEC_ID_PCM_ZORK,        0x0011 }, /* must come after adpcm_ima_wav in this list */
		{ CODEC_ID_ADPCM_YAMAHA,    0x0020 },
		{ CODEC_ID_TRUESPEECH,      0x0022 },
		{ CODEC_ID_GSM_MS,          0x0031 },
		{ CODEC_ID_ADPCM_G726,      0x0045 },
		{ CODEC_ID_MP2,             0x0050 },
		{ CODEC_ID_MP3,             0x0055 },
		{ CODEC_ID_AMR_NB,          0x0057 },
		{ CODEC_ID_AMR_WB,          0x0058 },
		{ CODEC_ID_ADPCM_IMA_DK4,   0x0061 },  /* rogue format number */
		{ CODEC_ID_ADPCM_IMA_DK3,   0x0062 },  /* rogue format number */
		{ CODEC_ID_ADPCM_IMA_WAV,   0x0069 },
		{ CODEC_ID_VOXWARE,         0x0075 },
		{ CODEC_ID_AAC,             0x00ff },
		{ CODEC_ID_SIPR,            0x0130 },
		{ CODEC_ID_WMAV1,           0x0160 },
		{ CODEC_ID_WMAV2,           0x0161 },
		{ CODEC_ID_WMAPRO,          0x0162 },
		{ CODEC_ID_WMALOSSLESS,     0x0163 },
		{ CODEC_ID_ADPCM_CT,        0x0200 },
		{ CODEC_ID_ATRAC3,          0x0270 },
		{ CODEC_ID_ADPCM_G722,      0x028F },
		{ CODEC_ID_IMC,             0x0401 },
		{ CODEC_ID_GSM_MS,          0x1500 },
		{ CODEC_ID_TRUESPEECH,      0x1501 },
		{ CODEC_ID_AAC_LATM,        0x1602 },
		{ CODEC_ID_AC3,             0x2000 },
		{ CODEC_ID_DTS,             0x2001 },
		{ CODEC_ID_SONIC,           0x2048 },
		{ CODEC_ID_SONIC_LS,        0x2048 },
		{ CODEC_ID_PCM_MULAW,       0x6c75 },
		{ CODEC_ID_AAC,             0x706d },
		{ CODEC_ID_AAC,             0x4143 },
		{ CODEC_ID_FLAC,            0xF1AC },
		{ CODEC_ID_ADPCM_SWF,       ('S'<<8)+'F' },
		{ CODEC_ID_VORBIS,          ('V'<<8)+'o' }, //HACK/FIXME, does vorbis in WAV/AVI have an (in)official id?

		/* FIXME: All of the IDs below are not 16 bit and thus illegal. */
		// for NuppelVideo (nuv.c)
		{ CODEC_ID_PCM_S16LE, MKTAG('R', 'A', 'W', 'A') },
		{ CODEC_ID_MP3,       MKTAG('L', 'A', 'M', 'E') },
		{ CODEC_ID_MP3,       MKTAG('M', 'P', '3', ' ') },
		{ CODEC_ID_NONE,      0 },
};

const int ff_mpeg4audio_sample_rates[16] = {
		96000, 88200, 64000, 48000, 44100, 32000,
		24000, 22050, 16000, 12000, 11025, 8000, 7350
};

const uint8_t ff_log2_tab[256]={
		0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};

const CodecMime ff_mkv_mime_tags[] = {
		{"text/plain"                 , CODEC_ID_TEXT},
		{"image/gif"                  , CODEC_ID_GIF},
		{"image/jpeg"                 , CODEC_ID_MJPEG},
		{"image/png"                  , CODEC_ID_PNG},
		{"image/tiff"                 , CODEC_ID_TIFF},
		{"application/x-truetype-font", CODEC_ID_TTF},
		{"application/x-font"         , CODEC_ID_TTF},

		{""                           , CODEC_ID_NONE}
};

///////////////////////////////////////////////////
//define the format

static EbmlSyntax ebml_header[] = {
		{ EBML_ID_EBMLREADVERSION,        EBML_UINT, 0, offsetof(Ebml,version), EBML_VERSION},
		{ EBML_ID_EBMLMAXSIZELENGTH,      EBML_UINT, 0, offsetof(Ebml,max_size), {8} },
		{ EBML_ID_EBMLMAXIDLENGTH,        EBML_UINT, 0, offsetof(Ebml,id_length), {4} },
		{ EBML_ID_DOCTYPE,                EBML_STR,  0, offsetof(Ebml,doctype), NULL },
		{ EBML_ID_DOCTYPEREADVERSION,     EBML_UINT, 0, offsetof(Ebml,doctype_version), 1 },
		{ EBML_ID_EBMLVERSION,            EBML_NONE },
		{ EBML_ID_DOCTYPEVERSION,         EBML_NONE },
		{ 0 }
};

static EbmlSyntax ebml_syntax[] = {
		{ EBML_ID_HEADER,                 EBML_NEST, 0, 0, {(uint64_t)ebml_header} },
		{ 0 }
};

static EbmlSyntax matroska_info[] = {
		{ MATROSKA_ID_TIMECODESCALE,      EBML_UINT,  0, offsetof(MatroskaDemuxContext,time_scale), { 1000000 } },
		{ MATROSKA_ID_DURATION,           EBML_FLOAT, 0, offsetof(MatroskaDemuxContext,duration) },
		{ MATROSKA_ID_TITLE,              EBML_UTF8,  0, offsetof(MatroskaDemuxContext,title) },
		{ MATROSKA_ID_WRITINGAPP,         EBML_NONE },
		{ MATROSKA_ID_MUXINGAPP,          EBML_NONE },
		{ MATROSKA_ID_DATEUTC,            EBML_NONE },
		{ MATROSKA_ID_SEGMENTUID,         EBML_NONE },
		{ 0 }
};

static EbmlSyntax matroska_track_video[] = {
		{ MATROSKA_ID_VIDEOFRAMERATE,     EBML_FLOAT,0, offsetof(MatroskaTrackVideo,frame_rate) },
		{ MATROSKA_ID_VIDEODISPLAYWIDTH,  EBML_UINT, 0, offsetof(MatroskaTrackVideo,display_width) },
		{ MATROSKA_ID_VIDEODISPLAYHEIGHT, EBML_UINT, 0, offsetof(MatroskaTrackVideo,display_height) },
		{ MATROSKA_ID_VIDEOPIXELWIDTH,    EBML_UINT, 0, offsetof(MatroskaTrackVideo,pixel_width) },
		{ MATROSKA_ID_VIDEOPIXELHEIGHT,   EBML_UINT, 0, offsetof(MatroskaTrackVideo,pixel_height) },
		{ MATROSKA_ID_VIDEOCOLORSPACE,    EBML_UINT, 0, offsetof(MatroskaTrackVideo,fourcc) },
		{ MATROSKA_ID_VIDEOPIXELCROPB,    EBML_NONE },
		{ MATROSKA_ID_VIDEOPIXELCROPT,    EBML_NONE },
		{ MATROSKA_ID_VIDEOPIXELCROPL,    EBML_NONE },
		{ MATROSKA_ID_VIDEOPIXELCROPR,    EBML_NONE },
		{ MATROSKA_ID_VIDEODISPLAYUNIT,   EBML_NONE },
		{ MATROSKA_ID_VIDEOFLAGINTERLACED,EBML_NONE },
		{ MATROSKA_ID_VIDEOSTEREOMODE,    EBML_NONE },
		{ MATROSKA_ID_VIDEOASPECTRATIO,   EBML_NONE },
		{ 0 }
};

static EbmlSyntax matroska_track_audio[] = {
		{ MATROSKA_ID_AUDIOSAMPLINGFREQ,  EBML_FLOAT,0, offsetof(MatroskaTrackAudio,samplerate), { 8000.0} },
		{ MATROSKA_ID_AUDIOOUTSAMPLINGFREQ,EBML_FLOAT,0,offsetof(MatroskaTrackAudio,out_samplerate) },
		{ MATROSKA_ID_AUDIOBITDEPTH,      EBML_UINT, 0, offsetof(MatroskaTrackAudio,bitdepth) },
		{ MATROSKA_ID_AUDIOCHANNELS,      EBML_UINT, 0, offsetof(MatroskaTrackAudio,channels), { 1} },
		{ 0 }
};

static EbmlSyntax matroska_track_encoding_compression[] = {
		{ MATROSKA_ID_ENCODINGCOMPALGO,   EBML_UINT, 0, offsetof(MatroskaTrackCompression,algo), { 0} },
		{ MATROSKA_ID_ENCODINGCOMPSETTINGS,EBML_BIN, 0, offsetof(MatroskaTrackCompression,settings) },
		{ 0 }
};

static EbmlSyntax matroska_track_encoding[] = {
		{ MATROSKA_ID_ENCODINGSCOPE,      EBML_UINT, 0, offsetof(MatroskaTrackEncoding,scope), { 1} },
		{ MATROSKA_ID_ENCODINGTYPE,       EBML_UINT, 0, offsetof(MatroskaTrackEncoding,type), { 0} },
		{ MATROSKA_ID_ENCODINGCOMPRESSION,EBML_NEST, 0, offsetof(MatroskaTrackEncoding,compression), {(uint64_t)matroska_track_encoding_compression} },
		{ MATROSKA_ID_ENCODINGORDER,      EBML_NONE },
		{ 0 }
};

static EbmlSyntax matroska_track_encodings[] = {
		{ MATROSKA_ID_TRACKCONTENTENCODING, EBML_NEST, sizeof(MatroskaTrackEncoding), offsetof(MatroskaTrack,encodings), {(uint64_t)matroska_track_encoding} },
		{ 0 }
};

static EbmlSyntax matroska_track[] = {
		{ MATROSKA_ID_TRACKNUMBER,          EBML_UINT, 0, offsetof(MatroskaTrack,num) },
		{ MATROSKA_ID_TRACKNAME,            EBML_UTF8, 0, offsetof(MatroskaTrack,name) },
		{ MATROSKA_ID_TRACKUID,             EBML_UINT, 0, offsetof(MatroskaTrack,uid) },
		{ MATROSKA_ID_TRACKTYPE,            EBML_UINT, 0, offsetof(MatroskaTrack,type) },
		{ MATROSKA_ID_CODECID,              EBML_STR,  0, offsetof(MatroskaTrack,codec_id) },
		{ MATROSKA_ID_CODECPRIVATE,         EBML_BIN,  0, offsetof(MatroskaTrack,codec_priv) },
		{ MATROSKA_ID_TRACKLANGUAGE,        EBML_UTF8, 0, offsetof(MatroskaTrack,language), { NULL } },
		{ MATROSKA_ID_TRACKDEFAULTDURATION, EBML_UINT, 0, offsetof(MatroskaTrack,default_duration) },
		{ MATROSKA_ID_TRACKTIMECODESCALE,   EBML_FLOAT,0, offsetof(MatroskaTrack,time_scale), { 1.0} },
		{ MATROSKA_ID_TRACKFLAGDEFAULT,     EBML_UINT, 0, offsetof(MatroskaTrack,flag_default), { 1} },
		{ MATROSKA_ID_TRACKFLAGFORCED,      EBML_UINT, 0, offsetof(MatroskaTrack,flag_forced), { 0} },
		{ MATROSKA_ID_TRACKVIDEO,           EBML_NEST, 0, offsetof(MatroskaTrack,video), { (uint64_t)matroska_track_video} },
		{ MATROSKA_ID_TRACKAUDIO,           EBML_NEST, 0, offsetof(MatroskaTrack,audio), { (uint64_t)matroska_track_audio} },
		{ MATROSKA_ID_TRACKCONTENTENCODINGS,EBML_NEST, 0, 0, { (uint64_t)matroska_track_encodings} },
		{ MATROSKA_ID_TRACKFLAGENABLED,     EBML_NONE },
		{ MATROSKA_ID_TRACKFLAGLACING,      EBML_NONE },
		{ MATROSKA_ID_CODECNAME,            EBML_NONE },
		{ MATROSKA_ID_CODECDECODEALL,       EBML_NONE },
		{ MATROSKA_ID_CODECINFOURL,         EBML_NONE },
		{ MATROSKA_ID_CODECDOWNLOADURL,     EBML_NONE },
		{ MATROSKA_ID_TRACKMINCACHE,        EBML_NONE },
		{ MATROSKA_ID_TRACKMAXCACHE,        EBML_NONE },
		{ MATROSKA_ID_TRACKMAXBLKADDID,     EBML_NONE },
		{ 0 }
};

static EbmlSyntax matroska_tracks[] = {
		{ MATROSKA_ID_TRACKENTRY,         EBML_NEST, sizeof(MatroskaTrack), offsetof(MatroskaDemuxContext,tracks), { (uint64_t)matroska_track} },
		{ 0 }
};

static EbmlSyntax matroska_attachment[] = {
		{ MATROSKA_ID_FILEUID,            EBML_UINT, 0, offsetof(MatroskaAttachement,uid) },
		{ MATROSKA_ID_FILENAME,           EBML_UTF8, 0, offsetof(MatroskaAttachement,filename) },
		{ MATROSKA_ID_FILEMIMETYPE,       EBML_STR,  0, offsetof(MatroskaAttachement,mime) },
		{ MATROSKA_ID_FILEDATA,           EBML_BIN,  0, offsetof(MatroskaAttachement,bin) },
		{ MATROSKA_ID_FILEDESC,           EBML_NONE },
		{ 0 }
};

static EbmlSyntax matroska_attachments[] = {
		{ MATROSKA_ID_ATTACHEDFILE,       EBML_NEST, sizeof(MatroskaAttachement), offsetof(MatroskaDemuxContext,attachments), {(uint64_t)matroska_attachment} },
		{ 0 }
};

static EbmlSyntax matroska_chapter_display[] = {
		{ MATROSKA_ID_CHAPSTRING,         EBML_UTF8, 0, offsetof(MatroskaChapter,title) },
		{ MATROSKA_ID_CHAPLANG,           EBML_NONE },
		{ 0 }
};

static EbmlSyntax matroska_chapter_entry[] = {
		{ MATROSKA_ID_CHAPTERTIMESTART,   EBML_UINT, 0, offsetof(MatroskaChapter,start), { AV_NOPTS_VALUE} },
		{ MATROSKA_ID_CHAPTERTIMEEND,     EBML_UINT, 0, offsetof(MatroskaChapter,end), { AV_NOPTS_VALUE} },
		{ MATROSKA_ID_CHAPTERUID,         EBML_UINT, 0, offsetof(MatroskaChapter,uid) },
		{ MATROSKA_ID_CHAPTERDISPLAY,     EBML_NEST, 0, 0, {(uint64_t)matroska_chapter_display} },
		{ MATROSKA_ID_CHAPTERFLAGHIDDEN,  EBML_NONE },
		{ MATROSKA_ID_CHAPTERFLAGENABLED, EBML_NONE },
		{ MATROSKA_ID_CHAPTERPHYSEQUIV,   EBML_NONE },
		{ MATROSKA_ID_CHAPTERATOM,        EBML_NONE },
		{ 0 }
};

static EbmlSyntax matroska_chapter[] = {
		{ MATROSKA_ID_CHAPTERATOM,        EBML_NEST, sizeof(MatroskaChapter), offsetof(MatroskaDemuxContext,chapters), {(uint64_t)matroska_chapter_entry} },
		{ MATROSKA_ID_EDITIONUID,         EBML_NONE },
		{ MATROSKA_ID_EDITIONFLAGHIDDEN,  EBML_NONE },
		{ MATROSKA_ID_EDITIONFLAGDEFAULT, EBML_NONE },
		{ MATROSKA_ID_EDITIONFLAGORDERED, EBML_NONE },
		{ 0 }
};

static EbmlSyntax matroska_chapters[] = {
		{ MATROSKA_ID_EDITIONENTRY,       EBML_NEST, 0, 0, {(uint64_t)matroska_chapter} },
		{ 0 }
};

static EbmlSyntax matroska_index_pos[] = {
		{ MATROSKA_ID_CUETRACK,           EBML_UINT, 0, offsetof(MatroskaIndexPos,track) },
		{ MATROSKA_ID_CUECLUSTERPOSITION, EBML_UINT, 0, offsetof(MatroskaIndexPos,pos)   },
		{ MATROSKA_ID_CUEBLOCKNUMBER,     EBML_NONE },
		{ 0 }
};

static EbmlSyntax matroska_index_entry[] = {
		{ MATROSKA_ID_CUETIME,            EBML_UINT, 0, offsetof(MatroskaIndex,time) },
		{ MATROSKA_ID_CUETRACKPOSITION,   EBML_NEST, sizeof(MatroskaIndexPos), offsetof(MatroskaIndex,pos), {(uint64_t)matroska_index_pos} },
		{ 0 }
};

static EbmlSyntax matroska_index[] = {
		{ MATROSKA_ID_POINTENTRY,         EBML_NEST, sizeof(MatroskaIndex), offsetof(MatroskaDemuxContext,index), {(uint64_t)matroska_index_entry} },
		{ 0 }
};

static EbmlSyntax matroska_simpletag[] = {
		{ MATROSKA_ID_TAGNAME,            EBML_UTF8, 0, offsetof(MatroskaTag,name) },
		{ MATROSKA_ID_TAGSTRING,          EBML_UTF8, 0, offsetof(MatroskaTag,string) },
		{ MATROSKA_ID_TAGLANG,            EBML_STR,  0, offsetof(MatroskaTag,lang), {NULL} },
		{ MATROSKA_ID_TAGDEFAULT,         EBML_UINT, 0, offsetof(MatroskaTag,def) },
		{ MATROSKA_ID_TAGDEFAULT_BUG,     EBML_UINT, 0, offsetof(MatroskaTag,def) },
		{ MATROSKA_ID_SIMPLETAG,          EBML_NEST, sizeof(MatroskaTag), offsetof(MatroskaTag,sub), {(uint64_t)matroska_simpletag} },
		{ 0 }
};

static EbmlSyntax matroska_tagtargets[] = {
		{ MATROSKA_ID_TAGTARGETS_TYPE,      EBML_STR,  0, offsetof(MatroskaTagTarget,type) },
		{ MATROSKA_ID_TAGTARGETS_TYPEVALUE, EBML_UINT, 0, offsetof(MatroskaTagTarget,typevalue), {50} },
		{ MATROSKA_ID_TAGTARGETS_TRACKUID,  EBML_UINT, 0, offsetof(MatroskaTagTarget,trackuid) },
		{ MATROSKA_ID_TAGTARGETS_CHAPTERUID,EBML_UINT, 0, offsetof(MatroskaTagTarget,chapteruid) },
		{ MATROSKA_ID_TAGTARGETS_ATTACHUID, EBML_UINT, 0, offsetof(MatroskaTagTarget,attachuid) },
		{ 0 }
};

static EbmlSyntax matroska_tag[] = {
		{ MATROSKA_ID_SIMPLETAG,          EBML_NEST, sizeof(MatroskaTag), offsetof(MatroskaTags,tag), {(uint64_t)matroska_simpletag} },
		{ MATROSKA_ID_TAGTARGETS,         EBML_NEST, 0, offsetof(MatroskaTags,target), {(uint64_t)matroska_tagtargets} },
		{ 0 }
};

static EbmlSyntax matroska_tags[] = {
		{ MATROSKA_ID_TAG,                EBML_NEST, sizeof(MatroskaTags), offsetof(MatroskaDemuxContext,tags), {(uint64_t)matroska_tag} },
		{ 0 }
};

static EbmlSyntax matroska_seekhead_entry[] = {
		{ MATROSKA_ID_SEEKID,             EBML_UINT, 0, offsetof(MatroskaSeekhead,id) },
		{ MATROSKA_ID_SEEKPOSITION,       EBML_UINT, 0, offsetof(MatroskaSeekhead,pos), {-1} },
		{ 0 }
};

static EbmlSyntax matroska_seekhead[] = {
		{ MATROSKA_ID_SEEKENTRY,          EBML_NEST, sizeof(MatroskaSeekhead), offsetof(MatroskaDemuxContext,seekhead), {(uint64_t)matroska_seekhead_entry} },
		{ 0 }
};


static EbmlSyntax matroska_segment[] = {
		{ MATROSKA_ID_INFO,           EBML_NEST, 0, 0, { (uint64_t)matroska_info       } },
		{ MATROSKA_ID_TRACKS,         EBML_NEST, 0, 0, { (uint64_t)matroska_tracks     } },
		{ MATROSKA_ID_ATTACHMENTS,    EBML_NEST, 0, 0, { (uint64_t)matroska_attachments} },
		{ MATROSKA_ID_CHAPTERS,       EBML_NEST, 0, 0, { (uint64_t)matroska_chapters   } },
		{ MATROSKA_ID_CUES,           EBML_NEST, 0, 0, { (uint64_t)matroska_index      } },
		{ MATROSKA_ID_TAGS,           EBML_NEST, 0, 0, { (uint64_t)matroska_tags       } },
		{ MATROSKA_ID_SEEKHEAD,       EBML_NEST, 0, 0, { (uint64_t)matroska_seekhead   } },
		{ MATROSKA_ID_CLUSTER,        EBML_STOP },
		{ 0 }
};

static EbmlSyntax matroska_segments[] = {
		{ MATROSKA_ID_SEGMENT,        EBML_NEST, 0, 0, { (uint64_t)matroska_segment    } },
		{ 0 }
};

static EbmlSyntax matroska_blockgroup[] = {
		{ MATROSKA_ID_BLOCK,          EBML_BIN,  0, offsetof(MatroskaBlock,bin) },
		{ MATROSKA_ID_SIMPLEBLOCK,    EBML_BIN,  0, offsetof(MatroskaBlock,bin) },
		{ MATROSKA_ID_BLOCKDURATION,  EBML_UINT, 0, offsetof(MatroskaBlock,duration), {AV_NOPTS_VALUE} },
		{ MATROSKA_ID_BLOCKREFERENCE, EBML_UINT, 0, offsetof(MatroskaBlock,reference) },
		{ 1,                          EBML_UINT, 0, offsetof(MatroskaBlock,non_simple), {1} },
		{ 0 }
};

static EbmlSyntax matroska_cluster[] = {
		{ MATROSKA_ID_CLUSTERTIMECODE,EBML_UINT,0, offsetof(MatroskaCluster,timecode) },
		{ MATROSKA_ID_BLOCKGROUP,     EBML_NEST, sizeof(MatroskaBlock), offsetof(MatroskaCluster,blocks), {(uint64_t)matroska_blockgroup} },
		{ MATROSKA_ID_SIMPLEBLOCK,    EBML_PASS, sizeof(MatroskaBlock), offsetof(MatroskaCluster,blocks), {(uint64_t)matroska_blockgroup} },
		{ MATROSKA_ID_CLUSTERPOSITION,EBML_NONE },
		{ MATROSKA_ID_CLUSTERPREVSIZE,EBML_NONE },
		{ 0 }
};

static EbmlSyntax matroska_clusters[] = {
		{ MATROSKA_ID_CLUSTER,        EBML_NEST, 0, 0, {(uint64_t)matroska_cluster} },
		{ MATROSKA_ID_INFO,           EBML_NONE },
		{ MATROSKA_ID_CUES,           EBML_NONE },
		{ MATROSKA_ID_TAGS,           EBML_NONE },
		{ MATROSKA_ID_SEEKHEAD,       EBML_NONE },
		{ 0 }
};




///////////////////////////////////////////////////////////
//some functions
double av_int2dbl(int64_t v){
	if(v+v > 0xFFEULL<<52)
		return 0.0/0.0;
	return ldexp(((v&((1LL<<52)-1)) + (1LL<<52)) * (v>>63|1), (v>>52&0x7FF)-1075);
}

float av_int2flt(int32_t v){
	if(v+v > 0xFF000000U)
		return 0.0/0.0;
	return ldexp(((v&0x7FFFFF) + (1<<23)) * (v>>31|1), (v>>23&0xFF)-150);
}







static const char *matroska_doctypes[] = { "matroska", "webm" };

int detect(	source_access_callbacks_t *source)			//return 0:detect failed;other value is seccessful
{
	uint64_t total = 0;
	int len_mask = 0x80, size = 1, n = 1, i;
	unsigned char buffer[4]={0};
	source->m_Read(source->m_pHandle,buffer,array_size(buffer),1);
	source->m_Seek(source->m_pHandle,0,SEEK_SET);

	/* EBML header? */
	if (AV_RB32(buffer) != EBML_ID_HEADER)
		return 0;

	return 1;
}


matroska_reader_t::matroska_reader_t()
{
	printf(">entry matroska_reader_t::matroska_reader_t\n");
	memset(&m_matroska,0,sizeof(MatroskaDemuxContext));
	printf("1\n");
	m_matroska.ctx = &m_fcontext;
	m_matroska.ctx->pb = &m_b;
	m_buffer = NULL;
	printf("2\n");
	m_buffer=av_malloc(ME_BUFFER_SIZE);
	if(NULL == m_buffer)
	{
		printf("m_buffer malloc failed\n");
		return ;
	}
	m_cluster_blocks = -1;
	this->videoNum = -1;
	this->audioNum = -1;
	this->lenOfNalPaketSize = 0;
	printf("<leave matroska_reader_t::matroska_reader_t\n");
}

matroska_reader_t::~matroska_reader_t()
{
	if(m_buffer)
	{
		av_free(m_buffer);
		m_buffer = NULL;
	}
}
///temp

int matroska_reader_t::init(source_access_callbacks_t *source)
{
	m_source = source;
	printf("entry matroska_reader_t::init\n");
	if(source && m_buffer)
	{
		if(0 != init_put_byte(&m_b,(uint8_t*)m_buffer,
				ME_BUFFER_SIZE,
				URL_RDONLY,
				source->m_pHandle,
				source->m_Read,
				NULL,
				source->m_Seek))
		{
			printf("matroska_reader_t::matroska_reader_t failed\n");
			return -1;
		}
		printf("init::file position is %d\n",url_ftell(&m_b));
		m_b.tell = source->m_Tell;
		//****source->m_Read(source->m_pHandle,m_buffer,ME_BUFFER_SIZE,0);

		//m_b.seek(source->m_pHandle,0,SEEK_SET);
		if(this->matroska_read_header()<0)
		{
			printf("Read the matroska head failed\n");
			return -1;
		}
		printf("=============init::file position is %d\n",url_ftell(&m_b));
	}
	return 0;
}

void matroska_reader_t::print_video_parameters(video_parameter_t *vpara)
{
	printf("strm_id=%d\n",vpara->strm_id);
	printf("strm_type=%d\n",vpara->strm_type);
	printf("h_size=%d\n",vpara->h_size);
	printf("v_size=%d\n",vpara->v_size);
	printf("bit_rate=%d\n",vpara->bit_rate);
	printf("interlaced=%d\n",vpara->interlaced);
	printf("aspect_ratio.n=%d\n",vpara->aspect_ratio.n);
	printf("aspect_ratio.d=%d\n",vpara->aspect_ratio.d);
	printf("frame_rate.n=%d\n",vpara->frame_rate.n);
	printf("frame_rate.d=%d\n",vpara->frame_rate.d);
	printf("pulldown_32=%d\n",vpara->pulldown_32);
	printf("profile=%d\n",vpara->profile);
	printf("digital_cc=%d\n",vpara->digital_cc);
	printf("line21=%d\n",vpara->line21);
	printf("still_pic=%d\n",vpara->still_pic);
}

int matroska_reader_t::get_video_parameter(video_parameter_t *vpara, int vid )
{
	if(vpara)
	{
		memset(vpara,0,sizeof(video_parameter_t));
		MatroskaTrack *tracks = (MatroskaTrack *)m_matroska.tracks.elem;
		MatroskaTrack *track = NULL;
		for (int i=0; i < m_matroska.tracks.nb_elem; i++)
		{
			track = &tracks[i];
			if(track->type == MATROSKA_TRACK_TYPE_VIDEO)
			{
				MatroskaTrackVideo* video = &track->video;

				es_stream_type_t es_type = ES_STREAM_TYPE_UNKNOWN;
				enum CodecID  codec_id = CODEC_ID_NONE;
				printf("vidoe codec_id=%s\n",track->codec_id);

				for(int j=0; ff_mkv_codec_tags[j].id != CODEC_ID_NONE; j++)
				{
					if(!strncmp(ff_mkv_codec_tags[j].str, track->codec_id,
							strlen(ff_mkv_codec_tags[j].str)))
					{
						codec_id= ff_mkv_codec_tags[j].id;
						es_type = ff_mkv_codec_tags[j].es_type;
						break;
					}
				}
				this->videoTrack = track;
				this->videoCodecID = codec_id;
				this->videoNum = (int)track->num;

				//vpara->frame_rate = convert_frame_rate(30);
				vpara->frame_rate.n = 30000;
				vpara->frame_rate.d = 1001;
				vpara->strm_id = 2;
				vpara->strm_type = es_type;
				vpara->bit_rate = 2742000;
				vpara->bit_rate = 0;
				vpara->h_size = track->video.pixel_width;
				vpara->v_size = track->video.pixel_height;

				//this->print_video_parameters(vpara);

				uint8_t* bufferTemp = NULL;
				bufferTemp = (uint8_t*)av_malloc(100);
				EbmlList *encodings_list = &track->encodings;
				MatroskaTrackEncoding *encodings = (MatroskaTrackEncoding *)encodings_list->elem;
				if (encodings_list->nb_elem > 1)
				{
					printf("Multiple combined encodings no supported");
				} else if (encodings_list->nb_elem == 1) {
					if (encodings[0].compression.algo == MATROSKA_TRACK_ENCODING_COMP_HEADERSTRIP)
					{
						int encodingSize = encodings[0].compression.settings.size;
						memcpy(bufferTemp,encodings[0].compression.settings.data,encodingSize);
						this->printBuffer(bufferTemp,encodingSize);
						this->videoEncodingInfo.append(bufferTemp,encodingSize);
						this->videoEncodingInfo.type = MATROSKA_TRACK_ENCODING_COMP_HEADERSTRIP;
					} else{
						encodings[0].scope = 0;
						printf("Unsupported encoding type");
					}
				}

				av_free(bufferTemp);


				uint8_t* pdata = track->codec_priv.data;
				int count = 0;
				switch(codec_id)
				{
				case CODEC_ID_H264:
					pdata++;// Reserved - 8bits
					pdata++;// Profile - 8bits
					pdata++;// Reserved - 8bits
					pdata++;// Level - 8bits
					this->lenOfNalPaketSize = ((*pdata++) & 0x03) + 1; // Reserved - 6bits; Size of NALU length minus 1 - 2bits
					count = (*pdata++) & 0x1F;// Reserved - 3bits; Number of Sequence Parameter Sets - 5bits
					while (count -- )
					{
						unsigned int t;
						t = (pdata[0] << 8) | pdata[1]; pdata += 2; // Sequence Parameter Sets, each is prefixed with a two byte big-endian size field
						printf("\t\tSPS Len = %d\n", t);
						videoPaketHeader.append(avc_start_code, sizeof(avc_start_code));
						videoPaketHeader.append(pdata, t);
						pdata += t;
					}
					// Read PPS
					count = *pdata++; // Number of Picture Parameter Sets - 8bits
					while (count -- )
					{
						unsigned int t;
						t = (pdata[0] << 8) | pdata[1]; pdata += 2; // Picture Parameter Sets, each is prefixed with a two byte big-endian size field
						printf("\t\tPPS Len = %d\n", t);
						videoPaketHeader.append(avc_start_code, sizeof(avc_start_code));
						videoPaketHeader.append(pdata, t);
						pdata += t;
					}
				}


				printf("vidoe default_duration=%d\n",track->default_duration);
				return 0;
			}
		}

	}
	return -1;
}

void matroska_reader_t::print_audio_parameters(audio_parameter_t *apara)
{
	printf("strm_id=%d\n",apara->strm_id);
	printf("strm_type=%d\n",apara->strm_type);
	printf("channels=%d\n",apara->channels);
	printf("quant_word_len=%d\n",apara->quant_word_len);
	printf("sample_freq=%d\n",apara->sample_freq);
	printf("bitrate=%d\n",apara->bitrate);
	printf("empasis=%d\n",apara->empasis);
	printf("down_mix_code=%d\n",apara->down_mix_code);
	printf("mute=%d\n",apara->mute);
	printf("dynamic_range=%d\n",apara->dynamic_range);
	printf("channel_assign=%d\n",apara->channel_assign);
	printf("lang_code[0]=%d\n",apara->lang_code[0]);
	printf("lang_code[1]=%d\n",apara->lang_code[1]);
	printf("lang_ext=%d\n",apara->lang_ext);
	printf("fourcc=%d\n",apara->fourcc);
	printf("format_tag=%d\n",apara->format_tag);
}


int matroska_reader_t::get_audio_parameter(audio_parameter_t *apara, int vid )
{
	if(apara)
	{
		memset(apara,0,sizeof(audio_parameter_t));
		MatroskaTrack *tracks = (MatroskaTrack *)m_matroska.tracks.elem;
		MatroskaTrack *track = NULL;
		for (int i=0; i < m_matroska.tracks.nb_elem; i++)
		{
			track = &tracks[i];
			if(track->type == MATROSKA_TRACK_TYPE_AUDIO)
			{
				MatroskaTrackAudio* audio = &track->audio;

				printf("audio_id=%s\n",track->codec_id);

				es_stream_type_t es_type = ES_STREAM_TYPE_UNKNOWN;
				enum CodecID  codec_id = CODEC_ID_NONE;
				printf("audio codec_id=%s\n",track->codec_id);
				for(int j=0; ff_mkv_codec_tags[j].id != CODEC_ID_NONE; j++)
				{
					if(!strncmp(ff_mkv_codec_tags[j].str, track->codec_id,
							strlen(ff_mkv_codec_tags[j].str)))
					{
						codec_id= ff_mkv_codec_tags[j].id;
						es_type = ff_mkv_codec_tags[j].es_type;
						break;
					}
				}
				this->audioTrack = track;
				this->audioCodecID = codec_id;
				this->audioNum = (int)track->num;
				//this->printBuffer(track->codec_priv.data,track->codec_priv.size);

				apara->channels = audio->channels;
				apara->sample_freq = audio->out_samplerate;
				printf("apara->sample_freq=%f  audio->out_samplerate=%f\n",audio->samplerate,audio->out_samplerate);
				apara->strm_type = es_type;
				apara->bitrate = 384000;
				apara->bitrate = 0;

				//this->print_audio_parameters(apara);

				///////build the adts header
				int tempBufSize = 100;
				this->audioPaketHeader.m_Size = 0;
				bit_stream bit;
				uint8_t* bufferTemp = NULL;
				bufferTemp = (uint8_t*)av_malloc(100);
				bit.open(bufferTemp);


				int profile = 0;
				int sri = 0;
				switch(codec_id)
				{
				case CODEC_ID_AAC:
					bit.set_bits(0xFFF,12); // SYNC
					///////????????????????????
					bit.set_bits(0, 1); // ID ?????
					bit.set_bits(0, 2); // layer
					bit.set_bits(1, 1); // protect_abs ||
					profile = matroska_aac_profile(track->codec_id);
					//printf("profile = %d \n",profile);
					bit.set_bits(profile, 2); // profile
					sri = matroska_aac_sri(track->audio.samplerate);
					bit.set_bits(sri, 4);
					bit.set_bits(0, 1); // privates
					bit.set_bits(track->audio.channels, 3);
					bit.set_bits(0, 1); // org_copy
					bit.set_bits(0, 1); // home
					bit.set_bits(0, 1); // copyright_id
					bit.set_bits(0, 1); // cpyrt_start
					bit.set_bits(AUDIO_AVC_HEADER_SIZE, 13); // frm_size
					bit.set_bits(0x7FF, 11);   // buf_full
					bit.set_bits(0, 2);    // num_rdb

					this->audioPaketHeader.append(bufferTemp,AUDIO_AVC_HEADER_SIZE);
					break;
//
//				case CODEC_ID_MP3:
//					bit.set_bits(0xFFFB,16);
//
//					this->audioPaketHeader.append(bufferTemp,2);
					break;
				default:
					break;
				}


				EbmlList *encodings_list = &track->encodings;
				MatroskaTrackEncoding *encodings = (MatroskaTrackEncoding *)encodings_list->elem;
				if (encodings_list->nb_elem > 1) {
					printf("Multiple combined encodings no supported");
				} else if (encodings_list->nb_elem == 1) {
					if (encodings[0].compression.algo == MATROSKA_TRACK_ENCODING_COMP_HEADERSTRIP)
					{
						int encodingSize = encodings[0].compression.settings.size;
						memcpy(bufferTemp,encodings[0].compression.settings.data,encodingSize);
						this->printBuffer(bufferTemp,encodingSize);
						this->audioEncodingInfo.append(bufferTemp,encodingSize);
						this->audioEncodingInfo.type = MATROSKA_TRACK_ENCODING_COMP_HEADERSTRIP;
					} else{
						encodings[0].scope = 0;
						printf("Unsupported encoding type");
					}
				}

				av_free(bufferTemp);
				return 0;
			}
		}
	}
	return -1;
}


void matroska_reader_t::printBuffer(uint8_t* data,int size)
{
	for(int i=0;i<size;i++)
	{
		printf("%02X ",data[i]);
	}
	printf("\n");
}

int matroska_reader_t::read_next_chunk(
		int & type,
		void * & buf,
		int & size,
		int64 & pts,
		int64 & dts,
		int64 & end_pts,
		int & au_start)
{
	au_start = 1;

	static uint32_t *lace_size = NULL;
	static int laces = 0;
	static int cur_lace_index = 0;
	static uint8_t* pdata = NULL;
	static int16_t block_time = 0;
	static int current_block = -1;
	static uint8_t flags = 0;
	static int typeInside = 0;
	static EbmlList *blocks_list;
	static MatroskaBlock *blocks;
	static int res = 0;

	printf("===>>> Entry matroska_reader_t::read_next_chunk\n");

	int n ;
	if(current_block < 0)
	{
		printf(">>>>>>begin reading cluster\n");

		memset(&m_matroska.current_cluster,0,sizeof(MatroskaCluster));
		res = ebml_parse(&m_matroska, matroska_clusters, &m_matroska.current_cluster);
		if(res < 0)
		{
			return res;
		}
		blocks_list = &m_matroska.current_cluster.blocks;
		blocks = (MatroskaBlock *)blocks_list->elem;
		current_block = 0;
	}

	if(current_block < blocks_list->nb_elem )
	{
		printf("current_block=%d\n",current_block);

		if (blocks[current_block].bin.size > 0 && blocks[current_block].bin.data )
		{
			int is_keyframe = blocks[current_block].non_simple ? !blocks[current_block].reference : -1;

			if(cur_lace_index == 0)
			{
				pdata = blocks[current_block].bin.data;
				size = blocks[current_block].bin.size;

				uint64_t num = 0;
				if ((n = matroska_ebmlnum_uint(&m_matroska, pdata, blocks[current_block].bin.size, &num)) < 0)
				{
					printf("EBML block data error\n");
					return res;
				}

				typeInside = (int)num;
				pdata += n;
				size -= n;

				//dts = (blocks[current_block].duration + m_matroska.current_cluster.timecode) / m_matroska.time_scale;
				block_time = AV_RB16(pdata);
				pts = (m_matroska.current_cluster.timecode + block_time) * m_matroska.time_scale / 1000;
				pdata += 2;
				size -= 2;

				//flag byte
				flags = *pdata++;
				size -= 1;
				printf("flag	=%02X\n",flags);

				switch ((flags & 0x06) >> 1) {
				case 0x0: /* no lacing */
					laces = 1;
					lace_size = (uint32_t*)av_mallocz(sizeof(int));
					lace_size[0] = size;
					break;

				case 0x1: /* Xiph lacing */
				case 0x2: /* fixed-size lacing */
				case 0x3: /* EBML lacing */
					laces = (*pdata) + 1;
					pdata += 1;
					size -= 1;
					lace_size = (uint32_t*)av_mallocz(laces * sizeof(int));

					switch ((flags & 0x06) >> 1) {
					case 0x1: /* Xiph lacing */ {
						uint8_t temp;
						uint32_t total = 0;
						for (n = 0; res == 0 && n < laces - 1; n++) {
							while (1) {
								if (size == 0) {
									res = -1;
									break;
								}
								temp = *pdata;
								lace_size[n] += temp;
								pdata += 1;
								size -= 1;
								if (temp != 0xff)
									break;
							}
							total += lace_size[n];
						}
						lace_size[n] = size - total;
						break;
					}

					case 0x2: /* fixed-size lacing */
						for (n = 0; n < laces; n++)
							lace_size[n] = size / laces;
						break;

					case 0x3: /* EBML lacing */ {
						uint32_t total;
						n = matroska_ebmlnum_uint(&m_matroska, pdata, size, &num);
						if (n < 0) {
							printf("EBML block data error\n");
							break;
						}
						pdata += n;
						size -= n;
						total = lace_size[0] = num;
						for (n = 1; res == 0 && n < laces - 1; n++) {
							int64_t snum;
							int r;
							r = matroska_ebmlnum_sint(&m_matroska, pdata, size, &snum);
							if (r < 0) {
								printf("EBML block data error\n");
								break;
							}
							pdata += r;
							size -= r;
							lace_size[n] = lace_size[n - 1] + snum;
							total += lace_size[n];
						}
						lace_size[n] = size - total;
						break;
					}
					}
					break;
				}
			}

			if(cur_lace_index < laces)
			{
				int pre_lace_sum = 0;
				for(int m=0;m < cur_lace_index;m++)
				{
					pre_lace_sum += lace_size[m];
				}

				if(typeInside == this->videoNum)
				{
					type = 0;
					uint8_t* p = NULL;
					int remainBlockSize = 0;
					int curNalSize = 0;
					memcpy(buf,this->videoPaketHeader.m_pData,this->videoPaketHeader.m_Size);
					uint8_t* pcur = (uint8_t*)buf + this->videoPaketHeader.m_Size;
					switch(this->videoEncodingInfo.type)
					{
					case MATROSKA_TRACK_ENCODING_COMP_HEADERSTRIP:
						pdata -= this->videoEncodingInfo.m_Size;
						memcpy(pdata,this->videoEncodingInfo.m_pData,this->videoEncodingInfo.m_Size);
					default:
						break;
					}

					switch(this->videoCodecID)
					{
					case CODEC_ID_H264:
						p = pdata;
						remainBlockSize = lace_size[cur_lace_index];
						while (remainBlockSize > 0)
						{
							for(int i=0;i<lenOfNalPaketSize;i++)
							{
								curNalSize = curNalSize << 8 | p[i];
							}
							memcpy(p,avc_start_code,sizeof(avc_start_code));
							p += lenOfNalPaketSize;
							remainBlockSize -= lenOfNalPaketSize;
							if(curNalSize > remainBlockSize)
							{
								curNalSize = remainBlockSize;
							}
							remainBlockSize -= curNalSize;
							p += curNalSize;
						}
						break;
					default:
						break;
					}
					memcpy(pcur,pdata + pre_lace_sum,lace_size[cur_lace_index]+this->videoEncodingInfo.m_Size);
					size = lace_size[cur_lace_index]+this->videoPaketHeader.m_Size + this->videoEncodingInfo.m_Size;

				}else if(typeInside == this->audioNum){
					type = 1;

					bit_stream bit;
					uint8_t* audioHeaderBuffer = NULL;
					switch(this->audioCodecID)
					{
					case CODEC_ID_AAC:
						audioHeaderBuffer = this->audioPaketHeader.m_pData;
						bit.open(audioHeaderBuffer);
						bit.skip_bits(30);
						bit.set_bits(AUDIO_AVC_HEADER_SIZE + lace_size[cur_lace_index],13);
						bit.set_bits(0x7FF,11);
						bit.set_bits(0,2);
						break;

					default:
						break;
					}
					memcpy(buf,this->audioPaketHeader.m_pData,this->audioPaketHeader.m_Size);
					uint8_t* pcur = (uint8_t*)buf + this->audioPaketHeader.m_Size;
					switch(this->audioEncodingInfo.type)
					{
					case MATROSKA_TRACK_ENCODING_COMP_HEADERSTRIP:
						memcpy(pcur,this->audioEncodingInfo.m_pData,this->audioEncodingInfo.m_Size);
						pcur += this->audioEncodingInfo.m_Size;
					default:
						break;
					}
					memcpy( pcur,pdata + pre_lace_sum,lace_size[cur_lace_index]);
					printf("\t\t>audio:");
					size = lace_size[cur_lace_index] +  this->audioPaketHeader.m_Size+this->audioEncodingInfo.m_Size;
				}

				printf("         cur_lace_index=%d lace_tatol=%d current_block=%d\n",
						cur_lace_index,laces,current_block);

				cur_lace_index ++ ;
				if(cur_lace_index >= laces)
				{
					av_free(lace_size);
					cur_lace_index = 0;
					printf("free lace_size done \n");
					current_block ++;
				}
			}
		}

	}
	if(current_block >= blocks_list->nb_elem )
	{
		current_block = -1;
		ebml_free(matroska_cluster, &m_matroska.current_cluster);
	}

	printf("<<<<===== Leave the matroska_reader_t::read_next_chunk\n");

	return 0;
}

int matroska_reader_t::matroska_read_header()
{
	EbmlList	*attachements_list	=	&m_matroska.attachments;
	EbmlList	*chapters_list		=	&m_matroska.chapters;
	Ebml 		ebml = {0};
	int 		i = 0;
	int			j = 0;
	int			res = 0;

	/* First read the EBML header. */
	if (ebml_parse(&m_matroska, ebml_syntax, &ebml)
			|| ebml.version > EBML_VERSION       || ebml.max_size > sizeof(uint64_t)
			|| ebml.id_length > sizeof(uint32_t) || ebml.doctype_version > 2) {
		printf("EBML header using unsupported features\n");
		ebml_free(ebml_syntax, &ebml);
		return -1;//failed
	}


	for (i = 0; i < array_size(matroska_doctypes); i++)
	{
		if (!strcmp(ebml.doctype, matroska_doctypes[i]))
		{
			printf("ebml.doctype=%s\n",ebml.doctype);
			break;
		}
	}
	if (i >= array_size(matroska_doctypes))
	{
		printf("Unknown EBML doctype '%s'\n", ebml.doctype);
	}
	ebml_free(ebml_syntax, &ebml);

	/* The next thing is a segment. */

	if ((res = ebml_parse(&m_matroska, matroska_segments, &m_matroska)) < 0)
		return res;
	matroska_execute_seekhead(&m_matroska);

	if (!m_matroska.time_scale)
		m_matroska.time_scale = 1000000;
	if (m_matroska.duration)
		m_matroska.ctx->duration = m_matroska.duration * m_matroska.time_scale * 1000 / AV_TIME_BASE;


	printf("=============%f     %d   ===========",m_matroska.duration,m_matroska.time_scale);
	//av_metadata_set2(&s->metadata, "title", m_matroska.title, 0)

	MatroskaTrack *tracks = (MatroskaTrack *)m_matroska.tracks.elem;
	for (i=0; i < m_matroska.tracks.nb_elem; i++) {
		MatroskaTrack *track = &tracks[i];
		printf("i=%d codec_id = %s\n",i,track->codec_id);
		enum CodecID codec_id = CODEC_ID_NONE;
		EbmlList *encodings_list = &tracks->encodings;
		MatroskaTrackEncoding *encodings = (MatroskaTrackEncoding *)encodings_list->elem;
		uint8_t *extradata = NULL;
		int extradata_size = 0;
		int extradata_offset = 0;
		ByteIOContext b;

		//* Apply some sanity checks. *
		if (track->type != MATROSKA_TRACK_TYPE_VIDEO &&
				track->type != MATROSKA_TRACK_TYPE_AUDIO &&
				track->type != MATROSKA_TRACK_TYPE_SUBTITLE) {
			printf("Unknown or unsupported track type %d\n",track->type);
			continue;
		}
		if (track->codec_id == NULL)
			continue;

		if (track->type == MATROSKA_TRACK_TYPE_VIDEO) {
			if (!track->default_duration)
				track->default_duration = 1000000000/track->video.frame_rate;
			if (!track->video.display_width)
				track->video.display_width = track->video.pixel_width;
			if (!track->video.display_height)
				track->video.display_height = track->video.pixel_height;
		} else if (track->type == MATROSKA_TRACK_TYPE_AUDIO) {
			if (!track->audio.out_samplerate)
				track->audio.out_samplerate = track->audio.samplerate;
		}
		if (encodings_list->nb_elem > 1) {
			printf("Multiple combined encodings no supported");
		} else if (encodings_list->nb_elem == 1) {
			if (encodings[0].type ||
					(encodings[0].compression.algo != MATROSKA_TRACK_ENCODING_COMP_HEADERSTRIP &&
							encodings[0].compression.algo != MATROSKA_TRACK_ENCODING_COMP_LZO)) {
				encodings[0].scope = 0;
				printf("Unsupported encoding type");
			} else if (track->codec_priv.size && encodings[0].scope&2) {
				uint8_t *codec_priv = track->codec_priv.data;
				int offset = matroska_decode_buffer(&track->codec_priv.data,
						&track->codec_priv.size,
						track);//????????????????????????????


				if (offset < 0) {
					track->codec_priv.data = NULL;
					track->codec_priv.size = 0;
					printf("Failed to decode codec private data\n");
				} else if (offset > 0) {
					track->codec_priv.data = (uint8_t*)av_malloc(track->codec_priv.size + offset);
					memcpy(track->codec_priv.data,			encodings[0].compression.settings.data, offset);
					memcpy(track->codec_priv.data+offset, 	codec_priv,		track->codec_priv.size);
					track->codec_priv.size += offset;
				}
				if (codec_priv != track->codec_priv.data)
					av_free(codec_priv);
			}
		}

		for(j=0; ff_mkv_codec_tags[j].id != CODEC_ID_NONE; j++){
			if(!strncmp(ff_mkv_codec_tags[j].str, track->codec_id,
					strlen(ff_mkv_codec_tags[j].str))){
				codec_id= ff_mkv_codec_tags[j].id;
				break;
			}
		}

		//st = track->stream = av_new_stream(s, 0);
		//if (st == NULL)
		//    return AVERROR(ENOMEM);
		AVCodecContext codec;

		if (!strcmp(track->codec_id, "V_MS/VFW/FOURCC")
				&& track->codec_priv.size >= 40
				&& track->codec_priv.data != NULL) {
			track->ms_compat = 1;
			track->video.fourcc = AV_RL32(track->codec_priv.data + 16);
			codec_id = ff_codec_get_id(ff_codec_bmp_tags, track->video.fourcc);
			extradata_offset = 40;
		} else if (!strcmp(track->codec_id, "A_MS/ACM")
				&& track->codec_priv.size >= 14
				&& track->codec_priv.data != NULL) {
			init_put_byte(&b, track->codec_priv.data, track->codec_priv.size,NULL,NULL,NULL,NULL,NULL);
			ff_get_wav_header(&b, &codec, track->codec_priv.size);
			codec_id = codec.codec_id;
			extradata_offset = VXMIN(track->codec_priv.size, 18);
		} else if (!strcmp(track->codec_id, "V_QUICKTIME")
				&& (track->codec_priv.size >= 86)
				&& (track->codec_priv.data != NULL)) {
			track->video.fourcc = AV_RL32(track->codec_priv.data);
			//codec_id=ff_codec_get_id(codec_movvideo_tags, track->video.fourcc);
		} else if (codec_id == CODEC_ID_PCM_S16BE) {
			switch (track->audio.bitdepth) {
			case  8:  codec_id = CODEC_ID_PCM_U8;     break;
			case 24:  codec_id = CODEC_ID_PCM_S24BE;  break;
			case 32:  codec_id = CODEC_ID_PCM_S32BE;  break;
			}
		} else if (codec_id == CODEC_ID_PCM_S16LE) {
			switch (track->audio.bitdepth) {
			case  8:  codec_id = CODEC_ID_PCM_U8;     break;
			case 24:  codec_id = CODEC_ID_PCM_S24LE;  break;
			case 32:  codec_id = CODEC_ID_PCM_S32LE;  break;
			}
		} else if (codec_id==CODEC_ID_PCM_F32LE && track->audio.bitdepth==64) {
			codec_id = CODEC_ID_PCM_F64LE;
		} else if (codec_id == CODEC_ID_AAC && !track->codec_priv.size) {
			int profile = matroska_aac_profile(track->codec_id);
			int sri = matroska_aac_sri(track->audio.samplerate);
			extradata = (uint8_t*)av_malloc(5);
			if (extradata == NULL)
				return -1;
			extradata[0] = (profile << 3) | ((sri&0x0E) >> 1);
			extradata[1] = ((sri&0x01) << 7) | (track->audio.channels<<3);
			if (strstr(track->codec_id, "SBR")) {
				//sri = matroska_aac_sri(track->audio.out_samplerate);
				extradata[2] = 0x56;
				extradata[3] = 0xE5;
				extradata[4] = 0x80 | (sri<<3);
				extradata_size = 5;
			} else
				extradata_size = 2;
		} else if (codec_id == CODEC_ID_TTA) {
			extradata_size = 30;
			extradata = (uint8_t*)av_malloc(extradata_size);
			if (extradata == NULL)
				return -1;
			init_put_byte(&b, extradata, extradata_size, 1,
					NULL, NULL, NULL, NULL);
			put_buffer(&b, "TTA1", 4);
			put_le16(&b, 1);
			put_le16(&b, track->audio.channels);
			put_le16(&b, track->audio.bitdepth);
			put_le32(&b, track->audio.out_samplerate);
			put_le32(&b, m_matroska.ctx->duration * track->audio.out_samplerate);
		} else if (codec_id == CODEC_ID_RV10 || codec_id == CODEC_ID_RV20 ||
				codec_id == CODEC_ID_RV30 || codec_id == CODEC_ID_RV40) {
			extradata_offset = 26;
		} else if (codec_id == CODEC_ID_RA_144) {
			track->audio.out_samplerate = 8000;
			track->audio.channels = 1;
		} else if (codec_id == CODEC_ID_RA_288 || codec_id == CODEC_ID_COOK ||
				codec_id == CODEC_ID_ATRAC3 || codec_id == CODEC_ID_SIPR) {
			int flavor;
			init_put_byte(&b, track->codec_priv.data,track->codec_priv.size,NULL,NULL,NULL,NULL,NULL);
			url_fskip(&b, 22);
			flavor                       = get_be16(&b);
			track->audio.coded_framesize = get_be32(&b);
			url_fskip(&b, 12);
			track->audio.sub_packet_h    = get_be16(&b);
			track->audio.frame_size      = get_be16(&b);
			track->audio.sub_packet_size = get_be16(&b);
			//track->audio.buf = malloc(track->audio.frame_size * track->audio.sub_packet_h);

		}
		track->codec_priv.size -= extradata_offset;

		if (codec_id == CODEC_ID_NONE)
			printf("Unknown/unsupported CodecID %s.\n", track->codec_id);

		if (track->time_scale < 0.01)
			track->time_scale = 1.0;
		//av_set_pts_info(st, 64, m_matroska.time_scale*track->time_scale, 1000*1000*1000); //* 64 bit pts in ns
	}

	MatroskaAttachement *attachements = (MatroskaAttachement *)attachements_list->elem;
	for (j=0; j<attachements_list->nb_elem; j++) {
		if (!(attachements[j].filename && attachements[j].mime &&
				attachements[j].bin.data && attachements[j].bin.size > 0)) {
			printf( "incomplete attachment\n");
		} else {

			for (i=0; ff_mkv_mime_tags[i].id != CODEC_ID_NONE; i++) {
				if (!strncmp(ff_mkv_mime_tags[i].str, attachements[j].mime,
						strlen(ff_mkv_mime_tags[i].str))) {
					//st->codec->codec_id = ff_mkv_mime_tags[i].id;
					break;
				}
			}
			//attachements[j].stream = st;
		}
	}

	return 0;
}

int matroska_reader_t::ebml_parse(MatroskaDemuxContext *matroska, EbmlSyntax *syntax,void *data)
{
	if (!matroska->current_id) {
		uint64_t id;
		int res = ebml_read_num(matroska, matroska->ctx->pb, 4, &id);
		if (res < 0)
			return res;
		matroska->current_id = id | 1 << 7*res;
	}
	return ebml_parse_id(matroska, syntax, matroska->current_id, data);
}

void matroska_reader_t::ebml_free(EbmlSyntax *syntax, void *data)
{
	int i, j;
	for (i=0; syntax[i].id; i++) {
		void *data_off = (char *)data + syntax[i].data_offset;
		switch (syntax[i].type) {
		case EBML_STR:
		case EBML_UTF8:  av_freep(data_off);                      break;
		case EBML_BIN:   av_freep(&((EbmlBin *)data_off)->data);  break;
		case EBML_NEST:
			if (syntax[i].list_elem_size) {
				EbmlList *list = (EbmlList *)data_off;
				char *ptr = (char*)list->elem;
				for (j=0; j<list->nb_elem; j++, ptr+=syntax[i].list_elem_size)
					ebml_free(syntax[i].def.n, ptr);
				av_free(list->elem);
			} else
				ebml_free(syntax[i].def.n, data_off);
		default:  break;
		}
	}
}

int matroska_reader_t::ebml_read_num(MatroskaDemuxContext *matroska, ByteIOContext *pb,
		int max_size, uint64_t *number)
{
	int read = 1, n = 1;
	uint64_t total = 0;

	if (!(total = get_byte(pb))) {
		/* we might encounter EOS here */
		if (!url_feof(pb)) {
			int64_t pos = url_ftell(pb);
			printf("Read error at pos. %d (0x%d\n",pos, pos);
		}
		return 0; /* EOS or actual I/O error */
	}

	/* get the length of the EBML number */
	read = 8 - ff_log2_tab[total];
	if (read > max_size) {
		int64_t pos = url_ftell(pb) - 1;
		printf("Invalid EBML number size tag 0x%02x at pos %x (0x%x)\n",(uint8_t) total, pos, pos);
		return -1;
	}

	/* read out length */
	total ^= 1 << ff_log2_tab[total];
	while (n++ < read)
		total = (total << 8) | get_byte(pb);

	*number = total;

	return read;
}

int matroska_reader_t::ebml_parse_id(MatroskaDemuxContext *matroska, EbmlSyntax *syntax,
		uint32_t id, void *data)
{
	int i;
	for (i=0; syntax[i].id; i++)
		if (id == syntax[i].id)
			break;
	if (!syntax[i].id && id == MATROSKA_ID_CLUSTER &&
			matroska->num_levels > 0 &&
			matroska->levels[matroska->num_levels-1].length == -1)
		return 0;  // we reached the end of an unknown size cluster
	if (!syntax[i].id && id != EBML_ID_VOID && id != EBML_ID_CRC32)
	{
		printf("Unknown entry 0x%X\n", id);
	}
	return ebml_parse_elem(matroska, &syntax[i], data);
}

int matroska_reader_t::ebml_parse_elem(MatroskaDemuxContext *matroska,
		EbmlSyntax *syntax, void *data)
{
	ByteIOContext *pb = matroska->ctx->pb;
	uint32_t id = syntax->id;
	uint64_t length;
	int res;

	data = (char *)data + syntax->data_offset;
	if (syntax->list_elem_size) {
		EbmlList *list = (EbmlList *)data;
		list->elem = realloc(list->elem, (list->nb_elem+1)*syntax->list_elem_size);
		data = (char*)list->elem + list->nb_elem*syntax->list_elem_size;
		memset(data, 0, syntax->list_elem_size);
		list->nb_elem++;
	}

	if (syntax->type != EBML_PASS && syntax->type != EBML_STOP) {
		matroska->current_id = 0;
		if ((res = ebml_read_length(matroska, pb, &length)) < 0)
			return res;
	}

	switch (syntax->type) {
	case EBML_UINT:  res = ebml_read_uint  (pb, length, (uint64_t*)data);  break;
	case EBML_FLOAT: res = ebml_read_float (pb, length, (double *)data);  break;
	case EBML_STR:
	case EBML_UTF8:  res = ebml_read_ascii (pb, length, (char**)data);  break;
	case EBML_BIN:   res = ebml_read_binary(pb, length, (EbmlBin *)data);  break;
	case EBML_NEST:  if ((res=ebml_read_master(matroska, length)) < 0)
		return res;
	if (id == MATROSKA_ID_SEGMENT)
		matroska->segment_start = url_ftell(matroska->ctx->pb);
	return ebml_parse_nest(matroska, syntax->def.n, data);
	case EBML_PASS:  return ebml_parse_id(matroska, syntax->def.n, id, data);
	case EBML_STOP:  return 1;
	default:         return url_fseek(pb,length,SEEK_CUR)<0 ? -1 : 0;
	}

	return res;
}

int matroska_reader_t::ebml_read_length(MatroskaDemuxContext *matroska, ByteIOContext *pb,
		uint64_t *number)
{
	int res = ebml_read_num(matroska, pb, 8, number);
	if (res > 0 && *number + 1 == 1ULL << (7 * res))
		*number = 0xffffffffffffffULL;
	return res;
}

int matroska_reader_t::ebml_read_uint(ByteIOContext *pb, int size, uint64_t *num)
{
	int n = 0;

	if (size > 8)
		return -1;

	/* big-endian ordering; build up number */
	*num = 0;
	while (n++ < size)
		*num = (*num << 8) | get_byte(pb);

	return 0;
}

int matroska_reader_t::ebml_read_float(ByteIOContext *pb, int size, double *num)
{
	if (size == 0) {
		*num = 0;
	} else if (size == 4) {
		*num= av_int2flt(get_be32(pb));
	} else if(size==8){
		*num= av_int2dbl(get_be64(pb));
	} else
		return -1;

	return 0;
}

int matroska_reader_t::ebml_read_ascii(ByteIOContext *pb, int size, char **str)
{
	av_free(*str);
	/* EBML strings are usually not 0-terminated, so we allocate one
	 * byte more, read the string and NULL-terminate it ourselves. */
	if (!(*str = (char*)av_malloc(size + 1)))
		return -1;
	if (get_buffer(pb, (uint8_t *) *str, size) != size) {
		av_freep(str);
		return -1;
	}
	(*str)[size] = '\0';

	return 0;
}

int matroska_reader_t::ebml_read_binary(ByteIOContext *pb, int length, EbmlBin *bin)
{
	av_free(bin->data);
	if (!(bin->data = (uint8_t*)av_malloc(length)))
		return -1;

	bin->size = length;
	bin->pos  = url_ftell(pb);
	if (get_buffer(pb, bin->data, length) != length) {
		av_freep(&bin->data);
		return -1;
	}

	return 0;
}

int matroska_reader_t::ebml_read_master(MatroskaDemuxContext *matroska, uint64_t length)
{
	ByteIOContext *pb = matroska->ctx->pb;
	MatroskaLevel *level;

	if (matroska->num_levels >= EBML_MAX_DEPTH) {
		printf("File moves beyond max. allowed depth (%d)\n", EBML_MAX_DEPTH);
		return -1;
	}

	level = &matroska->levels[matroska->num_levels++];
	level->start = url_ftell(pb);
	level->length = length;

	return 0;
}

int matroska_reader_t::ebml_parse_nest(MatroskaDemuxContext *matroska, EbmlSyntax *syntax,
		void *data)
{
	int i, res = 0;

	for (i=0; syntax[i].id; i++)
		switch (syntax[i].type) {
		case EBML_UINT:
			*(uint64_t *)((char *)data+syntax[i].data_offset) = syntax[i].def.u;
			break;
		case EBML_FLOAT:
			*(double   *)((char *)data+syntax[i].data_offset) = syntax[i].def.f;
			break;
		case EBML_STR:
		case EBML_UTF8:
			*(char    **)((char *)data+syntax[i].data_offset) = av_strdup(syntax[i].def.s);
			break;
		}

	while (!res && !ebml_level_end(matroska))
		res = ebml_parse(matroska, syntax, data);

	return res;
}

int matroska_reader_t::ebml_level_end(MatroskaDemuxContext *matroska)
{
	ByteIOContext *pb = matroska->ctx->pb;
	int64_t pos = url_ftell(pb);

	if (matroska->num_levels > 0) {
		MatroskaLevel *level = &matroska->levels[matroska->num_levels - 1];
		if (pos - level->start >= level->length || matroska->current_id) {
			matroska->num_levels--;
			return 1;
		}
	}
	return 0;
}

int matroska_reader_t::matroska_ebmlnum_uint(MatroskaDemuxContext *matroska,
		uint8_t *data, uint32_t size, uint64_t *num)
{
	ByteIOContext pb;
	init_put_byte(&pb, data, size, 0, NULL, NULL, NULL, NULL);
	return ebml_read_num(matroska, &pb, VXMIN(size, 8), num);
}

int matroska_reader_t::matroska_ebmlnum_sint(MatroskaDemuxContext *matroska,
		uint8_t *data, uint32_t size, int64_t *num)
{
	uint64_t unum;
	int res = 0;

	/* read as unsigned number first */
	if ((res = matroska_ebmlnum_uint(matroska, data, size, &unum)) < 0)
		return res;

	/* make signed (weird way) */
	*num = unum - ((1LL << (7*res - 1)) - 1);

	return res;
}

MatroskaTrack* matroska_reader_t::matroska_find_track_by_num(MatroskaDemuxContext *matroska,
		int num)
{
	MatroskaTrack *tracks = (MatroskaTrack *)matroska->tracks.elem;
	int i;

	for (i=0; i < matroska->tracks.nb_elem; i++)
		if (tracks[i].num == num)
			return &tracks[i];

	printf("Invalid track number %d\n", num);
	return NULL;
}

int matroska_reader_t::matroska_parse_cluster(MatroskaDemuxContext *matroska)
{
	MatroskaCluster cluster = { 0 };
	EbmlList *blocks_list;
	MatroskaBlock *blocks;
	int i, res;
	int64_t pos = url_ftell(matroska->ctx->pb);
	//matroska->prev_pkt = NULL;
	//if (matroska->current_id)
	//    pos -= 4;  /* sizeof the ID which was already read */
	res = ebml_parse(matroska, matroska_clusters, &cluster);
	blocks_list = &cluster.blocks;
	blocks = (MatroskaBlock *)blocks_list->elem;
	for (i=0; i<blocks_list->nb_elem; i++)
	{
		if (blocks[i].bin.size > 0 && blocks[i].bin.data)
		{
			int is_keyframe = blocks[i].non_simple ? !blocks[i].reference : -1;
			//	            res=matroska_parse_block(matroska,
			//	                                     blocks[i].bin.data, blocks[i].bin.size,
			//	                                     blocks[i].bin.pos,  cluster.timecode,
			//	                                     blocks[i].duration, is_keyframe,
			//	                                     pos);
		}
	}
	ebml_free(matroska_cluster, &cluster);
	if (res < 0)  matroska->done = 1;
	return res;
}

int matroska_reader_t::av_new_packet(AVPacket *pkt, int size)
{
	uint8_t *data= NULL;
	if((unsigned)size < (unsigned)size + FF_INPUT_BUFFER_PADDING_SIZE)
		data = (uint8_t*)av_malloc(size + FF_INPUT_BUFFER_PADDING_SIZE);
	if (data){
		memset(data + size, 0, FF_INPUT_BUFFER_PADDING_SIZE);
	}else
		size=0;

	av_init_packet(pkt);
	pkt->data = data;
	pkt->size = size;
	//pkt->destruct = av_destruct_packet;
	pkt->destruct = NULL;
	if(!data)
		return -1;
	return 0;
}

void matroska_reader_t::av_init_packet(AVPacket *pkt)
{
	pkt->pts   = AV_NOPTS_VALUE;
	pkt->dts   = AV_NOPTS_VALUE;
	pkt->pos   = -1;
	pkt->duration = 0;
	pkt->convergence_duration = 0;
	pkt->flags = 0;
	pkt->stream_index = 0;
	pkt->destruct= NULL;
}

void matroska_reader_t::av_destruct_packet(AVPacket *pkt)
{
	av_free(pkt->data);
	pkt->data = NULL; pkt->size = 0;
}

int matroska_reader_t::matroska_parse_block(MatroskaDemuxContext *matroska, uint8_t *data,
		int size, int64_t pos, uint64_t cluster_time,
		uint64_t duration, int is_keyframe,
		int64_t cluster_pos,
		int & type,
		void * & out_buffer,
		int & out_size,
		int64 & pts,
		int64 & dts,
		int64 & end_pts)
{
	uint64_t timecode = AV_NOPTS_VALUE;
	MatroskaTrack *track;
	int res = 0;
	AVStream *st;
	AVPacket *pkt;
	int16_t block_time;
	uint32_t *lace_size = NULL;
	int n, flags, laces = 0;
	uint64_t num;

	if ((n = matroska_ebmlnum_uint(matroska, data, size, &num)) < 0) {
		printf("EBML block data error\n");
		return res;
	}
	data += n;//character add
	size -= n;//size sub

	track = matroska_find_track_by_num(matroska, num);
	if (size <= 3 || !track /*|| !track->stream*/) {
		printf("Invalid stream 0x%X or size %u\n", num, size);
		return res;
	}
	type = num;
	st = track->stream;
	//		if (st->discard >= AVDISCARD_ALL)
	//			return res;
	if (duration == AV_NOPTS_VALUE)
		duration = track->default_duration / matroska->time_scale;

	block_time = AV_RB16(data);
	data += 2;
	flags = *data++;
	size -= 3;
	if (is_keyframe == -1)
		is_keyframe = flags & 0x80 ? AV_PKT_FLAG_KEY : 0;

	if (cluster_time != (uint64_t)-1
			&& (block_time >= 0 || cluster_time >= -block_time)) {
		dts = timecode = cluster_time + block_time;
		if (track->type == MATROSKA_TRACK_TYPE_SUBTITLE
				&& timecode < track->end_timecode)
			is_keyframe = 0;  /* overlapping subtitles are not key frame */
		//				if (is_keyframe)
		//					av_add_index_entry(st, cluster_pos, timecode, 0,0,AVINDEX_KEYFRAME);
		end_pts = pts = track->end_timecode = VXMAX(track->end_timecode, timecode+duration);
	}

	if (matroska->skip_to_keyframe && track->type != MATROSKA_TRACK_TYPE_SUBTITLE) {
		if (!is_keyframe || timecode < matroska->skip_to_timecode)
			return res;
		matroska->skip_to_keyframe = 0;
	}

	switch ((flags & 0x06) >> 1) {
	case 0x0: /* no lacing */
		laces = 1;
		lace_size = (uint32_t*)av_mallocz(sizeof(int));
		lace_size[0] = size;
		break;

	case 0x1: /* Xiph lacing */
	case 0x2: /* fixed-size lacing */
	case 0x3: /* EBML lacing */
		laces = (*data) + 1;
		data += 1;
		size -= 1;
		lace_size = (uint32_t*)av_mallocz(laces * sizeof(int));

		switch ((flags & 0x06) >> 1) {
		case 0x1: /* Xiph lacing */ {
			uint8_t temp;
			uint32_t total = 0;
			for (n = 0; res == 0 && n < laces - 1; n++) {
				while (1) {
					if (size == 0) {
						res = -1;
						break;
					}
					temp = *data;
					lace_size[n] += temp;
					data += 1;
					size -= 1;
					if (temp != 0xff)
						break;
				}
				total += lace_size[n];
			}
			lace_size[n] = size - total;
			break;
		}

		case 0x2: /* fixed-size lacing */
			for (n = 0; n < laces; n++)
				lace_size[n] = size / laces;
			break;

		case 0x3: /* EBML lacing */ {
			uint32_t total;
			n = matroska_ebmlnum_uint(matroska, data, size, &num);
			if (n < 0) {
				printf("EBML block data error\n");
				break;
			}
			data += n;
			size -= n;
			total = lace_size[0] = num;
			for (n = 1; res == 0 && n < laces - 1; n++) {
				int64_t snum;
				int r;
				r = matroska_ebmlnum_sint(matroska, data, size, &snum);
				if (r < 0) {
					printf("EBML block data error\n");
					break;
				}
				data += r;
				size -= r;
				lace_size[n] = lace_size[n - 1] + snum;
				total += lace_size[n];
			}
			lace_size[n] = size - total;
			break;
		}
		}
		break;
	}

	if (res == 0) {
		for (n = 0; n < laces; n++)
		{
			MatroskaTrackEncoding *encodings = (MatroskaTrackEncoding *)track->encodings.elem;
			int offset = 0, pkt_size = lace_size[n];
			uint8_t *pkt_data = data;

			if (pkt_size > size) {
				printf( "Invalid packet size\n");
				break;
			}

			if (encodings && encodings->scope & 1) {
				offset = matroska_decode_buffer(&pkt_data,&pkt_size, track);
				if (offset < 0)
					continue;
			}

			pkt = (AVPacket *)av_mallocz(sizeof(AVPacket));
			if (av_new_packet(pkt, pkt_size+offset) < 0) {
				av_free(pkt);
				res = -1;
				break;
			}
			if (offset)
				memcpy (pkt->data, encodings->compression.settings.data, offset);
			memcpy (pkt->data+offset, pkt_data, pkt_size);

			if (pkt_data != data)
				av_free(pkt_data);

			if (n == 0)
				pkt->flags = is_keyframe;
			pkt->stream_index = st->index;

			if (track->ms_compat)
				pkt->dts = timecode;
			else
				pkt->pts = timecode;
			pkt->pos = pos;
			if (st->codec->codec_id == CODEC_ID_TEXT)
				pkt->convergence_duration = duration;
			else if (track->type != MATROSKA_TRACK_TYPE_SUBTITLE)
				pkt->duration = duration;

			//				if (st->codec->codec_id == CODEC_ID_SSA)
			//					matroska_fix_ass_packet(matroska, pkt, duration);


			if (timecode != AV_NOPTS_VALUE)
				timecode = duration ? timecode + duration : AV_NOPTS_VALUE;
			data += lace_size[n];
			size -= lace_size[n];
		}
	}

	memcpy(out_buffer, data,size);
	out_size = size;

	av_free(lace_size);
	return res;
}


void matroska_reader_t::matroska_execute_seekhead(MatroskaDemuxContext *matroska)
{
	EbmlList *seekhead_list = &matroska->seekhead;
	MatroskaSeekhead *seekhead = (MatroskaSeekhead *)seekhead_list->elem;
	uint32_t level_up = matroska->level_up;
	printf("matroska->level_up=%d\n",matroska->level_up);
	int64_t before_pos = url_ftell(matroska->ctx->pb);
	uint32_t saved_id = matroska->current_id;
	MatroskaLevel level;
	int i;

	// we should not do any seeking in the streaming case
	if (url_is_streamed(matroska->ctx->pb) || (matroska->ctx->flags & AVFMT_FLAG_IGNIDX))
		return;

	for (i=0; i<seekhead_list->nb_elem; i++) {
		int64_t offset = seekhead[i].pos + matroska->segment_start;

		if (seekhead[i].pos <= before_pos
				|| seekhead[i].id == MATROSKA_ID_SEEKHEAD
				|| seekhead[i].id == MATROSKA_ID_CLUSTER)
			continue;

		/* seek */
		if (url_fseek(matroska->ctx->pb, offset, SEEK_SET) != offset)
			continue;

		/* We don't want to lose our seekhead level, so we add
		 * a dummy. This is a crude hack. */
		if (matroska->num_levels == EBML_MAX_DEPTH) {
			printf("Max EBML element depth (%d) reached, cannot parse further.\n", EBML_MAX_DEPTH);
			break;
		}

		level.start = 0;
		level.length = (uint64_t)-1;
		matroska->levels[matroska->num_levels] = level;
		matroska->num_levels++;
		matroska->current_id = 0;

		ebml_parse(matroska, matroska_segment, matroska);

		/* remove dummy level */
		while (matroska->num_levels) {
			uint64_t length = matroska->levels[--matroska->num_levels].length;
			if (length == (uint64_t)-1)
				break;
		}
	}

	/* seek back */
	url_fseek(matroska->ctx->pb, before_pos, SEEK_SET);
	matroska->level_up = level_up;
	matroska->current_id = saved_id;
}

unsigned int matroska_reader_t::ff_toupper4(unsigned int x)
{
	return     toupper( x     &0xFF)
			+ (toupper((x>>8 )&0xFF)<<8 )
			+ (toupper((x>>16)&0xFF)<<16)
			+ (toupper((x>>24)&0xFF)<<24);
}

enum CodecID matroska_reader_t::ff_codec_get_id(const AVCodecTag *tags, unsigned int tag)
{
	int i;
	for(i=0; tags[i].id != CODEC_ID_NONE;i++) {
		if(tag == tags[i].tag)
			return tags[i].id;
	}
	for(i=0; tags[i].id != CODEC_ID_NONE; i++) {
		if (ff_toupper4(tag) == ff_toupper4(tags[i].tag))
			return tags[i].id;
	}

	return  CODEC_ID_NONE;
}

int matroska_reader_t::init_put_byte(ByteIOContext *s,
		unsigned char *buffer,
		int buffer_size,
		int write_flag,
		void *opaque,
		size_t (*read_packet)(void *opaque, void *buf, size_t buf_size,int fullread),
		int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
		int64_t (*seek)(void *opaque, int64_t offset, int whence))
{
	s->buffer = buffer;
	s->buffer_size = buffer_size;
	s->buf_ptr = buffer;
	s->opaque = opaque;
	url_resetbuf(s, write_flag ? URL_WRONLY : URL_RDONLY);
	s->write_packet = write_packet;
	s->read_packet = read_packet;
	s->seek = seek;
	s->pos = 0;
	s->must_flush = 0;
	s->eof_reached = 0;
	s->error = 0;
	s->is_streamed = 0;
	s->max_packet_size = 0;
	s->update_checksum= NULL;
	if(!read_packet && !write_flag){
		s->pos = buffer_size;
		s->buf_end = s->buffer + buffer_size;
	}
	s->read_pause = NULL;
	s->read_seek  = NULL;
	return 0;
}

void matroska_reader_t::ff_get_wav_header(ByteIOContext *pb, AVCodecContext *codec, int size)
{

}

void matroska_reader_t::put_buffer(ByteIOContext *s, const char *buf, int size)
{
	while (size > 0) {
		int len = VXMIN(s->buf_end - s->buf_ptr, size);
		memcpy(s->buf_ptr, buf, len);
		s->buf_ptr += len;

		if (s->buf_ptr >= s->buf_end)
			flush_buffer(s);

		buf += len;
		size -= len;
	}
}

void matroska_reader_t::put_byte(ByteIOContext *s, int b)
{
	*(s->buf_ptr)++ = b;
	if (s->buf_ptr >= s->buf_end)
		flush_buffer(s);
}

void matroska_reader_t::put_le16(ByteIOContext *s, unsigned int val)
{
	put_byte(s, val);
	put_byte(s, val >> 8);
}

void matroska_reader_t::put_le32(ByteIOContext *s, unsigned int val)
{
	put_byte(s, val);
	put_byte(s, val >> 8);
	put_byte(s, val >> 16);
	put_byte(s, val >> 24);
}

unsigned int matroska_reader_t::get_be16(ByteIOContext *s)
{
	unsigned int val;
	val = get_byte(s) << 8;
	val |= get_byte(s);
	return val;
}

unsigned int matroska_reader_t::get_be32(ByteIOContext *s)
{
	unsigned int val;
	val = get_be16(s) << 16;
	val |= get_be16(s);
	return val;
}

uint64_t matroska_reader_t::get_be64(ByteIOContext *s)
{
	uint64_t val;
	val = (uint64_t)get_be32(s) << 32;
	val |= (uint64_t)get_be32(s);
	return val;
}

int matroska_reader_t::url_fskip(ByteIOContext *s, int64_t offset)
{
	int64_t ret = url_fseek(s, offset, SEEK_CUR);
	return ret < 0 ? ret : 0;
}

void matroska_reader_t::flush_buffer(ByteIOContext *s)
{
	if (s->buf_ptr > s->buffer) {
		if (s->write_packet && !s->error){
			int ret= s->write_packet(s->opaque, s->buffer, s->buf_ptr - s->buffer);
			if(ret < 0){
				s->error = ret;
			}
		}
		if(s->update_checksum){
			s->checksum= s->update_checksum(s->checksum, s->checksum_ptr, s->buf_ptr - s->checksum_ptr);
			s->checksum_ptr= s->buffer;
		}
		s->pos += s->buf_ptr - s->buffer;
	}
	s->buf_ptr = s->buffer;
}

int64_t matroska_reader_t::url_ftell(ByteIOContext *s)
{
	return url_fseek(s, 0, SEEK_CUR);
}

int matroska_reader_t::url_feof(ByteIOContext *s)
{
	if(!s)
		return 0;
	return s->eof_reached;
}

int matroska_reader_t::url_ferror(ByteIOContext *s)
{
	if(!s)
		return 0;
	return s->error;
}

int matroska_reader_t::url_is_streamed(ByteIOContext *s)
{
	return s->is_streamed;
}

void matroska_reader_t::fill_buffer(ByteIOContext *s)
{
	uint8_t *dst= !s->max_packet_size && s->buf_end - s->buffer < s->buffer_size ? s->buf_ptr : s->buffer;
	int len= s->buffer_size - (dst - s->buffer);
	int max_buffer_size = s->max_packet_size ? s->max_packet_size : IO_BUFFER_SIZE;

	/* no need to do anything if EOF already reached */
	if (s->eof_reached)
	{
		printf("the s->eof_reached is seted true\n");
		return;
	}

	if(s->update_checksum && dst == s->buffer){
		if(s->buf_end > s->checksum_ptr)
			s->checksum= s->update_checksum(s->checksum, s->checksum_ptr, s->buf_end - s->checksum_ptr);
		s->checksum_ptr= s->buffer;
	}

	/* make buffer smaller in case it ended up large after probing */
	if (s->buffer_size > max_buffer_size) {
		url_setbufsize(s, max_buffer_size);

		s->checksum_ptr = dst = s->buffer;
		len = s->buffer_size;
	}

	if(s->read_packet)
		len = s->read_packet(s->opaque, dst, len, 1);
	else
		len = 0;
	if (len <= 0) {
		/* do not modify buffer if EOF reached so that a seek back can
	           be done without rereading data */
		s->eof_reached = 1;
		if(len<0)
			s->error= len;
	} else {
		s->pos += len;
		printf("fill_buffer::pos=%d\n",s->pos);
		s->buf_ptr = dst;
		s->buf_end = dst + len;
	}
}


int64_t matroska_reader_t::url_fseek(ByteIOContext *s, int64_t offset, int whence)
{
	int64_t offset1;
	int64_t pos;
	int force = whence & AVSEEK_FORCE;
	whence &= ~AVSEEK_FORCE;

	if(!s)
		return -1;

	pos = s->pos - (s->write_flag ? 0 : (s->buf_end - s->buffer));

	if (whence != SEEK_CUR && whence != SEEK_SET)
		return -1;

	if (whence == SEEK_CUR) {
		offset1 = pos + (s->buf_ptr - s->buffer);
		if (offset == 0)
			return offset1;
		offset += offset1;
	}
	offset1 = offset - pos;
	if (!s->must_flush &&
			offset1 >= 0 && offset1 <= (s->buf_end - s->buffer)) {
		/* can do the seek inside the buffer */
		s->buf_ptr = s->buffer + offset1;
	} else if ((s->is_streamed ||
			offset1 <= s->buf_end + SHORT_SEEK_THRESHOLD - s->buffer) &&
			!s->write_flag && offset1 >= 0 &&
			(whence != SEEK_END || force)) {
		while(s->pos < offset && !s->eof_reached)
			fill_buffer(s);
		if (s->eof_reached)
			return -1;
		s->buf_ptr = s->buf_end + offset - s->pos;
	} else {
		int64_t res;
		if (!s->seek)
			return -1;
		if ((res = s->seek(s->opaque, offset, SEEK_SET)) < 0)
			return res;
		if (!s->write_flag)
			s->buf_end = s->buffer;
		s->buf_ptr = s->buffer;
		s->pos = offset;
	}
	s->eof_reached = 0;
	return offset;
}

int matroska_reader_t::url_resetbuf(ByteIOContext *s, int flags)
{
	if (flags & URL_WRONLY) {
		s->buf_end = s->buffer + s->buffer_size;
		s->write_flag = 1;
	} else {
		s->buf_end = s->buffer;
		s->write_flag = 0;
	}
	return 0;
}

int matroska_reader_t::get_byte(ByteIOContext *s)
{
	if (s->buf_ptr >= s->buf_end)
		fill_buffer(s);
	if (s->buf_ptr < s->buf_end)
		return *s->buf_ptr++;
	return 0;
}

int matroska_reader_t::get_buffer(ByteIOContext *s, unsigned char *buf, int size)
{
	int len, size1;

	size1 = size;
	while (size > 0) {
		len = s->buf_end - s->buf_ptr;
		if (len > size)
			len = size;
		if (len == 0) {
			if(size > s->buffer_size && !s->update_checksum){
				if(s->read_packet)
					len = s->read_packet(s->opaque, buf, size, 0);
				if (len <= 0) {
					/* do not modify buffer if EOF reached so that a seek back can
	                    be done without rereading data */
					s->eof_reached = 1;
					if(len<0)
						s->error= len;
					break;
				} else {
					s->pos += len;
					size -= len;
					buf += len;
					s->buf_ptr = s->buffer;
					s->buf_end = s->buffer/* + len*/;
				}
			}else{
				fill_buffer(s);
				len = s->buf_end - s->buf_ptr;
				if (len == 0)
					break;
			}
		} else {
			memcpy(buf, s->buf_ptr, len);
			buf += len;
			s->buf_ptr += len;
			size -= len;
		}
	}
	if (size1 == size) {
		if (url_ferror(s)) return url_ferror(s);
		if (url_feof(s))   return -1;
	}
	return size1 - size;
}

int matroska_reader_t::url_setbufsize(ByteIOContext *s, int buf_size)
{
	uint8_t *buffer;
	buffer = (uint8_t *)av_malloc(buf_size);
	if (!buffer)
		return -1;

	av_free(s->buffer);
	s->buffer = buffer;
	s->buffer_size = buf_size;
	s->buf_ptr = buffer;
	url_resetbuf(s, s->write_flag ? URL_WRONLY : URL_RDONLY);
	return 0;
}

int matroska_reader_t::matroska_aac_profile(char *codec_id)
{
	static const char * const aac_profiles[] = { "MAIN", "LC", "SSR" };
	int profile;

	for (profile=0; profile<array_size(aac_profiles); profile++)
		if (strstr(codec_id, aac_profiles[profile]))
			break;
	return profile + 1;
}

int matroska_reader_t::matroska_aac_sri(int samplerate)
{
	int sri;

	for (sri=0; sri<array_size(ff_mpeg4audio_sample_rates); sri++)
		if (ff_mpeg4audio_sample_rates[sri] == samplerate)
			break;
	return sri;
}

#ifdef INBUF_PADDED
#define GETB(c) (*(c).in++)
#else
#define GETB(c) get_byte(&(c))
#endif

void copy(LZOContext *c, int cnt) {
	register const uint8_t *src = c->in;
	register uint8_t *dst = c->out;
	if (cnt > c->in_end - src) {
		cnt = VXMAX(c->in_end - src, 0);
		c->error |= AV_LZO_INPUT_DEPLETED;
	}
	if (cnt > c->out_end - dst) {
		cnt = VXMAX(c->out_end - dst, 0);
		c->error |= AV_LZO_OUTPUT_FULL;
	}
#if defined(INBUF_PADDED) && defined(OUTBUF_PADDED)
	COPY4(dst, src);
	src += 4;
	dst += 4;
	cnt -= 4;
	if (cnt > 0)
#endif
		memcpy(dst, src, cnt);
	c->in = src + cnt;
	c->out = dst + cnt;
}

int get_byte(LZOContext *c) {
	if (c->in < c->in_end)
		return *c->in++;
	c->error |= AV_LZO_INPUT_DEPLETED;
	return 1;
}

int get_len(LZOContext *c, int x, int mask) {
	int cnt = x & mask;
	if (!cnt) {
		while (!(x = get_byte(c))) cnt += 255;
		cnt += mask + x;
	}
	return cnt;
}

void memcpy_backptr(uint8_t *dst, int back, int cnt) {
	const uint8_t *src = &dst[-back];
	if (back == 1) {
		memset(dst, *src, cnt);
	} else {
#ifdef OUTBUF_PADDED
		COPY2(dst, src);
		COPY2(dst + 2, src + 2);
		src += 4;
		dst += 4;
		cnt -= 4;
		if (cnt > 0) {
			COPY2(dst, src);
			COPY2(dst + 2, src + 2);
			COPY2(dst + 4, src + 4);
			COPY2(dst + 6, src + 6);
			src += 8;
			dst += 8;
			cnt -= 8;
		}
#endif
		if (cnt > 0) {
			int blocklen = back;
			while (cnt > blocklen) {
				memcpy(dst, src, blocklen);
				dst += blocklen;
				cnt -= blocklen;
				blocklen <<= 1;
			}
			memcpy(dst, src, cnt);
		}
	}
}



void copy_backptr(LZOContext *c, int back, int cnt) {
	register const uint8_t *src = &c->out[-back];
	register uint8_t *dst = c->out;
	if (src < c->out_start || src > dst) {
		c->error |= AV_LZO_INVALID_BACKPTR;
		return;
	}
	if (cnt > c->out_end - dst) {
		cnt = VXMAX(c->out_end - dst, 0);
		c->error |= AV_LZO_OUTPUT_FULL;
	}
	memcpy_backptr(dst, back, cnt);
	c->out = dst + cnt;
}


int matroska_reader_t::av_lzo1x_decode(void *out, int *outlen, const void *in, int *inlen) {
	int state= 0;
	int x;
	LZOContext c;
	c.in = (uint8_t*)in;
	c.in_end = (const uint8_t *)in + *inlen;
	c.out = c.out_start = (uint8_t*)out;
	c.out_end = (uint8_t *)out + * outlen;
	c.error = 0;
	x = GETB(c);
	if (x > 17) {
		copy(&c, x - 17);
		x = GETB(c);
		if (x < 16) c.error |= AV_LZO_ERROR;
	}
	if (c.in > c.in_end)
		c.error |= AV_LZO_INPUT_DEPLETED;
	while (!c.error) {
		int cnt, back;
		if (x > 15) {
			if (x > 63) {
				cnt = (x >> 5) - 1;
				back = (GETB(c) << 3) + ((x >> 2) & 7) + 1;
			} else if (x > 31) {
				cnt = get_len(&c, x, 31);
				x = GETB(c);
				back = (GETB(c) << 6) + (x >> 2) + 1;
			} else {
				cnt = get_len(&c, x, 7);
				back = (1 << 14) + ((x & 8) << 11);
				x = GETB(c);
				back += (GETB(c) << 6) + (x >> 2);
				if (back == (1 << 14)) {
					if (cnt != 1)
						c.error |= AV_LZO_ERROR;
					break;
				}
			}
		} else if(!state){
			cnt = get_len(&c, x, 15);
			copy(&c, cnt + 3);
			x = GETB(c);
			if (x > 15)
				continue;
			cnt = 1;
			back = (1 << 11) + (GETB(c) << 2) + (x >> 2) + 1;
		} else {
			cnt = 0;
			back = (GETB(c) << 2) + (x >> 2) + 1;
		}
		copy_backptr(&c, back, cnt + 2);
		state=
				cnt = x & 3;
		copy(&c, cnt);
		x = GETB(c);
	}
	*inlen = c.in_end - c.in;
	if (c.in > c.in_end)
		*inlen = 0;
	*outlen = c.out_end - c.out;
	return c.error;
}

int matroska_reader_t::matroska_decode_buffer(uint8_t** buf, int* buf_size,MatroskaTrack *track)
{
	MatroskaTrackEncoding *encodings = (MatroskaTrackEncoding *)track->encodings.elem;
	uint8_t* data = *buf;
	int isize = *buf_size;
	uint8_t* pkt_data = NULL;
	int pkt_size = isize;
	int result = 0;
	int olen;

	if (pkt_size >= 10000000)
		return -1;

	switch (encodings[0].compression.algo) {
	case MATROSKA_TRACK_ENCODING_COMP_HEADERSTRIP:
		return encodings[0].compression.settings.size;
	case MATROSKA_TRACK_ENCODING_COMP_LZO:
		do {
			olen = pkt_size *= 3;
			pkt_data = (uint8_t*)av_realloc(pkt_data, pkt_size+AV_LZO_OUTPUT_PADDING);
			result = av_lzo1x_decode(pkt_data, &olen, data, &isize);
		} while (result==AV_LZO_OUTPUT_FULL && pkt_size<10000000);
		if (result)
			goto failed;
		pkt_size -= olen;
		break;
	default:
		return -1;
	}

	*buf = pkt_data;
	*buf_size = pkt_size;
	return 0;
	failed:
	av_free(pkt_data);
	return -1;
}

void *matroska_reader_t::av_malloc(unsigned int size)
{
	void *ptr = NULL;

	ptr = malloc(size);
	return ptr;
}

void *matroska_reader_t::av_realloc(void *ptr, unsigned int  size)
{

	return realloc(ptr, size);

}

void matroska_reader_t::av_free(void *ptr)
{
	if (ptr)
	{
		free(ptr);
		ptr = NULL;
	}
}

void matroska_reader_t::av_freep(void *arg)
{
	void **ptr= (void**)arg;
	av_free(*ptr);
	*ptr = NULL;
}

void *matroska_reader_t::av_mallocz(unsigned int  size)
{
	void *ptr = av_malloc(size);
	if (ptr)
		memset(ptr, 0, size);
	return ptr;
}

char *matroska_reader_t::av_strdup(const char *s)
{
	char *ptr= NULL;
	if(s){
		int len = strlen(s) + 1;
		ptr = (char *)av_malloc(len);
		if (ptr)
			memcpy(ptr, s, len);
	}
	return ptr;
}



}


//#endif // ENABLE_MKV_SUPPORT
