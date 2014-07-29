
/*!
 ************************************************************************
 * \file image.h
 *
 * \brief
 *    headers for image processing
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Karsten Sühring                 <suehring@hhi.de> 
 *     - Inge Lille-Langoy               <inge.lille-langoy@telenor.com>
 *     - Alexis Michael Tourapis         <alexismt@ieee.org> 
 *  
 ************************************************************************
 */
#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "mbuffer.h"

extern StorablePicture *enc_picture;
extern StorablePicture **enc_frame_picture;
extern StorablePicture **enc_field_picture;
extern StorablePicture *enc_frame_picture_JV[MAX_PLANE];  //!< enc_frame to be used during 4:4:4 independent mode encoding

int  encode_one_frame (void);
Boolean dummy_slice_too_big(int bits_slice);
void copy_rdopt_data (Macroblock *currMB, int field_type);       // For MB level field/frame coding tools

void UnifiedOneForthPix (StorablePicture *s);
// For 4:4:4 independent mode
void UnifiedOneForthPix_JV (int nplane, StorablePicture *s);
void frame_picture (Picture *frame, int rd_pass);


#endif

