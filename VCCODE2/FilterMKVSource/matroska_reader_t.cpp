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


#define DEBUG_PRINT			printf
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
#define ME_BUFFER_SIZE			0x100000
#define FF_INPUT_BUFFER_PADDING_SIZE 8
#define AUDIO_AVC_HEADER_SIZE	7


#define AVSEEK_FLAG_BACKWARD 1 ///< seek backward
#define AVSEEK_FLAG_BYTE     2 ///< seeking based on position in bytes
#define AVSEEK_FLAG_ANY      4 ///< seek to any frame, even non-keyframes
#define AVSEEK_FLAG_FRAME    8 ///< seeking based on frame number

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
		{"A_MPEG/L3"        , CODEC_ID_MP3			,ES_STREAM_TYPE_MPEG1_AUDIO},
		{"A_PCM/FLOAT/IEEE" , CODEC_ID_PCM_F32LE	,ES_STREAM_TYPE_LPCM_AUDIO},
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


///////////////////////////////////////////////////////////
//some functions
double av_int2dbl(int64_t v){
// 	if(v+v > 0xFFELL<<52)
// 		return 0.0;
//	return ldexp(((v&((1LL<<52)-1)) + (1LL<<52)) * (v>>63|1), (v>>52&0x7FF)-1075);
	return 1.0;
}

float av_int2flt(int32_t v){
//	if(v+v > 0xFF000000U)
//		return 0.0/0.0;
//	return ldexp(((v&0x7FFFFF) + (1<<23)) * (v>>31|1), (v>>23&0xFF)-150);
	return 0.0;
}

int detect(	source_access_callbacks_t *source)			//return 0:detect failed;other value is seccessful
{
	uint64_t total = 0;
	int len_mask = 0x80;
	int size = 1;
	int n = 1;
	int i = 0;
	int res = 0;
	const char *matroska_doctypes[] = { "matroska", "webm" };

	uint8_t* buffer = (uint8_t*)malloc(1024);
	if(buffer == NULL){
		DEBUG_PRINT("detect malloc fialed\n");
		res = 0;
		goto exit;
	}
	source->m_Read(source->m_pHandle,buffer,array_size(buffer),1);
	source->m_Seek(source->m_pHandle,0,SEEK_SET);

	/* EBML header? */
	if (AV_RB32(buffer) != EBML_ID_HEADER){
		res = 0;
		goto exit;
	}
	total = buffer[4];
	while (size <= 8 && !(total & len_mask)) {
		size++;
		len_mask >>= 1;
	}
	if (size > 8){
		res = 0;
		goto exit;
	}
	total &= (len_mask - 1);
	while (n < size)
		total = (total << 8) | buffer[4 + n++];

	for (i = 0; i < array_size(matroska_doctypes); i++) {
		int probelen = strlen(matroska_doctypes[i]);
		for (n = 4+size; n <= 4+size+total-probelen; n++){
			if (!memcmp(buffer+n, matroska_doctypes[i], probelen)){
				res = 1;
				goto exit;
			}
		}
	}

exit:
	if(buffer){
		free(buffer);
		buffer = NULL;
	}
	return res;
}


matroska_reader_t::matroska_reader_t()
{
	DEBUG_PRINT(">entry matroska_reader_t::matroska_reader_t\n");
	memset(&m_matroska,0,sizeof(MatroskaDemuxContext));
	m_matroska.ctx = &m_fcontext;
	m_matroska.ctx->pb = &m_b;

	m_buffer = NULL;
	m_scanSpeed.speed = 0;
	m_scanSpeed.isTricking = 0;
	m_curLaceIndex = 0;
	this->videoNum = -1;
	this->audioNum = -1;
	this->m_lenOfNalPaketSize = 0;
	this->m_trickStartPosition = 0;
	m_type = -1;
	m_pdata = NULL;
	m_pLaceSize = NULL;
	m_laces = 0;
	m_lastKeyPosition = 0;
	m_timecode = 0;
	DEBUG_PRINT("<leave matroska_reader_t::matroska_reader_t\n");
}

matroska_reader_t::~matroska_reader_t()
{
	DEBUG_PRINT("coming matroska_reader_t::~matroska_reader_t\n");
	av_free(m_b.buffer);
	av_free(m_buffer);
	DEBUG_PRINT("coming matroska_reader_t::~matroska_reader_t1\n");
	avFree((void**)&m_pLaceSize);
	DEBUG_PRINT("coming matroska_reader_t::~matroska_reader_t2\n");

	this->matroska_release_tags(m_matroska.tags);
	this->matroska_release_seekhead(m_matroska.seekhead);
	this->matroska_release_attachments(m_matroska.attachments);
	this->matroska_release_chapters(m_matroska.chapters);
	this->matroska_release_index(m_matroska.index);
	this->matroska_release_tracks(m_matroska.tracks);

	DEBUG_PRINT("coming matroska_reader_t::~matroska_reader_t2\n");
	av_free(m_matroska.title);
	DEBUG_PRINT("coming matroska_reader_t::~matroska_reader_t leave\n");
}

int matroska_reader_t::set_scan_speed(int speed)
{
	int result = speed / 100;

	printf("%s speed=%d\n",__FILE__,speed);

	m_trickStartPosition = this->url_ftell(&m_b);
	m_curLaceIndex = 0;

	if(result == 2)	//ff
	{
		this->m_scanSpeed.speed = 0;
		this->m_scanSpeed.isTricking = 1;
	}
	if(result > 2)	//ff
	{
		this->m_scanSpeed.speed = result;
		this->m_scanSpeed.isTricking = 1;
	}
	if(result < 0) //rew
	{
		this->m_scanSpeed.speed = result;
		this->m_scanSpeed.isTricking = 1;
	}
	if(result == 1)	//normal
	{
		//if(m_scanSpeed.isTricking)
		{
			if(m_scanSpeed.speed == 0){
				if( this->findNextIFrame(m_curDataPosition) < 0) {
					return -1;
				}
				DEBUG_PRINT("the find next i frame position=%lld\n",m_curDataPosition);
			}
			this->url_fseek(&m_b,m_curDataPosition,SEEK_SET);
		}
		this->m_scanSpeed.speed = 0;
		this->m_scanSpeed.isTricking = 0;
	}

	return 0;
}

int matroska_reader_t::get_scan_speed()
{
	return this->m_scanSpeed.speed;
}


void matroska_reader_t::matroska_release_tag(ebml_list& _tag)
{
	MatroskaTag* tag = (MatroskaTag*)_tag.elem;
	for(int j=0;j<_tag.nb_elem;j++)
	{
		av_free(tag[j].lang);
		av_free(tag[j].name);
		av_free(tag[j].string);
		if(tag[j].sub.nb_elem > 0)
		{
			matroska_release_tag(tag[j].sub);
		}
	}
	av_free(_tag.elem);
}

