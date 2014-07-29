
/*!
 ************************************************************************
 * \file  nalu.c
 *
 * \brief
 *    Decoder NALU support functions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Stephan Wenger   <stewe@cs.tu-berlin.de>
 ************************************************************************
 */

#include "global.h"
#include "nalu.h"
#include "annexb.h"
#include "rtp.h"

/*!
 *************************************************************************************
 * \brief
 *    Converts a NALU to an RBSP
 *
 * \param
 *    nalu: nalu structure to be filled
 *
 * \return
 *    length of the RBSP in bytes
 *************************************************************************************
 */

int NALUtoRBSP (NALU_t *nalu)
{
  assert (nalu != NULL);

  nalu->len = EBSPtoRBSP (nalu->buf, nalu->len, 1) ;

  return nalu->len ;
}

/*!
************************************************************************
* \brief
*    Read the next NAL unit (with error handling)
************************************************************************
*/
int read_next_nalu(FILE *bitstream, NALU_t *nalu)
{
  int ret;

  if (params->FileFormat == PAR_OF_ANNEXB)
    ret = GetAnnexbNALU (bitstream, nalu);
  else
    ret = GetRTPNALU (bitstream, nalu);

  if (ret < 0)
  {
    snprintf (errortext, ET_SIZE, "Error while getting the NALU in file format %s, exit\n", params->FileFormat==PAR_OF_ANNEXB?"Annex B":"RTP");
    error (errortext, 601);
  }
  if (ret == 0)
  {
    FreeNALU(nalu);
    return 0;
  }

  //In some cases, zero_byte shall be present. If current NALU is a VCL NALU, we can't tell
  //whether it is the first VCL NALU at this point, so only non-VCL NAL unit is checked here.
  CheckZeroByteNonVCL(nalu);

  ret = NALUtoRBSP(nalu);

  if (ret < 0)
    error ("Invalid startcode emulation prevention found.", 602);


  // Got a NALU
  if (nalu->forbidden_bit)
  {
    error ("Found NALU with forbidden_bit set, bit error?", 603);
  }

  return nalu->len;
}
