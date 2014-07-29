/***************************************************************************\
*                                                                           *
*   Copyright 2006 ViXS Systems Inc.  All Rights Reserved.                  *
*                                                                           *
*===========================================================================*
*                                                                           *
*   File Name: ebml.h                                                       *
*                                                                           *
*   Description:                                                            *
*       This file contains the interface of the EBML file reader.           *
*                                                                           *
*===========================================================================*

\***************************************************************************/

#if !defined(__EBML_H__)
#define __EBML_H__


//#ifdef ENABLE_MKV_SUPPORT


#include "xctype.h"

namespace DMP {

	/* EBML version supported */
	#define EBML_VERSION 1

	/* top-level master-IDs */
	#define EBML_ID_HEADER             0x1A45DFA3

	/* IDs in the HEADER master */
	#define EBML_ID_EBMLVERSION        0x4286
	#define EBML_ID_EBMLREADVERSION    0x42F7
	#define EBML_ID_EBMLMAXIDLENGTH    0x42F2
	#define EBML_ID_EBMLMAXSIZELENGTH  0x42F3
	#define EBML_ID_DOCTYPE            0x4282
	#define EBML_ID_DOCTYPEVERSION     0x4287
	#define EBML_ID_DOCTYPEREADVERSION 0x4285

	/* general EBML types */
	#define EBML_ID_VOID               0xEC
	#define EBML_ID_CRC32              0xBF

	/*
	 * Matroska element IDs, max. 32 bits
	 */

	/* toplevel segment */
	#define MATROSKA_ID_SEGMENT    0x18538067

	/* Matroska top-level master IDs */
	#define MATROSKA_ID_INFO       0x1549A966
	#define MATROSKA_ID_TRACKS     0x1654AE6B
	#define MATROSKA_ID_CUES       0x1C53BB6B
	#define MATROSKA_ID_TAGS       0x1254C367
	#define MATROSKA_ID_SEEKHEAD   0x114D9B74
	#define MATROSKA_ID_ATTACHMENTS 0x1941A469
	#define MATROSKA_ID_CLUSTER    0x1F43B675
	#define MATROSKA_ID_CHAPTERS   0x1043A770

	/* IDs in the info master */
	#define MATROSKA_ID_TIMECODESCALE 0x2AD7B1
	#define MATROSKA_ID_DURATION   0x4489
	#define MATROSKA_ID_TITLE      0x7BA9
	#define MATROSKA_ID_WRITINGAPP 0x5741
	#define MATROSKA_ID_MUXINGAPP  0x4D80
	#define MATROSKA_ID_DATEUTC    0x4461
	#define MATROSKA_ID_SEGMENTUID 0x73A4

	/* ID in the tracks master */
	#define MATROSKA_ID_TRACKENTRY 0xAE

	/* IDs in the trackentry master */
	#define MATROSKA_ID_TRACKNUMBER 0xD7
	#define MATROSKA_ID_TRACKUID   0x73C5
	#define MATROSKA_ID_TRACKTYPE  0x83
	#define MATROSKA_ID_TRACKAUDIO 0xE1
	#define MATROSKA_ID_TRACKVIDEO 0xE0
	#define MATROSKA_ID_CODECID    0x86
	#define MATROSKA_ID_CODECPRIVATE 0x63A2
	#define MATROSKA_ID_CODECNAME  0x258688
	#define MATROSKA_ID_CODECINFOURL 0x3B4040
	#define MATROSKA_ID_CODECDOWNLOADURL 0x26B240
	#define MATROSKA_ID_CODECDECODEALL 0xAA
	#define MATROSKA_ID_TRACKNAME  0x536E
	#define MATROSKA_ID_TRACKLANGUAGE 0x22B59C
	#define MATROSKA_ID_TRACKFLAGENABLED 0xB9
	#define MATROSKA_ID_TRACKFLAGDEFAULT 0x88
	#define MATROSKA_ID_TRACKFLAGFORCED 0x55AA
	#define MATROSKA_ID_TRACKFLAGLACING 0x9C
	#define MATROSKA_ID_TRACKMINCACHE 0x6DE7
	#define MATROSKA_ID_TRACKMAXCACHE 0x6DF8
	#define MATROSKA_ID_TRACKDEFAULTDURATION 0x23E383
	#define MATROSKA_ID_TRACKCONTENTENCODINGS 0x6D80
	#define MATROSKA_ID_TRACKCONTENTENCODING 0x6240
	#define MATROSKA_ID_TRACKTIMECODESCALE 0x23314F
	#define MATROSKA_ID_TRACKMAXBLKADDID 0x55EE

