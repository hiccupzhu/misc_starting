
/*!
 ***********************************************************************
 *  \mainpage
 *     This is the H.264/AVC decoder reference software. For detailed documentation
 *     see the comments in each file.
 *
 *     The JM software web site is located at:
 *     http://iphome.hhi.de/suehring/tml
 *
 *     For bug reporting and known issues see:
 *     https://ipbt.hhi.de
 *
 *  \author
 *     The main contributors are listed in contributors.h
 *
 *  \version
 *     JM 14.2 (FRExt)
 *
 *  \note
 *     tags are used for document system "doxygen"
 *     available at http://www.doxygen.org
 */
/*!
 *  \file
 *     ldecod.c
 *  \brief
 *     H.264/AVC reference decoder project main()
 *  \author
 *     Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Inge Lille-Langøy       <inge.lille-langoy@telenor.com>
 *     - Rickard Sjoberg         <rickard.sjoberg@era.ericsson.se>
 *     - Stephan Wenger          <stewe@cs.tu-berlin.de>
 *     - Jani Lainema            <jani.lainema@nokia.com>
 *     - Sebastian Purreiter     <sebastian.purreiter@mch.siemens.de>
 *     - Byeong-Moon Jeon        <jeonbm@lge.com>
 *     - Gabi Blaettermann
 *     - Ye-Kui Wang             <wyk@ieee.org>
 *     - Valeri George           <george@hhi.de>
 *     - Karsten Suehring        <suehring@hhi.de>
 *
 ***********************************************************************
 */

#include "contributors.h"

#include <sys/stat.h>

#include "global.h"
#include "rtp.h"
#include "memalloc.h"
#include "mbuffer.h"
#include "leaky_bucket.h"
#include "fmo.h"
#include "annexb.h"
#include "output.h"
#include "cabac.h"
#include "parset.h"
#include "sei.h"
#include "erc_api.h"
#include "quant.h"

#define JM          "14 (FRExt)"
#define VERSION     "14.2"
#define EXT_VERSION "(FRExt)"

#define LOGFILE     "log.dec"
#define DATADECFILE "dataDec.txt"
#define TRACEFILE   "trace_dec.txt"

extern objectBuffer_t *erc_object_list;
extern ercVariables_t *erc_errorVar;
extern ColocatedParams *Co_located;
extern ColocatedParams *Co_located_JV[MAX_PLANE];  //!< Co_located to be used during 4:4:4 independent mode decoding

// I have started to move the inp and img structures into global variables.
// They are declared in the following lines.  Since inp is defined in conio.h
// and cannot be overridden globally, it is defined here as input
//
// Everywhere, params-> and img-> can now be used either globally or with
// the local override through the formal parameter mechanism

extern FILE* bits;
extern StorablePicture* dec_picture;

struct inp_par    *params;       //!< input parameters from input configuration file
struct snr_par    *snr;         //!< statistics
ImageParameters   *img;         //!< image parameters

int global_init_done = 0;

/*!
 ***********************************************************************
 * \brief
 *   print help message and exit
 ***********************************************************************
 */
void JMDecHelpExit (void)
{
  fprintf( stderr, "\n   ldecod [-h] {[defdec.cfg] | {[-p pocScale][-i bitstream.264]...[-o output.yuv] [-r reference.yuv] [-uv]}}\n\n"
    "## Parameters\n\n"

    "## Options\n"
    "   -h  :  prints function usage\n"
    "       :  parse <defdec.cfg> for decoder operation.\n"
    "   -i  :  Input file name. \n"
    "   -o  :  Output file name. If not specified default output is set as test_dec.yuv\n\n"
    "   -r  :  Reference file name. If not specified default output is set as test_rec.yuv\n\n"
    "   -p  :  Poc Scale. \n"
    "   -uv :  write chroma components for monochrome streams(4:2:0)\n"
    "   -lp :  By default the deblocking filter for High Intra-Only profile is off \n\t  regardless of the flags in the bitstream. In the presence of\n\t  this option, the loop filter usage will then be determined \n\t  by the flags and parameters in the bitstream.\n\n" 

    "## Supported video file formats\n"
    "   Input : .264 -> H.264 bitstream files. \n"
    "   Output: .yuv -> RAW file. Format depends on bitstream information. \n\n"

    "## Examples of usage:\n"
    "   ldecod\n"
    "   ldecod  -h\n"
    "   ldecod  default.cfg\n"
    "   ldecod  -i bitstream.264 -o output.yuv -r reference.yuv\n");

  exit(-1);
}


