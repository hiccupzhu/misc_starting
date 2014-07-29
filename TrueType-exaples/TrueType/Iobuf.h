#ifndef __H_CIOBUF__
#define __H_CIOBUF__
#include <stdint.h>
#include "TrueTye_def.h"

class CIobuf
{
public:
    CIobuf(void);
    ~CIobuf(void);
    
public:
    int init(void* fp);
    int get_data_size() const;
    int get_buffer_size() const;
    
    uint8_t     get_byte();
    uint16_t    get_be16();
    uint32_t    get_be24();
    uint32_t    get_be32();
    uint64_t    get_be64();
    
    int         fill_buffer();
    
    
private:
   uint8_t*     m_buf;
   uint8_t*     m_buf_ptr;
   uint8_t*     m_buf_end;
   
   FILE*        m_fp;
};


#endif