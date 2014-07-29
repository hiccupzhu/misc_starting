/*!
 *  \file
 *     report.c
 *  \brief
 *     Report related files()
 *  \author
 *   Main contributors (see contributors.h for copyright, address and affiliation details)
 *   - Karsten Suehring                <suehring@hhi.de>
 *   - Alexis Michael Tourapis         <alexismt@ieee.org>
 ***********************************************************************
 */

#include "contributors.h"

#include <time.h>
#include <math.h>
#include <sys/timeb.h>
#include "global.h"

#include "context_ini.h"
#include "explicit_gop.h"
#include "filehandle.h"
#include "fmo.h"
#include "image.h"
#include "intrarefresh.h"
#include "leaky_bucket.h"
#include "me_epzs.h"
#include "output.h"
#include "parset.h"
#include "report.h"


StatParameters   statistics,  *stats = &statistics;
DistortionParams distortions, *dist  = &distortions;
static char DistortionType[3][20] = {"SAD", "SSE", "Hadamard SAD"};

extern int frame_statistic_start;
extern ColocatedParams *Co_located;
extern ColocatedParams *Co_located_JV[MAX_PLANE];  //!< Co_located to be used during 4:4:4 independent mode encoding
extern void Clear_Motion_Search_Module (void);
void   report_log_mode(InputParameters *params, StatParameters *stats, int64 bit_use[NUM_SLICE_TYPES][2]);
/*!
 ************************************************************************
 * \brief
 *    Reports frame statistical data to a stats file
 ************************************************************************
 */
void report_frame_statistic()
{
  FILE *p_stat_frm = NULL;
  static int64 last_bit_ctr_n = 0;
  int i;
  char name[30];
  int bitcounter;
  StatParameters *cur_stats = &enc_picture->stats;

#ifndef WIN32
  time_t now;
  struct tm *l_time;
  char string[1000];
#else
  char timebuf[128];
#endif

  // write to log file
  if ((p_stat_frm = fopen("stat_frame.dat", "r")) == 0)            // check if file exists
  {
    if ((p_stat_frm = fopen("stat_frame.dat", "a")) == NULL)       // append new statistic at the end
    {
      snprintf(errortext, ET_SIZE, "Error open file %s  \n", "stat_frame.dat.dat");
      error(errortext, 500);
    }
    else                                            // Create header for new log file
    {
      fprintf(p_stat_frm, " --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \n");
      fprintf(p_stat_frm, "|            Encoder statistics. This file is generated during first encoding session, new sessions will be appended                                                                                                                                                                                                                                                                                                                                                              |\n");
      fprintf(p_stat_frm, " --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \n");
    }
  }
  else
  {
    fclose (p_stat_frm);
    if ((p_stat_frm = fopen("stat_frame.dat", "a")) == NULL)       // File exists, just open for appending
    {
      snprintf(errortext, ET_SIZE, "Error open file %s  \n", "stat_frame.dat.dat");
      error(errortext, 500);
    }
  }

  if (frame_statistic_start)
  {
    fprintf(p_stat_frm, "|     ver     | Date  | Time  |    Sequence                  |Frm | QP |P/MbInt|   Bits   |  SNRY  |  SNRU  |  SNRV  |  I4  |  I8  | I16  | IC0  | IC1  | IC2  | IC3  | PI4  | PI8  | PI16 |  P0  |  P1  |  P2  |  P3  | P1*4*| P1*8*| P2*4*| P2*8*| P3*4*| P3*8*|  P8  | P8:4 | P4*4*| P4*8*| P8:5 | P8:6 | P8:7 | BI4  | BI8  | BI16 |  B0  |  B1  |  B2  |  B3  | B0*4*| B0*8*| B1*4*| B1*8*| B2*4*| B2*8*| B3*4*| B3*8*|  B8  | B8:0 |B80*4*|B80*8*| B8:4 | B4*4*| B4*8*| B8:5 | B8:6 | B8:7 |\n");
    fprintf(p_stat_frm, " ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ \n");
  }

  //report
  fprintf(p_stat_frm, "|%4s/%s", VERSION, EXT_VERSION);

#ifdef WIN32
  _strdate( timebuf );
  fprintf(p_stat_frm, "| %1.5s |", timebuf);

  _strtime( timebuf);
  fprintf(p_stat_frm, " % 1.5s |", timebuf);
#else
  now = time ((time_t *) NULL); // Get the system time and put it into 'now' as 'calender time'
  time (&now);
  l_time = localtime (&now);
  strftime (string, sizeof string, "%d-%b-%Y", l_time);
  fprintf(p_stat_frm, "| %1.5s |", string );

  strftime (string, sizeof string, "%H:%M:%S", l_time);
  fprintf(p_stat_frm, " %1.5s |", string);
#endif

  for (i=0;i<30;i++)
    name[i]=params->infile[i + imax(0,(int) (strlen(params->infile)- 30))]; // write last part of path, max 30 chars

  fprintf(p_stat_frm, "%30.30s|", name);
  fprintf(p_stat_frm, "%3d |", frame_no);
  fprintf(p_stat_frm, "%3d |", img->qp);
  fprintf(p_stat_frm, "  %d/%d  |", params->PicInterlace, params->MbInterlace);


  if (img->frm_number == 0 && img->frame_num == 0)
  {
    bitcounter = (int) stats->bit_counter[I_SLICE];
  }
  else
  {
    bitcounter = (int) (stats->bit_ctr_n - last_bit_ctr_n);
    last_bit_ctr_n = stats->bit_ctr_n;
  }

  //report bitrate
  fprintf(p_stat_frm, " %9d|", bitcounter);

  //report snr's  
  fprintf(p_stat_frm, " %2.4f| %2.4f| %2.4f|", dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2]);

  //report modes
  //I-Modes
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[I_SLICE][I4MB ]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[I_SLICE][I8MB ]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[I_SLICE][I16MB]);

  //chroma intra mode
  fprintf(p_stat_frm, " %5d|", cur_stats->intra_chroma_mode[0]);
  fprintf(p_stat_frm, " %5d|", cur_stats->intra_chroma_mode[1]);
  fprintf(p_stat_frm, " %5d|", cur_stats->intra_chroma_mode[2]);
  fprintf(p_stat_frm, " %5d|", cur_stats->intra_chroma_mode[3]);

  //P-Modes
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[P_SLICE][I4MB ]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[P_SLICE][I8MB ]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[P_SLICE][I16MB]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[P_SLICE][0    ]);

  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[P_SLICE][1    ]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[P_SLICE][2    ]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[P_SLICE][3    ]);
  
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[P_SLICE][1][0]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[P_SLICE][1][1]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[P_SLICE][2][0]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[P_SLICE][2][1]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[P_SLICE][3][0]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[P_SLICE][3][1]);

  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[P_SLICE][P8x8 ]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[P_SLICE][4    ]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use_transform[P_SLICE][4][0]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use_transform[P_SLICE][4][1]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[P_SLICE][5    ]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[P_SLICE][6    ]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[P_SLICE][7    ]);

  //B-Modes
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[B_SLICE][I4MB ]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[B_SLICE][I8MB ]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[B_SLICE][I16MB]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[B_SLICE][0    ]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[B_SLICE][1    ]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[B_SLICE][2    ]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[B_SLICE][3    ]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use_transform[B_SLICE][0][0]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use_transform[B_SLICE][0][1]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use_transform[B_SLICE][1][0]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use_transform[B_SLICE][1][1]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use_transform[B_SLICE][2][0]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use_transform[B_SLICE][2][1]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use_transform[B_SLICE][3][0]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use_transform[B_SLICE][3][1]);

  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[B_SLICE][P8x8]);
  fprintf(p_stat_frm, " %d|", (cur_stats->b8_mode_0_use [B_SLICE][0] + cur_stats->b8_mode_0_use [B_SLICE][1]));
  fprintf(p_stat_frm, " %5d|", cur_stats->b8_mode_0_use [B_SLICE][0]);
  fprintf(p_stat_frm, " %5d|", cur_stats->b8_mode_0_use [B_SLICE][1]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[B_SLICE][4   ]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use_transform[B_SLICE][4][0]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use_transform[B_SLICE][4][1]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[B_SLICE][5   ]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[B_SLICE][6   ]);
  fprintf(p_stat_frm, " %5" FORMAT_OFF_T  "|", cur_stats->mode_use[B_SLICE][7   ]);

  fprintf(p_stat_frm, "\n");

  //save the last results
  frame_statistic_start = 0;
  fclose(p_stat_frm);
}