void Configure(int ac, char *av[])
{
  int CLcount;
  char *config_filename=NULL;
  CLcount = 1;


  strcpy(params->infile,"test.264");      //! set default bitstream name
  strcpy(params->outfile,"test_dec.yuv"); //! set default output file name
  strcpy(params->reffile,"test_rec.yuv"); //! set default reference file name
  params->FileFormat = PAR_OF_ANNEXB;
  params->ref_offset=0;
  params->poc_scale=2;
  params->silent = FALSE;
  params->intra_profile_deblocking = 0;

#ifdef _LEAKYBUCKET_
  params->R_decoder=500000;          //! Decoder rate
  params->B_decoder=104000;          //! Decoder buffer size
  params->F_decoder=73000;           //! Decoder initial delay
  strcpy(params->LeakyBucketParamFile,"leakybucketparam.cfg");    // file where Leaky Bucket params (computed by encoder) are stored
#endif

  if (ac==2)
  {
    if (0 == strncmp (av[1], "-h", 2))
    {
      JMDecHelpExit();
    }
    else if (0 == strncmp (av[1], "-s", 2))
    {
      params->silent = TRUE;
    }
    else
    {
      config_filename=av[1];
      init_conf(params, av[1]);
    }
    CLcount=2;
  }

  if (ac>=3)
  {
    if (0 == strncmp (av[1], "-i", 2))
    {
      strcpy(params->infile,av[2]);
      CLcount = 3;
    }
    if (0 == strncmp (av[1], "-h", 2))
    {
      JMDecHelpExit();
    }
    if (0 == strncmp (av[1], "-s", 2))
    {
      params->silent = TRUE;
    }
  }

  // Parse the command line

  while (CLcount < ac)
  {
    if (0 == strncmp (av[CLcount], "-h", 2))
    {
      JMDecHelpExit();
    }

    if (0 == strncmp (av[CLcount], "-s", 2))
    {
      params->silent = TRUE;
      CLcount ++;
    }

    if (0 == strncmp (av[CLcount], "-i", 2))  //! Input file
    {
      strcpy(params->infile,av[CLcount+1]);
      CLcount += 2;
    }
    else if (0 == strncmp (av[CLcount], "-o", 2))  //! Output File
    {
      strcpy(params->outfile,av[CLcount+1]);
      CLcount += 2;
    }
    else if (0 == strncmp (av[CLcount], "-r", 2))  //! Reference File
    {
      strcpy(params->reffile,av[CLcount+1]);
      CLcount += 2;
    }
    else if (0 == strncmp (av[CLcount], "-p", 2))  //! Poc Scale
    {
      sscanf (av[CLcount+1], "%d", &params->poc_scale);
      CLcount += 2;
    }
    else if (0 == strncmp (av[CLcount], "-uv", 3))  //! indicate UV writing for 4:0:0
    {
      params->write_uv = 1;
      CLcount ++;
    }
    else if (0 == strncmp (av[CLcount], "-lp", 3))  
    {
      params->intra_profile_deblocking = 1;
      CLcount ++;
    }
    else
    {
      //config_filename=av[CLcount];
      //init_conf(params, config_filename);
      snprintf(errortext, ET_SIZE, "Invalid syntax. Use ldecod -h for proper usage");
      error(errortext, 300);
    }
  }

#if TRACE
  if ((p_trace=fopen(TRACEFILE,"w"))==0)             // append new statistic at the end
  {
    snprintf(errortext, ET_SIZE, "Error open file %s!",TRACEFILE);
    error(errortext,500);
  }
#endif

  if ((p_out=open(params->outfile, OPENFLAGS_WRITE, OPEN_PERMISSIONS))==-1)
  {
    snprintf(errortext, ET_SIZE, "Error open file %s ",params->outfile);
    error(errortext,500);
  }
/*  if ((p_out2=fopen("out.yuv","wb"))==0)
  {
    snprintf(errortext, ET_SIZE, "Error open file %s ",params->outfile);
    error(errortext,500);
  }*/


  fprintf(stdout,"----------------------------- JM %s %s -----------------------------\n", VERSION, EXT_VERSION);
  fprintf(stdout," Decoder config file                    : %s \n",config_filename);
  fprintf(stdout,"--------------------------------------------------------------------------\n");
  fprintf(stdout," Input H.264 bitstream                  : %s \n",params->infile);
  fprintf(stdout," Output decoded YUV                     : %s \n",params->outfile);
  fprintf(stdout," Output status file                     : %s \n",LOGFILE);
  if ((p_ref=open(params->reffile,OPENFLAGS_READ))==-1)
  {
    fprintf(stdout," Input reference file                   : %s does not exist \n",params->reffile);
    fprintf(stdout,"                                          SNR values are not available\n");
  }
  else
    fprintf(stdout," Input reference file                   : %s \n",params->reffile);

  fprintf(stdout,"--------------------------------------------------------------------------\n");
#ifdef _LEAKYBUCKET_
  fprintf(stdout," Rate_decoder        : %8ld \n",params->R_decoder);
  fprintf(stdout," B_decoder           : %8ld \n",params->B_decoder);
  fprintf(stdout," F_decoder           : %8ld \n",params->F_decoder);
  fprintf(stdout," LeakyBucketParamFile: %s \n",params->LeakyBucketParamFile); // Leaky Bucket Param file
  calc_buffer(params);
  fprintf(stdout,"--------------------------------------------------------------------------\n");
#endif
  if (!params->silent)
  {
    fprintf(stdout,"POC must = frame# or field# for SNRs to be correct\n");
    fprintf(stdout,"--------------------------------------------------------------------------\n");
    fprintf(stdout,"  Frame          POC  Pic#   QP    SnrY     SnrU     SnrV   Y:U:V Time(ms)\n");
    fprintf(stdout,"--------------------------------------------------------------------------\n");
  }

}