	/* IDs in the trackvideo master */
	#define MATROSKA_ID_VIDEOFRAMERATE 0x2383E3
	#define MATROSKA_ID_VIDEODISPLAYWIDTH 0x54B0
	#define MATROSKA_ID_VIDEODISPLAYHEIGHT 0x54BA
	#define MATROSKA_ID_VIDEOPIXELWIDTH 0xB0
	#define MATROSKA_ID_VIDEOPIXELHEIGHT 0xBA
	#define MATROSKA_ID_VIDEOPIXELCROPB 0x54AA
	#define MATROSKA_ID_VIDEOPIXELCROPT 0x54BB
	#define MATROSKA_ID_VIDEOPIXELCROPL 0x54CC
	#define MATROSKA_ID_VIDEOPIXELCROPR 0x54DD
	#define MATROSKA_ID_VIDEODISPLAYUNIT 0x54B2
	#define MATROSKA_ID_VIDEOFLAGINTERLACED 0x9A
	#define MATROSKA_ID_VIDEOSTEREOMODE 0x53B9
	#define MATROSKA_ID_VIDEOASPECTRATIO 0x54B3
	#define MATROSKA_ID_VIDEOCOLORSPACE 0x2EB524

	/* IDs in the trackaudio master */
	#define MATROSKA_ID_AUDIOSAMPLINGFREQ 0xB5
	#define MATROSKA_ID_AUDIOOUTSAMPLINGFREQ 0x78B5

	#define MATROSKA_ID_AUDIOBITDEPTH 0x6264
	#define MATROSKA_ID_AUDIOCHANNELS 0x9F

	/* IDs in the content encoding master */
	#define MATROSKA_ID_ENCODINGORDER 0x5031
	#define MATROSKA_ID_ENCODINGSCOPE 0x5032
	#define MATROSKA_ID_ENCODINGTYPE 0x5033
	#define MATROSKA_ID_ENCODINGCOMPRESSION 0x5034
	#define MATROSKA_ID_ENCODINGCOMPALGO 0x4254
	#define MATROSKA_ID_ENCODINGCOMPSETTINGS 0x4255

	/* ID in the cues master */
	#define MATROSKA_ID_POINTENTRY 0xBB

	/* IDs in the pointentry master */
	#define MATROSKA_ID_CUETIME    0xB3
	#define MATROSKA_ID_CUETRACKPOSITION 0xB7

	/* IDs in the cuetrackposition master */
	#define MATROSKA_ID_CUETRACK   0xF7
	#define MATROSKA_ID_CUECLUSTERPOSITION 0xF1
	#define MATROSKA_ID_CUEBLOCKNUMBER 0x5378

	/* IDs in the tags master */
	#define MATROSKA_ID_TAG                 0x7373
	#define MATROSKA_ID_SIMPLETAG           0x67C8
	#define MATROSKA_ID_TAGNAME             0x45A3
	#define MATROSKA_ID_TAGSTRING           0x4487
	#define MATROSKA_ID_TAGLANG             0x447A
	#define MATROSKA_ID_TAGDEFAULT          0x4484
	#define MATROSKA_ID_TAGDEFAULT_BUG      0x44B4
	#define MATROSKA_ID_TAGTARGETS          0x63C0
	#define MATROSKA_ID_TAGTARGETS_TYPE       0x63CA
	#define MATROSKA_ID_TAGTARGETS_TYPEVALUE  0x68CA
	#define MATROSKA_ID_TAGTARGETS_TRACKUID   0x63C5
	#define MATROSKA_ID_TAGTARGETS_CHAPTERUID 0x63C4
	#define MATROSKA_ID_TAGTARGETS_ATTACHUID  0x63C6