double report_slice_pred_stats(FILE *p_stat, StatParameters *stats, int slice_type, double bit_use, char *slice_name)
{
  fprintf(p_stat,"\n ---------------------|----------------|-----------------|");
  fprintf(p_stat,"\n   %8s           |   Mode used    | MotionInfo bits |", slice_name);
  fprintf(p_stat,"\n ---------------------|----------------|-----------------|");
  fprintf(p_stat,"\n Mode  0  (copy)      |  %5" FORMAT_OFF_T  "         |    %8.2f     |", stats->mode_use[slice_type][0   ], (double)stats->bit_use_mode[slice_type][0   ] / bit_use);
  fprintf(p_stat,"\n Mode  1  (16x16)     |  %5" FORMAT_OFF_T  "         |    %8.2f     |", stats->mode_use[slice_type][1   ], (double)stats->bit_use_mode[slice_type][1   ] / bit_use);
  fprintf(p_stat,"\n Mode  2  (16x8)      |  %5" FORMAT_OFF_T  "         |    %8.2f     |", stats->mode_use[slice_type][2   ], (double)stats->bit_use_mode[slice_type][2   ] / bit_use);
  fprintf(p_stat,"\n Mode  3  (8x16)      |  %5" FORMAT_OFF_T  "         |    %8.2f     |", stats->mode_use[slice_type][3   ], (double)stats->bit_use_mode[slice_type][3   ] / bit_use);
  fprintf(p_stat,"\n Mode  4  (8x8)       |  %5" FORMAT_OFF_T  "         |    %8.2f     |", stats->mode_use[slice_type][P8x8], (double)stats->bit_use_mode[slice_type][P8x8] / bit_use);
  fprintf(p_stat,"\n Mode  5  intra 4x4   |  %5" FORMAT_OFF_T  "         |-----------------|", stats->mode_use[slice_type][I4MB]);
  fprintf(p_stat,"\n Mode  6  intra 8x8   |  %5" FORMAT_OFF_T  "         |", stats->mode_use[slice_type][I8MB]);
  fprintf(p_stat,"\n Mode  7+ intra 16x16 |  %5" FORMAT_OFF_T  "         |", stats->mode_use[slice_type][I16MB]);
  fprintf(p_stat,"\n Mode     intra IPCM  |  %5" FORMAT_OFF_T  "         |", stats->mode_use[slice_type][IPCM ]);

  return (double)(stats->bit_use_mode[slice_type][0] + stats->bit_use_mode[slice_type][1] + stats->bit_use_mode[slice_type][2]
  + stats->bit_use_mode[slice_type][3] + stats->bit_use_mode[slice_type][P8x8]) / bit_use;
}

/*!
 ***********************************************************************
 * \brief
 *    Terminates and reports statistics on error.
 *
 ***********************************************************************
 */
void report_stats_on_error(void)
{
  params->no_frames = img->frm_number;
  free_encoder_memory(img);
  exit (-1);
}


