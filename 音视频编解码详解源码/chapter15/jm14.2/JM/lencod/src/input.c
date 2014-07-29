
/*!
 *************************************************************************************
 * \file input.c
 *
 * \brief
 *    Input related functions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Karsten Sühring                 <suehring@hhi.de>
 *     - Alexis Michael Tourapis         <alexismt@ieee.org>
 *     
 *************************************************************************************
 */
#include "contributors.h"

#include <math.h>
#include <time.h>
#include <sys/timeb.h>

#include "global.h"
#include "input.h"
#include "report.h"

unsigned char *buf;
 
void buf2img_basic    ( imgpel** imgX, unsigned char* buf, int size_x, int size_y, int o_size_x, int o_size_y, int symbol_size_in_bytes, int bitshift);
void buf2img_endian   ( imgpel** imgX, unsigned char* buf, int size_x, int size_y, int o_size_x, int o_size_y, int symbol_size_in_bytes, int bitshift);
void buf2img_bitshift ( imgpel** imgX, unsigned char* buf, int size_x, int size_y, int o_size_x, int o_size_y, int symbol_size_in_bytes, int bitshift);
void (*buf2img)       ( imgpel** imgX, unsigned char* buf, int size_x, int size_y, int o_size_x, int o_size_y, int symbol_size_in_bytes, int bitshift);

/*!
 ************************************************************************
 * \brief
 *      checks if the System is big- or little-endian
 * \return
 *      0, little-endian (e.g. Intel architectures)
 *      1, big-endian (e.g. SPARC, MIPS, PowerPC)
 ************************************************************************
 */
void initInput(FrameFormat *source, FrameFormat *output)
{
  if (source->bit_depth[0] == output->bit_depth[0] && source->bit_depth[1] == output->bit_depth[1])
  {
    if (( sizeof(char) != sizeof (imgpel)) && testEndian())
      buf2img = buf2img_endian;
    else
      buf2img = buf2img_basic;
  }
  else
    buf2img = buf2img_bitshift;
}


/*!
 ************************************************************************
 * \brief
 *      checks if the System is big- or little-endian
 * \return
 *      0, little-endian (e.g. Intel architectures)
 *      1, big-endian (e.g. SPARC, MIPS, PowerPC)
 ************************************************************************
 */
int testEndian(void)
{
  short s;
  byte *p;

  p=(byte*)&s;

  s=1;

  return (*p==0);
}

#if (DEBUG_BITDEPTH)
/*!
 ************************************************************************
 * \brief
 *    Masking to ensure data within appropriate range
 ************************************************************************
 */
static void MaskMSBs (imgpel** imgX, int mask, int width, int height)
{
  int i,j;

  for (j=0; j < height; j++)
  {
    for (i=0; i < width; i++)
    {
      imgX[j][i]=(imgpel) (imgX[j][i] & mask);
    }
  }
}
#endif

/*!
 ************************************************************************
 * \brief
 *    Convert file read buffer to source picture structure
 ************************************************************************
 */