void matroska_reader_t::matroska_release_tags(ebml_list& _tags)
{
	MatroskaTags* tags = (MatroskaTags*)_tags.elem;
	for(int i=0;i<_tags.nb_elem;i++)
	{
		this->matroska_release_tag(tags[i].tag);
		av_free(tags[i].target.type);
	}
	av_free(_tags.elem);
}
void matroska_reader_t::matroska_release_seekhead(ebml_list& seekhead)
{
	av_free(m_matroska.seekhead.elem);
}
void matroska_reader_t::matroska_release_attachments(ebml_list& _attachments)
{
	MatroskaAttachement* attachment=(MatroskaAttachement*)_attachments.elem;
	for(int i=0;i<_attachments.nb_elem;i++)
	{
		av_free(attachment[i].filename);
		av_free(attachment[i].mime);
		av_free(attachment[i].bin.data);
	}
	av_free(_attachments.elem);
}
void matroska_reader_t::matroska_release_chapters(ebml_list& _chapters)
{
	MatroskaChapter *chapter = (MatroskaChapter *)_chapters.elem;
	for(int i=0;i<_chapters.nb_elem;i++)
	{
		av_free(chapter[i].title);
	}
	av_free(_chapters.elem);
}
void matroska_reader_t::matroska_release_index(ebml_list& _index)
{
	MatroskaIndex* index = (MatroskaIndex*)_index.elem;
	for(int i=0;i<_index.nb_elem;i++)
	{
		av_free(index[i].pos.elem);			//There is not pointer in MatroskaIndexPos
	}
	av_free(_index.elem);
}
void matroska_reader_t::matroska_release_tracks(ebml_list& _tracks)
{
	MatroskaTrack* track = (MatroskaTrack*)_tracks.elem;
	for(int i=0;i<_tracks.nb_elem;i++)
	{
		av_free(track[i].codec_id);
		av_free(track[i].codec_priv.data);
		MatroskaTrackEncoding* encoding = (MatroskaTrackEncoding*)track[i].encodings.elem;
		for(int j=0;j<track[i].encodings.nb_elem;j++)
		{
			av_free(encoding[j].compression.settings.data);
		}
		av_free(track[i].language);
		av_free(track[i].name);
		if(track[i].stream)
		{
			av_free(track[i].stream->index_entries);
			av_free(track[i].stream);
		}
	}
	av_free(_tracks.elem);
}

int matroska_reader_t::load(source_access_callbacks_t *source)
{
	m_source = source;
	m_buffer = (uint8_t*)av_malloc(1024*1024*1);
	printf("m_buffer = 0x%X\n",m_buffer);
	if(m_buffer == NULL)
	{
		DEBUG_PRINT("m_buffer malloc m_buffer failed\n");
		return -1;
	}
	DEBUG_PRINT("entry matroska_reader_t::init\n");
	if(source)
	{
		if(0 != init_put_byte(&m_b,NULL,//(uint8_t*)m_buffer,
				ME_BUFFER_SIZE,
				URL_RDONLY,
				source->m_pHandle,
				source->m_Read,
				NULL,
				source->m_Seek))
		{
			DEBUG_PRINT("matroska_reader_t::matroska_reader_t failed\n");
			return -1;
		}
		DEBUG_PRINT("init::file position is %d\n",url_ftell(&m_b));
		m_b.tell = source->m_Tell;
		//****source->m_Read(source->m_pHandle,m_buffer,ME_BUFFER_SIZE,0);

		if(m_b.is_streamed == 0)
		{
			m_b.seek(m_b.opaque,0,SEEK_END);
			m_matroska.fileLen = m_b.tell(m_b.opaque);
			m_b.seek(m_b.opaque,0,SEEK_SET);
		}
		if(this->matroska_read_header()<0)
		{
			DEBUG_PRINT("Read the matroska head failed\n");
			return -1;
		}
		DEBUG_PRINT("=============init::file position is %d\n",url_ftell(&m_b));
	}
	return 0;
}

int matroska_reader_t::get_video_parameter(video_parameter_t *vpara, int vid )
{
	if(vpara)
	{
		memset(vpara,0,sizeof(video_parameter_t));
		MatroskaTrack *tracks = (MatroskaTrack *)m_matroska.tracks.elem;
		MatroskaTrack *track = matroska_find_track_by_num(&m_matroska,this->videoNum);
		if(track)
		{
			if(track->type == MATROSKA_TRACK_TYPE_VIDEO)
			{
				MatroskaTrackVideo* video = &track->video;

				es_stream_type_t es_type = ES_STREAM_TYPE_UNKNOWN;
				enum CodecID  codec_id = CODEC_ID_NONE;
				DEBUG_PRINT("vidoe codec_id=%s\n",track->codec_id);

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

				//vpara->frame_rate = convert_frame_rate(30);
				vpara->frame_rate.n = 30000;
				vpara->frame_rate.d = 1001;
				vpara->strm_id = 2;
				vpara->strm_type = es_type;
				vpara->bit_rate = 2742000;
				vpara->bit_rate = 0;
				vpara->h_size = track->video.pixel_width;
				vpara->v_size = track->video.pixel_height;

				return 0;
			}
		}

	}
	return -1;
}

int matroska_reader_t::get_audio_parameter(audio_parameter_t *apara, int vid )
{
	if(apara)
	{
		memset(apara,0,sizeof(audio_parameter_t));
		MatroskaTrack *track = matroska_find_track_by_num(&m_matroska,this->audioNum);
		if(track)
		{
			if(track->type == MATROSKA_TRACK_TYPE_AUDIO)
			{
				MatroskaTrackAudio* audio = &track->audio;

				DEBUG_PRINT("audio_id=%s\n",track->codec_id);

				es_stream_type_t es_type = ES_STREAM_TYPE_UNKNOWN;
				enum CodecID  codec_id = CODEC_ID_NONE;
				DEBUG_PRINT("audio codec_id=%s\n",track->codec_id);
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

				apara->channels = audio->channels;
				apara->sample_freq = audio->out_samplerate;
				DEBUG_PRINT("apara->sample_freq=%f  audio->out_samplerate=%f\n",audio->samplerate,audio->out_samplerate);
				apara->strm_type = es_type;
				apara->bitrate = 384000;
				apara->bitrate = 0;

				//this->print_audio_parameters(apara);

				return 0;
			}
		}
	}
	return -1;
}