void report_stats(InputParameters *params, StatParameters *stats, int64 bit_use[NUM_SLICE_TYPES][2], float frame_rate)
{
  FILE *p_stat;                    //!< status file for the last encoding session
  double mean_motion_info_bit_use[NUM_SLICE_TYPES] = {0.0};
  int i;

  if (strlen(params->StatsFile) == 0)
    strcpy (params->StatsFile,"stats.dat");

  if ((p_stat = fopen(params->StatsFile, "wt")) == 0)
  {
    snprintf(errortext, ET_SIZE, "Error open file %s", params->StatsFile);
    error(errortext, 500);
  }

  fprintf(p_stat," -------------------------------------------------------------- \n");
  fprintf(p_stat,"  This file contains statistics for the last encoded sequence   \n");
  fprintf(p_stat," -------------------------------------------------------------- \n");
  fprintf(p_stat,   " Sequence                     : %s\n", params->infile);
  fprintf(p_stat,   " No.of coded pictures         : %4d\n", stats->frame_counter);
  fprintf(p_stat,   " Freq. for encoded bitstream  : %4.0f\n", frame_rate);

  fprintf(p_stat,   " I Slice Bitrate(kb/s)        : %6.2f\n", stats->bitrate_st[I_SLICE] / 1000);
  fprintf(p_stat,   " P Slice Bitrate(kb/s)        : %6.2f\n", stats->bitrate_st[P_SLICE] / 1000);
  fprintf(p_stat,   " B Slice Bitrate(kb/s)        : %6.2f\n", stats->bitrate_st[B_SLICE] / 1000);
  fprintf(p_stat,   " Total Bitrate(kb/s)          : %6.2f\n", stats->bitrate / 1000);

  for (i = 0; i < 3; i++)
  {
    fprintf(p_stat," ME Level %1d Metric            : %s\n", i, DistortionType[params->MEErrorMetric[i]]);
  }
  fprintf(p_stat," Mode Decision Metric         : %s\n", DistortionType[params->ModeDecisionMetric]);

  switch ( params->ChromaMEEnable )
  {
  case 1:
    fprintf(p_stat," ME for components            : YCbCr\n");
    break;
  default:
    fprintf(p_stat," ME for components            : Y\n");
    break;
  }

  fprintf(p_stat,  " Image format                 : %dx%d\n", params->output.width, params->output.height);

  if (params->intra_upd)
    fprintf(p_stat," Error robustness             : On\n");
  else
    fprintf(p_stat," Error robustness             : Off\n");

  fprintf(p_stat,  " Search range                 : %d\n", params->search_range);

  fprintf(p_stat,   " Total number of references   : %d\n", params->num_ref_frames);
  fprintf(p_stat,   " References for P slices      : %d\n", params->P_List0_refs ? params->P_List0_refs : params->num_ref_frames);

  if (stats->frame_ctr[B_SLICE]!=0)
  {
    fprintf(p_stat, " List0 refs for B slices      : %d\n", params->B_List0_refs ? params->B_List0_refs : params->num_ref_frames);
    fprintf(p_stat, " List1 refs for B slices      : %d\n", params->B_List1_refs ? params->B_List1_refs : params->num_ref_frames);
  }

  fprintf(p_stat,   " Profile/Level IDC            : (%d,%d)\n", params->ProfileIDC, params->LevelIDC);
  if (params->symbol_mode == CAVLC)
    fprintf(p_stat,   " Entropy coding method        : CAVLC\n");
  else
    fprintf(p_stat,   " Entropy coding method        : CABAC\n");

  if (params->MbInterlace)
    fprintf(p_stat, " MB Field Coding : On \n");

  if (params->SearchMode == EPZS)
    EPZSOutputStats(params, p_stat, 1);

  if (params->full_search == 2)
    fprintf(p_stat," Search range restrictions    : none\n");
  else if (params->full_search == 1)
    fprintf(p_stat," Search range restrictions    : older reference frames\n");
  else
    fprintf(p_stat," Search range restrictions    : smaller blocks and older reference frames\n");

  if (params->rdopt)
    fprintf(p_stat," RD-optimized mode decision   : used\n");
  else
    fprintf(p_stat," RD-optimized mode decision   : not used\n");

  fprintf(p_stat,"\n ---------------------|----------------|---------------|");
  fprintf(p_stat,"\n     Item             |     Intra      |   All frames  |");
  fprintf(p_stat,"\n ---------------------|----------------|---------------|");
  fprintf(p_stat,"\n SNR Y(dB)            |");
  fprintf(p_stat," %5.2f          |", dist->metric[PSNR].avslice[I_SLICE][0]);
  fprintf(p_stat," %5.2f         |", dist->metric[PSNR].average[0]);
  fprintf(p_stat,"\n SNR U/V (dB)         |");
  fprintf(p_stat," %5.2f/%5.2f    |", dist->metric[PSNR].avslice[I_SLICE][1], dist->metric[PSNR].avslice[I_SLICE][2]);
  fprintf(p_stat," %5.2f/%5.2f   |", dist->metric[PSNR].average[1], dist->metric[PSNR].average[2]);
  fprintf(p_stat,"\n ---------------------|----------------|---------------|");
  fprintf(p_stat,"\n");

  fprintf(p_stat,"\n ---------------------|----------------|---------------|---------------|");
  fprintf(p_stat,"\n     SNR              |        I       |       P       |       B       |");
  fprintf(p_stat,"\n ---------------------|----------------|---------------|---------------|");
  fprintf(p_stat,"\n SNR Y(dB)            |      %5.3f    |     %5.3f    |     %5.3f    |",
    dist->metric[PSNR].avslice[I_SLICE][0], dist->metric[PSNR].avslice[P_SLICE][0], dist->metric[PSNR].avslice[B_SLICE][0]);
  fprintf(p_stat,"\n SNR U(dB)            |      %5.3f    |     %5.3f    |     %5.3f    |",
    dist->metric[PSNR].avslice[I_SLICE][1], dist->metric[PSNR].avslice[P_SLICE][1], dist->metric[PSNR].avslice[B_SLICE][1]);
  fprintf(p_stat,"\n SNR V(dB)            |      %5.3f    |     %5.3f    |     %5.3f    |",
    dist->metric[PSNR].avslice[I_SLICE][2], dist->metric[PSNR].avslice[P_SLICE][2], dist->metric[PSNR].avslice[B_SLICE][2]);
  fprintf(p_stat,"\n ---------------------|----------------|---------------|---------------|");  
  fprintf(p_stat,"\n");

  // QUANT.
  fprintf(p_stat,"\n ---------------------|----------------|---------------|---------------|");
  fprintf(p_stat,"\n     Ave Quant        |        I       |       P       |       B       |");
  fprintf(p_stat,"\n ---------------------|----------------|---------------|---------------|");
  fprintf(p_stat,"\n        QP            |      %5.3f    |     %5.3f    |     %5.3f    |",
    (float)stats->quant[I_SLICE]/dmax(1.0,(float)stats->num_macroblocks[I_SLICE]),
    (float)stats->quant[P_SLICE]/dmax(1.0,(float)stats->num_macroblocks[P_SLICE]),
    (float)stats->quant[B_SLICE]/dmax(1.0,(float)stats->num_macroblocks[B_SLICE]));
  fprintf(p_stat,"\n ---------------------|----------------|---------------|---------------|");  
  fprintf(p_stat,"\n");

  // MODE
  fprintf(p_stat,"\n ---------------------|----------------|");
  fprintf(p_stat,"\n   Intra              |   Mode used    |");
  fprintf(p_stat,"\n ---------------------|----------------|");
  fprintf(p_stat,"\n Mode 0  intra 4x4    |  %5" FORMAT_OFF_T  "         |", stats->mode_use[I_SLICE][I4MB ]);
  fprintf(p_stat,"\n Mode 1  intra 8x8    |  %5" FORMAT_OFF_T  "         |", stats->mode_use[I_SLICE][I8MB ]);
  fprintf(p_stat,"\n Mode 2+ intra 16x16  |  %5" FORMAT_OFF_T  "         |", stats->mode_use[I_SLICE][I16MB]);
  fprintf(p_stat,"\n Mode    intra IPCM   |  %5" FORMAT_OFF_T  "         |", stats->mode_use[I_SLICE][IPCM ]);

  // P slices
  if (stats->frame_ctr[P_SLICE]!=0)
  {    
    mean_motion_info_bit_use[P_SLICE] = report_slice_pred_stats(p_stat, stats, P_SLICE,(double) bit_use[P_SLICE][0], "P Slice ");
  }
  // B slices
  if (stats->frame_ctr[B_SLICE]!=0)
  {
    mean_motion_info_bit_use[B_SLICE] = report_slice_pred_stats(p_stat, stats, B_SLICE,(double) bit_use[B_SLICE][0], "B Slice ");
  }
  // SP slices
  if (stats->frame_ctr[SP_SLICE]!=0)
  {    
    mean_motion_info_bit_use[SP_SLICE] = report_slice_pred_stats(p_stat, stats, SP_SLICE,(double) bit_use[SP_SLICE][0], "SP Slice");
  }


  fprintf(p_stat,"\n ---------------------|----------------|");
  fprintf(p_stat,"\n");

  fprintf(p_stat,"\n ---------------------|----------------|----------------|----------------|");
  fprintf(p_stat,"\n  Bit usage:          |      Intra     |      Inter     |    B frame     |");
  fprintf(p_stat,"\n ---------------------|----------------|----------------|----------------|");

  fprintf(p_stat,"\n Header               |");
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_header[I_SLICE] / bit_use[I_SLICE][0]);
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_header[P_SLICE] / bit_use[P_SLICE][0]);
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_header[B_SLICE] / bit_use[B_SLICE][0]);

  fprintf(p_stat,"\n Mode                 |");
  fprintf(p_stat," %10.2f     |", (float)stats->bit_use_mb_type[I_SLICE] / bit_use[I_SLICE][0]);
  fprintf(p_stat," %10.2f     |", (float)stats->bit_use_mb_type[P_SLICE] / bit_use[P_SLICE][0]);
  fprintf(p_stat," %10.2f     |", (float)stats->bit_use_mb_type[B_SLICE] / bit_use[B_SLICE][0]);

  fprintf(p_stat,"\n Motion Info          |");
  fprintf(p_stat,"        ./.     |");
  fprintf(p_stat," %10.2f     |", mean_motion_info_bit_use[P_SLICE]);
  fprintf(p_stat," %10.2f     |", mean_motion_info_bit_use[B_SLICE]);

  fprintf(p_stat,"\n CBP Y/C              |");
  fprintf(p_stat," %10.2f     |", (float) stats->tmp_bit_use_cbp[I_SLICE] / bit_use[I_SLICE][0]);
  fprintf(p_stat," %10.2f     |", (float) stats->tmp_bit_use_cbp[P_SLICE] / bit_use[P_SLICE][0]);   
  fprintf(p_stat," %10.2f     |", (float) stats->tmp_bit_use_cbp[B_SLICE] / bit_use[B_SLICE][0]);

  fprintf(p_stat,"\n Coeffs. Y            |");
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_coeff[0][I_SLICE] / bit_use[I_SLICE][0]);
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_coeff[0][P_SLICE] / bit_use[P_SLICE][0]);   
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_coeff[0][B_SLICE] / bit_use[B_SLICE][0]);

  fprintf(p_stat,"\n Coeffs. C            |");
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_coeffC[I_SLICE] / bit_use[I_SLICE][0]);
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_coeffC[P_SLICE] / bit_use[P_SLICE][0]);   
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_coeffC[B_SLICE] / bit_use[B_SLICE][0]);

  fprintf(p_stat,"\n Coeffs. CB           |");
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_coeff[1][I_SLICE] / bit_use[I_SLICE][0]);
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_coeff[1][P_SLICE] / bit_use[P_SLICE][0]);   
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_coeff[1][B_SLICE] / bit_use[B_SLICE][0]);

  fprintf(p_stat,"\n Coeffs. CB           |");
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_coeff[2][I_SLICE] / bit_use[I_SLICE][0]);
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_coeff[2][P_SLICE] / bit_use[P_SLICE][0]);   
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_coeff[2][B_SLICE] / bit_use[B_SLICE][0]);

  fprintf(p_stat,"\n Delta quant          |");
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_delta_quant[I_SLICE] / bit_use[I_SLICE][0]);
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_delta_quant[P_SLICE] / bit_use[P_SLICE][0]);   
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_delta_quant[B_SLICE] / bit_use[B_SLICE][0]);

  fprintf(p_stat,"\n Stuffing Bits        |");
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_stuffingBits[I_SLICE] / bit_use[I_SLICE][0]);
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_stuffingBits[P_SLICE] / bit_use[P_SLICE][0]);   
  fprintf(p_stat," %10.2f     |", (float) stats->bit_use_stuffingBits[B_SLICE] / bit_use[B_SLICE][0]);

  fprintf(p_stat,"\n ---------------------|----------------|----------------|----------------|");
  fprintf(p_stat,"\n average bits/frame   |");
  fprintf(p_stat," %10.2f     |", (float) bit_use[I_SLICE][1] / (float) bit_use[I_SLICE][0] );
  fprintf(p_stat," %10.2f     |", (float) bit_use[P_SLICE][1] / (float) bit_use[P_SLICE][0] );
  fprintf(p_stat," %10.2f     |", (float) bit_use[B_SLICE][1] / (float) bit_use[B_SLICE][0] );
  fprintf(p_stat,"\n ---------------------|----------------|----------------|----------------|");
  fprintf(p_stat,"\n");

  fclose(p_stat);
}