void buf2img_bitshift ( imgpel** imgX,           //!< Pointer to image plane
                       unsigned char* buf,       //!< Buffer for file output
                       int size_x,               //!< horizontal size of picture
                       int size_y,               //!< vertical size of picture
                       int o_size_x,             //!< horizontal size of picture
                       int o_size_y,             //!< vertical size of picture
                       int symbol_size_in_bytes, //!< number of bytes in file used for one pixel
                       int bitshift              //!< variable for bitdepth expansion
                       )
{
  int i,j;

  unsigned short tmp16, ui16;
  unsigned long  tmp32, ui32;
  // This test should be done once.
  if (((symbol_size_in_bytes << 3) - bitshift) > (sizeof(imgpel)<< 3))
  {
    error ("Source picture has higher bit depth than imgpel data type. \nPlease recompile with larger data type for imgpel.", 500);
  }

  if (testEndian())
  {
    if (size_x != o_size_x || size_y != o_size_y)
    {
      error ("Rescaling not supported in big endian architectures. ", 500);
    }

    // big endian
    switch (symbol_size_in_bytes)
    {
    case 1:
      {
        for(j = 0; j < o_size_y; j++)
          for(i = 0; i < o_size_x; i++)
          {
            imgX[j][i]= (imgpel) rshift_rnd(buf[i + j*size_x], bitshift);
          }
          break;
      }
    case 2:
      {
        for(j = 0; j < o_size_y; j++)
          for(i = 0; i < o_size_x; i++)
          {
            memcpy(&tmp16, buf+((i+j*size_x)*2), 2);
            ui16  = (tmp16 >> 8) | ((tmp16&0xFF)<<8);
            imgX[j][i] = (imgpel) rshift_rnd(ui16, bitshift);
          }
          break;
      }
    case 4:
      {
        for(j = 0; j < o_size_y; j++)
          for(i = 0; i < o_size_x; i++)
          {
            memcpy(&tmp32, buf+((i+j*size_x)*4), 4);
            ui32  = ((tmp32&0xFF00)<<8) | ((tmp32&0xFF)<<24) | ((tmp32&0xFF0000)>>8) | ((tmp32&0xFF000000)>>24);
            imgX[j][i] = (imgpel) rshift_rnd(ui32, bitshift);
          }
      }
    default:
      {
        error ("reading only from formats of 8, 16 or 32 bit allowed on big endian architecture", 500);
        break;
      }
    }
  }
  else
  {
    // little endian
      int j_pos;
      if (size_x == o_size_x && size_y == o_size_y)
      {
        for (j = 0; j < o_size_y; j++)
        {
          j_pos = j*size_x;
          for (i = 0; i < o_size_x; i++)
          {
            ui16=0;
            memcpy(&(ui16), buf + ((i + j_pos) * symbol_size_in_bytes), symbol_size_in_bytes);
            imgX[j][i] = (imgpel) rshift_rnd(ui16,bitshift);
          }
        }  
      }
      else
      {
        int iminwidth   = imin(size_x, o_size_x);
        int iminheight  = imin(size_y, o_size_y);
        int dst_offset_x  = 0, dst_offset_y = 0;        
        int offset_x = 0, offset_y = 0; // currently not used

        // determine whether we need to center the copied frame or crop it
        if ( o_size_x >= size_x ) 
          dst_offset_x = ( o_size_x  - size_x  ) >> 1;

        if (o_size_y >= size_y) 
          dst_offset_y = ( o_size_y - size_y ) >> 1;

        // check copied area to avoid copying memory garbage
        // source
        iminwidth  =  ( (offset_x + iminwidth ) > size_x ) ? (size_x  - offset_x) : iminwidth;
        iminheight =  ( (offset_y + iminheight) > size_y ) ? (size_y - offset_y) : iminheight;
        // destination
        iminwidth  =  ( (dst_offset_x + iminwidth ) > o_size_x  ) ? (o_size_x  - dst_offset_x) : iminwidth;
        iminheight =  ( (dst_offset_y + iminheight) > o_size_y )  ? (o_size_y - dst_offset_y) : iminheight;

        for (j=0; j < iminheight; j++)
        {        
          j_pos = (j + offset_y) * size_x + offset_x;
          for (i=0; i < iminwidth; i++)
          {
            ui16=0;
            memcpy(&(ui16), buf + ((i + j_pos) * symbol_size_in_bytes), symbol_size_in_bytes);
            imgX[j + dst_offset_y][i + dst_offset_x] = (imgpel) rshift_rnd(ui16,bitshift);
          }
        }    
      }
  }
}


/*!
 ************************************************************************
 * \brief
 *    Convert file read buffer to source picture structure
 ************************************************************************
 */
