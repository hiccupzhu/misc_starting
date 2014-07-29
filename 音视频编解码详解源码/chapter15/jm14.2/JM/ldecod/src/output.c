
/*!
 ************************************************************************
 * \file output.c
 *
 * \brief
 *    Output an image and Trance support
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Karsten Suehring               <suehring@hhi.de>
 ************************************************************************
 */

#include "contributors.h"

#include "global.h"
#include "mbuffer.h"
#include "image.h"
#include "memalloc.h"
#include "sei.h"

FrameStore* out_buffer;

StorablePicture *pending_output = NULL;
int              pending_output_state = FRAME;
int recovery_flag = 0;

extern int non_conforming_stream;

static void write_out_picture(StorablePicture *p, int p_out);
static void (*img2buf)     (imgpel** imgX, unsigned char* buf, int size_x, int size_y, int symbol_size_in_bytes, int crop_left, int crop_right, int crop_top, int crop_bottom);
static void img2buf_byte   (imgpel** imgX, unsigned char* buf, int size_x, int size_y, int symbol_size_in_bytes, int crop_left, int crop_right, int crop_top, int crop_bottom);
static void img2buf_normal (imgpel** imgX, unsigned char* buf, int size_x, int size_y, int symbol_size_in_bytes, int crop_left, int crop_right, int crop_top, int crop_bottom);
static void img2buf_endian (imgpel** imgX, unsigned char* buf, int size_x, int size_y, int symbol_size_in_bytes, int crop_left, int crop_right, int crop_top, int crop_bottom);

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


/*!
 ************************************************************************
 * \brief
 *      selects appropriate output function given system arch. and data
 * \return
 *
 ************************************************************************
 */
static void initOutput(int symbol_size_in_bytes)
{
  if (( sizeof(char) == sizeof (imgpel)) && ( sizeof(char) == symbol_size_in_bytes))
  {
    img2buf = img2buf_byte;
  }
  else
  {
    if (( sizeof(char) != sizeof (imgpel)) && testEndian())
      img2buf = img2buf_endian;
    else
      img2buf = img2buf_normal;
  }    
}

/*!
 ************************************************************************
 * \brief
 *    Convert image plane to temporary buffer for file writing
 * \param imgX
 *    Pointer to image plane
 * \param buf
 *    Buffer for file output
 * \param size_x
 *    horizontal size
 * \param size_y
 *    vertical size
 * \param symbol_size_in_bytes
 *    number of bytes used per pel
 * \param crop_left
 *    pixels to crop from left
 * \param crop_right
 *    pixels to crop from right
 * \param crop_top
 *    pixels to crop from top
 * \param crop_bottom
 *    pixels to crop from bottom
 ************************************************************************
 */