/*!
 ***********************************************************************
 * \brief
 *    main function for TML decoder
 ***********************************************************************
 */
int main(int argc, char **argv)
{
  int i;
  int nplane;

  // allocate memory for the structures
  if ((params =  (struct inp_par *)calloc(1, sizeof(struct inp_par)))==NULL) no_mem_exit("main: params");
  if ((snr   =  (struct snr_par *)calloc(1, sizeof(struct snr_par)))==NULL) no_mem_exit("main: snr");
  if ((img   =  (ImageParameters *)calloc(1, sizeof(ImageParameters)))==NULL) no_mem_exit("main: img");

  Configure (argc, argv);

  init_old_slice();

  switch (params->FileFormat)
  {
  case 0:
    OpenBitstreamFile (params->infile);
    break;
  case 1:
    OpenRTPFile (params->infile);
    break;
  default:
    printf ("Unsupported file format %d, exit\n", params->FileFormat);
  }

  // Allocate Slice data struct
  malloc_slice(params,img);

  init(img);

  dec_picture = NULL;

  dpb.init_done = 0;
  g_nFrame = 0;

  init_out_buffer();  

  img->idr_psnr_number=params->ref_offset;
  img->psnr_number=0;

  img->number=0;
  img->type = I_SLICE;

  img->dec_ref_pic_marking_buffer = NULL;

  // B pictures
  Bframe_ctr = snr->frame_ctr = 0;

  // time for total decoding session
  tot_time = 0;

  // reference flag initialization
  for(i=0;i<17;i++)
  {
    ref_flag[i] = 1;
  }

  while (decode_one_frame(img, params, snr) != EOS)
    ;

  report(params, img, snr);
  free_slice(img);
  FmoFinit();
  free_global_buffers();

  flush_dpb();

#if (PAIR_FIELDS_IN_OUTPUT)
  flush_pending_output(p_out);
#endif

  CloseBitstreamFile();

  close(p_out);
//  fclose(p_out2);
  if (p_ref!=-1)
    close(p_ref);
#if TRACE
  fclose(p_trace);
#endif

  ercClose(erc_errorVar);

  CleanUpPPS();

  free_dpb();
  uninit_out_buffer();
  if( IS_INDEPENDENT(img) )
  {
    for( nplane=0; nplane<MAX_PLANE; nplane++ )
    {
      free_colocated(Co_located_JV[nplane]);   
    }
  }
  else
  {
    free_colocated(Co_located);
  }
  free (params);
  free (snr);
  free (img);

  //while( !kbhit() );

  return 0;
}


/*!
 ***********************************************************************
 * \brief
 *    Initilize some arrays
 ***********************************************************************
 */
void init(ImageParameters *img)  //!< image parameters
{
  img->oldFrameSizeInMbs = -1;

  imgY_ref  = NULL;
  imgUV_ref = NULL;

  img->recovery_point = 0;
  img->recovery_point_found = 0;
  img->recovery_poc = 0x7fffffff; /* set to a max value */

#if (ENABLE_OUTPUT_TONEMAPPING)
  init_tone_mapping_sei();
#endif
}

