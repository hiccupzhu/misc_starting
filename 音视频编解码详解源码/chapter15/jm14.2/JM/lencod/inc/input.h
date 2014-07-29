
/*!
 ************************************************************************
 * \file input.h
 *
 * \brief
 *    Input related definitions
 *
 * \author
 *
 ************************************************************************
 */

#ifndef _INPUT_H_
#define _INPUT_H_

int testEndian(void);
void initInput(FrameFormat *source, FrameFormat *output);
int AllocateFrameMemory (ImageParameters *img, int size);
void DeleteFrameMemory (void);

void ReadOneFrame (int FrameNoInFile, int HeaderSize, FrameFormat *source, FrameFormat *output);
extern void (*buf2img) ( imgpel** imgX, unsigned char* buf, int size_x, int size_y, int osize_x, int o_size_y, int symbol_size_in_bytes, int bitshift);

#endif