void report_log(InputParameters *params, StatParameters *stats, float frame_rate)
{
  char name[40];
  int i;
#ifndef WIN32
  time_t now;
  struct tm *l_time;
  char string[1000];
#else
  char timebuf[128];
#endif

  if ((p_log = fopen("log.dat", "r")) == 0)         // check if file exists
  {
    if ((p_log = fopen("log.dat", "a")) == NULL)    // append new statistic at the end
    {
      snprintf(errortext, ET_SIZE, "Error open file %s  \n", "log.dat");
      error(errortext, 500);
    }
    else                                            // Create header for new log file
    {
      fprintf(p_log," ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ \n");
      fprintf(p_log,"|                          Encoder statistics. This file is generated during first encoding session, new sessions will be appended                                                                                                                                                                                 |\n");
      fprintf(p_log," ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ \n");
      fprintf(p_log,"|     ver     | Date  | Time  |               Sequence                 | #Img |P/MbInt| QPI| QPP| QPB| Format  |Iperiod| #B | FMES | Hdmd | S.R |#Ref | Freq |Coding|RD-opt|Intra upd|8x8Tr| SNRY 1| SNRU 1| SNRV 1| SNRY N| SNRU N| SNRV N|#Bitr I|#Bitr P|#Bitr B|#Bitr IPB|     Total Time   |      Me Time     |\n");
      fprintf(p_log," ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ \n");

    }
  }
  else
  {
    fclose (p_log);
    if ((p_log = fopen("log.dat", "a")) == NULL)         // File exists, just open for appending
    {
      snprintf(errortext, ET_SIZE, "Error open file %s  \n", "log.dat");
      error(errortext, 500);
    }
  }
  fprintf(p_log,"|%5s/%-5s", VERSION, EXT_VERSION);

#ifdef WIN32
  _strdate( timebuf );
  fprintf(p_log,"| %1.5s |", timebuf );

  _strtime( timebuf);
  fprintf(p_log," % 1.5s |", timebuf);
#else
  now = time ((time_t *) NULL); // Get the system time and put it into 'now' as 'calender time'
  time (&now);
  l_time = localtime (&now);
  strftime (string, sizeof string, "%d-%b-%Y", l_time);
  fprintf(p_log,"| %1.5s |", string );

  strftime (string, sizeof string, "%H:%M:%S", l_time);
  fprintf(p_log," %1.5s |", string );
#endif

  for (i=0; i < 40; i++)
    name[i] = params->infile[i + imax(0, ((int) strlen(params->infile)) - 40)]; // write last part of path, max 40 chars
  fprintf(p_log,"%40.40s|",name);

  fprintf(p_log,"%5d |  %d/%d  |", params->no_frames, params->PicInterlace, params->MbInterlace);
  fprintf(p_log," %-3d| %-3d| %-3d|", params->qp0, params->qpN, params->qpB);

  fprintf(p_log,"%4dx%-4d|", params->output.width, params->output.height);
  fprintf(p_log,"  %3d  |%3d |", params->intra_period, stats->successive_Bframe);


  switch( params->SearchMode ) 
  {
  case UM_HEX:
    fprintf(p_log,"  HEX |");
    break;
  case UM_HEX_SIMPLE:
    fprintf(p_log," SHEX |");
    break;
  case EPZS:
    fprintf(p_log," EPZS |");
    break;
  case FAST_FULL_SEARCH:
    fprintf(p_log,"  FFS |");
    break;
  default:
    fprintf(p_log,"  FS  |");
    break;
  }

  fprintf(p_log,"  %1d%1d%1d |", params->MEErrorMetric[F_PEL], params->MEErrorMetric[H_PEL], params->MEErrorMetric[Q_PEL]);

  fprintf(p_log," %3d | %2d  |", params->search_range, params->num_ref_frames );

  fprintf(p_log," %5.2f|", (img->framerate *(float) (stats->successive_Bframe + 1)) / (float)(params->jumpd + 1));

  if (params->symbol_mode == CAVLC)
    fprintf(p_log," CAVLC|");
  else
    fprintf(p_log," CABAC|");

  fprintf(p_log,"   %d  |", params->rdopt);

  if (params->intra_upd == 1)
    fprintf(p_log,"   ON    |");
  else
    fprintf(p_log,"   OFF   |");

  fprintf(p_log,"  %d  |", params->Transform8x8Mode);

  fprintf(p_log,"%7.3f|%7.3f|%7.3f|", 
    dist->metric[PSNR].avslice[I_SLICE][0],
    dist->metric[PSNR].avslice[I_SLICE][1],
    dist->metric[PSNR].avslice[I_SLICE][2]);
  fprintf(p_log,"%7.3f|%7.3f|%7.3f|", dist->metric[PSNR].average[0],dist->metric[PSNR].average[1],dist->metric[PSNR].average[2]);
  /*
  fprintf(p_log,"%-5.3f|%-5.3f|%-5.3f|", dist->metric[PSNR].avslice[I_SLICE][0], dist->metric[PSNR].avslice[I_SLICE][1], dist->metric[PSNR].avslice[I_SLICE][2]);
  fprintf(p_log,"%-5.3f|%-5.3f|%-5.3f|", dist->metric[PSNR].avslice[P_SLICE][0], dist->metric[PSNR].avslice[P_SLICE][1], dist->metric[PSNR].avslice[P_SLICE][2]);
  fprintf(p_log,"%-5.3f|%-5.3f|%-5.3f|", dist->metric[PSNR].avslice[B_SLICE][0], dist->metric[PSNR].avslice[B_SLICE][1], dist->metric[PSNR].avslice[B_SLICE][2]);
  */
  fprintf(p_log,"%7.0f|%7.0f|%7.0f|%9.0f|", stats->bitrate_st[I_SLICE],stats->bitrate_st[P_SLICE],stats->bitrate_st[B_SLICE], stats->bitrate);

  fprintf(p_log,"   %12d   |   %12d   |", (int)tot_time,(int)me_tot_time);


  fprintf(p_log,"\n");

  fclose(p_log);

  p_log = fopen("data.txt", "a");

  if ((stats->successive_Bframe != 0) && (stats->frame_ctr[B_SLICE] != 0)) // B picture used
  {
    fprintf(p_log, "%3d %2d %2d %2.2f %2.2f %2.2f %5" FORMAT_OFF_T  " "
      "%2.2f %2.2f %2.2f %5d "
      "%2.2f %2.2f %2.2f %5" FORMAT_OFF_T  " %5" FORMAT_OFF_T  " %.3f\n",
      params->no_frames, params->qp0, params->qpN,
      dist->metric[PSNR].avslice[I_SLICE][0],
      dist->metric[PSNR].avslice[I_SLICE][1],
      dist->metric[PSNR].avslice[I_SLICE][2],
      stats->bit_counter[I_SLICE],
      0.0,
      0.0,
      0.0,
      0,
      dist->metric[PSNR].average[0],
      dist->metric[PSNR].average[1],
      dist->metric[PSNR].average[2],
      (stats->bit_counter[I_SLICE] + stats->bit_ctr) / stats->frame_counter,
      stats->bit_counter[B_SLICE] / stats->frame_ctr[B_SLICE],
      (double) 0.001 * tot_time / (stats->frame_counter));
  }
  else
  {
    if (params->no_frames != 0)
      fprintf(p_log, "%3d %2d %2d %2.2f %2.2f %2.2f %5" FORMAT_OFF_T  " "
      "%2.2f %2.2f %2.2f %5d "
      "%2.2f %2.2f %2.2f %5" FORMAT_OFF_T  " %5d %.3f\n",
      params->no_frames, params->qp0, params->qpN,
      dist->metric[PSNR].avslice[I_SLICE][0],
      dist->metric[PSNR].avslice[I_SLICE][1],
      dist->metric[PSNR].avslice[I_SLICE][2],
      stats->bit_counter[I_SLICE],
      0.0,
      0.0,
      0.0,
      0,
      dist->metric[PSNR].average[0],
      dist->metric[PSNR].average[1],
      dist->metric[PSNR].average[2],
      (stats->bit_counter[I_SLICE] + stats->bit_ctr)/ stats->frame_counter,
      0,
      (double)0.001*tot_time/params->no_frames);
  }

  fclose(p_log);
}

