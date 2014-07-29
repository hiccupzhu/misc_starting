#include <stdio.h>
#include "subtitle_srt.h"

//#define FILE_NAME "E:\\subtitle\\srt\\the.53rd.annual.grammy.awards.srt"
//#define FILE_NAME "panda.srt"
#define FILE_NAME "subtitle1.srt"

int main(int argc,char* argv[])
{
    subtitle_srt srt;
    srt.load(FILE_NAME);
    srt.read_srt_data();
    return 0;
}