int matroska_reader_t::init_audio_parameter()
{
	MatroskaTrack *tracks = (MatroskaTrack *)m_matroska.tracks.elem;
	MatroskaTrack *track = NULL;
	for (int i=0; i < m_matroska.tracks.nb_elem; i++)
	{
		track = &tracks[i];
		if(track->type == MATROSKA_TRACK_TYPE_AUDIO)
		{
			MatroskaTrackAudio* audio = &track->audio;

			DEBUG_PRINT("audio_id=%s\n",track->codec_id);

			es_stream_type_t es_type = ES_STREAM_TYPE_UNKNOWN;
			enum CodecID  codec_id = CODEC_ID_NONE;
			DEBUG_PRINT("audio codec_id=%s\n",track->codec_id);
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
			track->enumCodecID = codec_id;
			if(this->audioNum == -1){
				this->audioNum = (int)track->num;
			}

			///////build the adts header
			int tempBufSize = 100;
			track->paketHeader.m_Size = 0;
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
				track->paketHeader.append(bufferTemp,AUDIO_AVC_HEADER_SIZE);
				break;
			default:
				break;
			}
			av_free(bufferTemp);


			EbmlList *encodings_list = &track->encodings;
			MatroskaTrackEncoding *encodings = (MatroskaTrackEncoding *)encodings_list->elem;
			if (encodings_list->nb_elem > 1) {
				DEBUG_PRINT("Multiple combined encodings no supported");
			} else if (encodings_list->nb_elem == 1) {
				if (encodings[0].compression.algo == MATROSKA_TRACK_ENCODING_COMP_HEADERSTRIP)
				{
					int encodingSize = encodings[0].compression.settings.size;
					track->encodingInfo.append(encodings[0].compression.settings.data,encodingSize);
					track->encodingInfo.type = MATROSKA_TRACK_ENCODING_COMP_HEADERSTRIP;
				} else{
					encodings[0].scope = 0;
					DEBUG_PRINT("Unsupported encoding type");
				}
			}
		}
	}
	return 0;
}

int	matroska_reader_t::init_video_parameter()
{
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
			track->enumCodecID = codec_id;
			if( this->videoNum == -1){
				this->videoNum = (int)track->num;
			}

			EbmlList *encodings_list = &track->encodings;
			MatroskaTrackEncoding *encodings = (MatroskaTrackEncoding *)encodings_list->elem;
			if (encodings_list->nb_elem > 1)
			{
				DEBUG_PRINT("Multiple combined encodings no supported");
			} else if (encodings_list->nb_elem == 1) {
				if (encodings[0].compression.algo == MATROSKA_TRACK_ENCODING_COMP_HEADERSTRIP)
				{
					int encodingSize = encodings[0].compression.settings.size;
					track->encodingInfo.append(encodings[0].compression.settings.data,encodingSize);
					track->encodingInfo.type = MATROSKA_TRACK_ENCODING_COMP_HEADERSTRIP;
				} else{
					encodings[0].scope = 0;
					DEBUG_PRINT("Unsupported encoding type");
				}
			}

			uint8_t* pdata = track->codec_priv.data;
			int count = 0;
			switch(codec_id)
			{
			case CODEC_ID_H264:
				pdata++;// Reserved - 8bits
				pdata++;// Profile - 8bits
				pdata++;// Reserved - 8bits
				pdata++;// Level - 8bits
				this->m_lenOfNalPaketSize = ((*pdata++) & 0x03) + 1; // Reserved - 6bits; Size of NALU length minus 1 - 2bits
				count = (*pdata++) & 0x1F;// Reserved - 3bits; Number of Sequence Parameter Sets - 5bits
				while (count -- )
				{
					unsigned int t;
					t = (pdata[0] << 8) | pdata[1]; pdata += 2; // Sequence Parameter Sets, each is prefixed with a two byte big-endian size field
					DEBUG_PRINT("\t\tSPS Len = %d\n", t);
					track->paketHeader.append(avc_start_code, sizeof(avc_start_code));
					track->paketHeader.append(pdata, t);
					pdata += t;
				}
				// Read PPS
				count = *pdata++; // Number of Picture Parameter Sets - 8bits
				while (count -- )
				{
					unsigned int t;
					t = (pdata[0] << 8) | pdata[1]; pdata += 2; // Picture Parameter Sets, each is prefixed with a two byte big-endian size field
					DEBUG_PRINT("\t\tPPS Len = %d\n", t);
					track->paketHeader.append(avc_start_code, sizeof(avc_start_code));
					track->paketHeader.append(pdata, t);
					pdata += t;
				}
			default:
				break;
			}
		}
	}
	return 0;
}

int matroska_reader_t::parseNum(MatroskaDemuxContext *matroska,int max_size,uint64_t* number)
{
	uint64_t total = 0;
	total = this->get_byte(&m_b);
	int read = 8 - ff_log2_tab[total];
	if (read > max_size) {
		return -1;
	}
	total ^= 1 << ff_log2_tab[total];
	int n = 1;
	while (n++ < read)
		total = (total << 8) | get_byte(&m_b);
	*number = total;
	return read;
}

uint64_t matroska_reader_t::parseID(MatroskaDemuxContext *matroska)
{
	uint64_t id = 0;
	int res = this->parseNum(matroska,4,&id);
	if(id == (uint64_t)-1)
	{
		DEBUG_PRINT("the parse id error\n");
	}
	id = id | 1 << 7*res;
	return id;
}

int matroska_reader_t::readUint(MatroskaDemuxContext *matroska, int size, uint64_t *num)
{
	int n = 0;

	if (size > 8)
		return -1;

	/* big-endian ordering; build up number */
	*num = 0;
	while (n++ < size)
		*num = (*num << 8) | get_byte(matroska->ctx->pb);

	return 0;
}

int matroska_reader_t::parseLaces(uint8_t*& pdata,int& sizeInside,uint8_t flags)
{
	int n = 0 ;
	int res = 0;
	switch ((flags & 0x06) >> 1) {
	case 0x0: /* no lacing */
		m_laces = 1;
		m_pLaceSize = (int*)av_mallocz(sizeof(int));
		m_pLaceSize[0] = sizeInside;
		break;

	case 0x1: /* Xiph lacing */
	case 0x2: /* fixed-size lacing */
	case 0x3: /* EBML lacing */
		m_laces = (*pdata) + 1;
		pdata += 1;
		sizeInside -= 1;
		m_pLaceSize = (int*)av_mallocz(m_laces * sizeof(int));

		switch ((flags & 0x06) >> 1) {
		case 0x1: /* Xiph lacing */ {
			uint8_t temp;
			uint32_t total = 0;
			for (n = 0; res == 0 && n < m_laces - 1; n++) {
				while (1) {
					if (sizeInside == 0) {
						res = -1;
						break;
					}
					temp = *pdata;
					m_pLaceSize[n] += temp;
					pdata += 1;
					sizeInside -= 1;
					if (temp != 0xff)
						break;
				}
				total += m_pLaceSize[n];
			}
			m_pLaceSize[n] = sizeInside - total;
			break;
		}

		case 0x2: /* fixed-size lacing */
			for (n = 0; n < m_laces; n++)
				m_pLaceSize[n] = sizeInside / m_laces;
			break;

		case 0x3: /* EBML lacing */ {
			uint32_t total;
			uint64_t num = 0;
			n = matroska_ebmlnum_uint(&m_matroska, pdata, sizeInside, &num);
			if (n < 0) {
				DEBUG_PRINT("EBML block data error\n");
				break;
			}
			pdata += n;
			sizeInside -= n;
			total = m_pLaceSize[0] = num;
			for (n = 1; res == 0 && n < m_laces - 1; n++) {
				int64_t snum;
				int r;
				r = matroska_ebmlnum_sint(&m_matroska, pdata, sizeInside, &snum);
				if (r < 0) {
					DEBUG_PRINT("EBML block data error\n");
					break;
				}
				pdata += r;
				sizeInside -= r;
				m_pLaceSize[n] = m_pLaceSize[n - 1] + snum;
				total += m_pLaceSize[n];
			}
			m_pLaceSize[n] = sizeInside - total;
			break;
		}
		}
		break;
	}
	return 0;
}