/*!
 ************************************************************************
 * \brief
 *    Reports the gathered information to appropriate outputs
 * \par Input:
 *    struct inp_par *inp,                                            \n
 *    ImageParameters *img,                                            \n
 *    struct stat_par *stats,                                          \n
 *
 * \par Output:
 *    None
 ************************************************************************
 */
void report( ImageParameters *img, InputParameters *params, StatParameters *stats)
{
  int64 bit_use[NUM_SLICE_TYPES][2];
  int i,j;
  int64 total_bits;
  float frame_rate;

  bit_use[ I_SLICE][0] = stats->frame_ctr[I_SLICE];
  bit_use[ P_SLICE][0] = imax(stats->frame_ctr[P_SLICE ] + stats->frame_ctr[SP_SLICE], 1);
  bit_use[ B_SLICE][0] = imax(stats->frame_ctr[B_SLICE ], 1);
  bit_use[SP_SLICE][0] = imax(stats->frame_ctr[SP_SLICE], 1);

  //  Accumulate bit usage for inter and intra frames
  for (j=0; j < NUM_SLICE_TYPES; j++)
  {
    bit_use[j][1] = 0;
  }

  for (j=0; j < NUM_SLICE_TYPES; j++)
  {
    for(i=0; i < MAXMODE; i++)
      bit_use[j][1] += stats->bit_use_mode[j][i];

    bit_use[j][1] += stats->bit_use_mb_type[j];
    bit_use[j][1] += stats->bit_use_header[j];    
    bit_use[j][1] += stats->tmp_bit_use_cbp[j];
    bit_use[j][1] += stats->bit_use_coeffC[j];
    bit_use[j][1] += stats->bit_use_coeff[0][j];   
    bit_use[j][1] += stats->bit_use_coeff[1][j]; 
    bit_use[j][1] += stats->bit_use_coeff[2][j]; 
    bit_use[j][1] += stats->bit_use_delta_quant[j];
    bit_use[j][1] += stats->bit_use_stuffingBits[j];
  }

  frame_rate = (img->framerate *(float)(stats->successive_Bframe + 1)) / (float) (params->jumpd + 1);

  //! Currently adding NVB bits on P rate. Maybe additional stats info should be created instead and added in log file  
  stats->bitrate_st[ I_SLICE] = (stats->bit_counter[ I_SLICE]) * (frame_rate) / (float) (stats->frame_counter);
  stats->bitrate_st[ P_SLICE] = (stats->bit_counter[ P_SLICE]) * (frame_rate) / (float) (stats->frame_counter);
  stats->bitrate_st[ B_SLICE] = (stats->bit_counter[ B_SLICE]) * (frame_rate) / (float) (stats->frame_counter);
  stats->bitrate_st[SP_SLICE] = (stats->bit_counter[SP_SLICE]) * (frame_rate) / (float) (stats->frame_counter);

  switch (params->Verbose)
  {
  case 0:
  case 1:
  default:
    fprintf(stdout,"------------------ Average data all frames  -----------------------------------\n\n");
    break;
  case 2:
    fprintf(stdout,"------------------------------------  Average data all frames  ---------------------------------\n\n");
    break;
  }

  if (params->Verbose != 0)
  {
    DistMetric *snr     = &dist->metric[PSNR    ];
    DistMetric *sse     = &dist->metric[SSE     ];
    DistMetric *snr_rgb = &dist->metric[PSNR_RGB];
    DistMetric *sse_rgb = &dist->metric[SSE_RGB ];

    int  impix    = params->output.size_cmp[0];
    int  impix_cr = params->output.size_cmp[1];

    float csnr_y = psnr(img->max_imgpel_value_comp_sq[0], impix   , sse->average[0]);
    float csnr_u = psnr(img->max_imgpel_value_comp_sq[1], impix_cr, sse->average[1]);
    float csnr_v = psnr(img->max_imgpel_value_comp_sq[2], impix_cr, sse->average[2]);

    fprintf(stdout,  " Total encoding time for the seq.  : %.3f sec (%.2f fps)\n", tot_time*0.001, 1000.0 * (stats->frame_counter) / tot_time);
    fprintf(stdout,  " Total ME time for sequence        : %.3f sec \n\n", me_tot_time*0.001);

    fprintf(stdout," Y { PSNR (dB), cSNR (dB), MSE }   : { %5.2f, %5.2f, %5.2f }\n", 
      snr->average[0], csnr_y, sse->average[0]/impix);
    fprintf(stdout," U { PSNR (dB), cSNR (dB), MSE }   : { %5.2f, %5.2f, %5.2f }\n",
      snr->average[1], csnr_u, sse->average[1]/impix_cr);
    fprintf(stdout," V { PSNR (dB), cSNR (dB), MSE }   : { %5.2f, %5.2f, %5.2f }\n",
      snr->average[2], csnr_v, sse->average[2]/impix_cr);

    if(params->DistortionYUVtoRGB == 1)
    {
      float csnr_r = psnr(img->max_imgpel_value_comp_sq[0], impix, sse_rgb->average[0]);
      float csnr_g = psnr(img->max_imgpel_value_comp_sq[1], impix, sse_rgb->average[1]);
      float csnr_b = psnr(img->max_imgpel_value_comp_sq[2], impix, sse_rgb->average[2]);

      fprintf(stdout," R { PSNR (dB), cSNR (dB), MSE }   : { %5.2f, %5.2f, %5.2f }\n", 
        snr_rgb->average[0], csnr_r, sse_rgb->average[0]/impix);
      fprintf(stdout," G { PSNR (dB), cSNR (dB), MSE }   : { %5.2f, %5.2f, %5.2f }\n",
        snr_rgb->average[1], csnr_g, sse_rgb->average[1]/impix);
      fprintf(stdout," B { PSNR (dB), cSNR (dB), MSE }   : { %5.2f, %5.2f, %5.2f }\n",
        snr_rgb->average[2], csnr_b, sse_rgb->average[2]/impix);
    }

    if (params->Distortion[SSIM] == 1)
    {
      if(params->DistortionYUVtoRGB == 1)
      {
        fprintf(stdout," SSIM {Y, R}                       : { %5.4f, %5.4f }\n", dist->metric[SSIM].average[0], dist->metric[SSIM_RGB].average[0]);
        fprintf(stdout," SSIM {U, G}                       : { %5.4f, %5.4f }\n", dist->metric[SSIM].average[1], dist->metric[SSIM_RGB].average[1]);
        fprintf(stdout," SSIM {V, B}                       : { %5.4f, %5.4f }\n", dist->metric[SSIM].average[2], dist->metric[SSIM_RGB].average[2]);
      }
      else
      {
        fprintf(stdout," Y SSIM                            : %5.4f\n", dist->metric[SSIM].average[0]);
        fprintf(stdout," U SSIM                            : %5.4f\n", dist->metric[SSIM].average[1]);
        fprintf(stdout," V SSIM                            : %5.4f\n", dist->metric[SSIM].average[2]);
      }
    }
    if (params->Distortion[MS_SSIM] == 1)
    {
      if(params->DistortionYUVtoRGB == 1)
      {
        fprintf(stdout," MS-SSIM {Y, R}                    : { %5.4f, %5.4f }\n", dist->metric[MS_SSIM].average[0], dist->metric[MS_SSIM_RGB].average[0]);
        fprintf(stdout," MS-SSIM {U, G}                    : { %5.4f, %5.4f }\n", dist->metric[MS_SSIM].average[1], dist->metric[MS_SSIM_RGB].average[1]);
        fprintf(stdout," MS-SSIM {V, B}                    : { %5.4f, %5.4f }\n", dist->metric[MS_SSIM].average[2], dist->metric[MS_SSIM_RGB].average[2]);
      }
      else
      {
        fprintf(stdout," Y MS-SSIM                         : %5.4f\n", dist->metric[MS_SSIM].average[0]);
        fprintf(stdout," U MS-SSIM                         : %5.4f\n", dist->metric[MS_SSIM].average[1]);
        fprintf(stdout," V MS-SSIM                         : %5.4f\n", dist->metric[MS_SSIM].average[2]);
      }
    }
    fprintf(stdout,"\n");
  }
  else
    fprintf(stdout,  " Total encoding time for the seq.  : %.3f sec (%.2f fps)\n\n", tot_time*0.001, 1000.0 * (stats->frame_counter) / tot_time);

  total_bits = stats->bit_ctr_parametersets;
  for (i = 0; i < NUM_SLICE_TYPES; i++)
    total_bits += stats->bit_counter[i];

  if (stats->frame_ctr[B_SLICE] != 0)
  {
    fprintf(stdout, " Total bits                        : %" FORMAT_OFF_T  " (I %" FORMAT_OFF_T  ", P %" FORMAT_OFF_T  ", B %" FORMAT_OFF_T  " NVB %d) \n",
      total_bits,  stats->bit_counter[I_SLICE], stats->bit_counter[P_SLICE], stats->bit_counter[B_SLICE], stats->bit_ctr_parametersets);
  }
  else if (params->sp_periodicity == 0)
  {
    fprintf(stdout, " Total bits                        : %" FORMAT_OFF_T  " (I %" FORMAT_OFF_T  ", P %" FORMAT_OFF_T  ", NVB %d) \n",
      total_bits, stats->bit_counter[I_SLICE], stats->bit_counter[P_SLICE], stats->bit_ctr_parametersets);
  }
  else
  {
    fprintf(stdout, " Total bits                        : %" FORMAT_OFF_T  " (I %" FORMAT_OFF_T  ", P %" FORMAT_OFF_T  ", NVB %d) \n",
      total_bits, stats->bit_counter[I_SLICE], stats->bit_counter[P_SLICE], stats->bit_ctr_parametersets);
  }

  frame_rate = (img->framerate *(float)(stats->successive_Bframe + 1)) / (float) (params->jumpd + 1);

  stats->bitrate= ((float) total_bits * frame_rate) / ((float) (stats->frame_counter));
  fprintf(stdout, " Bit rate (kbit/s)  @ %2.2f Hz     : %5.2f\n", frame_rate, stats->bitrate / 1000);
  
  for (i = 0; i < 5; i++)
    stats->bit_ctr_emulationprevention += stats->bit_use_stuffingBits[i];

  fprintf(stdout, " Bits to avoid Startcode Emulation : %" FORMAT_OFF_T  " \n", stats->bit_ctr_emulationprevention);
  fprintf(stdout, " Bits for parameter sets           : %d \n\n", stats->bit_ctr_parametersets);

  switch (params->Verbose)
  {
  case 0:
  case 1:
  default:
    fprintf(stdout,"-------------------------------------------------------------------------------\n");
    break;
  case 2:
    fprintf(stdout,"------------------------------------------------------------------------------------------------\n");
    break;
  }  
  fprintf(stdout,"Exit JM %s encoder ver %s ", JM, VERSION);
  fprintf(stdout,"\n");

  // status file
  report_stats(params, stats, bit_use, frame_rate);

  // write to log file
  report_log(params, stats, frame_rate);

  if (params->ReportFrameStats)
  {
    if ((p_log = fopen("stat_frame.dat", "a")) == NULL)       // append new statistic at the end
    {
      snprintf(errortext, ET_SIZE, "Error open file %s  \n", "stat_frame.dat.dat");
      //    error(errortext, 500);
    }
    else
    {
      fprintf(p_log," --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \n");
      fclose(p_log);
    }
    report_log_mode(params, stats, bit_use);
  }
}


