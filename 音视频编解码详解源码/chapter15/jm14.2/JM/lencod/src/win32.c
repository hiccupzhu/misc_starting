
/*!
 *************************************************************************************
 * \file win32.c
 *
 * \brief
 *    Platform dependent code
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Karsten Suehring                  <suehring@hhi.de>
 *************************************************************************************
 */

#include "global.h"


#ifdef _WIN32

static LARGE_INTEGER freq;

void gettime(TIME_T* time)
{
  QueryPerformanceCounter(time);
}

time_t timediff(TIME_T* start, TIME_T* end)
{
  static int first = 1;

  if(first) 
  {
    QueryPerformanceFrequency(&freq);
    first = 0;
  }
  return (time_t)((end->QuadPart - start->QuadPart)* 1000 /(freq.QuadPart));
}

#else

static struct timezone tz;

void gettime(TIME_T* time)
{
  gettimeofday(time, &tz);
}

time_t timediff(TIME_T* start, TIME_T* end)
{
  time_t t1, t2;

  t1 =  start->tv_sec * 1000 + (start->tv_usec/1000);
  t2 =  end->tv_sec  * 1000 + (end->tv_usec/1000);
  return t2-t1;
}
#endif