uint32_t matroska_reader_t::handleBlockReference(uint64_t& blockReference)
{
	blockReference = 0;
	uint64_t oldPosition = this->url_ftell(&m_b);
	uint32_t id = this->parseID(&m_matroska);
	int n =0 ;
	int len = 0;
	uint64_t eleSize = 0;
	switch(id){
	case MATROSKA_ID_BLOCKREFERENCE:
		len = this->parseNum(&m_matroska,8,&eleSize);
		n = this->readUint(&m_matroska,eleSize,&blockReference);
		break;
	default:
		this->url_fseek(&m_b,oldPosition,SEEK_SET);
		break;
	}
	return 0;
}

int matroska_reader_t::readClusterData(uint8_t* buffer,uint64_t &bufferSize,OUT int64_t &position,uint64_t &blockReference)
{
	int 	 	len = 0 ;
	static int cluster_count = 0;
	uint64_t tmpID1 = 0;
	uint64_t clusterPrevsize = 0;
	uint64_t clusterPosition = 0;
	uint64_t clusteSize = 0;
	uint64_t clusterTime = 0;
	uint64_t clusterNonSample = 1;
	uint64_t blockDuration = 0;
	int 	 n = 0;

	position = 0;

	while(true){
		m_matroska.current_id = this->parseID(&m_matroska);
		switch(m_matroska.current_id){
		case MATROSKA_ID_CLUSTER:
			cluster_count ++;
			DEBUG_PRINT("cluster_count=%d \n",cluster_count);
			if(cluster_count == 8)
				int nstop = 0;

			memset(&m_matroska.current_cluster,0,sizeof(MatroskaCluster));
			len = this->parseNum(&m_matroska,8,&clusteSize);
			m_matroska.current_cluster.clusteSize = clusteSize;
			break;
		case MATROSKA_ID_INFO:
		case MATROSKA_ID_CUES:
		case MATROSKA_ID_TAGS:
		case MATROSKA_ID_SEEKHEAD:
			len = this->parseNum(&m_matroska,8,&bufferSize);
			this->url_fseek(&m_b,bufferSize,SEEK_CUR);
			break;
		case MATROSKA_ID_CLUSTERTIMECODE:
			len = this->parseNum(&m_matroska,8,&bufferSize);
			n = this->readUint(&m_matroska,bufferSize,&clusterTime);
			m_matroska.current_cluster.clusterTime = clusterTime;
			break;

		case MATROSKA_ID_CLUSTERPOSITION:
			len = this->parseNum(&m_matroska,8,&bufferSize);
			n = this->readUint(&m_matroska,bufferSize,&clusterPosition);
			m_matroska.current_cluster.clusterPosition = clusterPosition;
			break;
		case MATROSKA_ID_CLUSTERPREVSIZE:
			len = this->parseNum(&m_matroska,8,&bufferSize);
			n = this->readUint(&m_matroska,bufferSize,&clusterPrevsize);
			m_matroska.current_cluster.clusterPrevsize = clusterPrevsize;
			break;
		case MATROSKA_ID_BLOCKGROUP:
			position = this->url_ftell(&m_b) - 1;
			m_matroska.current_cluster.clusterNonSample = 1;
			len = this->parseNum(&m_matroska,8,&bufferSize);
			if(bufferSize == 15990 - 3)
				int nstop = 0;
			tmpID1 = this->parseID(&m_matroska);
			switch(tmpID1){
			case MATROSKA_ID_BLOCK:
				len = this->parseNum(&m_matroska,8,&bufferSize);
				this->get_buffer(&m_b,buffer,bufferSize);
				this->handleBlockReference(blockReference);
				return 1;
				break;
			default:
				break;
			}
			break;
		case MATROSKA_ID_SIMPLEBLOCK:
			position = this->url_ftell(&m_b) - 1;
			m_matroska.current_cluster.clusterNonSample = 0;
			len = this->parseNum(&m_matroska,8,&bufferSize);
			this->get_buffer(&m_b,buffer,bufferSize);
			return 1;
			break;
		case MATROSKA_ID_BLOCKDURATION:
			len = this->parseNum(&m_matroska,8,&bufferSize);
			n = this->readUint(&m_matroska,bufferSize,&blockDuration);
			break;
		case MATROSKA_ID_BLOCKREFERENCE:
			len = this->parseNum(&m_matroska,8,&bufferSize);
			n = this->readUint(&m_matroska,bufferSize,&blockReference);
			break;
		case 1:
			len = this->parseNum(&m_matroska,8,&bufferSize);
			n = this->readUint(&m_matroska,bufferSize,&clusterNonSample);
			m_matroska.current_cluster.clusterNonSample = clusterNonSample;
			break;
		default:
			DEBUG_PRINT("unknown cluster id = 0x%X \n",m_matroska.current_id);
			if(this->url_feof(&m_b))				return -1;

			int64_t position = this->url_ftell(&m_b);
			len = this->parseNum(&m_matroska,8,&bufferSize);
			if(m_scanSpeed.speed == 0 && this->jumpedIsID(&m_b,bufferSize)){
				this->url_fseek(&m_b,bufferSize,SEEK_CUR);
			}else{
				this->url_fseek(&m_b,position,SEEK_SET);
				if(this->findNextCluster(&m_b) < 0)
					return -1;
				}
			}
	}
}

int matroska_reader_t::jumpedIsID(ByteIOContext *pb,int64_t length)
{
	int idarray[] = {0xa3, 0xa1, 0xa0};
	int64_t position = this->url_ftell(pb);
	if(this->url_feof(pb))	return -1;
	if(this->url_fseek(pb,length,SEEK_CUR) < 0)
	{
		this->url_fseek(pb,position,SEEK_SET);
		return -1;
	}
#define FOUND_ID 1
#define NOT_FOUND_ID 0
	uint64_t id = this->parseID(&m_matroska);
	for(int i=0; i < array_size(idarray); i++){
		if(id == idarray[i]){
			this->url_fseek(pb,position,SEEK_SET);
			return FOUND_ID;
		}
	}

	this->url_fseek(pb,position,SEEK_SET);
	return NOT_FOUND_ID;
}

