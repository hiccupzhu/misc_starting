#include <stdio.h>
#include <fstream>
#include <string>
#include <metype.h>
#include "Iobuf.h"

typedef struct {
    //unsigned short gbkcode;
    unsigned short unicode;
}GBK_UNICODE;

class gbk_unicode{
#define ALLOC_SIZE      0x10000
public:
    gbk_unicode():buf(NULL),
        buf_ptr(NULL),
        buf_end(NULL){
    }
    ~gbk_unicode(){
        SAFE_FREE(buf);
    }

public:
    int append_end(GBK_UNICODE e){
        int entry_size = sizeof(GBK_UNICODE);
        int data_size = get_data_size();
        int buf_size  = get_buf_size();
        if(buf_ptr + entry_size > buf_end){
            buf_size += ALLOC_SIZE;
            buf = (uint8_t*)realloc(buf,buf_size);
            buf_ptr = buf + data_size;
            buf_end = buf + buf_size ;
        }
        memcpy(buf_ptr,&e,sizeof(GBK_UNICODE));
        buf_ptr += sizeof(GBK_UNICODE);
        return 0;
    }

    GBK_UNICODE get_data(int index) const{
        return ((GBK_UNICODE*)buf)[index];
    }

    int get_buf_size() const{
        return buf_end - buf;
    }

    int get_data_size() const{
        return buf_ptr - buf;
    }

    int get_entries_counts() const{
        //return (buf_ptr - buf) / sizeof(GBK_UNICODE) ;
        return (buf_ptr - buf) >> 2; 
    }

    GBK_UNICODE* get_buf() const{
        return (GBK_UNICODE*)buf;
    }

    GBK_UNICODE& operator[](int index){
        int buf_size = get_buf_size();
        int data_size = get_data_size();

        int delta_size = (index + 1) * sizeof(GBK_UNICODE) - data_size;
        if(buf_ptr + delta_size > buf_end){
            if(delta_size <= ALLOC_SIZE){
                delta_size = ALLOC_SIZE;
            }
            buf_size += delta_size;
            buf = (uint8_t*)realloc(buf,buf_size);
            buf_ptr = buf + data_size;
            buf_end = buf + buf_size;
        }
        //buf_ptr = MMAX(buf_ptr, buf + (index + 1) * sizeof(GBK_UNICODE));
        buf_ptr = MMAX(buf_ptr, buf + ((index + 1) << 1));  //index + 1 is now position
        return ((GBK_UNICODE*)buf)[index];
    }
    
    int load_lib(char* filename){
        FILE* fp = fopen(filename,"rb");
        fseek(fp,0,SEEK_END);
        int filesize = ftell(fp);
        fseek(fp,0,SEEK_SET);
        
        SAFE_FREE(buf);
        buf = (uint8_t*)malloc(filesize);
        fread(buf,filesize,1,fp);
        
        fclose(fp);
        fp = NULL;
        
        buf_ptr = buf + filesize;
        
        buf_end = buf + filesize;
        
        
        
        
        return  0;
    }
public:
    static const int   offset = 0x8140;

private: 
    uint8_t*    buf;
    uint8_t*    buf_ptr;
    uint8_t*    buf_end;  
};

int unicode2utf8(uint16_t unicode,uint8_t* putf8,int& utf8_len)
{
    utf8_len = 0;
    if(unicode >= 0 && unicode <= 0x7F){
        utf8_len = 1;
    }else if(unicode <= 0x7FF){
        utf8_len = 2;
    }else if(unicode <= 0xFFFF){
        utf8_len = 3;
    }else if(unicode <= 0x1FFFFF){
        utf8_len = 4;
    }else if(unicode <= 0x3FFFFFF){
        utf8_len = 5;
    }else if(unicode <= 0x7FFFFFF){
        utf8_len = 6;
    }
    /*
    bytes   contains
    1       0xxxxxxx (0x00-0x7F) 
    2       110xxxxx 10xxxxxx (0xC2-0xDF)(0x80-0xBF)
    3       1110xxxx 10xxxxxx 10xxxxxx 
    4       11110xxx 10xxxxxx 10xxxxxx 10xxxxxx 
    5       111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 
    6       1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
    static const uint8_t dic[][6] = {
        { 0x0},
        { 0xC0, 0x80},
        { 0xE0, 0x80, 0x80},
        { 0xF0, 0x80, 0x80, 0x80},
        { 0xF8, 0x80, 0x80, 0x80, 0x80},
        { 0xFC, 0x80, 0x80, 0x80, 0x80, 0x80}
    };

    memcpy(putf8,dic[utf8_len - 1],utf8_len);
    int i = 0;
    for(i = 0; i < utf8_len - 1 ; i++){
        uint8_t bits6 = ((unicode >> (i * 6)) & 0x3F);
        putf8[utf8_len - 1 - i] |= bits6;
    }

    uint8_t bits_remain = unicode >> (i * 6);
    putf8[0] |= bits_remain;


    return 0;
} 


int main(int argc,char* argv[])
{
    gbk_unicode gbk;
    gbk.load_lib("f:\\gbk_unicode.lib");
    
    FILE* fp  = fopen("f:\\gbktest","rb");
    CIobuf iobuf;
    iobuf.init(fp);
    FILE* fpu = fopen("f:\\gbktest_test_utf8.txt","wb");
    
    uint8_t     utf8_buf[6] = {0};
    uint8_t     utf8_header[] = {0xEF, 0xBB, 0xBF};
    int         utf8_len = 0;
    fwrite(utf8_header,3,1,fpu);
    
    while(iobuf.is_buf_eof() == NULL){
        uint8_t ch = iobuf.get_byte();
        if(iobuf.is_buf_eof())  break;
        if(ch & 0x80){ 
            uint16_t gbcode = ch;
            gbcode = gbcode << 8 | iobuf.get_byte();
            uint16_t unicode16 = gbk[gbcode - gbk_unicode.offset].unicode;
            
            unicode2utf8(unicode16,utf8_buf,utf8_len);            
            fwrite(utf8_buf,1,utf8_len,fpu);
        }else{
            fwrite(&ch,1,1,fpu);
        }
    }
    
    fclose(fpu);
    fclose(fp);
    return 0;
}