/*!
 ***********************************************************************
 * \brief
 *    Initialize FREXT variables
 ***********************************************************************
 */
void init_frext(ImageParameters *img)  //!< image parameters
{
  //pel bitdepth init
  img->bitdepth_luma_qp_scale   = 6 * (img->bitdepth_luma - 8);
  if(img->bitdepth_luma > img->bitdepth_chroma || active_sps->chroma_format_idc == YUV400)
    img->pic_unit_bitsize_on_disk = (img->bitdepth_luma > 8)? 16:8;
  else
    img->pic_unit_bitsize_on_disk = (img->bitdepth_chroma > 8)? 16:8;
  img->dc_pred_value_comp[0]    = 1<<(img->bitdepth_luma - 1);
  img->max_imgpel_value_comp[0] = (1<<img->bitdepth_luma) - 1;
  img->mb_size[0][0] = img->mb_size[0][1] = MB_BLOCK_SIZE;

  if (active_sps->chroma_format_idc != YUV400)
  {
    //for chrominance part
    img->bitdepth_chroma_qp_scale = 6 * (img->bitdepth_chroma - 8);
    img->dc_pred_value_comp[1]    = (1 << (img->bitdepth_chroma - 1));
    img->dc_pred_value_comp[2]    = img->dc_pred_value_comp[1];
    img->max_imgpel_value_comp[1] = (1 << img->bitdepth_chroma) - 1;
    img->max_imgpel_value_comp[2] = (1 << img->bitdepth_chroma) - 1;
    img->num_blk8x8_uv = (1 << active_sps->chroma_format_idc) & (~(0x1));
    img->num_uv_blocks = (img->num_blk8x8_uv >> 1);
    img->num_cdc_coeff = (img->num_blk8x8_uv << 1);
    img->mb_size[1][0] = img->mb_size[2][0] = img->mb_cr_size_x  = (active_sps->chroma_format_idc==YUV420 || active_sps->chroma_format_idc==YUV422)?  8 : 16;
    img->mb_size[1][1] = img->mb_size[2][1] = img->mb_cr_size_y  = (active_sps->chroma_format_idc==YUV444 || active_sps->chroma_format_idc==YUV422)? 16 :  8;
  }
  else
  {
    img->bitdepth_chroma_qp_scale = 0;
    img->max_imgpel_value_comp[1] = 0;
    img->max_imgpel_value_comp[2] = 0;
    img->num_blk8x8_uv = 0;
    img->num_uv_blocks = 0;
    img->num_cdc_coeff = 0;
    img->mb_size[1][0] = img->mb_size[2][0] = img->mb_cr_size_x  = 0;
    img->mb_size[1][1] = img->mb_size[2][1] = img->mb_cr_size_y  = 0;
  }
  img->mb_size_blk[0][0] = img->mb_size_blk[0][1] = img->mb_size[0][0] >> 2;
  img->mb_size_blk[1][0] = img->mb_size_blk[2][0] = img->mb_size[1][0] >> 2;
  img->mb_size_blk[1][1] = img->mb_size_blk[2][1] = img->mb_size[1][1] >> 2;

  img->mb_size_shift[0][0] = img->mb_size_shift[0][1] = CeilLog2_sf (img->mb_size[0][0]);
  img->mb_size_shift[1][0] = img->mb_size_shift[2][0] = CeilLog2_sf (img->mb_size[1][0]);
  img->mb_size_shift[1][1] = img->mb_size_shift[2][1] = CeilLog2_sf (img->mb_size[1][1]);

  init_qp_process(img);
}


/*!
 ************************************************************************
 * \brief
 *    Read parameters from configuration file
 *
 * \par Input:
 *    Name of configuration filename
 *
 * \par Output
 *    none
 ************************************************************************
 */