void buf2img_basic ( imgpel** imgX,           //!< Pointer to image plane
                    unsigned char* buf,       //!< Buffer for file output
                    int size_x,               //!< horizontal size of picture
                    int size_y,               //!< vertical size of picture
                    int o_size_x,             //!< horizontal size of picture
                    int o_size_y,             //!< vertical size of picture
                    int symbol_size_in_bytes, //!< number of bytes in file used for one pixel
                    int dummy                 //!< dummy variable used for allowing function pointer use
                    )
{
  int i,j;
  unsigned char* temp_buf = buf;

  if (symbol_size_in_bytes> sizeof(imgpel))
  {
    error ("Source picture has higher bit depth than imgpel data type. \nPlease recompile with larger data type for imgpel.", 500);
  }

  if (( sizeof (imgpel) == symbol_size_in_bytes))
  {    
    // imgpel == pixel_in_file -> simple copy
    if (size_x == o_size_x && size_y == o_size_y)
      memcpy(&imgX[0][0], temp_buf, size_x * size_y * sizeof(imgpel));
    else
    {
      int iminwidth   = imin(size_x, o_size_x);
      int iminheight  = imin(size_y, o_size_y);
      int dst_offset_x  = 0, dst_offset_y = 0;
      int offset_x = 0, offset_y = 0; // currently not used
      
      // determine whether we need to center the copied frame or crop it
      if ( o_size_x >= size_x ) 
        dst_offset_x = ( o_size_x  - size_x  ) >> 1;

      if (o_size_y >= size_y) 
        dst_offset_y = ( o_size_y - size_y ) >> 1;

      // check copied area to avoid copying memory garbage
      // source
      iminwidth  =  ( (offset_x + iminwidth ) > size_x ) ? (size_x  - offset_x) : iminwidth;
      iminheight =  ( (offset_y + iminheight) > size_y ) ? (size_y - offset_y) : iminheight;
      // destination
      iminwidth  =  ( (dst_offset_x + iminwidth ) > o_size_x  ) ? (o_size_x  - dst_offset_x) : iminwidth;
      iminheight =  ( (dst_offset_y + iminheight) > o_size_y )  ? (o_size_y - dst_offset_y) : iminheight;

      for (i=0; i<iminheight;i++) {
        memcpy(&imgX[i + dst_offset_y][dst_offset_x], &(temp_buf[(i + offset_y) * size_x + offset_x]), iminwidth * sizeof(imgpel));
      }
    }
  }
  else
  {
    int j_pos;
    unsigned short ui16;
    if (size_x == o_size_x && size_y == o_size_y)
    {
      for (j=0; j < o_size_y; j++)
      {
        j_pos = j * size_x;
        for (i=0; i < o_size_x; i++)
        {
          ui16=0;          
          memcpy(&(ui16), buf + ((i + j_pos) * symbol_size_in_bytes), symbol_size_in_bytes);
          imgX[j][i]= (imgpel) ui16;
        }
      }    
    }
    else
    {
      int iminwidth   = imin(size_x, o_size_x);
      int iminheight  = imin(size_y, o_size_y);
      int dst_offset_x  = 0, dst_offset_y = 0;
      int offset_x = 0, offset_y = 0; // currently not used
      
      // determine whether we need to center the copied frame or crop it
      if ( o_size_x >= size_x ) 
        dst_offset_x = ( o_size_x  - size_x  ) >> 1;

      if (o_size_y >= size_y) 
        dst_offset_y = ( o_size_y - size_y ) >> 1;

      // check copied area to avoid copying memory garbage
      // source
      iminwidth  =  ( (offset_x + iminwidth ) > size_x ) ? (size_x  - offset_x) : iminwidth;
      iminheight =  ( (offset_y + iminheight) > size_y ) ? (size_y - offset_y) : iminheight;
      // destination
      iminwidth  =  ( (dst_offset_x + iminwidth ) > o_size_x  ) ? (o_size_x  - dst_offset_x) : iminwidth;
      iminheight =  ( (dst_offset_y + iminheight) > o_size_y )  ? (o_size_y - dst_offset_y) : iminheight;

      for (j = 0; j < iminheight; j++) {
        memcpy(&imgX[j + dst_offset_y][dst_offset_x], &(temp_buf[(j + offset_y) * size_x + offset_x]), iminwidth * sizeof(imgpel));
      }
      for (j=0; j < iminheight; j++)
      {        
        j_pos = (j + offset_y) * size_x + offset_x;
        for (i=0; i < iminwidth; i++)
        {
          ui16 = 0;
          memcpy(&(ui16), buf + ((i + j_pos) * symbol_size_in_bytes), symbol_size_in_bytes);
          imgX[j + dst_offset_y][i + dst_offset_x]= (imgpel) ui16;
        }
      }    
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Convert file read buffer to source picture structure
 ************************************************************************
 */
void buf2img_endian ( imgpel** imgX,          //!< Pointer to image plane
                    unsigned char* buf,       //!< Buffer for file output
                    int size_x,               //!< horizontal size of picture
                    int size_y,               //!< vertical size of picture
                    int o_size_x,             //!< horizontal size of picture
                    int o_size_y,             //!< vertical size of picture
                    int symbol_size_in_bytes, //!< number of bytes in file used for one pixel
                    int dummy                 //!< dummy variable used for allowing function pointer use
                    )
{
  int i,j;

  unsigned short tmp16, ui16;
  unsigned long  tmp32, ui32;

  if (symbol_size_in_bytes > sizeof(imgpel))
  {
    error ("Source picture has higher bit depth than imgpel data type. \nPlease recompile with larger data type for imgpel.", 500);
  }

  if (size_x != o_size_x || size_y != o_size_y)
  {
    error ("Rescaling not supported in big endian architectures. ", 500);
  }

  // big endian
  switch (symbol_size_in_bytes)
  {
  case 1:
    {
      for(j=0;j<size_y;j++)
      {
        for(i=0;i<size_x;i++)
        {
          imgX[j][i]= (imgpel) buf[i + j*size_x];
        }
      }
      break;
    }
  case 2:
    {
      for(j=0;j<size_y;j++)
      {
        for(i=0;i<size_x;i++)
        {
          memcpy(&tmp16, buf+((i+j*size_x)*2), 2);
          ui16  = (tmp16 >> 8) | ((tmp16&0xFF)<<8);
          imgX[j][i] = (imgpel) ui16;
        }
      }
      break;
    }
  case 4:
    {
      for(j=0;j<size_y;j++)
      {
        for(i=0;i<size_x;i++)
        {
          memcpy(&tmp32, buf+((i+j*size_x)*4), 4);
          ui32  = ((tmp32&0xFF00)<<8) | ((tmp32&0xFF)<<24) | ((tmp32&0xFF0000)>>8) | ((tmp32&0xFF000000)>>24);
          imgX[j][i] = (imgpel) ui32;
        }
      }
      break;
    }
  default:
    {
      error ("reading only from formats of 8, 16 or 32 bit allowed on big endian architecture", 500);
      break;
    }
  }   
}

/*!
 ************************************************************************
 * \brief
 *    Create Frame Memory buffer
 *
 ************************************************************************
 */
int AllocateFrameMemory (ImageParameters *img, int size)
{
  if (NULL == (buf = malloc (size * (img->pic_unit_size_on_disk >> 3))))
    return (1);
  else
    return (0);
}

/*!
 ************************************************************************
 * \brief
 *    Delete Frame Memory buffer
 *
 ************************************************************************
 */
void DeleteFrameMemory (void)
{
  if (buf)
    free (buf);
}

/*!
 ************************************************************************
 * \brief
 *    Reads one new frame from file
 *
 * \param FrameNoInFile
 *    Frame number in the source file
 * \param HeaderSize
 *    Number of bytes in the source file to be skipped
 * \param source
 *    source file (on disk) information 
 * \param output
 *    output file (for encoding) information
 ************************************************************************
 */
void ReadOneFrame (int FrameNoInFile, int HeaderSize, FrameFormat *source, FrameFormat *output)
{
  unsigned int symbol_size_in_bytes = img->pic_unit_size_on_disk/8;

  const int imgsize_y = source->width * source->height;
  const int imgsize_uv = source->width_cr * source->height_cr;

  const int bytes_y = imgsize_y * symbol_size_in_bytes;
  const int bytes_uv = imgsize_uv * symbol_size_in_bytes;
  int bit_scale;

  const int64 framesize_in_bytes = bytes_y + 2*bytes_uv;

  Boolean rgb_input = (Boolean) (params->rgb_input_flag==1 && params->yuv_format==3);

  assert (p_in != -1);

  // skip Header
  if (lseek (p_in, HeaderSize, SEEK_SET) != HeaderSize)
  {
    error ("ReadOneFrame: cannot fseek to (Header size) in p_in", -1);
  }

  // skip starting frames
  if (lseek (p_in, framesize_in_bytes * params->start_frame, SEEK_CUR) == -1)
  {
    snprintf(errortext, ET_SIZE, "ReadOneFrame: cannot advance file pointer in p_in beyond frame %d\n", params->start_frame);
    error (errortext,-1);
  }

  // seek to current frame
  if (lseek (p_in, framesize_in_bytes * (FrameNoInFile), SEEK_CUR) == -1)
  {
    snprintf(errortext, ET_SIZE, "ReadOneFrame: cannot advance file pointer in p_in beyond frame %d\n", params->start_frame + FrameNoInFile);
    error (errortext,-1);
  }

  // Here we are at the correct position for the source frame in the file.  
  // Now read it.
  if (img->pic_unit_size_on_disk%8 == 0)
  {
//    if(rgb_input)
//      lseek (p_in, framesize_in_bytes / 3, SEEK_CUR);

    if (read(p_in, buf, (int) framesize_in_bytes) != (int) framesize_in_bytes)
    {
      printf ("ReadOneFrame: cannot read %d bytes from input file, unexpected EOF, exiting...\n", (int) framesize_in_bytes);
      report_stats_on_error();
    }

    bit_scale = source->bit_depth[0] - output->bit_depth[0];

    if(rgb_input)
      buf2img(pImgOrg[0], buf + bytes_y, source->width, source->height, output->width, output->height, symbol_size_in_bytes, bit_scale);
    else
      buf2img(pImgOrg[0], buf, source->width, source->height, output->width, output->height, symbol_size_in_bytes, bit_scale);

#if (DEBUG_BITDEPTH)
    MaskMSBs(pImgOrg[0], ((1 << output->bit_depth[0]) - 1), output->width, output->height);
#endif

    if (img->yuv_format != YUV400)
    {
      bit_scale = source->bit_depth[1] - output->bit_depth[1];
      if(rgb_input)
      buf2img(pImgOrg[1], buf + bytes_y + bytes_uv, source->width_cr, source->height_cr, output->width_cr, output->height_cr, symbol_size_in_bytes, bit_scale);
      else
      buf2img(pImgOrg[1], buf + bytes_y, source->width_cr, source->height_cr, output->width_cr, output->height_cr, symbol_size_in_bytes, bit_scale);

      bit_scale = source->bit_depth[2] - output->bit_depth[2];
      if(rgb_input)
      buf2img(pImgOrg[2], buf, source->width_cr, source->height_cr, output->width_cr, output->height_cr, symbol_size_in_bytes, bit_scale);
      else
      buf2img(pImgOrg[2], buf + bytes_y + bytes_uv, source->width_cr, source->height_cr, output->width_cr, output->height_cr, symbol_size_in_bytes, bit_scale);

#if (DEBUG_BITDEPTH)
      MaskMSBs(pImgOrg[1], ((1 << output->bit_depth[1]) - 1), output->width_cr, output->height_cr);
      MaskMSBs(pImgOrg[2], ((1 << output->bit_depth[2]) - 1), output->width_cr, output->height_cr);
#endif
    }
  }
  else
  {
    printf ("ReadOneFrame (NOT IMPLEMENTED): pic unit size on disk must be divided by 8");
    exit (-1);
  }
}