/*!
 ************************************************************************
 * \brief
 *    Prints the header of the protocol.
 * \par Input:
 *    struct inp_par *inp
 * \par Output:
 *    none
 ************************************************************************
 */
void information_init ( ImageParameters *img, InputParameters *params, StatParameters *stats)
{
  int i;
  static char yuv_types[4][10] = {"YUV 4:0:0", "YUV 4:2:0", "YUV 4:2:2", "YUV 4:4:4"};
  switch (params->Verbose)
  {
  case 0:
  case 1:
  default:
    printf("------------------------------- JM %4.4s %7.7s -------------------------------\n", VERSION, EXT_VERSION);
    break;
  case 2:
    printf("--------------------------------------- JM %4.4s %7.7s ----------------------------------------\n", VERSION, EXT_VERSION);
    break;
  }

  fprintf(stdout,  " Input YUV file                    : %s \n", params->infile);
  fprintf(stdout,  " Output H.264 bitstream            : %s \n", params->outfile);
  if (p_dec != -1)
    fprintf(stdout,  " Output YUV file                   : %s \n", params->ReconFile);
  fprintf(stdout,  " YUV Format                        : %s \n", &yuv_types[img->yuv_format][0]);//img->yuv_format==YUV422?"YUV 4:2:2":(img->yuv_format==YUV444)?"YUV 4:4:4":"YUV 4:2:0");
  fprintf(stdout,  " Frames to be encoded I-P/B        : %d/%d\n", params->no_frames, (params->successive_Bframe*(params->no_frames-1)));
  if (params->Verbose != 0)
  {
    fprintf(stdout,  " Freq. for encoded bitstream       : %1.0f\n", img->framerate/(float)(params->jumpd+1));
    fprintf(stdout,  " PicInterlace / MbInterlace        : %d/%d\n", params->PicInterlace, params->MbInterlace);
    fprintf(stdout,  " Transform8x8Mode                  : %d\n", params->Transform8x8Mode);

    for (i=0; i<3; i++)
    {
      fprintf(stdout," ME Metric for Refinement Level %1d  : %s\n", i, DistortionType[params->MEErrorMetric[i]]);
    }
    fprintf(stdout,  " Mode Decision Metric              : %s\n", DistortionType[params->ModeDecisionMetric]);

    switch ( params->ChromaMEEnable )
    {
    case 1:
      fprintf(stdout," Motion Estimation for components  : YCbCr\n");
      break;
    default:
      fprintf(stdout," Motion Estimation for components  : Y\n");
      break;
    }

    fprintf(stdout,  " Image format                      : %dx%d (%dx%d)\n", params->output.width, params->output.height, img->width,img->height);

    if (params->intra_upd)
      fprintf(stdout," Error robustness                  : On\n");
    else
      fprintf(stdout," Error robustness                  : Off\n");
    fprintf(stdout,  " Search range                      : %d\n", params->search_range);

    fprintf(stdout,  " Total number of references        : %d\n", params->num_ref_frames);
    fprintf(stdout,  " References for P slices           : %d\n", params->P_List0_refs ? params->P_List0_refs : params->num_ref_frames);
    fprintf(stdout,  " List0 references for B slices     : %d\n", params->B_List0_refs ? params->B_List0_refs : params->num_ref_frames);
    fprintf(stdout,  " List1 references for B slices     : %d\n", params->B_List1_refs ? params->B_List1_refs : params->num_ref_frames);

    // Sequence Type
    fprintf(stdout,  " Sequence type                     :");
    if (stats->successive_Bframe > 0 && params->HierarchicalCoding)
    {
      fprintf(stdout, " Hierarchy (QP: I %d, P %d, B %d) \n",
        params->qp0, params->qpN, params->qpB);
    }
    else if (stats->successive_Bframe > 0)
    {
      char seqtype[80];
      int i,j;

      strcpy (seqtype,"I");

      for (j=0; j < 2; j++)
      {
        for (i=0; i < stats->successive_Bframe; i++)
        {
          if (params->BRefPictures)
            strncat(seqtype,"-RB", imax(0, (int) (79 - strlen(seqtype))));
          else
            strncat(seqtype,"-B", imax(0, (int) (79 - strlen(seqtype))));
        }
        strncat(seqtype,"-P", imax(0, (int) (79 - strlen(seqtype))));
      }
      if (params->BRefPictures)
        fprintf(stdout, " %s (QP: I %d, P %d, RB %d) \n", seqtype, params->qp0, params->qpN, iClip3(0, 51, params->qpB + params->qpBRSOffset));
      else
        fprintf(stdout, " %s (QP: I %d, P %d, B %d) \n", seqtype, params->qp0, params->qpN, params->qpB);
    }
    else if (stats->successive_Bframe == 0 && params->sp_periodicity == 0) 
      fprintf(stdout, " IPPP (QP: I %d, P %d) \n", params->qp0, params->qpN);
    else 
      fprintf(stdout, " I-P-P-SP-P (QP: I %d, P %d, SP (%d, %d)) \n",  params->qp0, params->qpN, params->qpsp, params->qpsp_pred);

    // report on entropy coding  method
    if (params->symbol_mode == CAVLC)
      fprintf(stdout," Entropy coding method             : CAVLC\n");
    else
      fprintf(stdout," Entropy coding method             : CABAC\n");

    fprintf(stdout,  " Profile/Level IDC                 : (%d,%d)\n", params->ProfileIDC, params->LevelIDC);

    if (params->SearchMode == UM_HEX)
      fprintf(stdout,  " Motion Estimation Scheme          : HEX\n");
    else if (params->SearchMode == UM_HEX_SIMPLE)
      fprintf(stdout,  " Motion Estimation Scheme          : SHEX\n");
    else if (params->SearchMode == EPZS)
    {
      fprintf(stdout,  " Motion Estimation Scheme          : EPZS\n");
      EPZSOutputStats(params, stdout, 0);
    }
    else if (params->SearchMode == FAST_FULL_SEARCH)
      fprintf(stdout,  " Motion Estimation Scheme          : Fast Full Search\n");
    else
      fprintf(stdout,  " Motion Estimation Scheme          : Full Search\n");

    if (params->full_search == 2)
      fprintf(stdout," Search range restrictions         : none\n");
    else if (params->full_search == 1)
      fprintf(stdout," Search range restrictions         : older reference frames\n");
    else
      fprintf(stdout," Search range restrictions         : smaller blocks and older reference frames\n");

    if (params->rdopt)
      fprintf(stdout," RD-optimized mode decision        : used\n");
    else
      fprintf(stdout," RD-optimized mode decision        : not used\n");

    switch(params->partition_mode)
    {
    case PAR_DP_1:
      fprintf(stdout," Data Partitioning Mode            : 1 partition \n");
      break;
    case PAR_DP_3:
      fprintf(stdout," Data Partitioning Mode            : 3 partitions \n");
      break;
    default:
      fprintf(stdout," Data Partitioning Mode            : not supported\n");
      break;
    }

    switch(params->of_mode)
    {
    case PAR_OF_ANNEXB:
      fprintf(stdout," Output File Format                : H.264/AVC Annex B Byte Stream Format \n");
      break;
    case PAR_OF_RTP:
      fprintf(stdout," Output File Format                : RTP Packet File Format \n");
      break;
    default:
      fprintf(stdout," Output File Format                : not supported\n");
      break;
    }
  }


  switch (params->Verbose)
  {
  case 0:
  default:
    printf("-------------------------------------------------------------------------------\n");
    printf("\nEncoding. Please Wait.\n\n");
    break;    
  case 1:
    printf("-------------------------------------------------------------------------------\n");
    printf("  Frame  Bit/pic    QP   SnrY    SnrU    SnrV    Time(ms) MET(ms) Frm/Fld Ref  \n");
    printf("-------------------------------------------------------------------------------\n");
    break;
  case 2:
    if (params->Distortion[SSIM] == 1)
    {
      printf("------------------------------------------------------------------------------------------------------------------------\n");
      printf("  Frame  Bit/pic WP QP QL   SnrY    SnrU    SnrV   SsimY   SsimU   SsimV    Time(ms) MET(ms) Frm/Fld   I D L0 L1 RDP Ref\n");
      printf("------------------------------------------------------------------------------------------------------------------------\n");
    }
    else
    {
      printf("------------------------------------------------------------------------------------------------\n");
      printf("  Frame  Bit/pic WP QP QL   SnrY    SnrU    SnrV    Time(ms) MET(ms) Frm/Fld   I D L0 L1 RDP Ref\n");
      printf("------------------------------------------------------------------------------------------------\n");
    }
    break;
   
  }
}