int matroska_reader_t::getClusterDataInfo(uint8_t* &pdata,int64 &pts,uint8_t &flags,int &typeInside,int &sizeInside,int &is_keyframe,uint64_t blockReference)
{
	int n = 0;
	uint64_t  timecode = 0;
	is_keyframe = m_matroska.current_cluster.clusterNonSample ? !(int)blockReference : -1;

	uint64_t num = 0;
	if ((n = matroska_ebmlnum_uint(&m_matroska, pdata, sizeInside, &num)) < 0)	{
		DEBUG_PRINT("EBML block data error\n");
		return -1;
	}


	typeInside = num;
	pdata += n;
	sizeInside -= n;

	int block_time = (int16)AV_RB16(pdata);
	timecode = m_matroska.current_cluster.clusterTime + block_time;
	m_timecode = timecode;
	pts = timecode * m_matroska.time_scale * 9 / 100000 + 20;

	pdata += 2;
	sizeInside -= 2;

	//flag byte
	flags = *pdata++;
	sizeInside -= 1;

	if(is_keyframe == -1)
		is_keyframe = flags & 0x80 ? AV_PKT_FLAG_KEY : 0;
	DEBUG_PRINT("flag	=%02X  is_keyframe=%d\n",flags,is_keyframe);

	return 0;
}

int matroska_reader_t::findNextIFrame(OUT int64_t &findPosition,int step/*=0*/)
{
	uint64_t  blockReference = 0;
	uint64_t  eleSize = 0;
	int 	  is_keyframe = 0;
	int       typeInside = -1;
	findPosition = 0;
	int64_t oldPosition = this->url_ftell(&m_b);

	while(!is_keyframe || typeInside != this->videoNum){
		if(this->readClusterData(m_buffer,eleSize,findPosition,blockReference) < 0)	{
			return -1;
		}
		int      sizeInside = eleSize;
		uint8_t* pdata = m_buffer;
		uint8_t  flags = 0;
		int64    pts = 0;

		if(this->getClusterDataInfo(pdata,pts,flags,typeInside,sizeInside,is_keyframe,blockReference) < 0 )	{
			return -1;
		}
	}
	this->url_fseek(&m_b,oldPosition,SEEK_SET);
	return 0;
}

int matroska_reader_t::readNextData(int& type,void* &buf,int &size,int64& pts,
									int& is_keyframe)
{
	if(m_curLaceIndex == 0){
	uint64_t  blockReference = 0;
	uint64_t  eleSize = 0;
		uint8_t   flags = 0;

		if(this->readClusterData(m_buffer,eleSize,m_curDataPosition,blockReference) < 0)	{
			return -1;
		}
		DEBUG_PRINT("current position=%lld   ",m_curDataPosition);
		int sizeInside = eleSize;
		m_pdata = m_buffer;

		if(this->getClusterDataInfo(m_pdata,pts,flags,m_type,sizeInside,is_keyframe,blockReference) < 0 )	{
			return -1;
		}

		//this->buildIndexList(is_keyframe,typeInside,pts);
		avFree((void**)&m_pLaceSize);
		this->parseLaces(m_pdata,sizeInside,flags);
	}

	if(m_type == this->videoNum){
		type = 0;

		if(this->m_scanSpeed.speed == 0 || is_keyframe){
			this->outputVideoData(buf,size,m_pdata,m_pLaceSize,is_keyframe);
			if(is_keyframe){
				m_lastKeyPosition = this->m_curDataPosition;
			}
		}else{
			m_curLaceIndex = 0;
			return 1;
		}
		DEBUG_PRINT("\t\t>video pts=%lld:",pts);
	}else if(m_type == this->audioNum && this->m_scanSpeed.isTricking == 0){
		type = 1;

		this->outputAudioData(buf,size,m_pdata,m_pLaceSize);
		DEBUG_PRINT("\t\t>audio pts=%lld:",pts);
	}else{
		m_curLaceIndex = 0;
		return 1;
	}
	return 0;
}


int matroska_reader_t::read_next_chunk(int & type,void * & buf,int & size,
										int64 & pts,int64 & dts,int64 & end_pts,
										int & au_start)
{
	type     = -1; 	pts     = 0;
	dts      =  0; 	end_pts = 0;
	au_start =  1;	size    = 0;

//	DEBUG_PRINT("%s coming\n",__func__);

Entry:
	int 	  is_keyframe = 0;

	if(m_scanSpeed.speed == 0){
		int res = this->readNextData(type,buf,size,pts,is_keyframe);
		if(res < 0){
			return -1;
		}else if(res > 0){
			goto Entry;
		}
	}else{
			int bitrate = m_matroska.bitrate;
		int speed = m_scanSpeed.speed;
		if(m_scanSpeed.speed < 0 && m_lastKeyPosition <= m_trickStartPosition){
			m_trickStartPosition = m_lastKeyPosition;
			}
		m_trickStartPosition += bitrate * speed;
		if(m_trickStartPosition < m_matroska.segment_start){			//back to the header of file
			return AVERROR(DEMUX_MKV_ERROR_HOF);
			}
		this->url_fseek(&m_b,m_trickStartPosition,SEEK_SET);
		if( this->findNextIFrame(m_curDataPosition) < 0) {
			return AVERROR(DEMUX_MKV_ERROR_EOF);
			}
		this->url_fseek(&m_b,m_curDataPosition,SEEK_SET);
		int res = this->readNextData(type,buf,size,pts,is_keyframe);
	if(res < 0){
			return AVERROR(DEMUX_MKV_ERROR_EOF);
	}else if(res > 0){
		goto Entry;
	}
		DEBUG_PRINT("the find next i frame position=%lld\n",m_curDataPosition);
	}

	DEBUG_PRINT("m_curLaceIndex=%d total=%d size=%d \n",m_curLaceIndex,m_laces,size);

	m_curLaceIndex ++ ;
	if(m_curLaceIndex >= m_laces){
		m_laces = 0;
		m_curLaceIndex = 0;
		DEBUG_PRINT("free lace done\n");
	}
//	DEBUG_PRINT("%s leave\n",__func__);
	return 0;
}

