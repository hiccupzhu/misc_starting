
/*!
 ***********************************************************************
 * \file
 *    configfile.c
 * \brief
 *    Configuration handling.
 * \author
 *  Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Stephan Wenger           <stewe@cs.tu-berlin.de>
 * \note
 *    In the future this module should hide the Parameters and offer only
 *    Functions for their access.  Modules which make frequent use of some parameters
 *    (e.g. picture size in macroblocks) are free to buffer them on local variables.
 *    This will not only avoid global variable and make the code more readable, but also
 *    speed it up.  It will also greatly facilitate future enhancements such as the
 *    handling of different picture sizes in the same sequence.                         \n
 *                                                                                      \n
 *    For now, everything is just copied to the inp_par structure (gulp)
 *
 **************************************************************************************
 * \par Configuration File Format
 **************************************************************************************
 * Format is line oriented, maximum of one parameter per line                           \n
 *                                                                                      \n
 * Lines have the following format:                                                     \n
 * \<ParameterName\> = \<ParameterValue\> # Comments \\n                                    \n
 * Whitespace is space and \\t
 * \par
 * \<ParameterName\> are the predefined names for Parameters and are case sensitive.
 *   See configfile.h for the definition of those names and their mapping to
 *   configinput->values.
 * \par
 * \<ParameterValue\> are either integers [0..9]* or strings.
 *   Integers must fit into the wordlengths, signed values are generally assumed.
 *   Strings containing no whitespace characters can be used directly.  Strings containing
 *   whitespace characters are to be inclosed in double quotes ("string with whitespace")
 *   The double quote character is forbidden (may want to implement something smarter here).
 * \par
 * Any Parameters whose ParameterName is undefined lead to the termination of the program
 * with an error message.
 *
 * \par Known bug/Shortcoming:
 *    zero-length strings (i.e. to signal an non-existing file
 *    have to be coded as "".
 *
 * \par Rules for using command files
 *                                                                                      \n
 * All Parameters are initially taken from DEFAULTCONFIGFILENAME, defined in configfile.h.
 * If an -f \<config\> parameter is present in the command line then this file is used to
 * update the defaults of DEFAULTCONFIGFILENAME.  There can be more than one -f parameters
 * present.  If -p <ParameterName = ParameterValue> parameters are present then these
 * override the default and the additional config file's settings, and are themselves
 * overridden by future -p parameters.  There must be whitespace between -f and -p commands
 * and their respective parameters
 ***********************************************************************
 */

#define INCLUDED_BY_CONFIGFILE_C

#include <sys/stat.h>

#include "global.h"
#include "configfile.h"
#include "fmo.h"
#include "conformance.h"

char *GetConfigFileContent (char *Filename);
static void ParseContent (char *buf, int bufsize);
static int ParameterNameToMapIndex (char *s);
static int InitEncoderParams(void);
static int TestEncoderParams(int bitdepth_qp_scale[3]);
static int DisplayEncoderParams(void);
static void PatchInp (void);

static int mb_width_cr[4] = {0,8, 8,16};
static int mb_height_cr[4]= {0,8,16,16};

#define MAX_ITEMS_TO_PARSE  10000


/*!
 ***********************************************************************
 * \brief
 *   print help message and exit
 ***********************************************************************
 */
void JMHelpExit (void)
{
  fprintf( stderr, "\n   lencod [-h] [-d defenc.cfg] {[-f curenc1.cfg]...[-f curencN.cfg]}"
    " {[-p EncParam1=EncValue1]..[-p EncParamM=EncValueM]}\n\n"
    "## Parameters\n\n"

    "## Options\n"
    "   -h :  prints function usage\n"
    "   -d :  use <defenc.cfg> as default file for parameter initializations.\n"
    "         If not used then file defaults to encoder.cfg in local directory.\n"
    "   -f :  read <curencM.cfg> for reseting selected encoder parameters.\n"
    "         Multiple files could be used that set different parameters\n"
    "   -p :  Set parameter <EncParamM> to <EncValueM>.\n"
    "         See default encoder.cfg file for description of all parameters.\n\n"

    "## Supported video file formats\n"
    "   RAW:  .yuv -> YUV 4:2:0\n\n"

    "## Examples of usage:\n"
    "   lencod\n"
    "   lencod  -h\n"
    "   lencod  -d default.cfg\n"
    "   lencod  -f curenc1.cfg\n"
    "   lencod  -f curenc1.cfg -p InputFile=\"e:\\data\\container_qcif_30.yuv\" -p SourceWidth=176 -p SourceHeight=144\n"
    "   lencod  -f curenc1.cfg -p FramesToBeEncoded=30 -p QPISlice=28 -p QPPSlice=28 -p QPBSlice=30\n");

  exit(-1);
}

/*!
 ************************************************************************
 * \brief
 *    Reads Input File Size 
 *
 ************************************************************************
 */
int64 getVideoFileSize(void)
{
   int64 fsize;   

   lseek(p_in, 0, SEEK_END); 
   fsize = tell((int) p_in); 
   lseek(p_in, 0, SEEK_SET); 

   return fsize;
}

/*!
 ************************************************************************
 * \brief
 *    Updates the number of frames to encode based on the file size
 *
 ************************************************************************
 */
static void getNumberOfFrames (void)
{
  int64 fsize = getVideoFileSize();
  int64 isize = (int64) params->output.size;
  int maxBitDepth = imax(params->output.bit_depth[0], params->output.bit_depth[1]);

  isize <<= (maxBitDepth > 8)? 1: 0;
  params->no_frames = (int) (((fsize - params->infile_header)/ isize) - params->start_frame - 1) / (1 + params->jumpd) + 1;
}

/*!
 ************************************************************************
 * \brief
 *    Updates images max values
 *
 ************************************************************************
 */
static void updateMaxValue(FrameFormat *format)
{
  format->max_value[0] = (1 << format->bit_depth[0]) - 1;
  format->max_value_sq[0] = format->max_value[0] * format->max_value[0];
  format->max_value[1] = (1 << format->bit_depth[1]) - 1;
  format->max_value_sq[1] = format->max_value[1] * format->max_value[1];
  format->max_value[2] = (1 << format->bit_depth[2]) - 1;
  format->max_value_sq[2] = format->max_value[2] * format->max_value[2];
}

/*!
 ************************************************************************
 * \brief
 *    Update output format parameters (resolution & bit-depth) given input
 *
 ************************************************************************
 */