/*!
 ************************************************************************
 * \brief
 *    Report mode distribution of the sequence to log_mode.dat
 ************************************************************************
 */
void report_log_mode(InputParameters *params, StatParameters *stats, int64 bit_use[NUM_SLICE_TYPES][2])
{
  FILE *p_stat;                    //!< status file for the last encoding session
  int i;
  char name[40];
#ifndef WIN32
  time_t now;
  struct tm *l_time;
  char string[1000];
#else
  char timebuf[128];
#endif

  if ((p_stat = fopen("log_mode.dat", "r")) == 0)         // check if file exists
  {
    if ((p_stat = fopen("log_mode.dat", "a")) == NULL)    // append new statistic at the end
    {
      snprintf(errortext, ET_SIZE, "Error open file %s  \n", "log_mode.dat");
      error(errortext, 500);
    }
    else                                            // Create header for new log file
    {
      fprintf(p_stat, " ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \n");
      fprintf(p_stat,"|                          Encoder statistics. This file is generated during first encoding session, new sessions will be appended                                                                                                                                                                                 |\n");
      fprintf(p_stat, " ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \n");
      fprintf(p_stat, "|     ver     | Date  | Time  |    Sequence                  | QP |  I4  |  I8  | I16  | IC0  | IC1  | IC2  | IC3  | PI4  | PI8  | PI16 |  P0  |  P1  |  P2  |  P3  | P1*4*| P1*8*| P2*4*| P2*8*| P3*4*| P3*8*|  P8  | P8:4 | P4*4*| P4*8*| P8:5 | P8:6 | P8:7 | BI4  | BI8  | BI16 |  B0  |  B1  |  B2  |  B3  | B0*4*| B0*8*| B1*4*| B1*8*| B2*4*| B2*8*| B3*4*| B3*8*|  B8  | B8:0 |B80*4*|B80*8*| B8:4 | B4*4*| B4*8*| B8:5 | B8:6 | B8:7 |\n");
      fprintf(p_stat, " ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \n");
    }
  }
  else
  {
    fclose (p_stat);
    if ((p_stat = fopen("log_mode.dat", "a")) == NULL)         // File exists, just open for appending
    {
      snprintf(errortext, ET_SIZE, "Error open file %s  \n", "log_mode.dat");
      error(errortext, 500);
    }
  }

  //report
  fprintf(p_stat, "|%4s/%s", VERSION, EXT_VERSION);

#ifdef WIN32
  _strdate( timebuf );
  fprintf(p_stat, "| %1.5s |", timebuf);

  _strtime( timebuf);
  fprintf(p_stat, " % 1.5s |", timebuf);
#else
  now = time ((time_t *) NULL); // Get the system time and put it into 'now' as 'calender time'
  time (&now);
  l_time = localtime (&now);
  strftime (string, sizeof string, "%d-%b-%Y", l_time);
  fprintf(p_stat, "| %1.5s |", string );

  strftime (string, sizeof string, "%H:%M:%S", l_time);
  fprintf(p_stat, " %1.5s |", string);
#endif

  for (i=0;i<30;i++)
    name[i]=params->infile[i + imax(0,(int) (strlen(params->infile)- 30))]; // write last part of path, max 30 chars

  fprintf(p_stat, "%30.30s|", name);
  fprintf(p_stat, "%3d |", img->qp);

  //report modes
  //I-Modes
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[I_SLICE][I4MB ]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[I_SLICE][I8MB ]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[I_SLICE][I16MB]);

  //chroma intra mode
  fprintf(p_stat, " %5d|", stats->intra_chroma_mode[0]);
  fprintf(p_stat, " %5d|", stats->intra_chroma_mode[1]);
  fprintf(p_stat, " %5d|", stats->intra_chroma_mode[2]);
  fprintf(p_stat, " %5d|", stats->intra_chroma_mode[3]);

  //P-Modes
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[P_SLICE][I4MB ]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[P_SLICE][I8MB ]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[P_SLICE][I16MB]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[P_SLICE][0    ]);

  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[P_SLICE][1    ]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[P_SLICE][2    ]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[P_SLICE][3    ]);
  
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[P_SLICE][1][0]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[P_SLICE][1][1]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[P_SLICE][2][0]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[P_SLICE][2][1]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[P_SLICE][3][0]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[P_SLICE][3][1]);

  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[P_SLICE][P8x8 ]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[P_SLICE][4    ]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[P_SLICE][4][0]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[P_SLICE][4][1]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[P_SLICE][5    ]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[P_SLICE][6    ]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[P_SLICE][7    ]);

  //B-Modes
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[B_SLICE][I4MB ]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[B_SLICE][I8MB ]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[B_SLICE][I16MB]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[B_SLICE][0    ]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[B_SLICE][1    ]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[B_SLICE][2    ]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[B_SLICE][3    ]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[B_SLICE][0][0]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[B_SLICE][0][1]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[B_SLICE][1][0]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[B_SLICE][1][1]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[B_SLICE][2][0]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[B_SLICE][2][1]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[B_SLICE][3][0]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[B_SLICE][3][1]);

  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[B_SLICE][P8x8]);
  fprintf(p_stat, " %d|", (stats->b8_mode_0_use [B_SLICE][0]+stats->b8_mode_0_use [B_SLICE][1]));
  fprintf(p_stat, " %5d|", stats->b8_mode_0_use [B_SLICE][0]);
  fprintf(p_stat, " %5d|", stats->b8_mode_0_use [B_SLICE][1]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[B_SLICE][4   ]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[B_SLICE][4][0]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use_transform[B_SLICE][4][1]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[B_SLICE][5   ]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[B_SLICE][6   ]);
  fprintf(p_stat, " %5" FORMAT_OFF_T  "|", stats->mode_use[B_SLICE][7   ]);

  fprintf(p_stat, "\n");
  fclose(p_stat);
}