	/* IDs in the seekhead master */
	#define MATROSKA_ID_SEEKENTRY  0x4DBB

	/* IDs in the seekpoint master */
	#define MATROSKA_ID_SEEKID     0x53AB
	#define MATROSKA_ID_SEEKPOSITION 0x53AC

	/* IDs in the cluster master */
	#define MATROSKA_ID_CLUSTERTIMECODE 0xE7
	#define MATROSKA_ID_CLUSTERPOSITION 0xA7
	#define MATROSKA_ID_CLUSTERPREVSIZE 0xAB
	#define MATROSKA_ID_BLOCKGROUP 0xA0
	#define MATROSKA_ID_SIMPLEBLOCK 0xA3

	/* IDs in the blockgroup master */
	#define MATROSKA_ID_BLOCK      0xA1
	#define MATROSKA_ID_BLOCKDURATION 0x9B
	#define MATROSKA_ID_BLOCKREFERENCE 0xFB

	/* IDs in the attachments master */
	#define MATROSKA_ID_ATTACHEDFILE        0x61A7
	#define MATROSKA_ID_FILEDESC            0x467E
	#define MATROSKA_ID_FILENAME            0x466E
	#define MATROSKA_ID_FILEMIMETYPE        0x4660
	#define MATROSKA_ID_FILEDATA            0x465C
	#define MATROSKA_ID_FILEUID             0x46AE

	/* IDs in the chapters master */
	#define MATROSKA_ID_EDITIONENTRY        0x45B9
	#define MATROSKA_ID_CHAPTERATOM         0xB6
	#define MATROSKA_ID_CHAPTERTIMESTART    0x91
	#define MATROSKA_ID_CHAPTERTIMEEND      0x92
	#define MATROSKA_ID_CHAPTERDISPLAY      0x80
	#define MATROSKA_ID_CHAPSTRING          0x85
	#define MATROSKA_ID_CHAPLANG            0x437C
	#define MATROSKA_ID_EDITIONUID          0x45BC
	#define MATROSKA_ID_EDITIONFLAGHIDDEN   0x45BD
	#define MATROSKA_ID_EDITIONFLAGDEFAULT  0x45DB
	#define MATROSKA_ID_EDITIONFLAGORDERED  0x45DD
	#define MATROSKA_ID_CHAPTERUID          0x73C4
	#define MATROSKA_ID_CHAPTERFLAGHIDDEN   0x98
	#define MATROSKA_ID_CHAPTERFLAGENABLED  0x4598
	#define MATROSKA_ID_CHAPTERPHYSEQUIV    0x63C3

enum CodecID {
    CODEC_ID_NONE,