void init_conf(struct inp_par *inp, char *config_filename)
{
  FILE *fd;
  int NAL_mode;

  // picture error concealment
  long int temp;
  char tempval[100];

  // read the decoder configuration file
  if((fd=fopen(config_filename,"r")) == NULL)
  {
    snprintf(errortext, ET_SIZE, "Error: Control file %s not found\n",config_filename);
    error(errortext, 300);
  }

  fscanf(fd,"%s",inp->infile);                // H.264 compressed input bitstream
  fscanf(fd,"%*[^\n]");

  fscanf(fd,"%s",inp->outfile);               // RAW (YUV/RGB) output file
  fscanf(fd,"%*[^\n]");

  fscanf(fd,"%s",inp->reffile);               // reference file
  fscanf(fd,"%*[^\n]");

  fscanf(fd,"%d",&(inp->write_uv));           // write UV in YUV 4:0:0 mode
  fscanf(fd,"%*[^\n]");

  fscanf(fd,"%d",&(NAL_mode));                // NAL mode
  fscanf(fd,"%*[^\n]");

  switch(NAL_mode)
  {
  case 0:
    inp->FileFormat = PAR_OF_ANNEXB;
    break;
  case 1:
    inp->FileFormat = PAR_OF_RTP;
    break;
  default:
    snprintf(errortext, ET_SIZE, "NAL mode %i is not supported", NAL_mode);
    error(errortext,400);
  }

  fscanf(fd,"%d,",&inp->ref_offset);   // offset used for SNR computation
  fscanf(fd,"%*[^\n]");

  fscanf(fd,"%d,",&inp->poc_scale);   // offset used for SNR computation
  fscanf(fd,"%*[^\n]");


  if (inp->poc_scale < 1 || inp->poc_scale > 10)
  {
    snprintf(errortext, ET_SIZE, "Poc Scale is %d. It has to be within range 1 to 10",inp->poc_scale);
    error(errortext,1);
  }

  inp->write_uv=1;

  // picture error concealment
  img->conceal_mode = inp->conceal_mode = 0;
  img->ref_poc_gap = inp->ref_poc_gap = 2;
  img->poc_gap = inp->poc_gap = 2;

#ifdef _LEAKYBUCKET_
  fscanf(fd,"%ld,",&inp->R_decoder);             // Decoder rate
  fscanf(fd, "%*[^\n]");
  fscanf(fd,"%ld,",&inp->B_decoder);             // Decoder buffer size
  fscanf(fd, "%*[^\n]");
  fscanf(fd,"%ld,",&inp->F_decoder);             // Decoder initial delay
  fscanf(fd, "%*[^\n]");
  fscanf(fd,"%s",inp->LeakyBucketParamFile);    // file where Leaky Bucket params (computed by encoder) are stored
  fscanf(fd,"%*[^\n]");
#endif

  /* since error concealment parameters are added at the end of
  decoder conf file we need to read the leakybucket params to get to
  those parameters */
#ifndef _LEAKYBUCKET_
  fscanf(fd,"%ld,",&temp);
  fscanf(fd, "%*[^\n]");
  fscanf(fd,"%ld,",&temp);
  fscanf(fd, "%*[^\n]");
  fscanf(fd,"%ld,",&temp);
  fscanf(fd, "%*[^\n]");
  fscanf(fd,"%s",tempval);
  fscanf(fd,"%*[^\n]");
#endif

  fscanf(fd,"%d",&inp->conceal_mode);   // Mode of Error Concealment
  fscanf(fd,"%*[^\n]");
  img->conceal_mode = inp->conceal_mode;
  fscanf(fd,"%d",&inp->ref_poc_gap);   // POC gap depending on pattern
  fscanf(fd,"%*[^\n]");
  img->ref_poc_gap = inp->ref_poc_gap;
  fscanf(fd,"%d",&inp->poc_gap);   // POC gap between consecutive frames in display order
  fscanf(fd,"%*[^\n]");
  img->poc_gap = inp->poc_gap;
  fscanf(fd,"%d,",&inp->silent);     // use silent decode mode
  fscanf(fd,"%*[^\n]");
  fscanf(fd,"%d,",&inp->intra_profile_deblocking);     // use deblocking filter in intra only profile
  fscanf(fd,"%*[^\n]");

  fclose (fd);
}

/*!
 ************************************************************************
 * \brief
 *    Reports the gathered information to appropriate outputs
 *
 * \par Input:
 *    struct inp_par *inp,
 *    ImageParameters *img,
 *    struct snr_par *stat
 *
 * \par Output:
 *    None
 ************************************************************************
 */
