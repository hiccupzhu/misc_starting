#include <stdio.h>
#include <fstream>
#include <string>
#include <metype.h>

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
public:
    static const int   offset = 0x8140;

private: 
    uint8_t*    buf;
    uint8_t*    buf_ptr;
    uint8_t*    buf_end;  
};


int main(int argc,char* argv[])
{
    gbk_unicode gbk;
    uint32_t gbkcode = 0;
    uint32_t unicode = 0;
    GBK_UNICODE     e = {0};

    std::string line;
    std::ifstream in("f:\\gbk_unicode.txt");
    if(in){
        while(getline(in, line)){
            int c = sscanf(line.c_str(),"%04X\t%04X",&gbkcode,&unicode);
            if(c >= 2){
                //e.gbkcode = (uint16_t)gbkcode;
                e.unicode = (uint16_t)unicode;
                gbk[gbkcode - gbk_unicode.offset] = e;
            }
        }
    }
    in.close();
    
    FILE* fp = fopen("f:\\gbk_unicode.lib","wb");
    
    fwrite(gbk.get_buf(),gbk.get_data_size(),1,fp);
    
    fclose(fp);

    return 0;
}