static void updateOutFormat(InputParameters *params)
{

  params->output.yuv_format = params->yuv_format;
  params->source.yuv_format = params->yuv_format;

  if (params->src_resize == 0)
  {
    params->output.width = params->source.width;
    params->output.height = params->source.height;
  }

  if (params->yuv_format == YUV400) // reset bitdepth of chroma for 400 content
  {
    params->source.bit_depth[1] = 8;
    params->output.bit_depth[1] = 8;
    params->source.width_cr  = 0;
    params->source.height_cr = 0;
    params->output.width_cr  = 0;
    params->output.height_cr = 0;
  }
  else
  {
    params->source.width_cr  = (params->source.width  * mb_width_cr [params->yuv_format]) >> 4;
    params->source.height_cr = (params->source.height * mb_height_cr[params->yuv_format]) >> 4;
    params->output.width_cr  = (params->output.width  * mb_width_cr [params->yuv_format]) >> 4;
    params->output.height_cr = (params->output.height * mb_height_cr[params->yuv_format]) >> 4;
  }

  // source size
  params->source.size_cmp[0] = params->source.width    * params->source.height;
  params->source.size_cmp[1] = params->source.width_cr * params->source.height_cr;
  params->source.size_cmp[2] = params->source.size_cmp[1];
  params->source.size        = params->source.size_cmp[0] + params->source.size_cmp[1] + params->source.size_cmp[2];
  params->source.mb_width    = params->source.width  / MB_BLOCK_SIZE;
  params->source.mb_height   = params->source.height / MB_BLOCK_SIZE;


  // output size (excluding padding)
  params->output.size_cmp[0] = params->output.width * params->source.height;
  params->output.size_cmp[1] = params->output.width_cr * params->source.height_cr;
  params->output.size_cmp[2] = params->output.size_cmp[1];
  params->output.size        = params->output.size_cmp[0] + params->output.size_cmp[1] + params->output.size_cmp[2];
  params->output.mb_width    = params->output.width  / MB_BLOCK_SIZE;
  params->output.mb_height   = params->output.height / MB_BLOCK_SIZE;


  // both chroma components have the same bitdepth
  params->source.bit_depth[2] = params->source.bit_depth[1];
  params->output.bit_depth[2] = params->output.bit_depth[1];
  
  // if no bitdepth rescale ensure bitdepth is same
  if (params->src_BitDepthRescale == 0) 
  {    
    params->output.bit_depth[0] = params->source.bit_depth[0];
    params->output.bit_depth[1] = params->source.bit_depth[1];
    params->output.bit_depth[2] = params->source.bit_depth[2];
  }
  
  updateMaxValue(&params->source);
  updateMaxValue(&params->output);
}


/*!
 ***********************************************************************
 * \brief
 *    Parse the command line parameters and read the config files.
 * \param ac
 *    number of command line parameters
 * \param av
 *    command line parameters
 ***********************************************************************
 */
void Configure (int ac, char *av[])
{
  char *content = NULL;
  int CLcount, ContentLen, NumberParams;
  char *filename=DEFAULTCONFIGFILENAME;

  memset (&configinput, 0, sizeof (InputParameters));
  //Set default parameters.
  printf ("Setting Default Parameters...\n");
  InitEncoderParams();

  // Process default config file
  CLcount = 1;

  if (ac==2)
  {
    if (0 == strncmp (av[1], "-h", 2))
    {
      JMHelpExit();
    }
  }

  if (ac>=3)
  {
    if (0 == strncmp (av[1], "-d", 2))
    {
      filename=av[2];
      CLcount = 3;
    }
    if (0 == strncmp (av[1], "-h", 2))
    {
      JMHelpExit();
    }
  }
  printf ("Parsing Configfile %s", filename);
  content = GetConfigFileContent (filename);
  if (NULL==content)
    error (errortext, 300);
  ParseContent (content, strlen(content));
  printf ("\n");
  free (content);

  // Parse the command line

  while (CLcount < ac)
  {
    if (0 == strncmp (av[CLcount], "-h", 2))
    {
      JMHelpExit();
    }

    if (0 == strncmp (av[CLcount], "-f", 2))  // A file parameter?
    {
      content = GetConfigFileContent (av[CLcount+1]);
      if (NULL==content)
        error (errortext, 300);
      printf ("Parsing Configfile %s", av[CLcount+1]);
      ParseContent (content, strlen (content));
      printf ("\n");
      free (content);
      CLcount += 2;
    } 
    else
    {
      if (0 == strncmp (av[CLcount], "-p", 2))  // A config change?
      {
        // Collect all data until next parameter (starting with -<x> (x is any character)),
        // put it into content, and parse content.

        CLcount++;
        ContentLen = 0;
        NumberParams = CLcount;

        // determine the necessary size for content
        while (NumberParams < ac && av[NumberParams][0] != '-')
          ContentLen += strlen (av[NumberParams++]);        // Space for all the strings
        ContentLen += 1000;                     // Additional 1000 bytes for spaces and \0s


        if ((content = malloc (ContentLen))==NULL) no_mem_exit("Configure: content");;
        content[0] = '\0';

        // concatenate all parameters identified before

        while (CLcount < NumberParams)
        {
          char *source = &av[CLcount][0];
          char *destin = &content[strlen (content)];

          while (*source != '\0')
          {
            if (*source == '=')  // The Parser expects whitespace before and after '='
            {
              *destin++=' '; *destin++='='; *destin++=' ';  // Hence make sure we add it
            } else
              *destin++=*source;
            source++;
          }
          *destin = '\0';
          CLcount++;
        }
        printf ("Parsing command line string '%s'", content);
        ParseContent (content, strlen(content));
        free (content);
        printf ("\n");
      }
      else
      {
        snprintf (errortext, ET_SIZE, "Error in command line, ac %d, around string '%s', missing -f or -p parameters?", CLcount, av[CLcount]);
        error (errortext, 300);
      }
    }
  }
  printf ("\n");
  PatchInp();
  if (params->DisplayEncParams)
    DisplayEncoderParams();
}

/*!
 ***********************************************************************
 * \brief
 *    allocates memory buf, opens file Filename in f, reads contents into
 *    buf and returns buf
 * \param Filename
 *    name of config file
 * \return
 *    if successfull, content of config file
 *    NULL in case of error. Error message will be set in errortext
 ***********************************************************************
 */
