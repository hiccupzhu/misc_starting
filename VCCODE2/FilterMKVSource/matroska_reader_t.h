/***************************************************************************\
*                                                                           *
*   Copyright 2006 ViXS Systems Inc.  All Rights Reserved.                  *
*                                                                           *
*===========================================================================*
*                                                                           *
*   File Name: matroska.h                                                   *
*                                                                           *
*   Description:                                                            *
*       This file contains the interface of the MATROSKA file reader.       *
*                                                                           *
*===========================================================================*

\***************************************************************************/

#if !defined(__MATROSKA_H__)
#define __MATROSKA_H__


//#ifdef ENABLE_MKV_SUPPORT

#include "ebml.h"

//namespace DMP {

#define IN
#define OUT


#define MKV_DEMUX_SUCCESS 					 0
#define MKV_DEMUX_FAILED					-1

#define DEMUX_MKV_ERROR_EOF					1			//end of file
#define DEMUX_MKV_ERROR_HOF					2			//head of file
#define DEMUX_MKV_ERROR_NOMEM				3			//malloc the memory failed
#define DEMUX_MKV_ERROR_IO					4			//read the byte failed



#ifndef AVERROR
#define AVERROR(e) (-(e))
#endif



	typedef enum {
		EBML_NONE,
		EBML_UINT,
		EBML_FLOAT,
		EBML_STR,
		EBML_UTF8,
		EBML_BIN,
		EBML_NEST,
		EBML_PASS,
		EBML_STOP,
	} EbmlType;

	typedef struct {
		int nb_elem;
		void *elem;
	} ebml_list,EbmlList;

	typedef const struct _EbmlSyntax {
	    uint32_t id;
	    EbmlType type;
	    int list_elem_size;
	    int data_offset;
	    union {
	        uint64_t    u;
	        double      f;
	        const char *s;
	        const struct _EbmlSyntax *n;
	    } def;
	}EbmlSyntax;

	typedef struct {
	    int      size;
	    uint8_t *data;
	    int64_t  pos;
	} EbmlBin;

	typedef struct {
	    uint64_t version;
	    uint64_t max_size;
	    uint64_t id_length;
	    char    *doctype;
	    uint64_t doctype_version;
	} Ebml;

	typedef struct {
	    uint64_t uid;
	    char *filename;
	    char *mime;
	    EbmlBin bin;
	} MatroskaAttachement;

	typedef struct {
	    uint64_t start;
	    uint64_t end;
	    uint64_t uid;
	    char    *title;

	    //AVChapter *chapter;
	} MatroskaChapter;

	typedef struct {
	    uint64_t algo;
	    EbmlBin  settings;
	} MatroskaTrackCompression;

	typedef struct {
	    uint64_t scope;
	    uint64_t type;
	    MatroskaTrackCompression compression;
	} MatroskaTrackEncoding;

	typedef struct {
	    double   frame_rate;
	    uint64_t display_width;
	    uint64_t display_height;
	    uint64_t pixel_width;
	    uint64_t pixel_height;
	    uint64_t fourcc;
	} MatroskaTrackVideo;

	typedef struct {
	    double   samplerate;
	    double   out_samplerate;
	    uint64_t bitdepth;
	    uint64_t channels;

	    /* real audio header (extracted from extradata) */
	    int      coded_framesize;
	    int      sub_packet_h;
	    int      frame_size;
	    int      sub_packet_size;
	    int      sub_packet_cnt;
	    int      pkt_cnt;
	    uint8_t *buf;
	} MatroskaTrackAudio;



	typedef struct {
		uint64_t track;
		uint64_t pos;
	} MatroskaIndexPos;

	typedef struct {
	    uint64_t time;
	    EbmlList pos;
	} MatroskaIndex;

	typedef struct {
	    char *name;
	    char *string;
	    char *lang;
	    uint64_t def;
	    EbmlList sub;
	} MatroskaTag;

	typedef struct {
	    char    *type;
	    uint64_t typevalue;
	    uint64_t trackuid;
	    uint64_t chapteruid;
	    uint64_t attachuid;
	} MatroskaTagTarget;

	typedef struct {
	    MatroskaTagTarget target;
	    EbmlList tag;
	} MatroskaTags;

	typedef struct {
	    uint64_t id;
	    uint64_t pos;
	} MatroskaSeekhead;

	typedef struct {
	    uint64_t start;
	    uint64_t length;
	} MatroskaLevel;

	typedef struct {
		uint64_t clusterPrevsize;
		uint64_t clusterPosition;
		uint64_t clusteSize;
		uint64_t clusterTime;
		uint64_t clusterNonSample;
	} MatroskaCluster;

	typedef struct {
	    uint64_t duration;
	    int64_t  reference;
	    uint64_t non_simple;
	    EbmlBin  bin;
	} MatroskaBlock;


	typedef struct {
		unsigned char *buffer;
		int buffer_size;
		unsigned char *buf_ptr, *buf_end;
		void *opaque;
		size_t (*read_packet)(void *opaque, void *buf, size_t buf_size,int fullread);
		int (*write_packet)(void *opaque, uint8_t *buf, int buf_size);
		int64_t (*seek)(void *opaque, int64_t offset, int whence);
		int64_t pos; /**< position in the file of the current buffer */
		int must_flush; /**< true if the next seek should flush */
		int eof_reached; /**< true if eof reached */
		int write_flag;  /**< true if open for writing */
		int is_streamed;
		int max_packet_size;
		unsigned long checksum;
		unsigned char *checksum_ptr;
		unsigned long (*update_checksum)(unsigned long checksum, const uint8_t *buf, unsigned int size);
		int error;         ///< contains the error code or 0 if no error happened
		int (*read_pause)(void *opaque, int pause);
		int64_t (*read_seek)(void *opaque, int stream_index,
							 int64_t timestamp, int flags);

		int64_t (*tell)(void *handle);
	} ByteIOContext;

	typedef struct AVFormatContext {
		ByteIOContext *pb;
		unsigned int nb_streams;

		int flags;
	#define AVFMT_FLAG_GENPTS       0x0001 ///< Generate missing pts even if it requires parsing future frames.
	#define AVFMT_FLAG_IGNIDX       0x0002 ///< Ignore index.
	#define AVFMT_FLAG_NONBLOCK     0x0004 ///< Do not block when reading packets from input.
	#define AVFMT_FLAG_IGNDTS       0x0008 ///< Ignore DTS on frames that contain both DTS & PTS
	#define AVFMT_FLAG_NOFILLIN     0x0010 ///< Do not infer any values from other values, just return what is stored in the container
	#define AVFMT_FLAG_NOPARSE      0x0020 ///< Do not use AVParsers, you also must set AVFMT_FLAG_NOFILLIN as the fillin code works on frames and no parsing -> no frames. Also seeking to frames can not work if parsing to find frame boundaries has been disabled
	#define AVFMT_FLAG_RTP_HINT     0x0040 ///< Add RTP hinting to the output file

		int64_t duration;
	}AVFormatContext;

	#ifndef EBML_MAX_DEPTH
	#define EBML_MAX_DEPTH 16
	#endif

	typedef struct {
		AVFormatContext *ctx;

		/* EBML stuff */
		int num_levels;
		MatroskaLevel levels[EBML_MAX_DEPTH];		//level infomation:start position \ length
		int level_up;								//???
		uint32_t current_id;						//current element id

		uint64_t time_scale;						//time scale in segmentinfo section
		double   duration;							//same as up
		char    *title;								//??

		ebml_list tracks;
		ebml_list attachments;
		ebml_list chapters;
		ebml_list index;
		ebml_list tags;
		ebml_list seekhead;
		MatroskaCluster current_cluster;			//current cluster

		int done;									//file is finished

		/* byte position of the segment inside the stream */
		int64_t segment_start;
		int 	bitrate;
		int64_t fileLen;

		/* What to skip before effectively reading a packet. */
		int skip_to_keyframe;
		uint64_t skip_to_timecode;
	} MatroskaDemuxContext;

	typedef struct AVCodecContext {
		enum CodecID codec_id;
	    int bit_rate;
	    int width, height;
	    int sample_rate; ///< samples per second
	    int channels;    ///< number of audio channels
	    int frame_size;
	} AVCodecContext;

	typedef struct AVPacket {
	    int64_t pts;
	    int64_t dts;
	    uint8_t *data;
	    int   size;
	    int   stream_index;
	    int   flags;
	    int   duration;
	    void  (*destruct)(struct AVPacket *);
	    void  *priv;
	    int64_t pos;
	    int64_t convergence_duration;
	} AVPacket;

	typedef struct AVRational{
	    int num; ///< numerator
	    int den; ///< denominator
	} AVRational;

	typedef struct AVIndexEntry {
	    int64_t pos;
	    int64_t timestamp;
	#define AVINDEX_KEYFRAME 0x0001
	    int flags:2;
	    int size:30; //Yeah, trying to keep the size of this small to reduce memory requirements (it is 24 vs. 32 bytes due to possible 8-byte alignment).
	    int min_distance;         /**< Minimum distance between this and the previous keyframe, used to avoid unneeded searching. */
	} AVIndexEntry;

	typedef struct AVStream {
		int id;
	    AVRational time_base;
	    int64_t start_time;
	    int64_t duration;
	    AVIndexEntry *index_entries; /**< Only used if the format does not
	                                    support seeking natively. */
	    int nb_index_entries;
	    unsigned int index_entries_allocated_size;
	    int64_t nb_frames;
	    int disposition;
	} AVStream;

	typedef struct {
		uint64_t num;
		uint64_t uid;
		uint64_t type;
		char    *name;
		char    *codec_id;
		EbmlBin  codec_priv;
		char    *language;
		double time_scale;
		uint64_t default_duration;
		uint64_t flag_default;
		uint64_t flag_forced;
		MatroskaTrackVideo video;
		MatroskaTrackAudio audio;
		EbmlList encodings;

		//AVStream *stream;
		AVStream *stream;
		int64_t end_timecode;
		int ms_compat;

		enum CodecID enumCodecID;
		ebml_raw_t paketHeader;
		ebml_raw_t encodingInfo;
	} MatroskaTrack;


	typedef struct{
		int speed;
		int isTricking;
	}ScanSpeed;

	class matroska_reader_t
	{
	public:
		matroska_reader_t();
		~matroska_reader_t();

	public:
		static int detect(source_access_callbacks_t *source);

	private:
		int		matroska_read_header();

		int		ebml_parse(MatroskaDemuxContext *matroska, EbmlSyntax *syntax,void *data);
		void	ebml_free(EbmlSyntax *syntax, void *data);
		int 	ebml_read_num(MatroskaDemuxContext *matroska,
						  ByteIOContext *pb,
						  int max_size,
						  uint64_t *number);
		int 	ebml_parse_id(MatroskaDemuxContext *matroska,
						  EbmlSyntax *syntax,
						  uint32_t id,
						  void *data);
		int 	ebml_parse_elem(MatroskaDemuxContext *matroska,
		                    EbmlSyntax *syntax,
		                    void *data);
		int 	ebml_read_length(MatroskaDemuxContext *matroska,
							 ByteIOContext *pb,
		                     uint64_t *number);
		int 	ebml_read_uint(ByteIOContext *pb,
						   int size,
						   uint64_t *num);
		int 	ebml_read_float(ByteIOContext *pb,
							int size,
							double *num);
		int 	ebml_read_ascii(ByteIOContext *pb,
							int size,
							char **str);
		int 	ebml_read_binary(ByteIOContext *pb,
							 int length,
							 EbmlBin *bin);
		int 	ebml_read_master(MatroskaDemuxContext *matroska,
							 uint64_t length);
		int 	ebml_parse_nest(MatroskaDemuxContext *matroska,
							EbmlSyntax *syntax,
		                    void *data);
		int 	ebml_level_end(MatroskaDemuxContext *matroska);
		int 	matroska_ebmlnum_uint(MatroskaDemuxContext *matroska,
		                              uint8_t *data,
		                              uint32_t size,
		                              uint64_t *num);
		int 	matroska_ebmlnum_sint(MatroskaDemuxContext *matroska,
		                              uint8_t *data,
		                              uint32_t size,
		                              int64_t *num);
		MatroskaTrack *matroska_find_track_by_num(MatroskaDemuxContext *matroska,int num);


		enum CodecID ff_codec_get_id(const AVCodecTag *tags, unsigned int tag);
		int 	init_put_byte(ByteIOContext *s,
		                  unsigned char *buffer,
		                  int buffer_size,
		                  int write_flag,
		                  void *opaque,
		                  size_t (*read_packet)(void *opaque, void *buf, size_t buf_size,int fullread),
		                  int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
		                  int64_t (*seek)(void *opaque, int64_t offset, int whence));
		unsigned int ff_toupper4(unsigned int x);

		void 		fill_buffer(ByteIOContext *s);
		void 		flush_buffer(ByteIOContext *s);
		void 		put_buffer(ByteIOContext *s, const char *buf, int size);
		void 		put_le16(ByteIOContext *s, unsigned int val);
		void 		put_le32(ByteIOContext *s, unsigned int val);
		void 		put_byte(ByteIOContext *s, int b);

		unsigned int 	get_be16(ByteIOContext *s);
		unsigned int 	get_be32(ByteIOContext *s);
		uint64_t 		get_be64(ByteIOContext *s);
		int 			get_byte(ByteIOContext *s);
		int 			get_buffer(ByteIOContext *s, unsigned char *buf, int size);

		int 		url_fskip(ByteIOContext *s, int64_t b);
		int64_t 	url_ftell(ByteIOContext *s);
		int64_t 	url_fseek(ByteIOContext *s, int64_t offset, int whence);
		int 		url_feof(ByteIOContext *s);
		int 		url_ferror(ByteIOContext *s);
		int 		url_setbufsize(ByteIOContext *s, int buf_size);
		int 		url_resetbuf(ByteIOContext *s, int flags);
		int 		url_is_streamed(ByteIOContext *s);

		void		matroska_execute_seekhead(MatroskaDemuxContext *matroska);
		int 		matroska_aac_profile(char *codec_id);
		int 		matroska_aac_sri(int samplerate);


		void*		av_malloc(unsigned int size);
		void*		av_realloc(void *ptr, unsigned int size);
		void*		av_fast_realloc(void *ptr, unsigned int *size, uint32_t min_size);
		void 		av_free(void *ptr);
		void*		av_mallocz(unsigned int size);
		char*		av_strdup(const char *s);
		void 		av_freep(void *ptr);
		int 		av_add_index_entry(AVStream *st,
		                            int64_t pos, int64_t timestamp, int size, int distance, int flags);
		int 		av_index_search_timestamp(AVStream *st, int64_t wanted_timestamp,
		                              int flags);
		int 		av_index_search_position(AVStream *st, int64_t wanted_position,
				                              int flags);
		AVStream *	av_new_stream(int id = 0);


		int 		av_new_packet(AVPacket *pkt, int size);
		void 		av_init_packet(AVPacket *pkt);
		void 		av_destruct_packet(AVPacket *pkt);

		void		matroska_release_tags(ebml_list& tags);
		void		matroska_release_tag(ebml_list& _tag);
		void		matroska_release_seekhead(ebml_list& seekhead);
		void		matroska_release_attachments(ebml_list& attachments);
		void		matroska_release_chapters(ebml_list& chapters);
		void		matroska_release_index(ebml_list& index);
		void		matroska_release_tracks(ebml_list& tracks);

		int			parseNum(MatroskaDemuxContext *matroska,int max_size,uint64_t* number);
		uint64_t	parseID(MatroskaDemuxContext *matroska);
		int 		parseLaces(uint8_t*& pdata,int& sizeInside,uint8_t flags);
		int 		readUint(MatroskaDemuxContext *matroska, int size, uint64_t *num);
		void 		avFree(void **ptr);

	private:
		int 		init_audio_parameter();
		int			init_video_parameter();
		uint32_t 	handleBlockReference(uint64_t& blockReference);

	//three functions,basicly
	public:
		int load(source_access_callbacks_t *source);
		int get_video_parameter(video_parameter_t *vpara, int vid );
		int get_audio_parameter(audio_parameter_t *apara, int aid );
		int read_next_chunk(int  & type,
							void * & buf,
							int & size,
							int64 & pts,
							int64 & dts,
							int64 & end_pts,
							int & au_start);
		int time_seek(uint64_t timestamp);
		int position_seek(int64 position);

		int set_scan_speed(int speed);
		int get_scan_speed();

		//int64 get_time_info(media_time_info_t* time_info);
		uint64_t time_tell();
		uint64_t position_tell();

		int findNextCluster(ByteIOContext *pb,int step=0);				//directly move the file pointer
		int findNextIFrame(OUT int64_t& findPosition,int step=0);		//do not move the file pointer
		int readClusterData(uint8_t* buffer,uint64_t &bufferSize,OUT int64_t &position,uint64_t &blockReference);
		int getClusterDataInfo(uint8_t* &pdata,int64 &pts,uint8_t &flags,int &typeInside,int &sizeInside,int &is_keyframe,uint64_t blockReference);
		int outputVideoData(void* &buf,int &size,uint8_t* &pdata,int *lace_size,int is_keyframe);
		int outputAudioData(void* &buf,int &size,uint8_t* &pdata,int *lace_size);
		int buildIndexList(int is_keyframe,int type,int64 pts);

		int readNextData(int& type,void* &buf,int &size,int64& pts,
						 int& is_keyframe);
		int jumpedIsID(ByteIOContext *pb,int64_t length);


	private:
		ByteIOContext m_b;
		uint8_t* 	m_buffer;
		int 		m_curLaceIndex;
		int 		m_lenOfNalPaketSize ;
		int64_t     m_curDataPosition;
		int64_t     m_trickStartPosition;
		int         m_type;
		uint8_t*    m_pdata;
		int*        m_pLaceSize;
		int         m_laces;
		int64_t     m_lastKeyPosition;
		int64_t     m_timecode;

		ScanSpeed                   m_scanSpeed;
		AVFormatContext m_fcontext;
		MatroskaDemuxContext m_matroska;
		source_access_callbacks_t* m_source;

		int audioNum;
		int videoNum;
	};

//}

//#endif // ENABLE_MKV_SUPPORT


#endif // __MATROSKA_H__