    /* video codecs */
    CODEC_ID_MPEG1VIDEO,
    CODEC_ID_MPEG2VIDEO, ///< preferred ID for MPEG-1/2 video decoding
    CODEC_ID_MPEG2VIDEO_XVMC,
    CODEC_ID_H261,
    CODEC_ID_H263,
    CODEC_ID_RV10,
    CODEC_ID_RV20,
    CODEC_ID_MJPEG,
    CODEC_ID_MJPEGB,
    CODEC_ID_LJPEG,
    CODEC_ID_SP5X,
    CODEC_ID_JPEGLS,
    CODEC_ID_MPEG4,
    CODEC_ID_RAWVIDEO,
    CODEC_ID_MSMPEG4V1,
    CODEC_ID_MSMPEG4V2,
    CODEC_ID_MSMPEG4V3,
    CODEC_ID_WMV1,
    CODEC_ID_WMV2,
    CODEC_ID_H263P,
    CODEC_ID_H263I,
    CODEC_ID_FLV1,
    CODEC_ID_SVQ1,
    CODEC_ID_SVQ3,
    CODEC_ID_DVVIDEO,
    CODEC_ID_HUFFYUV,
    CODEC_ID_CYUV,
    CODEC_ID_H264,
    CODEC_ID_INDEO3,
    CODEC_ID_VP3,
    CODEC_ID_THEORA,
    CODEC_ID_ASV1,
    CODEC_ID_ASV2,
    CODEC_ID_FFV1,
    CODEC_ID_4XM,
    CODEC_ID_VCR1,
    CODEC_ID_CLJR,
    CODEC_ID_MDEC,
    CODEC_ID_ROQ,
    CODEC_ID_INTERPLAY_VIDEO,
    CODEC_ID_XAN_WC3,
    CODEC_ID_XAN_WC4,
    CODEC_ID_RPZA,
    CODEC_ID_CINEPAK,
    CODEC_ID_WS_VQA,
    CODEC_ID_MSRLE,
    CODEC_ID_MSVIDEO1,
    CODEC_ID_IDCIN,
    CODEC_ID_8BPS,
    CODEC_ID_SMC,
    CODEC_ID_FLIC,
    CODEC_ID_TRUEMOTION1,
    CODEC_ID_VMDVIDEO,
    CODEC_ID_MSZH,
    CODEC_ID_ZLIB,
    CODEC_ID_QTRLE,
    CODEC_ID_SNOW,
    CODEC_ID_TSCC,
    CODEC_ID_ULTI,
    CODEC_ID_QDRAW,
    CODEC_ID_VIXL,
    CODEC_ID_QPEG,
#if LIBAVCODEC_VERSION_MAJOR < 53
    CODEC_ID_XVID,
#endif
    CODEC_ID_PNG,
    CODEC_ID_PPM,
    CODEC_ID_PBM,
    CODEC_ID_PGM,
    CODEC_ID_PGMYUV,
    CODEC_ID_PAM,
    CODEC_ID_FFVHUFF,
    CODEC_ID_RV30,
    CODEC_ID_RV40,
    CODEC_ID_VC1,
    CODEC_ID_WMV3,
    CODEC_ID_LOCO,
    CODEC_ID_WNV1,
    CODEC_ID_AASC,
    CODEC_ID_INDEO2,
    CODEC_ID_FRAPS,
    CODEC_ID_TRUEMOTION2,
    CODEC_ID_BMP,
    CODEC_ID_CSCD,
    CODEC_ID_MMVIDEO,
    CODEC_ID_ZMBV,
    CODEC_ID_AVS,
    CODEC_ID_SMACKVIDEO,
    CODEC_ID_NUV,
    CODEC_ID_KMVC,
    CODEC_ID_FLASHSV,
    CODEC_ID_CAVS,
    CODEC_ID_JPEG2000,
    CODEC_ID_VMNC,
    CODEC_ID_VP5,
    CODEC_ID_VP6,
    CODEC_ID_VP6F,
    CODEC_ID_TARGA,
    CODEC_ID_DSICINVIDEO,
    CODEC_ID_TIERTEXSEQVIDEO,
    CODEC_ID_TIFF,
    CODEC_ID_GIF,
    CODEC_ID_FFH264,
    CODEC_ID_DXA,
    CODEC_ID_DNXHD,
    CODEC_ID_THP,
    CODEC_ID_SGI,
    CODEC_ID_C93,
    CODEC_ID_BETHSOFTVID,
    CODEC_ID_PTX,
    CODEC_ID_TXD,
    CODEC_ID_VP6A,
    CODEC_ID_AMV,
    CODEC_ID_VB,
    CODEC_ID_PCX,
    CODEC_ID_SUNRAST,
    CODEC_ID_INDEO4,
    CODEC_ID_INDEO5,
    CODEC_ID_MIMIC,
    CODEC_ID_RL2,
    CODEC_ID_8SVX_EXP,
    CODEC_ID_8SVX_FIB,
    CODEC_ID_ESCAPE124,
    CODEC_ID_DIRAC,
    CODEC_ID_BFI,
    CODEC_ID_CMV,
    CODEC_ID_MOTIONPIXELS,
    CODEC_ID_TGV,
    CODEC_ID_TGQ,
    CODEC_ID_TQI,
    CODEC_ID_AURA,
    CODEC_ID_AURA2,
    CODEC_ID_V210X,
    CODEC_ID_TMV,
    CODEC_ID_V210,
    CODEC_ID_DPX,
    CODEC_ID_MAD,
    CODEC_ID_FRWU,
    CODEC_ID_FLASHSV2,
    CODEC_ID_CDGRAPHICS,
    CODEC_ID_R210,
    CODEC_ID_ANM,
    CODEC_ID_BINKVIDEO,
    CODEC_ID_IFF_ILBM,
    CODEC_ID_IFF_BYTERUN1,
    CODEC_ID_KGV1,
    CODEC_ID_YOP,
    CODEC_ID_VP8,
    CODEC_ID_PICTOR,
    CODEC_ID_ANSI,
    CODEC_ID_A64_MULTI,
    CODEC_ID_A64_MULTI5,
    CODEC_ID_R10K,
    CODEC_ID_MXPEG,
    CODEC_ID_LAGARITH,