static void img2buf_normal (imgpel** imgX, unsigned char* buf, int size_x, int size_y, int symbol_size_in_bytes, int crop_left, int crop_right, int crop_top, int crop_bottom)
{
  int i,j;

  int twidth  = size_x - crop_left - crop_right;
  int theight = size_y - crop_top - crop_bottom;

  int size = 0;

  // sizeof (imgpel) > sizeof(char)
  // little endian
  if (sizeof (imgpel) < symbol_size_in_bytes)
  {
    // this should not happen. we should not have smaller imgpel than our source material.
    size = sizeof (imgpel);
    // clear buffer
    memset (buf, 0, (twidth * theight * symbol_size_in_bytes));
  }
  else
  {
    size = symbol_size_in_bytes;
  }

  if ((crop_top || crop_bottom || crop_left || crop_right) || (size != 1))
  {
    for(i=crop_top;i<size_y-crop_bottom;i++)
    {
      int ipos = (i - crop_top) * (twidth);
      for(j=crop_left;j<size_x-crop_right;j++)
      {
        memcpy(buf+((j-crop_left+(ipos))*symbol_size_in_bytes),&(imgX[i][j]), size);
      }
    }
  }
  else
  {
    imgpel *cur_pixel = &(imgX[0][0]);;
    for(i = 0; i < size_y * size_x; i++)
    {          
      *(buf++)=(char) *(cur_pixel++);
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Convert image plane to temporary buffer for file writing
 * \param imgX
 *    Pointer to image plane
 * \param buf
 *    Buffer for file output
 * \param size_x
 *    horizontal size
 * \param size_y
 *    vertical size
 * \param symbol_size_in_bytes
 *    number of bytes used per pel
 * \param crop_left
 *    pixels to crop from left
 * \param crop_right
 *    pixels to crop from right
 * \param crop_top
 *    pixels to crop from top
 * \param crop_bottom
 *    pixels to crop from bottom
 ************************************************************************
 */
static void img2buf_byte (imgpel** imgX, unsigned char* buf, int size_x, int size_y, int symbol_size_in_bytes, int crop_left, int crop_right, int crop_top, int crop_bottom)
{
  int i;

  int twidth  = size_x - crop_left - crop_right;
  int theight = size_y - crop_top - crop_bottom;

  // imgpel == pixel_in_file == 1 byte -> simple copy
  buf += crop_left;
  for(i = 0; i < theight; i++) 
  {
    memcpy(buf, &(imgX[i + crop_top][crop_left]), twidth);
    buf += twidth;
  }
}

/*!
 ************************************************************************
 * \brief
 *    Convert image plane to temporary buffer for file writing
 * \param imgX
 *    Pointer to image plane
 * \param buf
 *    Buffer for file output
 * \param size_x
 *    horizontal size
 * \param size_y
 *    vertical size
 * \param symbol_size_in_bytes
 *    number of bytes used per pel
 * \param crop_left
 *    pixels to crop from left
 * \param crop_right
 *    pixels to crop from right
 * \param crop_top
 *    pixels to crop from top
 * \param crop_bottom
 *    pixels to crop from bottom
 ************************************************************************
 */
static void img2buf_endian (imgpel** imgX, unsigned char* buf, int size_x, int size_y, int symbol_size_in_bytes, int crop_left, int crop_right, int crop_top, int crop_bottom)
{
  int i,j;
  static unsigned char  ui8;
  static unsigned short tmp16, ui16;
  static unsigned long  tmp32, ui32;

  int twidth  = size_x - crop_left - crop_right;

  // big endian
  switch (symbol_size_in_bytes)
  {
  case 1:
    {
      for(i=crop_top;i<size_y-crop_bottom;i++)
        for(j=crop_left;j<size_x-crop_right;j++)
        {
          ui8 = (unsigned char) (imgX[i][j]);
          buf[(j-crop_left+((i-crop_top)*(twidth)))] = ui8;
        }
        break;
    }
  case 2:
    {
      for(i=crop_top;i<size_y-crop_bottom;i++)
        for(j=crop_left;j<size_x-crop_right;j++)
        {
          tmp16 = (unsigned short) (imgX[i][j]);
          ui16  = (unsigned short) ((tmp16 >> 8) | ((tmp16&0xFF)<<8));
          memcpy(buf+((j-crop_left+((i-crop_top)*(twidth)))*2),&(ui16), 2);
        }
        break;
    }
  case 4:
    {
      for(i=crop_top;i<size_y-crop_bottom;i++)
        for(j=crop_left;j<size_x-crop_right;j++)
        {
          tmp32 = (unsigned long) (imgX[i][j]);
          ui32  = (unsigned long) (((tmp32&0xFF00)<<8) | ((tmp32&0xFF)<<24) | ((tmp32&0xFF0000)>>8) | ((tmp32&0xFF000000)>>24));
          memcpy(buf+((j-crop_left+((i-crop_top)*(twidth)))*4),&(ui32), 4);
        }
        break;
    }
  default:
    {
      error ("writing only to formats of 8, 16 or 32 bit allowed on big endian architecture", 500);
      break;
    }
  }  
}


#if (PAIR_FIELDS_IN_OUTPUT)

void clear_picture(StorablePicture *p);

/*!
 ************************************************************************
 * \brief
 *    output the pending frame buffer
 * \param p_out
 *    Output file
 ************************************************************************
 */
void flush_pending_output(int p_out)
{
  if (pending_output_state!=FRAME)
  {
    write_out_picture(pending_output, p_out);
  }

  if (pending_output->imgY)
  {
    free_mem2Dpel (pending_output->imgY);
    pending_output->imgY=NULL;
  }
  if (pending_output->imgUV)
  {
    free_mem3Dpel (pending_output->imgUV, 2);
    pending_output->imgUV=NULL;
  }

  pending_output_state = FRAME;
}


/*!
 ************************************************************************
 * \brief
 *    Writes out a storable picture
 *    If the picture is a field, the output buffers the picture and tries
 *    to pair it with the next field.
 * \param p
 *    Picture to be written
 * \param p_out
 *    Output file
 ************************************************************************
 */
void write_picture(StorablePicture *p, int p_out, int real_structure)
{
   int i, add;

  if (real_structure==FRAME)
  {
    flush_pending_output(p_out);
    write_out_picture(p, p_out);
    return;
  }
  if (real_structure==pending_output_state)
  {
    flush_pending_output(p_out);
    write_picture(p, p_out, real_structure);
    return;
  }

  if (pending_output_state == FRAME)
  {
    pending_output->size_x = p->size_x;
    pending_output->size_y = p->size_y;
    pending_output->size_x_cr = p->size_x_cr;
    pending_output->size_y_cr = p->size_y_cr;
    pending_output->chroma_format_idc = p->chroma_format_idc;

    pending_output->frame_mbs_only_flag = p->frame_mbs_only_flag;
    pending_output->frame_cropping_flag = p->frame_cropping_flag;
    if (pending_output->frame_cropping_flag)
    {
      pending_output->frame_cropping_rect_left_offset = p->frame_cropping_rect_left_offset;
      pending_output->frame_cropping_rect_right_offset = p->frame_cropping_rect_right_offset;
      pending_output->frame_cropping_rect_top_offset = p->frame_cropping_rect_top_offset;
      pending_output->frame_cropping_rect_bottom_offset = p->frame_cropping_rect_bottom_offset;
    }

    get_mem2Dpel (&(pending_output->imgY), pending_output->size_y, pending_output->size_x);
    get_mem3Dpel (&(pending_output->imgUV), 2, pending_output->size_y_cr, pending_output->size_x_cr);

    clear_picture(pending_output);

    // copy first field
    if (real_structure == TOP_FIELD)
    {
      add = 0;
    }
    else
    {
      add = 1;
    }

    for (i=0; i<pending_output->size_y; i+=2)
    {
      memcpy(pending_output->imgY[(i+add)], p->imgY[(i+add)], p->size_x * sizeof(imgpel));
    }
    for (i=0; i<pending_output->size_y_cr; i+=2)
    {
      memcpy(pending_output->imgUV[0][(i+add)], p->imgUV[0][(i+add)], p->size_x_cr * sizeof(imgpel));
      memcpy(pending_output->imgUV[1][(i+add)], p->imgUV[1][(i+add)], p->size_x_cr * sizeof(imgpel));
    }
    pending_output_state = real_structure;
  }
  else
  {
    if (  (pending_output->size_x!=p->size_x) || (pending_output->size_y!= p->size_y)
       || (pending_output->frame_mbs_only_flag != p->frame_mbs_only_flag)
       || (pending_output->frame_cropping_flag != p->frame_cropping_flag)
       || ( pending_output->frame_cropping_flag &&
            (  (pending_output->frame_cropping_rect_left_offset   != p->frame_cropping_rect_left_offset)
             ||(pending_output->frame_cropping_rect_right_offset  != p->frame_cropping_rect_right_offset)
             ||(pending_output->frame_cropping_rect_top_offset    != p->frame_cropping_rect_top_offset)
             ||(pending_output->frame_cropping_rect_bottom_offset != p->frame_cropping_rect_bottom_offset)
            )
          )
       )
    {
      flush_pending_output(p_out);
      write_picture (p, p_out, real_structure);
      return;
    }
    // copy second field
    if (real_structure == TOP_FIELD)
    {
      add = 0;
    }
    else
    {
      add = 1;
    }

    for (i=0; i<pending_output->size_y; i+=2)
    {
      memcpy(pending_output->imgY[(i+add)], p->imgY[(i+add)], p->size_x * sizeof(imgpel));
    }
    for (i=0; i<pending_output->size_y_cr; i+=2)
    {
      memcpy(pending_output->imgUV[0][(i+add)], p->imgUV[0][(i+add)], p->size_x_cr * sizeof(imgpel));
      memcpy(pending_output->imgUV[1][(i+add)], p->imgUV[1][(i+add)], p->size_x_cr * sizeof(imgpel));
    }

    flush_pending_output(p_out);
  }
}

#else

/*!
 ************************************************************************
 * \brief
 *    Writes out a storable picture without doing any output modifications
 * \param p
 *    Picture to be written
 * \param p_out
 *    Output file
 * \param real_structure
 *    real picture structure
 ************************************************************************
 */
void write_picture(StorablePicture *p, int p_out, int real_structure)
{
  write_out_picture(p, p_out);
}


#endif

/*!
************************************************************************
* \brief
*    Writes out a storable picture
* \param p
*    Picture to be written
* \param p_out
*    Output file
************************************************************************
*/
static void write_out_picture(StorablePicture *p, int p_out)
{
  static int SubWidthC  [4]= { 1, 2, 2, 1};
  static int SubHeightC [4]= { 1, 2, 1, 1};

  int crop_left, crop_right, crop_top, crop_bottom;
  int symbol_size_in_bytes = (img->pic_unit_bitsize_on_disk >> 3);
  Boolean rgb_output = (Boolean) (active_sps->vui_seq_parameters.matrix_coefficients==0);
  unsigned char *buf;

  if (p->non_existing)
    return;

#if (ENABLE_OUTPUT_TONEMAPPING)
  // note: this tone-mapping is working for RGB format only. Sharp
  if (p->seiHasTone_mapping && rgb_output)
  {
    //printf("output frame %d with tone model id %d\n",  p->frame_num, p->tone_mapping_model_id);
    symbol_size_in_bytes = (p->tonemapped_bit_depth>8)? 2 : 1;
    tone_map(p->imgY, p->tone_mapping_lut, p->size_x, p->size_y);
    tone_map(p->imgUV[0], p->tone_mapping_lut, p->size_x_cr, p->size_y_cr);
    tone_map(p->imgUV[1], p->tone_mapping_lut, p->size_x_cr, p->size_y_cr);
  }
#endif

  if (p->frame_cropping_flag)
  {
    crop_left   = SubWidthC[p->chroma_format_idc] * p->frame_cropping_rect_left_offset;
    crop_right  = SubWidthC[p->chroma_format_idc] * p->frame_cropping_rect_right_offset;
    crop_top    = SubHeightC[p->chroma_format_idc]*( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_top_offset;
    crop_bottom = SubHeightC[p->chroma_format_idc]*( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_bottom_offset;
  }
  else
  {
    crop_left = crop_right = crop_top = crop_bottom = 0;
  }

  //printf ("write frame size: %dx%d\n", p->size_x-crop_left-crop_right,p->size_y-crop_top-crop_bottom );
  initOutput(symbol_size_in_bytes);

  // KS: this buffer should actually be allocated only once, but this is still much faster than the previous version
  buf = malloc (p->size_x*p->size_y*symbol_size_in_bytes);
  if (NULL==buf)
  {
    no_mem_exit("write_out_picture: buf");
  }

  if(rgb_output)
  {
    crop_left   = p->frame_cropping_rect_left_offset;
    crop_right  = p->frame_cropping_rect_right_offset;
    crop_top    = ( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_top_offset;
    crop_bottom = ( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_bottom_offset;

    img2buf_normal (p->imgUV[1], buf, p->size_x_cr, p->size_y_cr, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom);
    write(p_out, buf, (p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)*symbol_size_in_bytes);

    if (p->frame_cropping_flag)
    {
      crop_left   = SubWidthC[p->chroma_format_idc] * p->frame_cropping_rect_left_offset;
      crop_right  = SubWidthC[p->chroma_format_idc] * p->frame_cropping_rect_right_offset;
      crop_top    = SubHeightC[p->chroma_format_idc]*( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_top_offset;
      crop_bottom = SubHeightC[p->chroma_format_idc]*( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_bottom_offset;
    }
    else
    {
      crop_left = crop_right = crop_top = crop_bottom = 0;
    }
  }

  img2buf_normal (p->imgY, buf, p->size_x, p->size_y, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom);
  write(p_out, buf, (p->size_y-crop_bottom-crop_top)*(p->size_x-crop_right-crop_left)*symbol_size_in_bytes);

  if (p->chroma_format_idc!=YUV400)
  {
    crop_left   = p->frame_cropping_rect_left_offset;
    crop_right  = p->frame_cropping_rect_right_offset;
    crop_top    = ( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_top_offset;
    crop_bottom = ( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_bottom_offset;

    img2buf_normal (p->imgUV[0], buf, p->size_x_cr, p->size_y_cr, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom);
    write(p_out, buf, (p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)* symbol_size_in_bytes);

    if (!rgb_output)
    {
      img2buf_normal (p->imgUV[1], buf, p->size_x_cr, p->size_y_cr, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom);
      write(p_out, buf, (p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)*symbol_size_in_bytes);
    }
  }
  else
  {
    if (params->write_uv)
    {
      int i,j;
      imgpel cr_val = (imgpel) (1<<(img->bitdepth_luma - 1));

      get_mem3Dpel (&(p->imgUV), 1, p->size_y/2, p->size_x/2);
      for (j=0; j<p->size_y/2; j++)
        for (i=0; i<p->size_x/2; i++)
          p->imgUV[0][j][i]=cr_val;

      // fake out U=V=128 to make a YUV 4:2:0 stream
      img2buf_normal (p->imgUV[0], buf, p->size_x/2, p->size_y/2, symbol_size_in_bytes, crop_left/2, crop_right/2, crop_top/2, crop_bottom/2);

      write(p_out, buf, symbol_size_in_bytes * (p->size_y-crop_bottom-crop_top)/2 * (p->size_x-crop_right-crop_left)/2 );
      write(p_out, buf, symbol_size_in_bytes * (p->size_y-crop_bottom-crop_top)/2 * (p->size_x-crop_right-crop_left)/2 );

      free_mem3Dpel(p->imgUV);
      p->imgUV=NULL;
    }
  }

  free(buf);

//  fsync(p_out);
}

/*!
 ************************************************************************
 * \brief
 *    Initialize output buffer for direct output
 ************************************************************************
 */
void init_out_buffer(void)
{
  out_buffer = alloc_frame_store();  

#if (PAIR_FIELDS_IN_OUTPUT)
  pending_output = calloc (sizeof(StorablePicture), 1);
  if (NULL==pending_output) no_mem_exit("init_out_buffer");
  pending_output->imgUV = NULL;
  pending_output->imgY  = NULL;
#endif
}

/*!
 ************************************************************************
 * \brief
 *    Uninitialize output buffer for direct output
 ************************************************************************
 */
void uninit_out_buffer(void)
{
  free_frame_store(out_buffer);
  out_buffer=NULL;
#if (PAIR_FIELDS_IN_OUTPUT)
  flush_pending_output(p_out);
  free (pending_output);
#endif
}

/*!
 ************************************************************************
 * \brief
 *    Initialize picture memory with (Y:0,U:128,V:128)
 ************************************************************************
 */
void clear_picture(StorablePicture *p)
{
  int i,j;

  for(i=0;i<p->size_y;i++)
  {
    for (j=0; j<p->size_x; j++)
      p->imgY[i][j] = (imgpel) img->dc_pred_value_comp[0];
  }
  for(i=0;i<p->size_y_cr;i++)
  {
    for (j=0; j<p->size_x_cr; j++)
      p->imgUV[0][i][j] = (imgpel) img->dc_pred_value_comp[1];
  }
  for(i=0;i<p->size_y_cr;i++)
  {
    for (j=0; j<p->size_x_cr; j++)
      p->imgUV[1][i][j] = (imgpel) img->dc_pred_value_comp[2];
  }

}

/*!
 ************************************************************************
 * \brief
 *    Write out not paired direct output fields. A second empty field is generated
 *    and combined into the frame buffer.
 * \param fs
 *    FrameStore that contains a single field
 * \param p_out
 *    Output file
 ************************************************************************
 */
void write_unpaired_field(FrameStore* fs, int p_out)
{
  StorablePicture *p;
  assert (fs->is_used<3);

  if(fs->is_used & 0x01)
  {
    // we have a top field
    // construct an empty bottom field
    p = fs->top_field;
    fs->bottom_field = alloc_storable_picture(BOTTOM_FIELD, p->size_x, 2*p->size_y, p->size_x_cr, 2*p->size_y_cr);
    fs->bottom_field->chroma_format_idc = p->chroma_format_idc;
    clear_picture(fs->bottom_field);
    dpb_combine_field_yuv(fs);
    write_picture (fs->frame, p_out, TOP_FIELD);
  }

  if(fs->is_used & 0x02)
  {
    // we have a bottom field
    // construct an empty top field
    p = fs->bottom_field;
    fs->top_field = alloc_storable_picture(TOP_FIELD, p->size_x, 2*p->size_y, p->size_x_cr, 2*p->size_y_cr);
    fs->top_field->chroma_format_idc = p->chroma_format_idc;
    clear_picture(fs->top_field);
    fs ->top_field->frame_cropping_flag = fs->bottom_field->frame_cropping_flag;
    if(fs ->top_field->frame_cropping_flag)
    {
      fs ->top_field->frame_cropping_rect_top_offset = fs->bottom_field->frame_cropping_rect_top_offset;
      fs ->top_field->frame_cropping_rect_bottom_offset = fs->bottom_field->frame_cropping_rect_bottom_offset;
      fs ->top_field->frame_cropping_rect_left_offset = fs->bottom_field->frame_cropping_rect_left_offset;
      fs ->top_field->frame_cropping_rect_right_offset = fs->bottom_field->frame_cropping_rect_right_offset;
    }
    dpb_combine_field_yuv(fs);
    write_picture (fs->frame, p_out, BOTTOM_FIELD);
  }

  fs->is_used = 3;
}

/*!
 ************************************************************************
 * \brief
 *    Write out unpaired fields from output buffer.
 * \param p_out
 *    Output file
 ************************************************************************
 */
void flush_direct_output(int p_out)
{
  write_unpaired_field(out_buffer, p_out);

  free_storable_picture(out_buffer->frame);
  out_buffer->frame = NULL;
  free_storable_picture(out_buffer->top_field);
  out_buffer->top_field = NULL;
  free_storable_picture(out_buffer->bottom_field);
  out_buffer->bottom_field = NULL;
  out_buffer->is_used = 0;
}


/*!
 ************************************************************************
 * \brief
 *    Write a frame (from FrameStore)
 * \param fs
 *    FrameStore containing the frame
 * \param p_out
 *    Output file
 ************************************************************************
 */
void write_stored_frame( FrameStore *fs,int p_out)
{
  // make sure no direct output field is pending
  flush_direct_output(p_out);

  if (fs->is_used<3)
  {
    write_unpaired_field(fs, p_out);
  }
  else
  {
    if (fs->recovery_frame)
      recovery_flag = 1;
    if ((!non_conforming_stream) || recovery_flag)
      write_picture(fs->frame, p_out, FRAME);
  }

  fs->is_output = 1;
}

/*!
 ************************************************************************
 * \brief
 *    Directly output a picture without storing it in the DPB. Fields
 *    are buffered before they are written to the file.
 * \param p
 *    Picture for output
 * \param p_out
 *    Output file
 ************************************************************************
 */
void direct_output(StorablePicture *p, int p_out)
{
  if (p->structure==FRAME)
  {
    // we have a frame (or complementary field pair)
    // so output it directly
    flush_direct_output(p_out);
    write_picture (p, p_out, FRAME);
    calculate_frame_no(p);
    if (-1!=p_ref && !params->silent)
      find_snr(snr, p, p_ref);
    free_storable_picture(p);
    return;
  }

  if (p->structure == TOP_FIELD)
  {
    if (out_buffer->is_used &1)
      flush_direct_output(p_out);
    out_buffer->top_field = p;
    out_buffer->is_used |= 1;
  }

  if (p->structure == BOTTOM_FIELD)
  {
    if (out_buffer->is_used &2)
      flush_direct_output(p_out);
    out_buffer->bottom_field = p;
    out_buffer->is_used |= 2;
  }

  if (out_buffer->is_used == 3)
  {
    // we have both fields, so output them
    dpb_combine_field_yuv(out_buffer);
    write_picture (out_buffer->frame, p_out, FRAME);

    calculate_frame_no(p);
    if (-1!=p_ref && !params->silent)
      find_snr(snr, out_buffer->frame, p_ref);
    free_storable_picture(out_buffer->frame);
    out_buffer->frame = NULL;
    free_storable_picture(out_buffer->top_field);
    out_buffer->top_field = NULL;
    free_storable_picture(out_buffer->bottom_field);
    out_buffer->bottom_field = NULL;
    out_buffer->is_used = 0;
  }
}

