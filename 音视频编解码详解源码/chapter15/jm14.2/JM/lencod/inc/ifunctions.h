
/*!
 ************************************************************************
 *  \file
 *     ifunctions.h
 *
 *  \brief
 *     define some inline functions that are used within the encoder.
 *
 *  \author
 *      Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Karsten Sühring                 <suehring@hhi.de>
 *      - Alexis Tourapis                 <alexismt@ieee.org>
 *
 ************************************************************************
 */
#ifndef _IFUNCTIONS_H_
#define _IFUNCTIONS_H_

# if !defined(WIN32) && (__STDC_VERSION__ < 199901L)
  #define static
  #define inline
#endif
#include <math.h>

static inline int imin(int a, int b)
{
  return ((a) < (b)) ? (a) : (b);
}

static inline int imax(int a, int b)
{
  return ((a) > (b)) ? (a) : (b);
}

static inline double dmin(double a, double b)
{
  return ((a) < (b)) ? (a) : (b);
}

static inline double dmax(double a, double b)
{
  return ((a) > (b)) ? (a) : (b);
}

static inline int64 i64min(int64 a, int64 b)
{
  return ((a) < (b)) ? (a) : (b);
}

static inline int64 i64max(int64 a, int64 b)
{
  return ((a) > (b)) ? (a) : (b);
}

static inline int iabs(int x)
{
  return ((x) < 0) ? -(x) : (x);
}

static inline double dabs(double x)
{
  return ((x) < 0) ? -(x) : (x);
}

static inline int64 i64abs(int64 x)
{
  return ((x) < 0) ? -(x) : (x);
}

static inline double dabs2(double x)
{
  return (x) * (x);
}

static inline int iabs2(int x) 
{
  return (x) * (x);
}

static inline int64 i64abs2(int64 x)
{
  return (x) * (x);
}

static inline int isign(int x)
{
  return ((x) < 0) ? -1 : 1;
}

static inline int isignab(int a, int b)
{
  return ((b) < 0) ? -iabs(a) : iabs(a);
}

static inline int rshift_rnd(int x, int a)
{
  return (a > 0) ? ((x + (1 << (a-1) )) >> a) : (x << (-a));
}

static inline int rshift_rnd_sign(int x, int a)
{
  return (x > 0) ? ( ( x + (1 << (a-1)) ) >> a ) : (-( ( iabs(x) + (1 << (a-1)) ) >> a ));
}

static inline unsigned int rshift_rnd_us(unsigned int x, unsigned int a)
{
  return (a > 0) ? ((x + (1 << (a-1))) >> a) : x;
}

static inline int rshift_rnd_sf(int x, int a)
{
  return ((x + (1 << (a-1) )) >> a);
}

static inline unsigned int rshift_rnd_us_sf(unsigned int x, unsigned int a)
{
  return ((x + (1 << (a-1))) >> a);
}

static inline int iClip1(int high, int x)
{
  x = imax(x, 0);
  x = imin(x, high);

  return x;
}

static inline int iClip3(int low, int high, int x)
{
  x = imax(x, low);
  x = imin(x, high);

  return x;
}

static inline double dClip3(double low, double high, double x)
{
  x = dmax(x, low);
  x = dmin(x, high);

  return x;
}

static inline int weighted_cost(int factor, int bits)
{
  return (((factor)*(bits))>>LAMBDA_ACCURACY_BITS);
}

static inline int RSD(int x)
{
 return ((x&2)?(x|1):(x&(~1)));
}

static inline int power2(int x) 
{
  return 1 << (x);
}

static inline int float2int (float x)
{
  return (int)((x < 0) ? (x - 0.5f) : (x + 0.5f));
}

#if ZEROSNR
static inline float psnr(int max_sample_sq, int samples, float sse_distortion ) 
{
  return (float) (10.0 * log10(max_sample_sq * (double) ((double) samples / (sse_distortion == 0.0 ? 1.0 : sse_distortion))));
}
#else
static inline float psnr(int max_sample_sq, int samples, float sse_distortion ) 
{
  return (float) (sse_distortion == 0.0 ? 0.0 : (10.0 * log10(max_sample_sq * (double) ((double) samples / sse_distortion))));
}
#endif


# if !defined(WIN32) && (__STDC_VERSION__ < 199901L)
  #undef static
  #undef inline
#endif

#endif