    /* various PCM "codecs" */
    CODEC_ID_PCM_S16LE= 0x10000,
    CODEC_ID_PCM_S16BE,
    CODEC_ID_PCM_U16LE,
    CODEC_ID_PCM_U16BE,
    CODEC_ID_PCM_S8,
    CODEC_ID_PCM_U8,
    CODEC_ID_PCM_MULAW,
    CODEC_ID_PCM_ALAW,
    CODEC_ID_PCM_S32LE,
    CODEC_ID_PCM_S32BE,
    CODEC_ID_PCM_U32LE,
    CODEC_ID_PCM_U32BE,
    CODEC_ID_PCM_S24LE,
    CODEC_ID_PCM_S24BE,
    CODEC_ID_PCM_U24LE,
    CODEC_ID_PCM_U24BE,
    CODEC_ID_PCM_S24DAUD,
    CODEC_ID_PCM_ZORK,
    CODEC_ID_PCM_S16LE_PLANAR,
    CODEC_ID_PCM_DVD,
    CODEC_ID_PCM_F32BE,
    CODEC_ID_PCM_F32LE,
    CODEC_ID_PCM_F64BE,
    CODEC_ID_PCM_F64LE,
    CODEC_ID_PCM_BLURAY,
    CODEC_ID_PCM_LXF,

    /* various ADPCM codecs */
    CODEC_ID_ADPCM_IMA_QT= 0x11000,
    CODEC_ID_ADPCM_IMA_WAV,
    CODEC_ID_ADPCM_IMA_DK3,
    CODEC_ID_ADPCM_IMA_DK4,
    CODEC_ID_ADPCM_IMA_WS,
    CODEC_ID_ADPCM_IMA_SMJPEG,
    CODEC_ID_ADPCM_MS,
    CODEC_ID_ADPCM_4XM,
    CODEC_ID_ADPCM_XA,
    CODEC_ID_ADPCM_ADX,
    CODEC_ID_ADPCM_EA,
    CODEC_ID_ADPCM_G726,
    CODEC_ID_ADPCM_CT,
    CODEC_ID_ADPCM_SWF,
    CODEC_ID_ADPCM_YAMAHA,
    CODEC_ID_ADPCM_SBPRO_4,
    CODEC_ID_ADPCM_SBPRO_3,
    CODEC_ID_ADPCM_SBPRO_2,
    CODEC_ID_ADPCM_THP,
    CODEC_ID_ADPCM_IMA_AMV,
    CODEC_ID_ADPCM_EA_R1,
    CODEC_ID_ADPCM_EA_R3,
    CODEC_ID_ADPCM_EA_R2,
    CODEC_ID_ADPCM_IMA_EA_SEAD,
    CODEC_ID_ADPCM_IMA_EA_EACS,
    CODEC_ID_ADPCM_EA_XAS,
    CODEC_ID_ADPCM_EA_MAXIS_XA,
    CODEC_ID_ADPCM_IMA_ISS,
    CODEC_ID_ADPCM_G722,

    /* AMR */
    CODEC_ID_AMR_NB= 0x12000,
    CODEC_ID_AMR_WB,

    /* RealAudio codecs*/
    CODEC_ID_RA_144= 0x13000,
    CODEC_ID_RA_288,

    /* various DPCM codecs */
    CODEC_ID_ROQ_DPCM= 0x14000,
    CODEC_ID_INTERPLAY_DPCM,
    CODEC_ID_XAN_DPCM,
    CODEC_ID_SOL_DPCM,