char *GetConfigFileContent (char *Filename)
{
  long FileSize;
  FILE *f;
  char *buf;

  if (NULL == (f = fopen (Filename, "r")))
  {
      snprintf (errortext, ET_SIZE, "Cannot open configuration file %s.", Filename);
      return NULL;
  }

  if (0 != fseek (f, 0, SEEK_END))
  {
    snprintf (errortext, ET_SIZE, "Cannot fseek in configuration file %s.", Filename);
    return NULL;
  }

  FileSize = ftell (f);
  if (FileSize < 0 || FileSize > 100000)
  {
    snprintf (errortext, ET_SIZE, "Unreasonable Filesize %ld reported by ftell for configuration file %s.", FileSize, Filename);
    return NULL;
  }
  if (0 != fseek (f, 0, SEEK_SET))
  {
    snprintf (errortext, ET_SIZE, "Cannot fseek in configuration file %s.", Filename);
    return NULL;
  }

  if ((buf = malloc (FileSize + 1))==NULL) no_mem_exit("GetConfigFileContent: buf");

  // Note that ftell() gives us the file size as the file system sees it.  The actual file size,
  // as reported by fread() below will be often smaller due to CR/LF to CR conversion and/or
  // control characters after the dos EOF marker in the file.

  FileSize = fread (buf, 1, FileSize, f);
  buf[FileSize] = '\0';


  fclose (f);
  return buf;
}


/*!
 ***********************************************************************
 * \brief
 *    Parses the character array buf and writes global variable input, which is defined in
 *    configfile.h.  This hack will continue to be necessary to facilitate the addition of
 *    new parameters through the Map[] mechanism (Need compiler-generated addresses in map[]).
 * \param buf
 *    buffer to be parsed
 * \param bufsize
 *    buffer size of buffer
 ***********************************************************************
 */
void ParseContent (char *buf, int bufsize)
{

  char *items[MAX_ITEMS_TO_PARSE];
  int MapIdx;
  int item = 0;
  int InString = 0, InItem = 0;
  char *p = buf;
  char *bufend = &buf[bufsize];
  int IntContent;
  double DoubleContent;
  int i;

// Stage one: Generate an argc/argv-type list in items[], without comments and whitespace.
// This is context insensitive and could be done most easily with lex(1).

  while (p < bufend)
  {
    switch (*p)
    {
      case 13:
        p++;
        break;
      case '#':                 // Found comment
        *p = '\0';              // Replace '#' with '\0' in case of comment immediately following integer or string
        while (*p != '\n' && p < bufend)  // Skip till EOL or EOF, whichever comes first
          p++;
        InString = 0;
        InItem = 0;
        break;
      case '\n':
        InItem = 0;
        InString = 0;
        *p++='\0';
        break;
      case ' ':
      case '\t':              // Skip whitespace, leave state unchanged
        if (InString)
          p++;
        else
        {                     // Terminate non-strings once whitespace is found
          *p++ = '\0';
          InItem = 0;
        }
        break;

      case '"':               // Begin/End of String
        *p++ = '\0';
        if (!InString)
        {
          items[item++] = p;
          InItem = ~InItem;
        }
        else
          InItem = 0;
        InString = ~InString; // Toggle
        break;

      default:
        if (!InItem)
        {
          items[item++] = p;
          InItem = ~InItem;
        }
        p++;
    }
  }

  item--;

  for (i=0; i<item; i+= 3)
  {
    if (0 > (MapIdx = ParameterNameToMapIndex (items[i])))
    {
      //snprintf (errortext, ET_SIZE, " Parsing error in config file: Parameter Name '%s' not recognized.", items[i]);
      //error (errortext, 300);
      printf ("\n\tParsing error in config file: Parameter Name '%s' not recognized.", items[i]);
      continue;
    }
    if (strcasecmp ("=", items[i+1]))
    {
      snprintf (errortext, ET_SIZE, " Parsing error in config file: '=' expected as the second token in each line.");
      error (errortext, 300);
    }

    // Now interpret the Value, context sensitive...

    switch (Map[MapIdx].Type)
    {
      case 0:           // Numerical
        if (1 != sscanf (items[i+2], "%d", &IntContent))
        {
          snprintf (errortext, ET_SIZE, " Parsing error: Expected numerical value for Parameter of %s, found '%s'.", items[i], items[i+2]);
          error (errortext, 300);
        }
        * (int *) (Map[MapIdx].Place) = IntContent;
        printf (".");
        break;
      case 1:
        strncpy ((char *) Map[MapIdx].Place, items [i+2], FILE_NAME_SIZE);
        printf (".");
        break;
      case 2:           // Numerical double
        if (1 != sscanf (items[i+2], "%lf", &DoubleContent))
        {
          snprintf (errortext, ET_SIZE, " Parsing error: Expected numerical value for Parameter of %s, found '%s'.", items[i], items[i+2]);
          error (errortext, 300);
        }
        * (double *) (Map[MapIdx].Place) = DoubleContent;
        printf (".");
        break;
      default:
        error ("Unknown value type in the map definition of configfile.h",-1);
    }
  }
  memcpy (params, &configinput, sizeof (InputParameters));
}

/*!
 ***********************************************************************
 * \brief
 *    Returns the index number from Map[] for a given parameter name.
 * \param s
 *    parameter name string
 * \return
 *    the index number if the string is a valid parameter name,         \n
 *    -1 for error
 ***********************************************************************
 */
static int ParameterNameToMapIndex (char *s)
{
  int i = 0;

  while (Map[i].TokenName != NULL)
    if (0==strcasecmp (Map[i].TokenName, s))
      return i;
    else
      i++;
  return -1;
}

/*!
 ***********************************************************************
 * \brief
 *    Sets initial values for encoding parameters.
 * \return
 *    -1 for error
 ***********************************************************************
 */
static int InitEncoderParams(void)
{
  int i = 0;

  while (Map[i].TokenName != NULL)
  {
    if (Map[i].Type == 0)
        * (int *) (Map[i].Place) = (int) Map[i].Default;
    else if (Map[i].Type == 2)
    * (double *) (Map[i].Place) = Map[i].Default;
      i++;
  }
  return -1;
}

/*!
 ***********************************************************************
 * \brief
 *    Validates encoding parameters.
 * \return
 *    -1 for error
 ***********************************************************************
 */