int matroska_reader_t::outputVideoData(void* &buf,int &size,uint8_t* &pdata,int *lace_size,int is_keyframe)
{
	uint8_t* p = NULL;
	uint8_t* pcur = NULL;
	int remainBlockSize = 0;
	uint32_t curNalSize = 0;

	int pre_lace_sum = 0;
	for(int m=0;m < m_curLaceIndex;m++)	{
		pre_lace_sum += lace_size[m];
	}
	MatroskaTrack *track = this->matroska_find_track_by_num(&m_matroska,this->videoNum);
	if(track == NULL)		return -1;

	memcpy(buf,track->paketHeader.m_pData,track->paketHeader.m_Size);
	pcur = (uint8_t*)buf + track->paketHeader.m_Size;
	int enInfo = track->encodingInfo.m_Size;
	switch(track->encodingInfo.type)
	{
	case MATROSKA_TRACK_ENCODING_COMP_HEADERSTRIP:
		memmove(pdata + enInfo,pdata,lace_size[m_curLaceIndex]);
		memcpy(pdata,track->encodingInfo.m_pData,enInfo);
	default:
		break;
	}

	switch(track->enumCodecID)
	{
	case CODEC_ID_H264:
		p = pdata;
		DEBUG_PRINT("%s %d 3_ p=%08X lace_size[m_curLaceIndex]=%d\n",__FILE__,__LINE__,p,lace_size[m_curLaceIndex]);
		remainBlockSize = lace_size[m_curLaceIndex];
		while (remainBlockSize > 0 && remainBlockSize <= lace_size[m_curLaceIndex])
		{
			curNalSize = 0;
			for(int i=0;i<m_lenOfNalPaketSize;i++)
			{
				curNalSize = curNalSize << 8 | p[i];
			}
			DEBUG_PRINT("%s %d 3_ curNalSize=%d\n",__FILE__,__LINE__,curNalSize);
			memcpy(p,avc_start_code,sizeof(avc_start_code));
			p += m_lenOfNalPaketSize;
			remainBlockSize -= m_lenOfNalPaketSize;
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

	memcpy(pcur,pdata + pre_lace_sum,lace_size[m_curLaceIndex] + track->encodingInfo.m_Size);
	size = lace_size[m_curLaceIndex] + track->paketHeader.m_Size + track->encodingInfo.m_Size;

	return 0;
}

int matroska_reader_t::outputAudioData(void* &buf,int &size,uint8_t* &pdata,int *lace_size)
{
	int pre_lace_sum = 0;
	for(int m=0;m < m_curLaceIndex;m++)	{
		pre_lace_sum += lace_size[m];
	}

	bit_stream bit;
	uint8_t* audioHeaderBuffer = NULL;
	MatroskaTrack *track = this->matroska_find_track_by_num(&m_matroska,this->audioNum);
	if(track == NULL)		return -1;
	switch(track->enumCodecID)
	{
	case CODEC_ID_AAC:
		audioHeaderBuffer = track->paketHeader.m_pData;
		bit.open(audioHeaderBuffer);
		bit.skip_bits(30);
		bit.set_bits(7 + lace_size[m_curLaceIndex],13);
		bit.set_bits(0x7FF,11);
		bit.set_bits(0,2);
		break;
	default:
		break;
	}
	memcpy(buf,track->paketHeader.m_pData,track->paketHeader.m_Size);
	uint8_t* pcur = (uint8_t*)buf + track->paketHeader.m_Size;
	switch(track->encodingInfo.type)
	{
	case MATROSKA_TRACK_ENCODING_COMP_HEADERSTRIP:
		memcpy(pcur,track->encodingInfo.m_pData,track->encodingInfo.m_Size);
		pcur += track->encodingInfo.m_Size;
	default:
		break;
	}
	memcpy( pcur,pdata + pre_lace_sum,lace_size[m_curLaceIndex]);
	size = lace_size[m_curLaceIndex] +  track->paketHeader.m_Size + track->encodingInfo.m_Size;
	return 0;
}

int matroska_reader_t::buildIndexList(int is_keyframe,int type,int64 pts)
{
	MatroskaTrack *track = matroska_find_track_by_num(&m_matroska, this->videoNum);
	if (!track || !track->stream) {
		DEBUG_PRINT("Invalid stream %lld \n", this->videoNum);
		return -1;
	}
	AVStream *st = track->stream;
	//pts = timecode * m_matroska.time_scale * 9 / 100000 + 20
	uint64_t timecode = (pts - 20) * 100000 / 9 / m_matroska.time_scale;
	if (is_keyframe && type == this->videoNum
			&& m_matroska.current_cluster.clusterPosition)
	{
		av_add_index_entry(st, m_matroska.current_cluster.clusterPosition + m_matroska.segment_start,
				timecode, 0,0,AVINDEX_KEYFRAME);
	}
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

	DEBUG_PRINT(" First read the EBML header.\n");
	if (ebml_parse(&m_matroska, ebml_syntax, &ebml)
			|| ebml.version > EBML_VERSION       || ebml.max_size > sizeof(uint64_t)
			|| ebml.id_length > sizeof(uint32_t) || ebml.doctype_version > 2) {
		DEBUG_PRINT("EBML header using unsupported features\n");
		ebml_free(ebml_syntax, &ebml);
		return -1;//failed
	}

	const char *matroska_doctypes[] = { "matroska", "webm" };
	for (i = 0; i < array_size(matroska_doctypes); i++)	{
		if (!strcmp(ebml.doctype, matroska_doctypes[i]))		{
			DEBUG_PRINT("ebml.doctype=%s\n",ebml.doctype);
			break;
		}
	}
	if (i >= array_size(matroska_doctypes))	{
		DEBUG_PRINT("Unknown EBML doctype '%s'\n", ebml.doctype);
		return -1;
	}
	ebml_free(ebml_syntax, &ebml);


	if ((res = ebml_parse(&m_matroska, matroska_segments, &m_matroska)) < 0)
		return res;
	DEBUG_PRINT("matroska_execute_seekhead\n");
	matroska_execute_seekhead(&m_matroska);

	this->url_fseek(&m_b,-4,SEEK_CUR);

	m_curDataPosition = m_matroska.segment_start;

	EbmlList *index_list1 = &m_matroska.index;
	printf("=========index_list->nb_elem = %d\n",index_list1->nb_elem);

	if (!m_matroska.time_scale)
		m_matroska.time_scale = 1000000;
	if (m_matroska.duration)
		m_matroska.ctx->duration = (int)m_matroska.duration * (int)m_matroska.time_scale * 1000 / AV_TIME_BASE;

	MatroskaTrack *tracks = (MatroskaTrack *)m_matroska.tracks.elem;
	for (i=0; i < m_matroska.tracks.nb_elem; i++) {
		MatroskaTrack *track = &tracks[i];
		int n = track->num;
		track->stream = this->av_new_stream();
	}

	res = this->init_video_parameter();
	if(res < 0)	{
		DEBUG_PRINT("init_video_parameter failed\n");
		return -1;
	}
	res = this->init_audio_parameter();
	if(res < 0)	{
		DEBUG_PRINT("init_audio_parameter failed\n");
		return -1;
	}

	m_matroska.bitrate = m_matroska.fileLen / (m_matroska.duration / 1000);
	if(m_matroska.bitrate == 0){
		m_matroska.bitrate = 500;
	}

	EbmlList *index_list = NULL;
	MatroskaIndex *index = NULL;
	int index_scale = m_matroska.time_scale;

	index_list = &m_matroska.index;
	if(index_list){
		index = (MatroskaIndex *)index_list->elem;
		if(index){
			for (i=0; i<index_list->nb_elem; i++) {
				EbmlList *pos_list = &index[i].pos;
				MatroskaIndexPos *pos = (MatroskaIndexPos *)pos_list->elem;
				for (j=0; j<pos_list->nb_elem; j++) {
					MatroskaTrack *track = matroska_find_track_by_num(&m_matroska,
							pos[j].track);
					if (track && track->stream){
						int64_t timecode = index[i].time * index_scale / AV_TIME_BASE;
						av_add_index_entry(track->stream,
								pos[j].pos + m_matroska.segment_start,
								timecode , 0, 0,
								AVINDEX_KEYFRAME);
					}
				}
			}
		}
	}

	return MKV_DEMUX_SUCCESS;
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
			DEBUG_PRINT("Read error at pos. %d (0x%d\n",pos, pos);
		}
		return 0; /* EOS or actual I/O error */
	}

	/* get the length of the EBML number */
	read = 8 - ff_log2_tab[total];
	if (read > max_size) {
		int64_t pos = url_ftell(pb) - 1;
		DEBUG_PRINT("Invalid EBML number size tag 0x%02x at pos %x (0x%x)\n",(uint8_t) total, pos, pos);
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
		DEBUG_PRINT("Unknown entry 0x%X\n", id);
	}
	return ebml_parse_elem(matroska, &syntax[i], data);
}