    /* audio codecs */
    CODEC_ID_MP2= 0x15000,
    CODEC_ID_MP3, ///< preferred ID for decoding MPEG audio layer 1, 2 or 3
    CODEC_ID_AAC,
    CODEC_ID_AC3,
    CODEC_ID_DTS,
    CODEC_ID_VORBIS,
    CODEC_ID_DVAUDIO,
    CODEC_ID_WMAV1,
    CODEC_ID_WMAV2,
    CODEC_ID_MACE3,
    CODEC_ID_MACE6,
    CODEC_ID_VMDAUDIO,
    CODEC_ID_SONIC,
    CODEC_ID_SONIC_LS,
    CODEC_ID_FLAC,
    CODEC_ID_MP3ADU,
    CODEC_ID_MP3ON4,
    CODEC_ID_SHORTEN,
    CODEC_ID_ALAC,
    CODEC_ID_WESTWOOD_SND1,
    CODEC_ID_GSM, ///< as in Berlin toast format
    CODEC_ID_QDM2,
    CODEC_ID_COOK,
    CODEC_ID_TRUESPEECH,
    CODEC_ID_TTA,
    CODEC_ID_SMACKAUDIO,
    CODEC_ID_QCELP,
    CODEC_ID_WAVPACK,
    CODEC_ID_DSICINAUDIO,
    CODEC_ID_IMC,
    CODEC_ID_MUSEPACK7,
    CODEC_ID_MLP,
    CODEC_ID_GSM_MS, /* as found in WAV */
    CODEC_ID_ATRAC3,
    CODEC_ID_VOXWARE,
    CODEC_ID_APE,
    CODEC_ID_NELLYMOSER,
    CODEC_ID_MUSEPACK8,
    CODEC_ID_SPEEX,
    CODEC_ID_WMAVOICE,
    CODEC_ID_WMAPRO,
    CODEC_ID_WMALOSSLESS,
    CODEC_ID_ATRAC3P,
    CODEC_ID_EAC3,
    CODEC_ID_SIPR,
    CODEC_ID_MP1,
    CODEC_ID_TWINVQ,
    CODEC_ID_TRUEHD,
    CODEC_ID_MP4ALS,
    CODEC_ID_ATRAC1,
    CODEC_ID_BINKAUDIO_RDFT,
    CODEC_ID_BINKAUDIO_DCT,
    CODEC_ID_AAC_LATM,

    /* subtitle codecs */
    CODEC_ID_DVD_SUBTITLE= 0x17000,
    CODEC_ID_DVB_SUBTITLE,
    CODEC_ID_TEXT,  ///< raw UTF-8 text
    CODEC_ID_XSUB,
    CODEC_ID_SSA,
    CODEC_ID_MOV_TEXT,
    CODEC_ID_HDMV_PGS_SUBTITLE,
    CODEC_ID_DVB_TELETEXT,
    CODEC_ID_SRT,

    /* other specific kind of codecs (generally used for attachments) */
    CODEC_ID_TTF= 0x18000,

    CODEC_ID_PROBE= 0x19000, ///< codec_id is not known (like CODEC_ID_NONE) but lavf should attempt to identify it

    CODEC_ID_MPEG2TS= 0x20000, /**< _FAKE_ codec to indicate a raw MPEG-2 TS
                                * stream (only used by libavformat) */
    CODEC_ID_FFMETADATA=0x21000,   ///< Dummy codec for streams containing only metadata information.
};

	typedef enum {
	  MATROSKA_TRACK_TYPE_NONE     = 0x0,
	  MATROSKA_TRACK_TYPE_VIDEO    = 0x1,
	  MATROSKA_TRACK_TYPE_AUDIO    = 0x2,
	  MATROSKA_TRACK_TYPE_COMPLEX  = 0x3,
	  MATROSKA_TRACK_TYPE_LOGO     = 0x10,
	  MATROSKA_TRACK_TYPE_SUBTITLE = 0x11,
	  MATROSKA_TRACK_TYPE_CONTROL  = 0x20,
	} MatroskaTrackType;