void report(struct inp_par *inp, ImageParameters *img, struct snr_par *snr)
{
  #define OUTSTRING_SIZE 255
  char string[OUTSTRING_SIZE];
  FILE *p_log;
  char yuv_formats[4][4]= { {"400"}, {"420"}, {"422"}, {"444"} };

#ifndef WIN32
  time_t  now;
  struct tm *l_time;
#else
  char timebuf[128];
#endif

  if (params->silent == FALSE)
  {
    fprintf(stdout,"-------------------- Average SNR all frames ------------------------------\n");
    fprintf(stdout," SNR Y(dB)           : %5.2f\n",snr->snra[0]);
    fprintf(stdout," SNR U(dB)           : %5.2f\n",snr->snra[1]);
    fprintf(stdout," SNR V(dB)           : %5.2f\n",snr->snra[2]);
    fprintf(stdout," Total decoding time : %.3f sec (%.3f fps)\n",tot_time*0.001,(snr->frame_ctr ) * 1000.0 / tot_time);
    fprintf(stdout,"--------------------------------------------------------------------------\n");
    fprintf(stdout," Exit JM %s decoder, ver %s ",JM, VERSION);
    fprintf(stdout,"\n");
  }
  else
  {
    fprintf(stdout,"\n----------------------- Decoding Completed -------------------------------\n");
    fprintf(stdout," Total decoding time : %.3f sec (%.3f fps)\n",tot_time*0.001, (snr->frame_ctr) * 1000.0 / tot_time);
    fprintf(stdout,"--------------------------------------------------------------------------\n");
    fprintf(stdout," Exit JM %s decoder, ver %s ",JM, VERSION);
    fprintf(stdout,"\n");
  }

  // write to log file

  snprintf(string, OUTSTRING_SIZE, "%s", LOGFILE);
  if ((p_log=fopen(string,"r"))==0)                    // check if file exist
  {
    if ((p_log=fopen(string,"a"))==0)
    {
      snprintf(errortext, ET_SIZE, "Error open file %s for appending",string);
      error(errortext, 500);
    }
    else                                              // Create header to new file
    {
      fprintf(p_log," -------------------------------------------------------------------------------------------------------------------\n");
      fprintf(p_log,"|  Decoder statistics. This file is made first time, later runs are appended               |\n");
      fprintf(p_log," ------------------------------------------------------------------------------------------------------------------- \n");
      fprintf(p_log,"|   ver  | Date  | Time  |    Sequence        |#Img| Format  | YUV |Coding|SNRY 1|SNRU 1|SNRV 1|SNRY N|SNRU N|SNRV N|\n");
      fprintf(p_log," -------------------------------------------------------------------------------------------------------------------\n");
    }
  }
  else
  {
    fclose(p_log);
    p_log=fopen(string,"a");                    // File exist,just open for appending
  }

  fprintf(p_log,"|%s/%-4s", VERSION, EXT_VERSION);

#ifdef WIN32
  _strdate( timebuf );
  fprintf(p_log,"| %1.5s |",timebuf );

  _strtime( timebuf);
  fprintf(p_log," % 1.5s |",timebuf);
#else
  now = time ((time_t *) NULL); // Get the system time and put it into 'now' as 'calender time'
  time (&now);
  l_time = localtime (&now);
  strftime (string, sizeof string, "%d-%b-%Y", l_time);
  fprintf(p_log,"| %1.5s |",string );

  strftime (string, sizeof string, "%H:%M:%S", l_time);
  fprintf(p_log,"| %1.5s |",string );
#endif

  fprintf(p_log,"%20.20s|",inp->infile);

  fprintf(p_log,"%3d |",img->number);
  fprintf(p_log,"%4dx%-4d|", img->width, img->height);
  fprintf(p_log," %s |", &(yuv_formats[img->yuv_format][0]));

  if (active_pps)
  {
    if (active_pps->entropy_coding_mode_flag == UVLC)
      fprintf(p_log," CAVLC|");
    else
      fprintf(p_log," CABAC|");
  }


  fprintf(p_log,"%6.3f|",snr->snr1[0]);
  fprintf(p_log,"%6.3f|",snr->snr1[1]);
  fprintf(p_log,"%6.3f|",snr->snr1[2]);
  fprintf(p_log,"%6.3f|",snr->snra[0]);
  fprintf(p_log,"%6.3f|",snr->snra[1]);
  fprintf(p_log,"%6.3f|\n",snr->snra[2]);

  fclose(p_log);

  snprintf(string, OUTSTRING_SIZE,"%s", DATADECFILE);
  p_log=fopen(string,"a");

  if(Bframe_ctr != 0) // B picture used
  {
    fprintf(p_log, "%3d %2d %2d %2.2f %2.2f %2.2f %5d "
      "%2.2f %2.2f %2.2f %5d "
      "%2.2f %2.2f %2.2f %5d %.3f\n",
      img->number, 0, img->qp,
      snr->snr1[0],
      snr->snr1[1],
      snr->snr1[2],
      0,
      0.0,
      0.0,
      0.0,
      0,
      snr->snra[0],
      snr->snra[1],
      snr->snra[2],
      0,
      (double)0.001*tot_time/(img->number+Bframe_ctr-1));
  }
  else
  {
    fprintf(p_log, "%3d %2d %2d %2.2f %2.2f %2.2f %5d "
      "%2.2f %2.2f %2.2f %5d "
      "%2.2f %2.2f %2.2f %5d %.3f\n",
      img->number, 0, img->qp,
      snr->snr1[0],
      snr->snr1[1],
      snr->snr1[2],
      0,
      0.0,
      0.0,
      0.0,
      0,
      snr->snra[0],
      snr->snra[1],
      snr->snra[2],
      0,
      (double)0.001*tot_time/img->number);
  }
  fclose(p_log);
}