int matroska_reader_t::ebml_parse_elem(MatroskaDemuxContext *matroska,
		EbmlSyntax *syntax, void *data)
{
	ByteIOContext *pb = matroska->ctx->pb;
	uint32_t id = syntax->id;
	uint64_t length = 0;
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
		if ((res = ebml_read_length(matroska, pb, &length)) < 0)	//this place read the cluster size
			return res;
	}

	switch (syntax->type) {
	case EBML_UINT:  res = ebml_read_uint  (pb, length, (uint64_t*)data);  break;
	case EBML_FLOAT: res = ebml_read_float (pb, length, (double *)data);  break;
	case EBML_STR:
	case EBML_UTF8:  res = ebml_read_ascii (pb, length, (char**)data);  break;
	case EBML_BIN:   res = ebml_read_binary(pb, length, (EbmlBin *)data);  break;
	case EBML_NEST:  if ((res=ebml_read_master(matroska, length)) < 0)			//set level infomation:start position \ length
		return res;
	if (id == MATROSKA_ID_SEGMENT)
		matroska->segment_start = url_ftell(matroska->ctx->pb);
	return ebml_parse_nest(matroska, syntax->def.n, data);
	case EBML_PASS:  return ebml_parse_id(matroska, syntax->def.n, id, data);
	case EBML_STOP:  return 1;
	default:	return url_fseek(pb,length,SEEK_CUR)<0 ? -1 : 0;
	}

	return res;
}

