#include <string>
#include <fstream>
#include <iostream>
#include <time.h>
#include "subtitle_srt.h"

#define BUFFER_SIZE     1024 * 4
#define SRT_DATA_POOL_SIZE    0x1000

subtitle_srt::subtitle_srt(void):
m_fp(NULL)
{
    read_buf = NULL;
    read_buf_ptr = NULL;
    read_buf_end = NULL;
    m_index = 0;
}

subtitle_srt::~subtitle_srt(void)
{
    m_srt.clear();
    SAFE_FREE(text_buf);
    SAFE_FREE(read_buf);
}

int subtitle_srt::detect(uint8_t* filename)
{
    return 0;
}

int subtitle_srt::load(char* filename)
{
    m_fp = fopen(filename,"rb");
    if(m_fp == NULL){
        return SRT_ERR_OPEN_FILE;
    }
    fseek(m_fp,0,SEEK_END);
    file_size = ftell(m_fp);
    fseek(m_fp,0,SEEK_SET);

    read_buf = (char*)malloc(file_size);
    if(read_buf == NULL) return SRT_EER_ONMEM;
    read_buf_ptr = read_buf;
    fread(read_buf,1,file_size,m_fp);
    read_buf_end = read_buf + file_size;

    text_buf = (char*)calloc(1,SRT_DATA_POOL_SIZE);
    if(text_buf == NULL)    return SRT_EER_ONMEM;
    text_buf_ptr = text_buf;
    text_buf_end = text_buf + SRT_DATA_POOL_SIZE;
    return S_OK;
}

int subtitle_srt::get_read_buf_size() const
{
    return read_buf_end - read_buf;
}

int subtitle_srt::get_read_buf_remain() const
{
    return read_buf_end - read_buf_ptr;
}

int subtitle_srt::find_first_char(const char* src,int len,char ch)
{
    for(int i = 0; i < len;i++){
        if(src[i] == ch)    return i;
    }
    return -1;
}

int subtitle_srt::get_text_data_size() const
{
    return text_buf_ptr - text_buf;
}

int subtitle_srt::get_text_buf_size() const
{
    return text_buf_end - text_buf;
}

int subtitle_srt::analysis_text(char* src,int& style_len)
{
    style_len = 0;

    char    cache[10] = {0};
    int     bottom = 0;
    int     top = 0;
    int     style = 0;
    int     i = 0;
    if(src[0] == '<'){
        for(i = 0; top != bottom || cache[0] == '\0';i++){
            if(src[i] == '<'){
                cache[top ++] = src[i];
            }
            if(src[i] == '>'){
                cache[top --] = 0;
            }
            if(top > 0){
                style <<= 8;
                style |= src[i];
            }
        }
        style_len = i;
    }
    return style;
}

void print_hex(char* data,int len)
{
    for(int i =0 ;i < len ;i ++)
    {
        printf("%02X ",data[i]);
    }
    printf("\n");
}

int subtitle_srt::string_trim(char** src,int& len)
{
    char tp[] = {0x0D,0x0A};
    int index = 0;
    if((*src)[index] == tp[0] || (*src)[index] == tp[1]){
        (*src) ++;
        len --;
        index ++;
        if((*src)[index] == tp[0] || (*src)[index] == tp[1]){
            (*src) ++;
            len --;
        }
    }
    index = len - 1;
    if((*src)[index] == tp[0] || (*src)[index] == tp[1]){
        len --;
        index --;
        if((*src)[index] == tp[0] || (*src)[index] == tp[1]){
            len --;
        }
    }
    if(len < 0){
        printf("may be error the len is a negative\n");
    }
    return 0;
}

int subtitle_srt::append_str(char* data,int len)
{
    int data_size = get_text_data_size();
    int buf_size = get_text_buf_size();
    if(text_buf_ptr + len > text_buf_end){
        printf("reallocate the text_buf\n");
        buf_size += SRT_DATA_POOL_SIZE;
        text_buf = (char*)realloc(text_buf,buf_size);
        text_buf_ptr = text_buf + data_size;
        text_buf_end = text_buf + buf_size;
    }
    if(len != NULL){
        string_trim(&data,len);
        memcpy(text_buf_ptr,data,len);
    }
    text_buf_ptr += len;
    *(text_buf_ptr++) = '\0';
    return 0;
}

int subtitle_srt::get_next_line_off(const char* src,int len)
{
    int first_pos = find_first_char(src,get_read_buf_remain(),0x0D );

    return first_pos + 2;
}

