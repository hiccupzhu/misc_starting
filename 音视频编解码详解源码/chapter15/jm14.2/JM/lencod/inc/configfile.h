
/*!
 ***********************************************************************
 *  \file
 *     configfile.h
 *  \brief
 *     Prototypes for configfile.c and definitions of used structures.
 ***********************************************************************
 */

#include "fmo.h"

#ifndef _CONFIGFILE_H_
#define _CONFIGFILE_H_

#define DEFAULTCONFIGFILENAME "encoder.cfg"

#define PROFILE_IDC     88
#define LEVEL_IDC       21


typedef struct {
  char *TokenName;
  void *Place;
  int Type;
  double Default;
  int param_limits; //! 0: no limits, 1: both min and max, 2: only min (i.e. no negatives), 3: special case for QPs since min needs bitdepth_qp_scale
  double min_limit;
  double max_limit;
} Mapping;



InputParameters configinput;


#ifdef INCLUDED_BY_CONFIGFILE_C
// Mapping_Map Syntax:
// {NAMEinConfigFile,  &configinput.VariableName, Type, InitialValue, LimitType, MinLimit, MaxLimit}
// Types : {0:int, 1:text, 2: double}
// LimitType: {0:none, 1:both, 2:minimum, 3: QP based}
// We could separate this based on types to make it more flexible and allow also defaults for text types.
Mapping Map[] = {
    {"ProfileIDC",               &configinput.ProfileIDC,                   0,   (double) PROFILE_IDC,      0,  0.0,              0.0              },
    {"IntraProfile",             &configinput.IntraProfile,                 0,   0.0,                       3,  0.0,              1.0              }, 
    {"LevelIDC",                 &configinput.LevelIDC,                     0,   (double) LEVEL_IDC,        0,  0.0,              0.0              },
    {"FrameRate",                &configinput.FrameRate,                    2,   (double) INIT_FRAME_RATE,  1,  0.0,            100.0              },
    {"ResendSPS",                &configinput.ResendSPS,                    0,   0.0,                       1,  0.0,              1.0              },
    {"StartFrame",               &configinput.start_frame,                  0,   0.0,                       2,  0.0,              0.0              },
    {"IntraPeriod",              &configinput.intra_period,                 0,   0.0,                       2,  0.0,              0.0              },
    {"IDRPeriod",                &configinput.idr_period,                   0,   0.0,                       2,  0.0,              0.0              },
    {"IntraDelay",               &configinput.intra_delay,                  0,   0.0,                       2,  0.0,              0.0              },
    {"AdaptiveIntraPeriod",      &configinput.adaptive_intra_period,        0,   0.0,                       1,  0.0,              1.0              },
    {"AdaptiveIDRPeriod",        &configinput.adaptive_idr_period,          0,   0.0,                       1,  0.0,              2.0              },
    {"EnableOpenGOP",            &configinput.EnableOpenGOP,                0,   0.0,                       1,  0.0,              1.0              },
    {"EnableIDRGOP",             &configinput.EnableIDRGOP,                 0,   0.0,                       1,  0.0,              1.0              },    
    {"FramesToBeEncoded",        &configinput.no_frames,                    0,   1.0,                       2, -1.0,              0.0              },
    {"QPISlice",                 &configinput.qp0,                          0,   24.0,                      3,  (double) MIN_QP,  (double) MAX_QP  },
    {"QPPSlice",                 &configinput.qpN,                          0,   24.0,                      3,  (double) MIN_QP,  (double) MAX_QP  },
    {"QPBSlice",                 &configinput.qpB,                          0,   24.0,                      3,  (double) MIN_QP,  (double) MAX_QP  },
    {"FrameSkip",                &configinput.jumpd,                        0,   0.0,                       2,  0.0,              0.0              },
    {"DisableSubpelME",          &configinput.DisableSubpelME,              0,   0.0,                       1,  0.0,              1.0              },
    {"SearchRange",              &configinput.search_range,                 0,   16.0,                      2,  0.0,              0.0              },
    {"NumberReferenceFrames",    &configinput.num_ref_frames,               0,   1.0,                       1,  0.0,             16.0              },
    {"PList0References",         &configinput.P_List0_refs,                 0,   0.0,                       1,  0.0,             16.0              },
    {"BList0References",         &configinput.B_List0_refs,                 0,   0.0,                       1,  0.0,             16.0              },
    {"BList1References",         &configinput.B_List1_refs,                 0,   1.0,                       1,  0.0,             16.0              },
    {"Log2MaxFNumMinus4",        &configinput.Log2MaxFNumMinus4,            0,   0.0,                       1, -1.0,             12.0              },
    {"Log2MaxPOCLsbMinus4",      &configinput.Log2MaxPOCLsbMinus4,          0,   2.0,                       1, -1.0,             12.0              },
    {"GenerateMultiplePPS",      &configinput.GenerateMultiplePPS,          0,   0.0,                       1,  0.0,              1.0              },
    {"ResendPPS",                &configinput.ResendPPS,                    0,   0.0,                       1,  0.0,              1.0              },
    {"SourceWidth",              &configinput.source.width,                 0,   176.0,                     2, 16.0,              0.0              },
    {"SourceHeight",             &configinput.source.height,                0,   144.0,                     2, 16.0,              0.0              },
    {"SourceResize",             &configinput.src_resize,                   0,   0.0,                       1,  0.0,              1.0              },
    {"OutputWidth",              &configinput.output.width,                 0,   176.0,                     2, 16.0,              0.0              },
    {"OutputHeight",             &configinput.output.height,                0,   144.0,                     2, 16.0,              0.0              },
    {"MbLineIntraUpdate",        &configinput.intra_upd,                    0,   0.0,                       1,  0.0,              1.0              },
    {"SliceMode",                &configinput.slice_mode,                   0,   0.0,                       1,  0.0,              3.0              },
    {"SliceArgument",            &configinput.slice_argument,               0,   1.0,                       2,  1.0,              1.0              },
    {"UseConstrainedIntraPred",  &configinput.UseConstrainedIntraPred,      0,   0.0,                       1,  0.0,              1.0              },
    {"InputFile",                &configinput.infile,                       1,   0.0,                       0,  0.0,              0.0              },
    {"InputHeaderLength",        &configinput.infile_header,                0,   0.0,                       2,  0.0,              1.0              },
    {"OutputFile",               &configinput.outfile,                      1,   0.0,                       0,  0.0,              0.0              },
    {"ReconFile",                &configinput.ReconFile,                    1,   0.0,                       0,  0.0,              0.0              },
    {"TraceFile",                &configinput.TraceFile,                    1,   0.0,                       0,  0.0,              0.0              },
    {"StatsFile",                &configinput.StatsFile,                    1,   0.0,                       0,  0.0,              0.0              },
    {"DisposableP",              &configinput.DisposableP,                  0,   0.0,                       1,  0.0,              1.0              },
    {"DispPQPOffset",            &configinput.DispPQPOffset,                0,   0.0,                       0,-51.0,             51.0              },
    {"NumberBFrames",            &configinput.successive_Bframe,            0,   0.0,                       2,  0.0,              0.0              },
    {"PReplaceBSlice",           &configinput.PReplaceBSlice,               0,   0.0,                       1,  0.0,              1.0              },
    {"BRefPicQPOffset",          &configinput.qpBRSOffset,                  0,   0.0,                       0,-51.0,             51.0              },
    {"DirectModeType",           &configinput.direct_spatial_mv_pred_flag,  0,   0.0,                       1,  0.0,              1.0              },
    {"DirectInferenceFlag",      &configinput.directInferenceFlag,          0,   1.0,                       1,  0.0,              1.0              },
    {"SPPicturePeriodicity",     &configinput.sp_periodicity,               0,   0.0,                       2,  0.0,              0.0              },
    {"QPSPSlice",                &configinput.qpsp,                         0,   24.0,                      3,  (double) MIN_QP,  (double) MAX_QP  },
    {"QPSP2Slice",               &configinput.qpsp_pred,                    0,   24.0,                      3,  (double) MIN_QP,  (double) MAX_QP  },
    {"SI_FRAMES",                &configinput.si_frame_indicator,           0,   0.0,                       1,  0.0,              1.0              },
    {"SP_output",                &configinput.sp_output_indicator,          0,   0.0,                       1,  0.0,              1.0              },
    {"SP_output_name",           &configinput.sp_output_filename,           1,   0.0,                       0,  0.0,              0.0              },
    {"SP2_FRAMES",               &configinput.sp2_frame_indicator,          0,   0.0,                       1,  0.0,              1.0              },
    {"SP2_input_name1",          &configinput.sp2_input_filename1,          1,   0.0,                       0,  0.0,              0.0              },
    {"SP2_input_name2",          &configinput.sp2_input_filename2,          1,   0.0,                       0,  0.0,              0.0              },
    {"SymbolMode",               &configinput.symbol_mode,                  0,   0.0,                       1,  (double) CAVLC,  (double) CABAC   },
    {"OutFileMode",              &configinput.of_mode,                      0,   0.0,                       1,  0.0,              1.0              },
    {"PartitionMode",            &configinput.partition_mode,               0,   0.0,                       1,  0.0,              1.0              },
    {"PSliceSkip",               &configinput.InterSearch[0][0],            0,   1.0,                       1,  0.0,              1.0              },
    {"PSliceSearch16x16",        &configinput.InterSearch[0][1],            0,   1.0,                       1,  0.0,              1.0              },
    {"PSliceSearch16x8",         &configinput.InterSearch[0][2],            0,   1.0,                       1,  0.0,              1.0              },
    {"PSliceSearch8x16",         &configinput.InterSearch[0][3],            0,   1.0,                       1,  0.0,              1.0              },
    {"PSliceSearch8x8",          &configinput.InterSearch[0][4],            0,   1.0,                       1,  0.0,              1.0              },
    {"PSliceSearch8x4",          &configinput.InterSearch[0][5],            0,   1.0,                       1,  0.0,              1.0              },
    {"PSliceSearch4x8",          &configinput.InterSearch[0][6],            0,   1.0,                       1,  0.0,              1.0              },
    {"PSliceSearch4x4",          &configinput.InterSearch[0][7],            0,   1.0,                       1,  0.0,              1.0              },
    {"BSliceDirect",               &configinput.InterSearch[1][0],            0,   1.0,                       1,  0.0,              1.0              },
    {"BSliceSearch16x16",        &configinput.InterSearch[1][1],            0,   1.0,                       1,  0.0,              1.0              },
    {"BSliceSearch16x8",         &configinput.InterSearch[1][2],            0,   1.0,                       1,  0.0,              1.0              },
    {"BSliceSearch8x16",         &configinput.InterSearch[1][3],            0,   1.0,                       1,  0.0,              1.0              },
    {"BSliceSearch8x8",          &configinput.InterSearch[1][4],            0,   1.0,                       1,  0.0,              1.0              },
    {"BSliceSearch8x4",          &configinput.InterSearch[1][5],            0,   1.0,                       1,  0.0,              1.0              },
    {"BSliceSearch4x8",          &configinput.InterSearch[1][6],            0,   1.0,                       1,  0.0,              1.0              },
    {"BSliceSearch4x4",          &configinput.InterSearch[1][7],            0,   1.0,                       1,  0.0,              1.0              },
    {"DisableIntraInInter",      &configinput.DisableIntraInInter,          0,   0.0,                       1,  0.0,              1.0              },
    {"IntraDisableInterOnly",    &configinput.IntraDisableInterOnly,        0,   0.0,                       1,  0.0,              1.0              },
    {"DisableIntra4x4",          &configinput.DisableIntra4x4,              0,   0.0,                       1,  0.0,              1.0              },       
    {"DisableIntra16x16",        &configinput.DisableIntra16x16,            0,   0.0,                       1,  0.0,              1.0              },   
    {"Intra4x4ParDisable",       &configinput.Intra4x4ParDisable,           0,   0.0,                       1,  0.0,              1.0              },
    {"Intra4x4DiagDisable",      &configinput.Intra4x4DiagDisable,          0,   0.0,                       1,  0.0,              1.0              },
    {"Intra4x4DirDisable",       &configinput.Intra4x4DirDisable,           0,   0.0,                       1,  0.0,              1.0              },
    {"Intra16x16ParDisable",     &configinput.Intra16x16ParDisable,         0,   0.0,                       1,  0.0,              1.0              },
    {"Intra16x16PlaneDisable",   &configinput.Intra16x16PlaneDisable,       0,   0.0,                       1,  0.0,              1.0              },
    {"EnableIPCM",               &configinput.EnableIPCM,                   0,   0.0,                       1,  0.0,              1.0              },
    {"ChromaIntraDisable",       &configinput.ChromaIntraDisable,           0,   0.0,                       1,  0.0,              1.0              },
    // For future use
    {"FastMDEnable",             &configinput.FastMDEnable,                 0,   0.0,                       1,  0.0,              0.0              },           
    {"FastIntraMD",              &configinput.FastIntraMD,                  0,   0.0,                       1,  0.0,              0.0              },    
    {"FastIntra4x4",             &configinput.FastIntra4x4,                 0,   0.0,                       1,  0.0,              0.0              },
    {"FastIntra16x16",           &configinput.FastIntra16x16,               0,   0.0,                       1,  0.0,              0.0              },
    {"FastIntra8x8",             &configinput.FastIntra8x8,                 0,   0.0,                       1,  0.0,              0.0              },
    {"FastIntraChroma",          &configinput.FastIntraChroma,              0,   0.0,                       1,  0.0,              0.0              },
    
#ifdef _ADAPT_LAST_GROUP_
    {"LastFrameNumber",          &configinput.last_frame,                   0,   0.0,                       2,  0.0,              0.0              },
#endif
#ifdef _CHANGE_QP_
    {"ChangeQPI",                &configinput.qp02,                         0,   24.0,                      3,  (double) MIN_QP,  (double) MAX_QP  },
    {"ChangeQPP",                &configinput.qpN2,                         0,   24.0,                      3,  (double) MIN_QP,  (double) MAX_QP  },
    {"ChangeQPB",                &configinput.qpB2,                         0,   24.0,                      3,  (double) MIN_QP,  (double) MAX_QP  },
    {"ChangeQPBSRefOffset",      &configinput.qpBRS2Offset,                 0,   0.0,                       1,-51.0,             51.0              },
    {"ChangeQPStart",            &configinput.qp2start,                     0,   0.0,                       2,  0.0,              0.0              },
#endif
    {"RDOptimization",           &configinput.rdopt,                        0,   0.0,                       1,  0.0,              3.0              },
    {"DistortionSSIM",           &configinput.Distortion[SSIM],             0,   0.0,                       1,  0.0,              1.0              },
    {"DistortionMS_SSIM",        &configinput.Distortion[MS_SSIM],          0,   0.0,                       1,  0.0,              1.0              },
    {"SSIMOverlapSize",          &configinput.SSIMOverlapSize,              0,   1.0,                       2,  1.0,              1.0              },
    {"DistortionYUVtoRGB",       &configinput.DistortionYUVtoRGB,           0,   0.0,                       1,  0.0,              1.0              },
    {"CtxAdptLagrangeMult",      &configinput.CtxAdptLagrangeMult,          0,   0.0,                       1,  0.0,              1.0              },
    {"FastCrIntraDecision",      &configinput.FastCrIntraDecision,          0,   0.0,                       1,  0.0,              1.0              },
    {"DisableThresholding",      &configinput.disthres,                     0,   0.0,                       1,  0.0,              1.0              },
    {"DisableBSkipRDO",          &configinput.nobskip,                      0,   0.0,                       1,  0.0,              1.0              },
    {"LossRateA",                &configinput.LossRateA,                    0,   0.0,                       2,  0.0,              0.0              },
    {"LossRateB",                &configinput.LossRateB,                    0,   0.0,                       2,  0.0,              0.0              },
    {"LossRateC",                &configinput.LossRateC,                    0,   0.0,                       2,  0.0,              0.0              },
    {"NumberOfDecoders",         &configinput.NoOfDecoders,                 0,   0.0,                       2,  0.0,              0.0              },
    {"ErrorConcealment",         &configinput.ErrorConcealment,             0,   0.0,                       2,  0.0,              0.0              },
    {"RestrictRefFrames",        &configinput.RestrictRef ,                 0,   0.0,                       1,  0.0,              1.0              },
#ifdef _LEAKYBUCKET_
    {"NumberofLeakyBuckets",     &configinput.NumberLeakyBuckets,           0,   2.0,                       1,  2.0,              255.0            },
    {"LeakyBucketRateFile",      &configinput.LeakyBucketRateFile,          1,   0.0,                       0,  0.0,              0.0              },
    {"LeakyBucketParamFile",     &configinput.LeakyBucketParamFile,         1,   0.0,                       0,  0.0,              0.0              },
#endif
    {"PicInterlace",             &configinput.PicInterlace,                 0,   0.0,                       1,  0.0,              2.0              },
    {"MbInterlace",              &configinput.MbInterlace,                  0,   0.0,                       1,  0.0,              3.0              },

    {"IntraBottom",              &configinput.IntraBottom,                  0,   0.0,                       1,  0.0,              1.0              },

    {"NumFramesInELayerSubSeq",  &configinput.NumFramesInELSubSeq,          0,   0.0,                       2,  0.0,              0.0              },
    {"RandomIntraMBRefresh",     &configinput.RandomIntraMBRefresh,         0,   0.0,                       2,  0.0,              0.0              },
    {"WeightedPrediction",       &configinput.WeightedPrediction,           0,   0.0,                       1,  0.0,              1.0              },
    {"WeightedBiprediction",     &configinput.WeightedBiprediction,         0,   0.0,                       1,  0.0,              2.0              },
    {"WPMethod",                 &configinput.WPMethod,                     0,   0.0,                       1,  0.0,              1.0              }, 
    {"ChromaWeightSupport",      &configinput.ChromaWeightSupport,          0,   0.0,                       1,  0.0,              1.0              },    
    {"EnhancedBWeightSupport",   &configinput.EnhancedBWeightSupport,       0,   0.0,                       1,  0.0,              1.0              },    
    {"UseWeightedReferenceME",   &configinput.UseWeightedReferenceME,       0,   0.0,                       1,  0.0,              1.0              },
    {"RDPictureDecision",        &configinput.RDPictureDecision,            0,   0.0,                       1,  0.0,              1.0              },
    {"RDPictureIntra",           &configinput.RDPictureIntra,               0,   0.0,                       1,  0.0,              1.0              },
    {"RDPSliceWeightOnly",       &configinput.RDPSliceWeightOnly,           0,   1.0,                       1,  0.0,              2.0              },
    {"RDPSliceBTest",            &configinput.RDPSliceBTest,                0,   0.0,                       1,  0.0,              1.0              },
    {"RDBSliceWeightOnly",       &configinput.RDBSliceWeightOnly,           0,   0.0,                       1,  0.0,              2.0              },

    {"SkipIntraInInterSlices",   &configinput.SkipIntraInInterSlices,       0,   0.0,                       1,  0.0,              1.0              },
    {"BReferencePictures",       &configinput.BRefPictures,                 0,   0.0,                       1,  0.0,              2.0              },
    {"HierarchicalCoding",       &configinput.HierarchicalCoding,           0,   0.0,                       1,  0.0,              3.0              },
    {"HierarchyLevelQPEnable",   &configinput.HierarchyLevelQPEnable,       0,   0.0,                       1,  0.0,              1.0              },
    {"ExplicitHierarchyFormat",  &configinput.ExplicitHierarchyFormat,      1,   0.0,                       0,  0.0,              0.0              },
    {"ReferenceReorder",         &configinput.ReferenceReorder,             0,   0.0,                       1,  0.0,              1.0              },
    {"PocMemoryManagement",      &configinput.PocMemoryManagement,          0,   0.0,                       1,  0.0,              1.0              },

    //Bipredicting Motion Estimation parameters
    {"BiPredMotionEstimation",   &configinput.BiPredMotionEstimation,       0,   0.0,                       1,  0.0,              1.0              },
    {"BiPredSearch16x16",        &configinput.BiPredSearch[0],              0,   1.0,                       1,  0.0,              1.0              },
    {"BiPredSearch16x8",         &configinput.BiPredSearch[1],              0,   0.0,                       1,  0.0,              1.0              },
    {"BiPredSearch8x16",         &configinput.BiPredSearch[2],              0,   0.0,                       1,  0.0,              1.0              },
    {"BiPredSearch8x8",          &configinput.BiPredSearch[3],              0,   0.0,                       1,  0.0,              1.0              },
    
    {"BiPredMERefinements",      &configinput.BiPredMERefinements,          0,   0.0,                       1,  0.0,              5.0              },
    {"BiPredMESearchRange",      &configinput.BiPredMESearchRange,          0,   8.0,                       2,  0.0,              0.0              },
    {"BiPredMESubPel",           &configinput.BiPredMESubPel,               0,   1.0,                       1,  0.0,              2.0              },

    {"DFParametersFlag",         &configinput.DFSendParameters,             0,   0.0,                       1,  0.0,              1.0              },
    {"DFDisableRefISlice",       &configinput.DFDisableIdc[1][I_SLICE],     0,   0.0,                       1,  0.0,              2.0              },
    {"DFDisableNRefISlice",      &configinput.DFDisableIdc[0][I_SLICE],     0,   0.0,                       1,  0.0,              2.0              },
    {"DFDisableRefPSlice",       &configinput.DFDisableIdc[1][P_SLICE],     0,   0.0,                       1,  0.0,              2.0              },
    {"DFDisableNRefPSlice",      &configinput.DFDisableIdc[0][P_SLICE],     0,   0.0,                       1,  0.0,              2.0              },
    {"DFDisableRefBSlice",       &configinput.DFDisableIdc[1][B_SLICE],     0,   0.0,                       1,  0.0,              2.0              },
    {"DFDisableNRefBSlice",      &configinput.DFDisableIdc[0][B_SLICE],     0,   0.0,                       1,  0.0,              2.0              },
    {"DFDisableRefSPSlice",      &configinput.DFDisableIdc[1][SP_SLICE],    0,   0.0,                       1,  0.0,              2.0              },
    {"DFDisableNRefSPSlice",     &configinput.DFDisableIdc[0][SP_SLICE],    0,   0.0,                       1,  0.0,              2.0              },
    {"DFDisableRefSISlice",      &configinput.DFDisableIdc[1][SI_SLICE],    0,   0.0,                       1,  0.0,              2.0              },
    {"DFDisableNRefSISlice",     &configinput.DFDisableIdc[0][SI_SLICE],    0,   0.0,                       1,  0.0,              2.0              },
    {"DFAlphaRefISlice",         &configinput.DFAlpha[1][I_SLICE],          0,   0.0,                       1, -6.0,              6.0              },
    {"DFAlphaNRefISlice",        &configinput.DFAlpha[0][I_SLICE],          0,   0.0,                       1, -6.0,              6.0              },
    {"DFAlphaRefPSlice",         &configinput.DFAlpha[1][P_SLICE],          0,   0.0,                       1, -6.0,              6.0              },
    {"DFAlphaNRefPSlice",        &configinput.DFAlpha[0][P_SLICE],          0,   0.0,                       1, -6.0,              6.0              },
    {"DFAlphaRefBSlice",         &configinput.DFAlpha[1][B_SLICE],          0,   0.0,                       1, -6.0,              6.0              },
    {"DFAlphaNRefBSlice",        &configinput.DFAlpha[0][B_SLICE],          0,   0.0,                       1, -6.0,              6.0              },
    {"DFAlphaRefSPSlice",        &configinput.DFAlpha[1][SP_SLICE],         0,   0.0,                       1, -6.0,              6.0              },
    {"DFAlphaNRefSPSlice",       &configinput.DFAlpha[0][SP_SLICE],         0,   0.0,                       1, -6.0,              6.0              },
    {"DFAlphaRefSISlice",        &configinput.DFAlpha[1][SI_SLICE],         0,   0.0,                       1, -6.0,              6.0              },
    {"DFAlphaNRefSISlice",       &configinput.DFAlpha[0][SI_SLICE],         0,   0.0,                       1, -6.0,              6.0              },
    {"DFBetaRefISlice",          &configinput.DFBeta[1][I_SLICE],           0,   0.0,                       1, -6.0,              6.0              },
    {"DFBetaNRefISlice",         &configinput.DFBeta[0][I_SLICE],           0,   0.0,                       1, -6.0,              6.0              },
    {"DFBetaRefPSlice",          &configinput.DFBeta[1][P_SLICE],           0,   0.0,                       1, -6.0,              6.0              },
    {"DFBetaNRefPSlice",         &configinput.DFBeta[0][P_SLICE],           0,   0.0,                       1, -6.0,              6.0              },
    {"DFBetaRefBSlice",          &configinput.DFBeta[1][B_SLICE],           0,   0.0,                       1, -6.0,              6.0              },
    {"DFBetaNRefBSlice",         &configinput.DFBeta[0][B_SLICE],           0,   0.0,                       1, -6.0,              6.0              },
    {"DFBetaRefSPSlice",         &configinput.DFBeta[1][SP_SLICE],          0,   0.0,                       1, -6.0,              6.0              },
    {"DFBetaNRefSPSlice",        &configinput.DFBeta[0][SP_SLICE],          0,   0.0,                       1, -6.0,              6.0              },
    {"DFBetaRefSISlice",         &configinput.DFBeta[1][SI_SLICE],          0,   0.0,                       1, -6.0,              6.0              },
    {"DFBetaNRefSISlice",        &configinput.DFBeta[0][SI_SLICE],          0,   0.0,                       1, -6.0,              6.0              },

    {"SparePictureOption",       &configinput.SparePictureOption,           0,   0.0,                       1,  0.0,              1.0              },
    {"SparePictureDetectionThr", &configinput.SPDetectionThreshold,         0,   0.0,                       2,  0.0,              0.0              },
    {"SparePicturePercentageThr",&configinput.SPPercentageThreshold,        0,   0.0,                       2,  0.0,            100.0              },

    {"num_slice_groups_minus1",  &configinput.num_slice_groups_minus1,      0,   0.0,                       1,  0.0,  (double)MAXSLICEGROUPIDS - 1 },
    {"slice_group_map_type",     &configinput.slice_group_map_type,         0,   0.0,                       1,  0.0,              6.0              },
    {"slice_group_change_direction_flag", &configinput.slice_group_change_direction_flag, 0,   0.0,         1,  0.0,              2.0              },
    {"slice_group_change_rate_minus1",    &configinput.slice_group_change_rate_minus1,    0,   0.0,         2,  0.0,              1.0              },
    {"SliceGroupConfigFileName", &configinput.SliceGroupConfigFileName,     1,   0.0,                       0,  0.0,              0.0              },

    {"UseRedundantPicture",      &configinput.redundant_pic_flag,           0,   0.0,                       1,  0.0,              1.0              },
    {"NumRedundantHierarchy",    &configinput.NumRedundantHierarchy,        0,   0.0,                       1,  0.0,              4.0              },
    {"PrimaryGOPLength",         &configinput.PrimaryGOPLength,             0,   1.0,                       1,  1.0,              16.0             },
    {"NumRefPrimary",            &configinput.NumRefPrimary,                0,   1.0,                       1,  1.0,              16.0             },

    {"PicOrderCntType",          &configinput.pic_order_cnt_type,           0,   0.0,                       1,  0.0,              2.0              },

    {"ContextInitMethod",        &configinput.context_init_method,          0,   0.0,                       1,  0.0,              1.0              },
    {"FixedModelNumber",         &configinput.model_number,                 0,   0.0,                       1,  0.0,              2.0              },

    {"ReportFrameStats",         &configinput.ReportFrameStats,             0,   0.0,                       1,  0.0,              1.0              },
    {"DisplayEncParams",         &configinput.DisplayEncParams,             0,   0.0,                       1,  0.0,              1.0              },
    {"Verbose",                  &configinput.Verbose,                      0,   1.0,                       1,  0.0,              2.0              },

    {"ChromaMCBuffer",           &configinput.ChromaMCBuffer,               0,   0.0,                       1,  0.0,              1.0              },
    {"ChromaMEEnable",           &configinput.ChromaMEEnable,               0,   0.0,                       1,  0.0,              2.0              },
    {"ChromaMEWeight",           &configinput.ChromaMEWeight,               0,   1.0,                       2,  1.0,              0.0              },
    {"MEDistortionFPel",         &configinput.MEErrorMetric[F_PEL],         0,   0.0,                       1,  0.0,              3.0              },
    {"MEDistortionHPel",         &configinput.MEErrorMetric[H_PEL],         0,   0.0,                       1,  0.0,              3.0              },
    {"MEDistortionQPel",         &configinput.MEErrorMetric[Q_PEL],         0,   0.0,                       1,  0.0,              3.0              },
    {"MDDistortion",             &configinput.ModeDecisionMetric,           0,   2.0,                       1,  0.0,              2.0              },
    {"SkipDeBlockNonRef",        &configinput.SkipDeBlockNonRef,            0,   0.0,                       1,  0.0,              1.0              },

    // Rate Control
    {"RateControlEnable",        &configinput.RCEnable,                     0,   0.0,                       1,  0.0,              1.0              },
    {"Bitrate",                  &configinput.bit_rate,                     0,   0.0,                       2,  0.0,              0.0              },
    {"InitialQP",                &configinput.SeinitialQP,                  0,   0.0,                       3,  (double) MIN_QP,  (double) MAX_QP  },
    {"BasicUnit",                &configinput.basicunit,                    0,   0.0,                       2,  0.0,              0.0              },
    {"ChannelType",              &configinput.channel_type,                 0,   0.0,                       1,  0.0,              1.0              },
    {"RCUpdateMode",             &configinput.RCUpdateMode,                 0,   0.0,                       1,  0.0,              4.0              },
    {"RCISliceBitRatio",         &configinput.RCISliceBitRatio,             2,   1.0,                       1,  0.0,              20.0             },
    {"RCBSliceBitRatio0",        &configinput.RCBSliceBitRatio[0],          2,   0.5,                       1,  0.0,              20.0             },
    {"RCBSliceBitRatio1",        &configinput.RCBSliceBitRatio[1],          2,   0.25,                      1,  0.0,              20.0             },
    {"RCBSliceBitRatio2",        &configinput.RCBSliceBitRatio[2],          2,   0.25,                      1,  0.0,              20.0             },
    {"RCBSliceBitRatio3",        &configinput.RCBSliceBitRatio[3],          2,   0.25,                      1,  0.0,              20.0             },
    {"RCBSliceBitRatio4",        &configinput.RCBSliceBitRatio[4],          2,   0.25,                      1,  0.0,              20.0             },
    {"RCBoverPRatio",            &configinput.RCBoverPRatio,                2,   0.45,                      1,  0.0,              1000.0           },
    {"RCIoverPRatio",            &configinput.RCIoverPRatio,                2,   3.80,                      1,  0.0,              1000.0           },
    {"RCMinQPPSlice",            &configinput.RCMinQP[P_SLICE],             0,   (double) MIN_QP,           3,  (double) MIN_QP,  (double) MAX_QP  },
    {"RCMaxQPPSlice",            &configinput.RCMaxQP[P_SLICE],             0,   (double) MAX_QP,           3,  (double) MIN_QP,  (double) MAX_QP  },
    {"RCMinQPBSlice",            &configinput.RCMinQP[B_SLICE],             0,   (double) MIN_QP,           3,  (double) MIN_QP,  (double) MAX_QP  },
    {"RCMaxQPBSlice",            &configinput.RCMaxQP[B_SLICE],             0,   (double) MAX_QP,           3,  (double) MIN_QP,  (double) MAX_QP  },
    {"RCMinQPISlice",            &configinput.RCMinQP[I_SLICE],             0,   (double) MIN_QP,           3,  (double) MIN_QP,  (double) MAX_QP  },
    {"RCMaxQPISlice",            &configinput.RCMaxQP[I_SLICE],             0,   (double) MAX_QP,           3,  (double) MIN_QP,  (double) MAX_QP  },
    {"RCMinQPSPSlice",           &configinput.RCMinQP[SP_SLICE],            0,   (double) MIN_QP,           3,  (double) MIN_QP,  (double) MAX_QP  },
    {"RCMaxQPSPSlice",           &configinput.RCMaxQP[SP_SLICE],            0,   (double) MAX_QP,           3,  (double) MIN_QP,  (double) MAX_QP  },
    {"RCMinQPSISlice",           &configinput.RCMinQP[SI_SLICE],            0,   (double) MIN_QP,           3,  (double) MIN_QP,  (double) MAX_QP  },
    {"RCMaxQPSISlice",           &configinput.RCMaxQP[SI_SLICE],            0,   (double) MAX_QP,           3,  (double) MIN_QP,  (double) MAX_QP  },
    {"RCMaxQPChange",            &configinput.RCMaxQPChange,                0,   4.0,                       1,  0.0,              10.0             },

    // Q_Matrix
    {"QmatrixFile",              &configinput.QmatrixFile,                  1,   0.0,                       0,  0.0,              0.0              },
    {"ScalingMatrixPresentFlag", &configinput.ScalingMatrixPresentFlag,     0,   0.0,                       1,  0.0,              3.0              },
    {"ScalingListPresentFlag0",  &configinput.ScalingListPresentFlag[0],    0,   0.0,                       1,  0.0,              3.0              },
    {"ScalingListPresentFlag1",  &configinput.ScalingListPresentFlag[1],    0,   0.0,                       1,  0.0,              3.0              },
    {"ScalingListPresentFlag2",  &configinput.ScalingListPresentFlag[2],    0,   0.0,                       1,  0.0,              3.0              },
    {"ScalingListPresentFlag3",  &configinput.ScalingListPresentFlag[3],    0,   0.0,                       1,  0.0,              3.0              },
    {"ScalingListPresentFlag4",  &configinput.ScalingListPresentFlag[4],    0,   0.0,                       1,  0.0,              3.0              },
    {"ScalingListPresentFlag5",  &configinput.ScalingListPresentFlag[5],    0,   0.0,                       1,  0.0,              3.0              },
    {"ScalingListPresentFlag6",  &configinput.ScalingListPresentFlag[6],    0,   0.0,                       1,  0.0,              3.0              },
    {"ScalingListPresentFlag7",  &configinput.ScalingListPresentFlag[7],    0,   0.0,                       1,  0.0,              3.0              },
    {"ScalingListPresentFlag8",  &configinput.ScalingListPresentFlag[8],    0,   0.0,                       1,  0.0,              3.0              },
    {"ScalingListPresentFlag9",  &configinput.ScalingListPresentFlag[9],    0,   0.0,                       1,  0.0,              3.0              },
    {"ScalingListPresentFlag10", &configinput.ScalingListPresentFlag[10],   0,   0.0,                       1,  0.0,              3.0              },
    {"ScalingListPresentFlag11", &configinput.ScalingListPresentFlag[11],   0,   0.0,                       1,  0.0,              3.0              },

    // Chroma QP Offset
    {"ChromaQPOffset",           &configinput.chroma_qp_index_offset,       0,   0.0,                       1,-51.0,             51.0              },

    // Fidelity Range Extensions
// New Input bit-depth entries
    {"SourceBitDepthLuma",       &configinput.source.bit_depth[0],          0,   8.0,                       1,  8.0,             14.0              },
    {"SourceBitDepthChroma",     &configinput.source.bit_depth[1],          0,   8.0,                       1,  8.0,             14.0              },
    {"SourceBitDepthRescale",    &configinput.src_BitDepthRescale,          0,   0.0,                       1,  0.0,              1.0              },
// Old variables replaced from BitDepthLuma and BitDepthChroma. Considered only if SourceBitDepthRescale is 1.
    {"OutputBitDepthLuma",       &configinput.output.bit_depth[0],          0,   8.0,                       1,  8.0,             14.0              },
    {"OutputBitDepthChroma",     &configinput.output.bit_depth[1],          0,   8.0,                       1,  8.0,             14.0              },

    {"YUVFormat",                &configinput.yuv_format,                   0,   1.0,                       1,  0.0,              3.0              },
    {"RGBInput",                 &configinput.rgb_input_flag,               0,   0.0,                       1,  0.0,              1.0              },
    {"CbQPOffset",               &configinput.cb_qp_index_offset,           0,   0.0,                       1,-51.0,             51.0              },
    {"CrQPOffset",               &configinput.cr_qp_index_offset,           0,   0.0,                       1,-51.0,             51.0              },
    {"Transform8x8Mode",         &configinput.Transform8x8Mode,             0,   0.0,                       1,  0.0,              2.0              },

    // Lossless Coding
    {"QPPrimeYZeroTransformBypassFlag", &configinput.lossless_qpprime_y_zero_flag,      0,   0.0,           1,  0.0,              1.0              },

    // Explicit Lambda Parameters for RDO
    {"UseExplicitLambdaParams",  &configinput.UseExplicitLambdaParams,      0,   0.0,                       1,  0.0,              3.0              },
    {"UpdateLambdaChromaME",     &configinput.UpdateLambdaChromaME,         0,   0.0,                       1,  0.0,              3.0              },    
    {"FixedLambdaPSlice",        &configinput.FixedLambda[0],               2,   0.1,                       2,  0.0,              0.0              },
    {"FixedLambdaBSlice",        &configinput.FixedLambda[1],               2,   0.1,                       2,  0.0,              0.0              },
    {"FixedLambdaISlice",        &configinput.FixedLambda[2],               2,   0.1,                       2,  0.0,              0.0              },
    {"FixedLambdaSPSlice",       &configinput.FixedLambda[3],               2,   0.1,                       2,  0.0,              0.0              },
    {"FixedLambdaSISlice",       &configinput.FixedLambda[4],               2,   0.1,                       2,  0.0,              0.0              },
    {"FixedLambdaRefBSlice",     &configinput.FixedLambda[5],               2,   0.1,                       2,  0.0,              0.0              },
    {"LambdaWeightPSlice",       &configinput.LambdaWeight[0],              2,   0.68,                      2,  0.0,              0.0              },
    {"LambdaWeightBSlice",       &configinput.LambdaWeight[1],              2,   2.00,                      2,  0.0,              0.0              },
    {"LambdaWeightISlice",       &configinput.LambdaWeight[2],              2,   0.65,                      2,  0.0,              0.0              },
    {"LambdaWeightSPSlice",      &configinput.LambdaWeight[3],              2,   1.50,                      2,  0.0,              0.0              },
    {"LambdaWeightSISlice",      &configinput.LambdaWeight[4],              2,   0.65,                      2,  0.0,              0.0              },
    {"LambdaWeightRefBSlice",    &configinput.LambdaWeight[5],              2,   1.50,                      2,  0.0,              0.0              },
    
    // Usage of explicit "initial" offsets for quantization
    {"QOffsetMatrixFile",        &configinput.QOffsetMatrixFile,            1,   0.0,                       0,  0.0,              0.0              },
    {"OffsetMatrixPresentFlag",  &configinput.OffsetMatrixPresentFlag,      0,   0.0,                       1,  0.0,              1.0              },

    // Adaptive rounding technique based on JVT-N011
    {"AdaptiveRounding",         &configinput.AdaptiveRounding,             0,   0.0,                       1,  0.0,              1.0              },
    {"AdaptRoundingFixed",       &configinput.AdaptRoundingFixed,           0,   1.0,                       1,  0.0,              1.0              },    
    {"AdaptRndPeriod",           &configinput.AdaptRndPeriod,               0,  16.0,                       2,  1.0,              0.0              },
    {"AdaptRndChroma",           &configinput.AdaptRndChroma,               0,   0.0,                       1,  0.0,              1.0              },
    {"AdaptRndWFactorIRef",      &configinput.AdaptRndWFactor[1][I_SLICE],  0,   4.0,                       1,  0.0,           4096.0              },
    {"AdaptRndWFactorPRef",      &configinput.AdaptRndWFactor[1][P_SLICE],  0,   4.0,                       1,  0.0,           4096.0              },
    {"AdaptRndWFactorBRef",      &configinput.AdaptRndWFactor[1][B_SLICE],  0,   4.0,                       1,  0.0,           4096.0              },
    {"AdaptRndWFactorINRef",     &configinput.AdaptRndWFactor[0][I_SLICE],  0,   4.0,                       1,  0.0,           4096.0              },
    {"AdaptRndWFactorPNRef",     &configinput.AdaptRndWFactor[0][P_SLICE],  0,   4.0,                       1,  0.0,           4096.0              },
    {"AdaptRndWFactorBNRef",     &configinput.AdaptRndWFactor[0][B_SLICE],  0,   4.0,                       1,  0.0,           4096.0              },

    {"AdaptRndCrWFactorIRef",    &configinput.AdaptRndCrWFactor[1][I_SLICE],0,   4.0,                       1,  0.0,           4096.0              },
    {"AdaptRndCrWFactorPRef",    &configinput.AdaptRndCrWFactor[1][P_SLICE],0,   4.0,                       1,  0.0,           4096.0              },
    {"AdaptRndCrWFactorBRef",    &configinput.AdaptRndCrWFactor[1][B_SLICE],0,   4.0,                       1,  0.0,           4096.0              },
    {"AdaptRndCrWFactorINRef",   &configinput.AdaptRndCrWFactor[0][I_SLICE],0,   4.0,                       1,  0.0,           4096.0              },
    {"AdaptRndCrWFactorPNRef",   &configinput.AdaptRndCrWFactor[0][P_SLICE],0,   4.0,                       1,  0.0,           4096.0              },
    {"AdaptRndCrWFactorBNRef",   &configinput.AdaptRndCrWFactor[0][B_SLICE],0,   4.0,                       1,  0.0,           4096.0              },

    // Fast Mode Decision
    {"EarlySkipEnable",          &configinput.EarlySkipEnable,              0,   0.0,                       1,  0.0,              1.0              },
    {"SelectiveIntraEnable",     &configinput.SelectiveIntraEnable,         0,   0.0,                       1,  0.0,              1.0              },

    //================================
    // Motion Estimation (ME) Parameters
    //================================
    {"RestrictSearchRange",      &configinput.full_search,                  0,   2.0,                       1,  0.0,              2.0              },
    // Fast ME enable
    {"SearchMode",               &configinput.SearchMode,                   0,   0.0,                       1, -1.0,              4.0              },
    // Parameters for UMHEX control
    {"UMHexDSR",                 &configinput.UMHexDSR,                     0,   1.0,                       1,  0.0,              1.0              },
    {"UMHexScale",               &configinput.UMHexScale,                   0,   1.0,                       0,  0.0,              0.0              },
    // Parameters for EPZS control
    {"EPZSPattern",              &configinput.EPZSPattern,                  0,   2.0,                       1,  0.0,              5.0              },
    {"EPZSDualRefinement",       &configinput.EPZSDual,                     0,   3.0,                       1,  0.0,              6.0              },
    {"EPZSFixedPredictors",      &configinput.EPZSFixed,                    0,   2.0,                       1,  0.0,              2.0              },
    {"EPZSTemporal",             &configinput.EPZSTemporal,                 0,   1.0,                       1,  0.0,              1.0              },
    {"EPZSSpatialMem",           &configinput.EPZSSpatialMem,               0,   1.0,                       1,  0.0,              1.0              },
    {"EPZSBlockType",            &configinput.EPZSBlockType,                0,   1.0,                       1,  0.0,              1.0              },
    {"EPZSMinThresScale",        &configinput.EPZSMinThresScale,            0,   0.0,                       0,  0.0,              0.0              },
    {"EPZSMaxThresScale",        &configinput.EPZSMaxThresScale,            0,   2.0,                       0,  0.0,              0.0              },
    {"EPZSMedThresScale",        &configinput.EPZSMedThresScale,            0,   1.0,                       0,  0.0,              0.0              },
    {"EPZSSubPelME",             &configinput.EPZSSubPelME,                 0,   1.0,                       1,  0.0,              1.0              },
    {"EPZSSubPelMEBiPred",       &configinput.EPZSSubPelMEBiPred,           0,   1.0,                       1,  0.0,              1.0              },
    {"EPZSSubPelGrid",           &configinput.EPZSSubPelGrid,               0,   0.0,                       1,  0.0,              1.0              },
    {"EPZSSubPelThresScale",     &configinput.EPZSSubPelThresScale,         0,   2.0,                       0,  0.0,              0.0              },

    // Tone mapping SEI cfg file
    {"ToneMappingSEIPresentFlag",&configinput.ToneMappingSEIPresentFlag,    0,   0.0,                       1,  0.0,              1.0              },
    {"ToneMappingFile",          &configinput.ToneMappingFile,              1,   0.0,                       0,  0.0,              0.0              },

    {"SeparateColourPlane",      &configinput.separate_colour_plane_flag,   0,   0.0,                       1,  0.0,              1.0              },
    {"WeightY",                  &configinput.WeightY,                      2,   1.00,                      1,  0.0,              4.0              },
    {"WeightCb",                 &configinput.WeightCb,                     2,   1.00,                      1,  0.0,              4.0              },
    {"WeightCr",                 &configinput.WeightCr,                     2,   1.00,                      1,  0.0,              4.0              },
    {"WPMCPrecision",            &configinput.WPMCPrecision,                0,   0.0,                       1,  0.0,              2.0              },
    {"WPMCPrecFullRef",          &configinput.WPMCPrecFullRef,              0,   0.0,                       1,  0.0,              1.0              },
    {"WPMCPrecBSlice",           &configinput.WPMCPrecBSlice,               0,   1.0,                       1,  0.0,              2.0              },

    // Trellis based quantization
    {"UseRDOQuant",              &configinput.UseRDOQuant,                  0,   0.0,                       1,  0.0,              1.0              },
    {"RDOQ_DC",                  &configinput.RDOQ_DC,                      0,   1.0,                       1,  0.0,              1.0              },
    {"RDOQ_CR",                  &configinput.RDOQ_CR,                      0,   1.0,                       1,  0.0,              1.0              },
    {"RDOQ_QP_Num",              &configinput.RDOQ_QP_Num,                  0,   1.0,                       1,  1.0,              9.0              },
    {"RDOQ_CP_Mode",             &configinput.RDOQ_CP_Mode,                 0,   0.0,                       1,  0.0,              1.0              },
    {"RDOQ_CP_MV",               &configinput.RDOQ_CP_MV,                   0,   0.0,                       1,  0.0,              1.0              },
    {"RDOQ_Fast",                &configinput.RDOQ_Fast,                    0,   0.0,                       1,  0.0,              1.0              },
    // VUI parameters
    {"GenerateSEIMessage",       &configinput.GenerateSEIMessage,           0,   0.0,                       1,  0.0,              1.0              },
    {"EnableVUISupport",         &configinput.EnableVUISupport,             0,   0.0,                       1,  0.0,              1.0              },
    {"VUI_aspect_ratio_info_present_flag",     &configinput.VUI.aspect_ratio_info_present_flag,     0,   0.0,                       1,  0.0,              1.0              },
    {"VUI_aspect_ratio_idc",                   &configinput.VUI.aspect_ratio_idc,                   0,   0.0,                       1,  0.0,            255.0              },
    {"VUI_sar_width",                          &configinput.VUI.sar_width,                          0,   0.0,                       2,  0.0,              0.0              },
    {"VUI_sar_height",                         &configinput.VUI.sar_height,                         0,   0.0,                       2,  0.0,              0.0              },
    {"VUI_overscan_info_present_flag",         &configinput.VUI.overscan_info_present_flag,         0,   0.0,                       1,  0.0,              1.0              },
    {"VUI_overscan_appropriate_flag",          &configinput.VUI.overscan_appropriate_flag,          0,   0.0,                       1,  0.0,              1.0              },
    {"VUI_video_signal_type_present_flag",     &configinput.VUI.video_signal_type_present_flag,     0,   0.0,                       1,  0.0,              1.0              },
    {"VUI_video_format",                       &configinput.VUI.video_format,                       0,   5.0,                       1,  0.0,              7.0              },
    {"VUI_video_full_range_flag",              &configinput.VUI.video_full_range_flag,              0,   0.0,                       1,  0.0,              1.0              },
    {"VUI_colour_description_present_flag",    &configinput.VUI.colour_description_present_flag,    0,   0.0,                       1,  0.0,              1.0              },
    {"VUI_colour_primaries",                   &configinput.VUI.colour_primaries,                   0,   2.0,                       1,  0.0,            255.0              },
    {"VUI_transfer_characteristics",           &configinput.VUI.transfer_characteristics,           0,   2.0,                       1,  0.0,            255.0              },
    {"VUI_matrix_coefficients",                &configinput.VUI.matrix_coefficients,                0,   2.0,                       1,  0.0,            255.0              },
    {"VUI_chroma_location_info_present_flag",  &configinput.VUI.chroma_location_info_present_flag,  0,   0.0,                       1,  0.0,              1.0              },
    {"VUI_chroma_sample_loc_type_top_field",   &configinput.VUI.chroma_sample_loc_type_top_field,   0,   0.0,                       1,  0.0,              5.0              },
    {"VUI_chroma_sample_loc_type_bottom_field",&configinput.VUI.chroma_sample_loc_type_bottom_field,0,   0.0,                       1,  0.0,              5.0              },
    {"VUI_timing_info_present_flag",           &configinput.VUI.timing_info_present_flag,           0,   0.0,                       1,  0.0,              1.0              },
    {"VUI_num_units_in_tick",                  &configinput.VUI.num_units_in_tick,                  0,1000.0,                       2,  0.0,              0.0              },
    {"VUI_time_scale",                         &configinput.VUI.time_scale,                         0,60000.0,                      2,  0.0,              0.0              },
    {"VUI_fixed_frame_rate_flag",              &configinput.VUI.fixed_frame_rate_flag,              0,   0.0,                       1,  0.0,              1.0              },
    {"VUI_nal_hrd_parameters_present_flag",    &configinput.VUI.nal_hrd_parameters_present_flag,    0,   0.0,                       1,  0.0,              1.0              },
    {"VUI_nal_cpb_cnt_minus1",                 &configinput.VUI.nal_cpb_cnt_minus1,                 0,   0.0,                       1,  0.0,             31.0              },
    {"VUI_nal_bit_rate_scale",                 &configinput.VUI.nal_bit_rate_scale,                 0,   0.0,                       2,  0.0,              0.0              },
    {"VUI_nal_cpb_size_scale",                 &configinput.VUI.nal_cpb_size_scale,                 0,   0.0,                       2,  0.0,              0.0              },
    {"VUI_nal_bit_rate_value_minus1",          &configinput.VUI.nal_bit_rate_value_minus1,          0,   0.0,                       2,  0.0,              0.0              },
    {"VUI_nal_cpb_size_value_minus1",          &configinput.VUI.nal_cpb_size_value_minus1,          0,   0.0,                       2,  0.0,              0.0              },
    {"VUI_nal_vbr_cbr_flag",                   &configinput.VUI.nal_vbr_cbr_flag,                   0,   1.0,                       1,  0.0,              1.0              },
    {"VUI_nal_initial_cpb_removal_delay_length_minus1", &configinput.VUI.nal_initial_cpb_removal_delay_length_minus1,  0, 23.0,     2,  0.0,              0.0              },
    {"VUI_nal_cpb_removal_delay_length_minus1",&configinput.VUI.nal_cpb_removal_delay_length_minus1,0,  23.0,                       2,  0.0,              0.0              },
    {"VUI_nal_dpb_output_delay_length_minus1", &configinput.VUI.nal_dpb_output_delay_length_minus1, 0,  23.0,                       2,  0.0,              0.0              },
    {"VUI_nal_time_offset_length",             &configinput.VUI.nal_time_offset_length,             0,  24.0,                       2,  0.0,              0.0              },
    {"VUI_vcl_hrd_parameters_present_flag",    &configinput.VUI.vcl_hrd_parameters_present_flag,    0,   0.0,                       1,  0.0,              1.0              },
    {"VUI_vcl_cpb_cnt_minus1",                 &configinput.VUI.vcl_cpb_cnt_minus1,                 0,   0.0,                       1,  0.0,             31.0              },
    {"VUI_vcl_bit_rate_scale",                 &configinput.VUI.vcl_bit_rate_scale,                 0,   0.0,                       2,  0.0,              0.0              },
    {"VUI_vcl_cpb_size_scale",                 &configinput.VUI.vcl_cpb_size_scale,                 0,   0.0,                       2,  0.0,              0.0              },
    {"VUI_vcl_bit_rate_value_minus1",          &configinput.VUI.vcl_bit_rate_value_minus1,          0,   0.0,                       2,  0.0,              0.0              },
    {"VUI_vcl_cpb_size_value_minus1",          &configinput.VUI.vcl_cpb_size_value_minus1,          0,   0.0,                       2,  0.0,              0.0              },
    {"VUI_vcl_vbr_cbr_flag",                   &configinput.VUI.vcl_vbr_cbr_flag,                   0,   0.0,                       1,  0.0,              1.0              },
    {"VUI_vcl_initial_cpb_removal_delay_length_minus1", &configinput.VUI.vcl_initial_cpb_removal_delay_length_minus1,  0,  23.0,    2,  0.0,              0.0              },
    {"VUI_vcl_cpb_removal_delay_length_minus1",&configinput.VUI.vcl_cpb_removal_delay_length_minus1,0,  23.0,                       2,  0.0,              0.0              },
    {"VUI_vcl_dpb_output_delay_length_minus1", &configinput.VUI.vcl_dpb_output_delay_length_minus1, 0,  23.0,                       2,  0.0,              0.0              },
    {"VUI_vcl_time_offset_length",             &configinput.VUI.vcl_time_offset_length,             0,  24.0,                       2,  0.0,              0.0              },
    {"VUI_low_delay_hrd_flag",                 &configinput.VUI.low_delay_hrd_flag,                 0,   0.0,                       1,  0.0,              1.0              },
    {"VUI_pic_struct_present_flag",            &configinput.VUI.pic_struct_present_flag,            0,   0.0,                       1,  0.0,              1.0              },
    {"VUI_bitstream_restriction_flag",         &configinput.VUI.bitstream_restriction_flag,         0,   0.0,                       1,  0.0,              1.0              },
    {"VUI_motion_vectors_over_pic_boundaries_flag", &configinput.VUI.motion_vectors_over_pic_boundaries_flag,      0,   1.0,        1,  0.0,              1.0              },
    {"VUI_max_bytes_per_pic_denom",            &configinput.VUI.max_bytes_per_pic_denom,            0,   2.0,                       1,  0.0,             16.0              },
    {"VUI_max_bits_per_mb_denom",              &configinput.VUI.max_bits_per_mb_denom,              0,   1.0,                       1,  0.0,             16.0              },
    {"VUI_log2_max_mv_length_vertical",        &configinput.VUI.log2_max_mv_length_vertical,        0,  16.0,                       1,  0.0,             16.0              },
    {"VUI_log2_max_mv_length_horizontal",      &configinput.VUI.log2_max_mv_length_horizontal,      0,  16.0,                       1,  0.0,             16.0              },
    {"VUI_num_reorder_frames",                 &configinput.VUI.num_reorder_frames,                 0,  16.0,                       1,  0.0,             16.0              },
    {"VUI_max_dec_frame_buffering",            &configinput.VUI.max_dec_frame_buffering,            0,  16.0,                       1,  0.0,             16.0              },
    {"SEIMessageText",           &configinput.SEIMessageText,               1,   0.0,                       0,  0.0,              0.0              },
    {NULL,                       NULL,                                     -1,   0.0,                       0,  0.0,              0.0              }
};

#endif

#ifndef INCLUDED_BY_CONFIGFILE_C
extern Mapping Map[];
#endif


void Configure (int ac, char *av[]);
void PatchInputNoFrames(void);

#endif