static int TestEncoderParams(int bitdepth_qp_scale[3])
{
  int i = 0;

  while (Map[i].TokenName != NULL)
  {
    if (Map[i].param_limits == 1)
    {
      if (Map[i].Type == 0)
      {
        if ( * (int *) (Map[i].Place) < (int) Map[i].min_limit || * (int *) (Map[i].Place) > (int) Map[i].max_limit )
        {
          snprintf(errortext, ET_SIZE, "Error in input parameter %s. Check configuration file. Value should be in [%d, %d] range.", Map[i].TokenName, (int) Map[i].min_limit,(int)Map[i].max_limit );
          error (errortext, 400);
        }

      }
      else if (Map[i].Type == 2)
      {
        if ( * (double *) (Map[i].Place) < Map[i].min_limit || * (double *) (Map[i].Place) > Map[i].max_limit )
        {
          snprintf(errortext, ET_SIZE, "Error in input parameter %s. Check configuration file. Value should be in [%.2f, %.2f] range.", Map[i].TokenName,Map[i].min_limit ,Map[i].max_limit );
          error (errortext, 400);
        }
      }
    }
    else if (Map[i].param_limits == 2)
    {
      if (Map[i].Type == 0)
      {
        if ( * (int *) (Map[i].Place) < (int) Map[i].min_limit )
        {
          snprintf(errortext, ET_SIZE, "Error in input parameter %s. Check configuration file. Value should not be smaller than %d.", Map[i].TokenName, (int) Map[i].min_limit);
          error (errortext, 400);
        }
      }
      else if (Map[i].Type == 2)
      {
        if ( * (double *) (Map[i].Place) < Map[i].min_limit )
        {
          snprintf(errortext, ET_SIZE, "Error in input parameter %s. Check configuration file. Value should not be smaller than %2.f.", Map[i].TokenName,Map[i].min_limit);
          error (errortext, 400);
        }
      }
    }
    else if (Map[i].param_limits == 3) // Only used for QPs
    {
      
      if (Map[i].Type == 0)
      {
        int cur_qp = * (int *) (Map[i].Place);
        int min_qp = (int) (Map[i].min_limit - bitdepth_qp_scale[0]);
        int max_qp = (int) Map[i].max_limit;
        if (( cur_qp < min_qp ) || ( cur_qp > max_qp ))
        {
          snprintf(errortext, ET_SIZE, "Error in input parameter %s. Check configuration file. Value should be in [%d, %d] range.", Map[i].TokenName, min_qp, max_qp );
          error (errortext, 400);
        }
      }
    }

    i++;
  }
  return -1;
}



/*!
 ***********************************************************************
 * \brief
 *    Outputs encoding parameters.
 * \return
 *    -1 for error
 ***********************************************************************
 */
static int DisplayEncoderParams(void)
{
  int i = 0;

  printf("******************************************************\n");
  printf("*               Encoder Parameters                   *\n");
  printf("******************************************************\n");
  while (Map[i].TokenName != NULL)
  {
    if (Map[i].Type == 0)
      printf("Parameter %s = %d\n",Map[i].TokenName,* (int *) (Map[i].Place));
    else if (Map[i].Type == 1)
      printf("Parameter %s = ""%s""\n",Map[i].TokenName,(char *)  (Map[i].Place));
    else if (Map[i].Type == 2)
      printf("Parameter %s = %.2f\n",Map[i].TokenName,* (double *) (Map[i].Place));
      i++;
  }
  printf("******************************************************\n");
  return -1;
}

/*!
 ************************************************************************
 * \brief
 *    calculate Ceil(Log2(uiVal))
 ************************************************************************
 */
unsigned CeilLog2( unsigned uiVal)
{
  unsigned uiTmp = uiVal-1;
  unsigned uiRet = 0;

  while( uiTmp != 0 )
  {
    uiTmp >>= 1;
    uiRet++;
  }
  return uiRet;
}

/*!
 ************************************************************************
 * \brief
 *    read the slice group configuration file. Returns without action
 *    if type is not 0, 2 or 6
 ************************************************************************
 */
void read_slice_group_info()
{
  FILE * sgfile=NULL;
  int i;
  int ret;

  if ((params->slice_group_map_type != 0) && (params->slice_group_map_type != 2) && (params->slice_group_map_type != 6))
  {
    // nothing to do
    return;
  }

  // do we have a file name (not only NULL character)
  if (strlen (params->SliceGroupConfigFileName) <= 1)
    error ("No slice group config file name specified", 500);
    
  // open file
  sgfile = fopen(params->SliceGroupConfigFileName,"r");

  if ( NULL==sgfile )
  {
    snprintf(errortext, ET_SIZE, "Error opening slice group file %s", params->SliceGroupConfigFileName);
    error (errortext, 500);
  }

  switch (params->slice_group_map_type)
  {
  case 0:
    params->run_length_minus1=(int *)malloc(sizeof(int)*(params->num_slice_groups_minus1+1));
    if ( NULL==params->run_length_minus1 )
    {
      fclose(sgfile);
      no_mem_exit("PatchInp: params->run_length_minus1");
    }

    // each line contains one 'run_length_minus1' value
    for(i=0;i<=params->num_slice_groups_minus1;i++)
    {
      ret = fscanf(sgfile,"%d",(params->run_length_minus1+i));
      fscanf(sgfile,"%*[^\n]");
      if ( 1!=ret )
      {
        fclose(sgfile);
        snprintf(errortext, ET_SIZE, "Error while reading slice group config file (line %d)", i+1);
        error (errortext, 500);
      }
    }
    break;

  case 2:
    params->top_left=(int *)malloc(sizeof(int)*params->num_slice_groups_minus1);
    params->bottom_right=(int *)malloc(sizeof(int)*params->num_slice_groups_minus1);
    if (NULL==params->top_left)
    {
      fclose(sgfile);
      no_mem_exit("PatchInp: params->top_left");
    }
    if (NULL==params->bottom_right)
    {
      fclose(sgfile);
      no_mem_exit("PatchInp: params->bottom_right");
    }

    // every two lines contain 'top_left' and 'bottom_right' value
    for(i=0;i<params->num_slice_groups_minus1;i++)
    {
      ret = fscanf(sgfile,"%d",(params->top_left+i));
      fscanf(sgfile,"%*[^\n]");
      if ( 1!=ret )
      {
        fclose(sgfile);
        snprintf(errortext, ET_SIZE, "Error while reading slice group config file (line %d)", 2*i +1);
        error (errortext, 500);
      }
      ret = fscanf(sgfile,"%d",(params->bottom_right+i));
      fscanf(sgfile,"%*[^\n]");
      if ( 1!=ret )
      {
        fclose(sgfile);
        snprintf(errortext, ET_SIZE, "Error while reading slice group config file (line %d)", 2*i + 2);
        error (errortext, 500);
      }
    }
    break;

  case 6:
    {
      int tmp;
      int frame_mb_only;
      int mb_width, mb_height, mapunit_height;

      frame_mb_only = !(params->PicInterlace || params->MbInterlace);
      mb_width  = (params->output.width + img->auto_crop_right)>>4;
      mb_height = (params->output.height + img->auto_crop_bottom)>>4;
      mapunit_height = mb_height / (2-frame_mb_only);

      params->slice_group_id=(byte * ) malloc(sizeof(byte)*mapunit_height*mb_width);
      if (NULL==params->slice_group_id)
      {
        fclose(sgfile);
        no_mem_exit("PatchInp: params->slice_group_id");
      }

      // each line contains slice_group_id for one Macroblock
      for (i=0;i<mapunit_height*mb_width;i++)
      {
        ret = fscanf(sgfile,"%d", &tmp);
        params->slice_group_id[i]= (byte) tmp;
        if ( 1!=ret )
        {
          fclose(sgfile);
          snprintf(errortext, ET_SIZE, "Error while reading slice group config file (line %d)", i + 1);
          error (errortext, 500);
        }
        if ( *(params->slice_group_id+i) > params->num_slice_groups_minus1 )
        {
          fclose(sgfile);
          snprintf(errortext, ET_SIZE, "Error while reading slice group config file: slice_group_id not allowed (line %d)", i + 1);
          error (errortext, 500);
        }
        fscanf(sgfile,"%*[^\n]");
      }
    }
    break;

  default:
    // we should not get here
    error ("Wrong slice group type while reading config file", 500);
    break;
  }

  // close file again
  fclose(sgfile);
}

