#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include "Iobuf.h"
#define IO_SIZE  0x10000

CIobuf::CIobuf(void):
m_buf(NULL),m_buf_ptr(NULL),m_buf_end(NULL)
{
    m_buf_eof = 0;
    m_file_eof = 0;
}

CIobuf::~CIobuf(void)
{
    SAFE_FREE(m_buf);
}

int CIobuf::init(void* fp)
{
    if(fp == NULL)  return -1;
    m_fp = (FILE*)fp;
    m_buf = (uint8_t*)malloc(IO_SIZE);
    if(m_buf == NULL)   return -1;
    //m_buf_ptr = m_buf;
    m_buf_end = m_buf + IO_SIZE;
    m_buf_ptr = m_buf_end;
    
    return 0;
}

int CIobuf::get_data_size() const
{
    return m_buf_ptr - m_buf;
}

int CIobuf::get_buffer_size() const
{
    return m_buf_end - m_buf;
}

uint8_t CIobuf::get_byte()
{
    if(m_buf_ptr >= m_buf_end){
        fill_buffer();
    }
    if(m_buf_ptr < m_buf_end)
        return *m_buf_ptr ++;
        
    m_buf_eof = 1; 
    return 0;
}

uint16_t CIobuf::get_be16()
{
    uint16_t val = 0;
    val  = get_byte() << 8;
    val |= get_byte();
    return val;
}

uint32_t CIobuf::get_be24()
{
    uint32_t val = 0;
    val  = get_be16() << 8;
    val |= get_byte();
    return val;
}

uint32_t CIobuf::get_be32()
{
    uint32_t val = 0;
    val = get_be16() << 16;
    val |= get_be16();
    return val;
}

uint64_t CIobuf::get_be64()
{
    uint64_t    val = 0;
    val = get_be32() << 32;
    val |= get_be32();
    return val;
}

int CIobuf::fill_buffer()
{
    if(m_file_eof == 0){
        int buf_size = get_buffer_size();
        int real_size = fread(m_buf,1,buf_size,m_fp);
        if(real_size != buf_size){  
            m_buf_end = m_buf + real_size; 
            m_file_eof = 1;
        }
        m_buf_ptr = m_buf;
    }
    return 0;
}

int CIobuf::is_buf_eof(){
    return m_buf_eof;
}

int CIobuf::is_file_eof(){
    return m_file_eof;
}