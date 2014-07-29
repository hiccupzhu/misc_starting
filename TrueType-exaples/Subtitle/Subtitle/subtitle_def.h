#ifndef __H_SUBTITLE_DEF__
#define __H_SUBTITLE_DEF__
#include <stdint.h>
#include <memory.h>
#include <malloc.h>

typedef int32_t H_RESULT;
#define SUCCEEDED(hr)	    (((H_RESULT)hr) >= 0)
#define FAILED(hr)		    (((H_RESULT)hr) < 0)
#define FAILED_LEVEL(hr)    (H_RESULT(hr) & 0x7f000000)
#define FAILED_ID(hr)       (H_RESULT(hr) & 0x00ffffff)

#define SAFE_FREE(x) if((x)){\
    free((x));\
    (x) = NULL;\
                     }
#define S_OK                    0
#define SRT_ERR_OPEN_FILE       0x80000001
#define SRT_EER_ONMEM           0x80000002

#define MAX(x,y)    ((x) > (y) ? (x) : (y))
#define MIN(x,y)    ((x) < (y) ? (x) : (y))

class string_buf{
public:
    string_buf(){
        text_buf = NULL;
        text_buf_ptr = NULL;
        text_buf_end = NULL;

        int res = set_buf_size(0x10000);
        if(FAILED(res)) return;
    }
    virtual ~string_buf(){
        SAFE_FREE(text_buf);
    }

public:
    int append(char* data,int len){
        return 0;
    }

    int set_buf_size(int size){
        SAFE_FREE(text_buf);
        text_buf = (char*)malloc(size);
        if(text_buf == NULL)    return SRT_EER_ONMEM;
        text_buf_ptr = text_buf;
        text_buf_end = text_buf + size;
    }

protected:
    char*   text_buf;
    char*   text_buf_ptr;
    char*   text_buf_end;
};

#endif