int subtitle_srt::find_next_str_entry(const char* src,int len)
{
    const char* start_p = src;
    int res  = -1;
    int hs, ms, ss, he, me, se;
    unsigned int x1 = 0;
    unsigned int x2 = 0;
    unsigned int y1 = 0;
    unsigned int y2 = 0;
    int _start,_stop;
    int distance = 0;
    while(len > 0){
        int c = sscanf(src, "%d:%2d:%2d%*1[,.]%3d --> %d:%2d:%2d%*1[,.]%3d"
            "%*[ ]X1:%u X2:%u Y1:%u Y2:%u",
            &hs, &ms, &ss, &_start, &he, &me, &se, &_stop,
            x1, x2, y1, y2);
        int next_line_off = get_next_line_off(src,len);
        if(c >= 8 || next_line_off == 2){
            res = src - start_p;
            break;
        }
        src += next_line_off;
        len -= (src - start_p);
    }
    return res;
}

int subtitle_srt::read_srt_data()
{    
    int start_time = 0;
    int stop_time = 0;
    int x1 = 0;
    int x2 = 0;
    int y1 = 0;
    int y2 = 0;
    SRT_ENTRY e = {0};
    int hs, ms, ss, he, me, se;
    int style_len = 0;
    
    while(read_buf_ptr < read_buf_end){
        
        int c = sscanf(read_buf_ptr, "%d:%2d:%2d%*1[,.]%3d --> %d:%2d:%2d%*1[,.]%3d"
            "%*[ ]X1:%u X2:%u Y1:%u Y2:%u",
            &hs, &ms, &ss, &start_time, &he, &me, &se, &stop_time,
            x1, x2, y1, y2);
        
        read_buf_ptr += get_next_line_off(read_buf_ptr,get_read_buf_remain());

        if (c >= 8) {
            memset(&e,0,sizeof(SRT_ENTRY));
            e.start_time = 1000*(ss + 60*(ms + 60*hs)) + start_time;
            e.stop_time  = 1000*(se + 60*(me + 60*he)) + stop_time ;
           
            int text_len = find_next_str_entry(read_buf_ptr,get_read_buf_remain());
            e.off_pos = get_text_data_size();
            //e.style = analysis_text(read_buf_ptr,style_len);
            
            
            append_str(read_buf_ptr + style_len,text_len - style_len);
            
            read_buf_ptr += text_len + 2;
            m_srt[e.start_time] = e;
        }
    }

    SAFE_FREE(read_buf);

    //sort_subtitle();

    FILE* fp = fopen("srt_info.txt","wb");
    for(int i =0 ;i <m_srt.size();i++){
        fprintf(fp,"stat_time=%lld stop_time=%lld data:%s\n",
            m_srt[i].start_time,m_srt[i].stop_time,text_buf + m_srt[i].off_pos);
        printf("data:%s\n",text_buf + m_srt[i].off_pos);
    }
    fclose(fp);

    return 0;
}

int subtitle_srt::fill_buffer()
{
    return 0;
}

int subtitle_srt::get_next_data(char*& buffer,int64_t& start_time,int64_t& stop_time)
{
    buffer          = text_buf + m_srt[m_index].off_pos;
    start_time      = m_srt[m_index].start_time;
    stop_time       = m_srt[m_index].stop_time;
    return 0;
}

int subtitle_srt::find_best_data(int64_t wanted_time,char*& buffer,
                                 int64_t& start_time,int64_t& stop_time)
{
    int a = -1;
    int b = m_srt.size();

    if(m_srt[b - 1].start_time < wanted_time){
        a = b - 1;
    }

    if(m_srt[0].start_time >= wanted_time){
        buffer = text_buf + m_srt[0].off_pos;
        start_time = m_srt[0].start_time;
        stop_time = m_srt[0].stop_time;
        return 0;
    }

    int m = 0;
    while(b - a > 1){
        m = (a + b) >> 1;
        if(wanted_time >= m_srt[m].start_time)
            a = m;
        if(wanted_time <=  m_srt[m].start_time)
            b = m;
    }

    a = MAX(0 , a);
    buffer = text_buf + m_srt[a].off_pos;
    start_time = m_srt[a].start_time;
    stop_time  = m_srt[a].stop_time;

    return 0;
}

int subtitle_srt::sort_subtitle()
{
    int     start = 0;
    int     stop  = m_srt.size() - 1;

    //randomize_quicksort(m_srt,start,stop);
    

    return 0;
}