int matroska_reader_t::ebml_read_length(MatroskaDemuxContext *matroska, ByteIOContext *pb,
		uint64_t *number)
{
	int res = ebml_read_num(matroska, pb, 8, number);
	if (res > 0 && *number + 1 == 1 << (7 * res))
		*number = -1;
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
		DEBUG_PRINT("File moves beyond max. allowed depth (%d)\n", EBML_MAX_DEPTH);
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
	*num = unum - (((uint64_t)1 << (7*res - 1)) - 1);

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

	DEBUG_PRINT("Invalid track number %d\n", num);
	return NULL;
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

void matroska_reader_t::matroska_execute_seekhead(MatroskaDemuxContext *matroska)
{
	EbmlList *seekhead_list = &matroska->seekhead;
	MatroskaSeekhead *seekhead = (MatroskaSeekhead *)seekhead_list->elem;
	uint32_t level_up = matroska->level_up;
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
		{
//			DEBUG_PRINT("%s failed\n",__func__);
			continue;
		}
//		DEBUG_PRINT("%s seek complete\n",__func__);

		/* We don't want to lose our seekhead level, so we add
		 * a dummy. This is a crude hack. */
		if (matroska->num_levels == EBML_MAX_DEPTH) {
			DEBUG_PRINT("Max EBML element depth (%d) reached, cannot parse further.\n", EBML_MAX_DEPTH);
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
		DEBUG_PRINT("the s->eof_reached is seted true\n");
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
		DEBUG_PRINT("fill_buffer::pos=%lld\n",s->pos);
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
	{
		fill_buffer(s);
	}
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
	//static const char * const aac_profiles[] = { "MAIN", "LC", "SSR" };
	static const char * const aac_profiles[] = { "AAC", "LC", "SSR" };
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

void *matroska_reader_t::av_fast_realloc(void *ptr, unsigned int *size, uint32_t min_size)
{
	if(min_size < *size)
		return ptr;

	min_size= VXMAX(17*min_size/16 + 32, min_size);

	ptr= av_realloc(ptr, min_size);
	if(!ptr) //we could set this to the unmodified min_size but this is safer if the user lost the ptr and uses NULL now
		min_size= 0;

	*size= min_size;

	return ptr;
}

#define UINT_MAX      0xffffffff    /* maximum unsigned int value */

int matroska_reader_t::av_add_index_entry(AVStream *st,
		int64_t pos, int64_t timestamp, int size, int distance, int flags)
{
	AVIndexEntry *entries, *ie;
	int index;

	if((unsigned)st->nb_index_entries + 1 >= UINT_MAX / sizeof(AVIndexEntry))
		return -1;

	entries = (AVIndexEntry *)av_fast_realloc(st->index_entries,
			&st->index_entries_allocated_size,
			(st->nb_index_entries + 1) * sizeof(AVIndexEntry));
	if(!entries)
		return -1;

	st->index_entries= entries;

	index= av_index_search_timestamp(st, timestamp, AVSEEK_FLAG_ANY);

	if(index<0){
		index= st->nb_index_entries++;
		ie= &entries[index];
	}else{
		ie= &entries[index];
		if(ie->timestamp != timestamp){
			if(ie->timestamp <= timestamp)
				return -1;
			memmove(entries + index + 1, entries + index, sizeof(AVIndexEntry)*(st->nb_index_entries - index));
			st->nb_index_entries++;
		}else if(ie->pos == pos && distance < ie->min_distance) //do not reduce the distance
		distance= ie->min_distance;
	}

	ie->pos = pos;
	ie->timestamp = timestamp;
	ie->min_distance= distance;
	ie->size= size;
	ie->flags = flags;

	return index;
}

int matroska_reader_t::av_index_search_timestamp(AVStream *st, int64_t wanted_timestamp,
		int flags)
{
	AVIndexEntry *entries= (AVIndexEntry *)st->index_entries;
	int nb_entries= st->nb_index_entries;
	int a, b, m;
	int64_t timestamp;

	a = - 1;
	b = nb_entries;

	//optimize appending index entries at the end
	if(b && entries[b-1].timestamp < wanted_timestamp)
		a= b-1;

	while (b - a > 1) {
		m = (a + b) >> 1;
		timestamp = entries[m].timestamp;
		if(timestamp >= wanted_timestamp)
			b = m;
		if(timestamp <= wanted_timestamp)
			a = m;
	}
	m= (flags & AVSEEK_FLAG_BACKWARD) ? a : b;

	if(!(flags & AVSEEK_FLAG_ANY)){
		while(m>=0 && m<nb_entries && !(entries[m].flags & AVINDEX_KEYFRAME)){
			m += (flags & AVSEEK_FLAG_BACKWARD) ? -1 : 1;
		}
	}

	if(m == nb_entries)
		return -1;
	return  m;
}

int matroska_reader_t::av_index_search_position(AVStream *st, int64_t wanted_position,
		int flags)
{
	AVIndexEntry *entries= (AVIndexEntry *)st->index_entries;
	int nb_entries= st->nb_index_entries;
	int a, b, m;
	int64_t position;

	a = - 1;
	b = nb_entries;

	//optimize appending index entries at the end
	if(b && entries[b-1].pos < wanted_position)
		a= b-1;

	while (b - a > 1) {
		m = (a + b) >> 1;
		position = entries[m].pos;
		if(position >= wanted_position)
			b = m;
		if(position <= wanted_position)
			a = m;
	}
	m= (flags & AVSEEK_FLAG_BACKWARD) ? a : b;

	if(!(flags & AVSEEK_FLAG_ANY)){
		while(m>=0 && m<nb_entries && !(entries[m].flags & AVINDEX_KEYFRAME)){
			m += (flags & AVSEEK_FLAG_BACKWARD) ? -1 : 1;
		}
	}

	if(m == nb_entries)
		return -1;
	return  m;
}

AVStream *matroska_reader_t::av_new_stream(int id/*=0*/ )
{
	AVStream *st;
	st = (AVStream *)av_mallocz(sizeof(AVStream));
	if (!st)
		return NULL;
	st->id = id;

	return st;
}

int matroska_reader_t::time_seek(uint64_t timestamp)
{
	int index = 0;
	int res = 0;
	AVStream* st = NULL;
	AVIndexEntry *entries = NULL;
	MatroskaTrack *track = matroska_find_track_by_num(&m_matroska,this->videoNum);
	if(track){
		st = track->stream;
		if(st)
		{
			entries = st->index_entries;
			if(entries) {
				index = av_index_search_timestamp(st, timestamp, AVSEEK_FLAG_ANY);
				uint64_t ff = m_matroska.time_scale;
				if(index >= 0)
				{
					res = url_fseek(&m_b,entries[index].pos,SEEK_SET);
					if(res < 0){
					return AVERROR(DEMUX_MKV_ERROR_EOF);
					}
				}
			}
		}
		int64_t findPosition = 0;
		if(this->findNextIFrame(findPosition) < 0){
			return -1;
		}
		DEBUG_PRINT("%s index=%d findPosition=%lld\n",__FILE__,index,findPosition);
		this->url_fseek(&m_b,findPosition,SEEK_SET);
	}
	m_curLaceIndex = 0;
	return MKV_DEMUX_SUCCESS;
}

int matroska_reader_t::position_seek(int64 position)
{
	int index = 0;
	int res = 0;
	if(position < m_matroska.segment_start){
		position = m_matroska.segment_start;
	}
	this->url_fseek(&m_b,position,SEEK_SET);

	int64_t findPosition = 0;
	if(this->findNextIFrame(findPosition) < 0){
		return -1;
	}
	DEBUG_PRINT("%s  findPosition=%lld\n",__FILE__,findPosition);
	this->url_fseek(&m_b,findPosition,SEEK_SET);
	m_curLaceIndex = 0;

	return res;
}

void matroska_reader_t::avFree(void **ptr)
{
	if(*ptr)
	{
		free(*ptr);
		*ptr = NULL;
	}
}

uint64_t matroska_reader_t::time_tell()
{
	return m_timecode;
}
uint64_t matroska_reader_t::position_tell()
{
	return this->url_ftell(&m_b);
}

/*
int64 matroska_reader_t::get_time_info(media_time_info_t* time_info)
{
	AVStream* st = NULL;
	AVIndexEntry *entries = NULL;
	AVIndexEntry entry;
	MatroskaTrack *track = matroska_find_track_by_num(&m_matroska,this->videoNum);
	if(track){
		st = track->stream;
		int nbEntries = st->nb_index_entries;
		if(st && nbEntries)
		{
			entries = st->index_entries;
			if(entries)
			{
				time_info->first_pts = entries[0].timestamp ;
				DEBUG_PRINT("entries[0].timestamp=%lld  entries[nbEntries-1].timestamp=%lld\n",
						entries[0].timestamp,
						entries[nbEntries-1].timestamp);
				time_info->first_pts_time = entries[0].timestamp * 9 / 100000 + 20;
				time_info->last_pts = entries[nbEntries-1].timestamp;
				time_info->last_pts_time = entries[nbEntries-1].timestamp * 9 / 100000 + 20;
			}
		}
	}
	time_info->time_length = m_matroska.duration / 1000;

	return time_info->time_length;
}

*/
int matroska_reader_t::findNextCluster(ByteIOContext *pb,int step)
{
	uint32_t id = 0;
	int count = 0;
	uint64_t clusterPosition = 0;
	uint64_t position = this->url_ftell(pb);
#define 	NOFINDCLUSTER	0
#define     FINDCLUSTER		1
redo:
	while(id != MATROSKA_ID_CLUSTER){
		id = id << 8;
		id = id | get_byte(pb);
		if(this->url_feof(&m_b))
			return -1;
		if(step > 0){
			count ++;
			if(count >= step)
			{
				this->url_fseek(pb,position,SEEK_SET);
				return NOFINDCLUSTER;
			}
		}
	}
	clusterPosition = this->url_ftell(pb) - 4;		//the "4" is the length of MATROSKA_ID_CLUSTER
	uint64_t eleSize = 0;
	if(this->parseNum(&m_matroska,8,&eleSize) < 0){
		return -1;
	}
	uint32_t tmpid = this->parseID(&m_matroska);
	switch(tmpid){
	case MATROSKA_ID_CLUSTERTIMECODE:
	case MATROSKA_ID_CLUSTERPREVSIZE:
	case MATROSKA_ID_BLOCKGROUP:
	case MATROSKA_ID_SIMPLEBLOCK:
	case MATROSKA_ID_CLUSTERPOSITION:
		this->url_fseek(pb,clusterPosition,SEEK_SET);
		break;
	default:
		id = id & 0x00ffffff;
		goto redo;
	}

	return FINDCLUSTER;
	}

//	}

//#endif // ENABLE_MKV_SUPPORT