/*!
 ************************************************************************
 * \brief
 *    Allocates a stand-alone partition structure.  Structure should
 *    be freed by FreePartition();
 *    data structures
 *
 * \par Input:
 *    n: number of partitions in the array
 * \par return
 *    pointer to DataPartition Structure, zero-initialized
 ************************************************************************
 */

DataPartition *AllocPartition(int n)
{
  DataPartition *partArr, *dataPart;
  int i;

  partArr = (DataPartition *) calloc(n, sizeof(DataPartition));
  if (partArr == NULL)
  {
    snprintf(errortext, ET_SIZE, "AllocPartition: Memory allocation for Data Partition failed");
    error(errortext, 100);
  }

  for (i=0; i<n; i++) // loop over all data partitions
  {
    dataPart = &(partArr[i]);
    dataPart->bitstream = (Bitstream *) calloc(1, sizeof(Bitstream));
    if (dataPart->bitstream == NULL)
    {
      snprintf(errortext, ET_SIZE, "AllocPartition: Memory allocation for Bitstream failed");
      error(errortext, 100);
    }
    dataPart->bitstream->streamBuffer = (byte *) calloc(MAX_CODED_FRAME_SIZE, sizeof(byte));
    if (dataPart->bitstream->streamBuffer == NULL)
    {
      snprintf(errortext, ET_SIZE, "AllocPartition: Memory allocation for streamBuffer failed");
      error(errortext, 100);
    }
  }
  return partArr;
}




/*!
 ************************************************************************
 * \brief
 *    Frees a partition structure (array).
 *
 * \par Input:
 *    Partition to be freed, size of partition Array (Number of Partitions)
 *
 * \par return
 *    None
 *
 * \note
 *    n must be the same as for the corresponding call of AllocPartition
 ************************************************************************
 */


void FreePartition (DataPartition *dp, int n)
{
  int i;

  assert (dp != NULL);
  assert (dp->bitstream != NULL);
  assert (dp->bitstream->streamBuffer != NULL);
  for (i=0; i<n; i++)
  {
    free (dp[i].bitstream->streamBuffer);
    free (dp[i].bitstream);
  }
  free (dp);
}


/*!
 ************************************************************************
 * \brief
 *    Allocates the slice structure along with its dependent
 *    data structures
 *
 * \par Input:
 *    Input Parameters struct inp_par *inp,  ImageParameters *img
 ************************************************************************
 */
void malloc_slice(struct inp_par *inp, ImageParameters *img)
{
  Slice *currSlice;

  img->currentSlice = (Slice *) calloc(1, sizeof(Slice));
  if ( (currSlice = img->currentSlice) == NULL)
  {
    snprintf(errortext, ET_SIZE, "Memory allocation for Slice datastruct in NAL-mode %d failed", inp->FileFormat);
    error(errortext,100);
  }
  //  img->currentSlice->rmpni_buffer=NULL;
  //! you don't know whether we do CABAC hre, hence initialize CABAC anyway
  // if (inp->symbol_mode == CABAC)
  if (1)
  {
    // create all context models
    currSlice->mot_ctx = create_contexts_MotionInfo();
    currSlice->tex_ctx = create_contexts_TextureInfo();
  }
  currSlice->max_part_nr = 3;  //! assume data partitioning (worst case) for the following mallocs()
  currSlice->partArr = AllocPartition(currSlice->max_part_nr);
}


/*!
 ************************************************************************
 * \brief
 *    Memory frees of the Slice structure and of its dependent
 *    data structures
 *
 * \par Input:
 *    Input Parameters struct inp_par *inp,  ImageParameters *img
 ************************************************************************
 */