	typedef enum {
	  MATROSKA_TRACK_ENCODING_COMP_ZLIB        = 0,
	  MATROSKA_TRACK_ENCODING_COMP_BZLIB       = 1,
	  MATROSKA_TRACK_ENCODING_COMP_LZO         = 2,
	  MATROSKA_TRACK_ENCODING_COMP_HEADERSTRIP = 3,
	} MatroskaTrackEncodingCompAlgo;


	/*
	 * Matroska Codec IDs, strings
	 */

	typedef struct CodecTags{
	    char str[20];
	    enum CodecID id;
	    es_stream_type_t es_type;
	}CodecTags;

	typedef struct CodecMime{
	    char str[32];
	    enum CodecID id;
	}CodecMime;

	typedef struct AVCodecTag {
	    enum CodecID id;
	    unsigned int tag;
	} AVCodecTag;

	enum AVDiscard{
	    /* We leave some space between them for extensions (drop some
	     * keyframes for intra-only or drop just some bidir frames). */
	    AVDISCARD_NONE   =-16, ///< discard nothing
	    AVDISCARD_DEFAULT=  0, ///< discard useless packets like 0 size packets in avi
	    AVDISCARD_NONREF =  8, ///< discard all non reference
	    AVDISCARD_BIDIR  = 16, ///< discard all bidirectional frames
	    AVDISCARD_NONKEY = 32, ///< discard all frames except keyframes
	    AVDISCARD_ALL    = 48, ///< discard all
	};

	enum AVStreamParseType {
	    AVSTREAM_PARSE_NONE,
	    AVSTREAM_PARSE_FULL,       /**< full parsing and repack */
	    AVSTREAM_PARSE_HEADERS,    /**< Only parse headers, do not repack. */
	    AVSTREAM_PARSE_TIMESTAMPS, /**< full parsing and interpolation of timestamps for frames not starting on a packet boundary */
	    AVSTREAM_PARSE_FULL_ONCE,  /**< full parsing and repack of the first frame only, only implemented for H.264 currently */
	};



	struct ebml_raw_t
	{
	    size_t m_Size;
	    int type;
	    unsigned char *m_pData;

	    ebml_raw_t();
	    ~ebml_raw_t();
	    void set(const void *pData, size_t Size);
	    void append(const void *pData, size_t Size);
	    ebml_raw_t(const void *pData, size_t Size);
	};


	class bit_stream
	{
	public:
		bit_stream()
		{
		}

		~bit_stream()
		{
		}

		void open(void *buf, int bit = 0)
		{
	        start = (uint8 *)buf;
	        p = start;
	        start_bit = bit;
	        bits = start_bit;
		}

	    int delete_eptb(int len)
	    {
	        uint8 *p = start;
	        int delete_bytes = 0;

	        while(len > 0)
	        {
	            if ((p[0] == 0x00) && (p[1] == 0x00) && (p[2] == 0x03) && (len >= 4))
	            {
	                memcpy(&p[2], &p[3], len - 3);
	                p += 2;
	                len -= 3;
	                delete_bytes ++;
	            }
	            else
	            {
	                p ++;
	                len --;
	            }
	        }
	        return delete_bytes;
	    }

	    int get_1bit()
	    {
	        int ret;
	        ret = (*p & (1 << (7 - bits))) ? 1 : 0;
	        bits = (bits + 1) % 8;
	        if (!bits)
	        {
	            p ++;
	        }
	        return ret;
	    }

	    uint32 get_bits(int len)
	    {
	        uint32 ret = 0;

	        while(len --)
	        {
	            ret = (ret << 1) | get_1bit();
	        }

	        return ret;
	    }

	    void set_1bit(int bit)
	    {
	#if 0
	        uint32 mask;

	        mask = 1 << (7 - bits);
	        *p = (unsigned char)((*p & ~mask) | (bit ? mask : 0));
	        bits = (bits + 1) % 8;
	        if (!bits)
	        {
	            p ++;
	        }
	#else
	        if (bits == 0)
	        {
	            *p = 0;
	        }
	        if (bit)
	        {
	            *p |= 1 << (7 - bits);
	        }
	        bits ++;
	        if (bits == 8)
	        {
	            p ++;
	            bits = 0;
	        }
	#endif
	    }