/*!
 ***********************************************************************
 * \brief
 *    Checks the input parameters for consistency.
 ***********************************************************************
 */
static void PatchInp (void)
{
  int i,j;
  int storedBplus1;
  int bitdepth_qp_scale[3];

  if (params->src_BitDepthRescale)
  {
    bitdepth_qp_scale [0] = 6*(params->output.bit_depth[0] - 8);
    bitdepth_qp_scale [1] = 6*(params->output.bit_depth[1] - 8);
    bitdepth_qp_scale [2] = 6*(params->output.bit_depth[2] - 8);
  }
  else
  {
    bitdepth_qp_scale [0] = 6*(params->source.bit_depth[0] - 8);
    bitdepth_qp_scale [1] = 6*(params->source.bit_depth[1] - 8);
    bitdepth_qp_scale [2] = 6*(params->source.bit_depth[2] - 8);
  }

  TestEncoderParams(bitdepth_qp_scale);

  if (params->FrameRate == 0.0)
    params->FrameRate = INIT_FRAME_RATE;

  // Set block sizes

  // Skip/Direct16x16
  params->part_size[0][0] = 4;
  params->part_size[0][1] = 4;
  // 16x16
  params->part_size[1][0] = 4;
  params->part_size[1][1] = 4;
  // 16x8
  params->part_size[2][0] = 4;
  params->part_size[2][1] = 2;
  // 8x16
  params->part_size[3][0] = 2;
  params->part_size[3][1] = 4;
  // 8x8
  params->part_size[4][0] = 2;
  params->part_size[4][1] = 2;
  // 8x4
  params->part_size[5][0] = 2;
  params->part_size[5][1] = 1;
  // 4x8
  params->part_size[6][0] = 1;
  params->part_size[6][1] = 2;
  // 4x4
  params->part_size[7][0] = 1;
  params->part_size[7][1] = 1;

  for (j = 0; j<8;j++)
  {
    for (i = 0; i<2; i++)
    {
      params->blc_size[j][i] = params->part_size[j][i] * BLOCK_SIZE;
    }
  }

  if (params->idr_period && params->intra_delay && params->idr_period <= params->intra_delay)
  {
    snprintf(errortext, ET_SIZE, " IntraDelay cannot be larger than or equal to IDRPeriod.");
    error (errortext, 500);
  }

  if (params->idr_period && params->intra_delay && params->EnableIDRGOP == 0)
  {
    snprintf(errortext, ET_SIZE, " IntraDelay can only be used with only 1 IDR or with EnableIDRGOP=1.");
    error (errortext, 500);
  }

  if (params->idr_period && params->intra_delay && params->adaptive_idr_period)
  {
    snprintf(errortext, ET_SIZE, " IntraDelay can not be used with AdaptiveIDRPeriod.");
    error (errortext, 500);
  }

  storedBplus1 = (params->BRefPictures ) ? params->successive_Bframe + 1: 1;
  
  if (params->Log2MaxFNumMinus4 == -1)
  {    
    log2_max_frame_num_minus4 = iClip3(0,12, (int) (CeilLog2(params->no_frames * storedBplus1) - 4));    
  }
  else  
    log2_max_frame_num_minus4 = params->Log2MaxFNumMinus4;
  max_frame_num = 1 << (log2_max_frame_num_minus4 + 4);

  if (log2_max_frame_num_minus4 == 0 && params->num_ref_frames == 16)
  {
    snprintf(errortext, ET_SIZE, " NumberReferenceFrames=%d and Log2MaxFNumMinus4=%d may lead to an invalid value of frame_num.", params->num_ref_frames, params-> Log2MaxFNumMinus4);
    error (errortext, 500);
  }

  // set proper log2_max_pic_order_cnt_lsb_minus4.
  if (params->Log2MaxPOCLsbMinus4 == - 1)
    log2_max_pic_order_cnt_lsb_minus4 = iClip3(0,12, (int) (CeilLog2( 2*params->no_frames * (params->jumpd + 1)) - 4));
  else
    log2_max_pic_order_cnt_lsb_minus4 = params->Log2MaxPOCLsbMinus4;
  max_pic_order_cnt_lsb = 1 << (log2_max_pic_order_cnt_lsb_minus4 + 4);

  if (((1<<(log2_max_pic_order_cnt_lsb_minus4 + 3)) < params->jumpd * 4) && params->Log2MaxPOCLsbMinus4 != -1)
    error("log2_max_pic_order_cnt_lsb_minus4 might not be sufficient for encoding. Increase value.",400);

  // B picture consistency check
  if(params->successive_Bframe > params->jumpd)
  {
    snprintf(errortext, ET_SIZE, "Number of B-frames %d can not exceed the number of frames skipped", params->successive_Bframe);
    error (errortext, 400);
  }

  // Direct Mode consistency check
  if(params->successive_Bframe && params->direct_spatial_mv_pred_flag != DIR_SPATIAL && params->direct_spatial_mv_pred_flag != DIR_TEMPORAL)
  {
    snprintf(errortext, ET_SIZE, "Unsupported direct mode=%d, use TEMPORAL=0 or SPATIAL=1", params->direct_spatial_mv_pred_flag);
    error (errortext, 400);
  }

  if (params->PicInterlace>0 || params->MbInterlace>0)
  {
    if (params->directInferenceFlag==0)
      printf("\nWarning: DirectInferenceFlag set to 1 due to interlace coding.");
    params->directInferenceFlag = 1;
  }

  // Open Files
  if ((p_in=open(params->infile, OPENFLAGS_READ))==-1)
  {
    snprintf(errortext, ET_SIZE, "Input file %s does not exist",params->infile);
    error (errortext, 500);
  }

  updateOutFormat(params);

  if (params->no_frames == -1)
  {
    getNumberOfFrames();
  }

  if (params->no_frames < 1)
  {      
    snprintf(errortext, ET_SIZE, "Not enough frames to encode (%d)", params->no_frames);
    error (errortext, 500);
  }


  if (strlen (params->ReconFile) > 0 && (p_dec=open(params->ReconFile, OPENFLAGS_WRITE, OPEN_PERMISSIONS))==-1)
  {
    snprintf(errortext, ET_SIZE, "Error open file %s", params->ReconFile);
    error (errortext, 500);
  }

#if TRACE
  if (strlen (params->TraceFile) > 0 && (p_trace=fopen(params->TraceFile,"w"))==NULL)
  {
    snprintf(errortext, ET_SIZE, "Error open file %s", params->TraceFile);
    error (errortext, 500);
  }
#endif

  if (params->output.width % 16 != 0)
  {
    img->auto_crop_right = 16 - (params->output.width % 16);
  }
  else
  {
    img->auto_crop_right=0;
  }
  if (params->PicInterlace || params->MbInterlace)
  {
    if ((params->output.height & 0x01) != 0)
    {
      error ("even number of lines required for interlaced coding", 500);
    }
    
    if (params->output.height % 32 != 0)
    {
      img->auto_crop_bottom = 32-(params->output.height % 32);
    }
    else
    {
      img->auto_crop_bottom=0;
    }
  }
  else
  {
    if (params->output.height % 16 != 0)
    {
      img->auto_crop_bottom = 16-(params->output.height % 16);
    }
    else
    {
      img->auto_crop_bottom=0;
    }
  }
  if (img->auto_crop_bottom || img->auto_crop_right)
  {
    fprintf (stderr, "Warning: Automatic cropping activated: Coded frame Size: %dx%d\n", 
      params->output.width + img->auto_crop_right, params->output.height + img->auto_crop_bottom);
  }

  if ((params->slice_mode==1)&&(params->MbInterlace!=0))
  {
    if ((params->slice_argument%2)!=0)
    {
      fprintf ( stderr, "Warning: slice border within macroblock pair. ");
      if (params->slice_argument > 1)
      {
        params->slice_argument--;
      }
      else
      {
        params->slice_argument++;
      }
      fprintf ( stderr, "Using %d MBs per slice.\n", params->slice_argument);
    }
  }

  // read the slice group configuration file. Only for types 0, 2 or 6
  if ( 0 != params->num_slice_groups_minus1 )
  {
    read_slice_group_info();
  }

  if (params->WPMCPrecision && (params->RDPictureDecision != 1 || params->GenerateMultiplePPS != 1) )
  {
    snprintf(errortext, ET_SIZE, "WPMCPrecision requires both RDPictureDecision=1 and GenerateMultiplePPS=1.\n");
    error (errortext, 400);
  }
  if (params->WPMCPrecision && params->WPMCPrecFullRef && params->num_ref_frames < 16 )
  {
    params->num_ref_frames++;
    if ( params->P_List0_refs )
      params->P_List0_refs++;
    else
      params->P_List0_refs = params->num_ref_frames;
    if ( params->B_List0_refs )
      params->B_List0_refs++;
    else
      params->B_List0_refs = params->num_ref_frames;
    if ( params->B_List1_refs )
      params->B_List1_refs++;
    else
      params->B_List1_refs = params->num_ref_frames;
  }
  else if ( params->WPMCPrecision && params->WPMCPrecFullRef )
  {
    snprintf(errortext, ET_SIZE, "WPMCPrecFullRef requires NumberReferenceFrames < 16.\n");
    error (errortext, 400);
  }

  if (params->ReferenceReorder && params->MbInterlace )
  {
    snprintf(errortext, ET_SIZE, "ReferenceReorder not supported with MBAFF\n");
    error (errortext, 400);
  }

  if (params->PocMemoryManagement && params->MbInterlace )
  {
    snprintf(errortext, ET_SIZE, "PocMemoryManagement not supported with MBAFF\n");
    error (errortext, 400);
  }

  if ((!params->rdopt)&&(params->MbInterlace))
  {
    snprintf(errortext, ET_SIZE, "MB AFF is not compatible with non-rd-optimized coding.");
    error (errortext, 500);
  }

  // check RDoptimization mode and profile. FMD does not support Frex Profiles.
  if (params->rdopt==2 && ( params->ProfileIDC>=FREXT_HP || params->ProfileIDC==FREXT_CAVLC444 ))
  {
    snprintf(errortext, ET_SIZE, "Fast Mode Decision methods not supported in FREX Profiles");
    error (errortext, 500);
  }

  if ( (params->MEErrorMetric[Q_PEL] == ERROR_SATD && params->MEErrorMetric[H_PEL] == ERROR_SAD && params->MEErrorMetric[F_PEL] == ERROR_SAD)
    && params->SearchMode > FAST_FULL_SEARCH && params->SearchMode < EPZS)
  {
    snprintf(errortext, ET_SIZE, "MEDistortionQPel=2, MEDistortionHPel=0, MEDistortionFPel=0 is not allowed when SearchMode is set to 1 or 2.");
    error (errortext, 500);
  }

  // Tian Dong: May 31, 2002
  // The number of frames in one sub-seq in enhanced layer should not exceed
  // the number of reference frame number.
  if ( params->NumFramesInELSubSeq > params->num_ref_frames || params->NumFramesInELSubSeq < 0 )
  {
    snprintf(errortext, ET_SIZE, "NumFramesInELSubSeq (%d) is out of range [0,%d).", params->NumFramesInELSubSeq, params->num_ref_frames);
    error (errortext, 500);
  }
  // Tian Dong: Enhanced GOP is not supported in bitstream mode. September, 2002
  if ( params->NumFramesInELSubSeq > 0 && params->of_mode == PAR_OF_ANNEXB )
  {
    snprintf(errortext, ET_SIZE, "Enhanced GOP is not supported in bitstream mode and RTP mode yet.");
    error (errortext, 500);
  }
  // Tian Dong (Sept 2002)
  // The AFF is not compatible with spare picture for the time being.
  if ((params->PicInterlace || params->MbInterlace) && params->SparePictureOption == TRUE)
  {
    snprintf(errortext, ET_SIZE, "AFF is not compatible with spare picture.");
    error (errortext, 500);
  }

  // Only the RTP mode is compatible with spare picture for the time being.
  if (params->of_mode != PAR_OF_RTP && params->SparePictureOption == TRUE)
  {
    snprintf(errortext, ET_SIZE, "Only RTP output mode is compatible with spare picture features.");
    error (errortext, 500);
  }

  if( (params->WeightedPrediction > 0 || params->WeightedBiprediction > 0) && (params->MbInterlace))
  {
    snprintf(errortext, ET_SIZE, "Weighted prediction coding is not supported for MB AFF currently.");
    error (errortext, 500);
  }
  if ( params->NumFramesInELSubSeq > 0 && params->WeightedPrediction > 0)
  {
    snprintf(errortext, ET_SIZE, "Enhanced GOP is not supported in weighted prediction coding mode yet.");
    error (errortext, 500);
  }

  //! the number of slice groups is forced to be 1 for slice group type 3-5
  if(params->num_slice_groups_minus1 > 0)
  {
    if( (params->slice_group_map_type >= 3) && (params->slice_group_map_type<=5) )
      params->num_slice_groups_minus1 = 1;
  }

  // Rate control
  if(params->RCEnable)
  {
    if (params->basicunit == 0)
      params->basicunit = (params->output.height + img->auto_crop_bottom)*(params->output.width + img->auto_crop_right)/256;

    if ( ((params->output.height + img->auto_crop_bottom)*(params->output.width + img->auto_crop_right)/256) % params->basicunit != 0)
    {
      snprintf(errortext, ET_SIZE, "Frame size in macroblocks must be a multiple of BasicUnit.");
      error (errortext, 500);
    }

    if ( params->RCUpdateMode == RC_MODE_1 && 
      !( (params->intra_period == 1 || params->idr_period == 1 || params->BRefPictures == 2 ) && !params->successive_Bframe ) )
    {
      snprintf(errortext, ET_SIZE, "Use RCUpdateMode = 1 only for all intra or all B-slice coding.");
      error (errortext, 500);
    }

    if ( params->BRefPictures == 2 && params->intra_period == 0 && params->RCUpdateMode != RC_MODE_1 )
    {
      snprintf(errortext, ET_SIZE, "Use RCUpdateMode = 1 for all B-slice coding.");
      error (errortext, 500);
    }

    if ( params->HierarchicalCoding && params->RCUpdateMode != RC_MODE_2 && params->RCUpdateMode != RC_MODE_3 )
    {
      snprintf(errortext, ET_SIZE, "Use RCUpdateMode = 2 or 3 for hierarchical B-picture coding.");
      error (errortext, 500);
    }

    if ( (params->RCUpdateMode != RC_MODE_1) && (params->intra_period == 1) )
    {
      snprintf(errortext, ET_SIZE, "Use RCUpdateMode = 1 for all intra coding.");
      error (errortext, 500);
    }
  }

  if ((params->successive_Bframe)&&(params->BRefPictures)&&(params->idr_period)&&(params->pic_order_cnt_type!=0))
  {
    error("Stored B pictures combined with IDR pictures only supported in Picture Order Count type 0\n",-1000);
  }

  if( !params->direct_spatial_mv_pred_flag && params->num_ref_frames<2 && params->successive_Bframe >0)
    error("temporal direct needs at least 2 ref frames\n",-1000);

  if (params->rdopt == 0)
  {
    if ((params->DisableSubpelME && params->MEErrorMetric[F_PEL] != params->ModeDecisionMetric))
    {
      snprintf(errortext, ET_SIZE, "\nLast refinement level (FPel) distortion not the same as Mode decision distortion.\nPlease update MEDistortionFPel (%d) and/or  MDDistortion(%d).", params->MEErrorMetric[F_PEL], params->ModeDecisionMetric);
      error (errortext, 500);
    }
    else if (params->MEErrorMetric[Q_PEL] != params->ModeDecisionMetric)
    {
      snprintf(errortext, ET_SIZE, "\nLast refinement level (QPel) distortion not the same as Mode decision distortion.\nPlease update MEDistortionQPel (%d) and/or  MDDistortion(%d).", params->MEErrorMetric[Q_PEL], params->ModeDecisionMetric);
      error (errortext, 500);
    }
  }
  // frext
  if(params->Transform8x8Mode && params->sp_periodicity /*SP-frames*/)
  {
    snprintf(errortext, ET_SIZE, "\nThe new 8x8 mode is not implemented for sp-frames.");
    error (errortext, 500);
  }

  if(params->Transform8x8Mode && ( params->ProfileIDC<FREXT_HP && params->ProfileIDC!=FREXT_CAVLC444 ))
  {
    snprintf(errortext, ET_SIZE, "\nTransform8x8Mode may be used only with ProfileIDC %d to %d.", FREXT_HP, FREXT_Hi444);
    error (errortext, 500);
  }

  if (params->DisableIntra4x4 == 1 && params->DisableIntra16x16 == 1 && params->EnableIPCM == 0 && params->Transform8x8Mode == 0)
  {
    snprintf(errortext, ET_SIZE, "\nAt least one intra prediction mode needs to be enabled.");
    error (errortext, 500);
  }

  if(params->ScalingMatrixPresentFlag && ( params->ProfileIDC<FREXT_HP && params->ProfileIDC!=FREXT_CAVLC444 ))
  {
    snprintf(errortext, ET_SIZE, "\nScalingMatrixPresentFlag may be used only with ProfileIDC %d to %d.", FREXT_HP, FREXT_Hi444);
    error (errortext, 500);
  }

  if(params->yuv_format==YUV422 && ( params->ProfileIDC < FREXT_Hi422 && params->ProfileIDC!=FREXT_CAVLC444 ))
  {
    snprintf(errortext, ET_SIZE, "\nFRExt Profile(YUV Format) Error!\nYUV422 can be used only with ProfileIDC %d or %d\n",FREXT_Hi422, FREXT_Hi444);
    error (errortext, 500);
  }
  if(params->yuv_format==YUV444 && ( params->ProfileIDC < FREXT_Hi444 && params->ProfileIDC!=FREXT_CAVLC444 ))
  {
    snprintf(errortext, ET_SIZE, "\nFRExt Profile(YUV Format) Error!\nYUV444 can be used only with ProfileIDC %d.\n",FREXT_Hi444);
    error (errortext, 500);
  }

  if (params->successive_Bframe && ((params->BiPredMotionEstimation) && (params->search_range < params->BiPredMESearchRange)))
  {
    snprintf(errortext, ET_SIZE, "\nBiPredMESearchRange must be smaller or equal SearchRange.");
    error (errortext, 500);
  }

  // check consistency
  if ( params->ChromaMEEnable && !(params->ChromaMCBuffer) ) 
  {
    snprintf(errortext, ET_SIZE, "\nChromaMCBuffer must be set to 1 if ChromaMEEnable is set.");
    error (errortext, 500);
  }

  if ( params->ChromaMEEnable && params->yuv_format ==  YUV400) 
  {
    fprintf(stderr, "Warning: ChromaMEEnable cannot be used with YUV400 color format, disabling ChromaMEEnable.\n");
    params->ChromaMEEnable = 0;
  }

  if ( (params->ChromaMCBuffer == 0) && (( params->yuv_format ==  YUV444) && (!params->separate_colour_plane_flag)) )
  {
    fprintf(stderr, "Warning: Enabling ChromaMCBuffer for YUV444 combined color coding.\n");
    params->ChromaMCBuffer = 1;
  }


  if (params->EnableOpenGOP)
    params->ReferenceReorder = 1;

  if (params->SearchMode != EPZS)
    params->EPZSSubPelGrid = 0;

  params->EPZSGrid = params->EPZSSubPelGrid << 1;

  if (params->redundant_pic_flag)
  {
    if (params->PicInterlace || params->MbInterlace)
    {
      snprintf(errortext, ET_SIZE, "Redundant pictures cannot be used with interlaced tools.");
      error (errortext, 500);
    }
    if (params->RDPictureDecision)
    {
      snprintf(errortext, ET_SIZE, "Redundant pictures cannot be used with RDPictureDecision.");
      error (errortext, 500);
    }
    if (params->successive_Bframe)
    {
      snprintf(errortext, ET_SIZE, "Redundant pictures cannot be used with B frames.");
      error (errortext, 500);
    }
    if (params->PrimaryGOPLength < (1 << params->NumRedundantHierarchy))
    {
      snprintf(errortext, ET_SIZE, "PrimaryGOPLength must be equal or greater than 2^NumRedundantHierarchy.");
      error (errortext, 500);
    }
    if (params->num_ref_frames < params->PrimaryGOPLength)
    {
      snprintf(errortext, ET_SIZE, "NumberReferenceFrames must be greater than or equal to PrimaryGOPLength.");
      error (errortext, 500);
    }
  }

  if (params->num_ref_frames == 1 && params->successive_Bframe)
  {
    fprintf( stderr, "\nWarning: B slices used but only one reference allocated within reference buffer.\n");
    fprintf( stderr, "         Performance may be considerably compromised! \n");
    fprintf( stderr, "         2 or more references recommended for use with B slices.\n");
  }
  if ((params->HierarchicalCoding || params->BRefPictures) && params->successive_Bframe)
  {
    fprintf( stderr, "\nWarning: Hierarchical coding or Referenced B slices used.\n");
    fprintf( stderr, "         Make sure that you have allocated enough references\n");
    fprintf( stderr, "         in reference buffer to achieve best performance.\n");
  }

  if (params->FastMDEnable == 0)
  {
    params->FastIntraMD = 0;
    params->FastIntra16x16 = 0;
    params->FastIntra4x4 = 0;
    params->FastIntra8x8 = 0;
    params->FastIntraChroma = 0;
  }

  if (params->UseRDOQuant == 1)
  {
    if (params->rdopt == 0)
    {
      snprintf(errortext, ET_SIZE, "RDO Quantization not supported with low complexity RDO.");
      error (errortext, 500);
    }

    if (params->MbInterlace != 0)
    {
      printf("RDO Quantization currently not supported with MBAFF. Option disabled.\n");
      params->UseRDOQuant = 0;
      params->RDOQ_QP_Num = 1;
      params->RDOQ_CP_MV = 0;
      params->RDOQ_CP_Mode = 0;
    }
    else
    {
      params->AdaptiveRounding = 0;
      printf("AdaptiveRounding is disabled when RDO Quantization is used\n");
      if (params->RDOQ_QP_Num < 2)
      {
        params->RDOQ_CP_MV = 0;
        params->RDOQ_CP_Mode = 0;
      }
    }
  }
  else
  {
    params->RDOQ_QP_Num = 1;
    params->RDOQ_CP_MV = 0;
    params->RDOQ_CP_Mode = 0;
  }

  ProfileCheck();
  LevelCheck();
}

void PatchInputNoFrames(void)
{
  // Tian Dong: May 31, 2002
  // If the frames are grouped into two layers, "FramesToBeEncoded" in the config file
  // will give the number of frames which are in the base layer. Here we let params->no_frames
  // be the total frame numbers.
  params->no_frames = 1 + (params->no_frames - 1) * (params->NumFramesInELSubSeq + 1);
}