void free_slice(ImageParameters *img)
{
  Slice *currSlice = img->currentSlice;

  FreePartition (currSlice->partArr, 3);
  // if (inp->symbol_mode == CABAC)
  if (1)
  {
    // delete all context models
    delete_contexts_MotionInfo(currSlice->mot_ctx);
    delete_contexts_TextureInfo(currSlice->tex_ctx);
  }
  free(img->currentSlice);

  currSlice = NULL;
}

/*!
 ************************************************************************
 * \brief
 *    Dynamic memory allocation of frame size related global buffers
 *    buffers are defined in global.h, allocated memory must be freed in
 *    void free_global_buffers()
 *
 *  \par Input:
 *    Input Parameters struct inp_par *inp, Image Parameters ImageParameters *img
 *
 *  \par Output:
 *     Number of allocated bytes
 ***********************************************************************
 */
int init_global_buffers(void)
{
  int memory_size=0;
  int i;

  if (global_init_done)
  {
    free_global_buffers();
  }

  // allocate memory for reference frame in find_snr
  memory_size += get_mem2Dpel(&imgY_ref, img->height, img->width);

  if (active_sps->chroma_format_idc != YUV400)
    memory_size += get_mem3Dpel(&imgUV_ref, 2, img->height_cr, img->width_cr);
  else
    imgUV_ref=NULL;

  // allocate memory in structure img
  if( IS_INDEPENDENT(img) )
  {
    for( i=0; i<MAX_PLANE; i++ )
    {
      if(((img->mb_data_JV[i]) = (Macroblock *) calloc(img->FrameSizeInMbs, sizeof(Macroblock))) == NULL)
        no_mem_exit("init_global_buffers: img->mb_data");
    }
    img->mb_data = NULL;
  }
  else
  {
    if(((img->mb_data) = (Macroblock *) calloc(img->FrameSizeInMbs, sizeof(Macroblock))) == NULL)
      no_mem_exit("init_global_buffers: img->mb_data");
  }

  if(((img->intra_block) = (int*)calloc(img->FrameSizeInMbs, sizeof(int))) == NULL)
    no_mem_exit("init_global_buffers: img->intra_block");

  memory_size += get_mem2Dint(&PicPos,img->FrameSizeInMbs + 1,2);  //! Helper array to access macroblock positions. We add 1 to also consider last MB.

  for (i = 0; i < (int) img->FrameSizeInMbs + 1;i++)
  {
    PicPos[i][0] = (i % img->PicWidthInMbs);
    PicPos[i][1] = (i / img->PicWidthInMbs);
  }

  memory_size += get_mem2D(&(img->ipredmode), 4*img->FrameHeightInMbs, 4*img->PicWidthInMbs);

  memory_size += get_mem3Dint(&(img->wp_weight), 2, MAX_REFERENCE_PICTURES, 3);
  memory_size += get_mem3Dint(&(img->wp_offset), 6, MAX_REFERENCE_PICTURES, 3);
  memory_size += get_mem4Dint(&(img->wbp_weight), 6, MAX_REFERENCE_PICTURES, MAX_REFERENCE_PICTURES, 3);

  // CAVLC mem
  memory_size += get_mem4Dint(&(img->nz_coeff), img->FrameSizeInMbs, 3, BLOCK_SIZE, BLOCK_SIZE);

  memory_size += get_mem2Dint(&(img->siblock), img->FrameHeightInMbs, img->PicWidthInMbs);

  global_init_done = 1;

  img->oldFrameSizeInMbs = img->FrameSizeInMbs;

  return (memory_size);
}

/*!
 ************************************************************************
 * \brief
 *    Free allocated memory of frame size related global buffers
 *    buffers are defined in global.h, allocated memory is allocated in
 *    int init_global_buffers()
 *
 * \par Input:
 *    Input Parameters struct inp_par *inp, Image Parameters ImageParameters *img
 *
 * \par Output:
 *    none
 *
 ************************************************************************
 */
void free_global_buffers(void)
{
  free_mem2Dpel (imgY_ref);
  if (imgUV_ref)
    free_mem3Dpel (imgUV_ref);

  // CAVLC free mem
  free_mem4Dint(img->nz_coeff);

  free_mem2Dint(img->siblock);

  // free mem, allocated for structure img
  if (img->mb_data != NULL)
    free(img->mb_data);

  free_mem2Dint(PicPos);

  free (img->intra_block);
  free_mem2D(img->ipredmode);

  free_mem3Dint(img->wp_weight);
  free_mem3Dint(img->wp_offset);
  free_mem4Dint(img->wbp_weight);

  global_init_done = 0;

}