	    void set_bits(uint32 value, int len)
	    {
	        uint32 mask;
	        mask = 1 << (len - 1);

	        while(len --)
	        {
	            set_1bit(value & mask);
	            value <<= 1;
	        }
	    }

	    uint32 get_bits_eptb(int len) // with Emulation Prevention Three Byte
	    {
	        int i = 0;
	        uint32 ret = 0;

	        while(len --)
	        {
	            ret = (ret << 1) | get_1bit();
	            i ++;
	            if ((i >= 24) && !(i % 8) && (ret & 0xFFFFFF) == 0x000003)
	            {
	                ret >>= 8;
	                len += 8;
	                i -= 8;
	            }
	        }

	        return ret;
	    }

	    uint32 get_ue()
	    {
	        int len = 0;
	        uint32 v = 0;

	        while(!get_1bit() && (len < 32)) len ++;    // It must be less than 32 bits
	        v = get_bits(len);
	        return v + (1 << len) - 1;
	    }

	    int get_se()
	    {
	        int len = 0;
	        int v = 0;
	        int temp;

	        while(!get_1bit() && (len < 32)) len ++;    // It must be less than 32 bits
	        v = get_bits(len);
		    temp = v + (1 << len) - 1;
		    v = (temp + 1) >> 1;
		    if((temp & 0x1) == 0) //lsb is sign bit
		    {
			    v = -v;
		    }
	        return v;
	    }

	    void restore()
	    {
	        p = start;
	        bits = start_bit;
	    }

	    void save()
	    {
	        start = p;
	        start_bit = bits;
	    }

	    int more_bits_avail()
	    {
	        return (bits != 0);
	    }

	    int bit_size()
	    {
	        return (uint32)(p - start) * 8 + bits - start_bit;
	    }

	    void skip_bits(int len)
	    {
	        p += (bits + len) / 8;
	        bits = (bits + len) % 8;
	    }

	    void get_bytes(void *buf, size_t size)
	    {
	        memcpy(buf, p, size);
	        p += size;
	    }

	    void put_bytes(const void *buf, size_t size)
	    {
	        memcpy(p, buf, size);
	        p += size;
	    }

	    void get_byte(unsigned char &value)
	    {
	        value = *p++;
	    }

	    void put_byte(unsigned char value)
	    {
	        *p++ = value;
	    }

	    void get_uimsbf(unsigned int &value)
	    {
	        value = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
	        p += 4;
	    }

	    void get_uimsbf(unsigned short &value)
	    {
	        value = (p[0] << 8) + p[1];
	        p += 2;
	    }

	    void get_uimsbf(unsigned char &value)
	    {
	        value = *p++;
	    }

	    void put_uimsbf(unsigned short &value)
	    {
	        *p++ = value >> 8;
	        *p++ = (unsigned char)value;
	    }

	    void put_uimsbf(unsigned char &value)
	    {
	        *p++ = value;
	    }

	    void put_uimsbf(unsigned int value)
	    {
	        *p++ = value >> 24;
	        *p++ = value >> 16;
	        *p++ = value >> 8;
	        *p++ = value;
	    }

	    void get_bslbf(unsigned short &value)
	    {
	        value = (p[0] << 8) + p[1];
	        p += 2;
	    }

	    void get_bslbf(unsigned char &value)
	    {
	        value = *p++;
	    }

	    void put_bslbf(unsigned short value)
	    {
	        *p++ = value >> 8;
	        *p++ = (unsigned char)value;
	    }

	    void put_bslbf(unsigned char value)
	    {
	        *p++ = value;
	    }

	    void put_padding(unsigned int bytes)
	    {
	        memset(p, 0, bytes);
	        p += bytes;
	    }


	private:
		uint8 *p;
		uint8 *start;
		int bits;
		int start_bit;

	};


	#define AV_NOPTS_VALUE          (uint64_t)-1
	#define AV_TIME_BASE            1000000
//	#define AV_NOPTS_VALUE          (uint64_t)0x8000000000000000
}

#endif // __EBML_H__
