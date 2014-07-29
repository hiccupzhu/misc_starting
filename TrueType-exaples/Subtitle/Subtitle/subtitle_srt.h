#ifndef __H_SUBTITLE_SRT__
#define __H_SUBTITLE_SRT__
#include <vector>
#include "subtitle_def.h"

template <typename T>
class mvect{
    typedef struct TENTRY{
        int64_t     key;
        T           entry;
        TENTRY*     prev;
    };
#define MVECT_INIT_ALLOC_SIZE   0X100000
public:
    mvect(){
        buf     = NULL;
        buf_ptr = NULL;
        buf_end = NULL;
        
    }
    ~mvect(){
        clear();
    }
    
public:
    void clear(){
        SAFE_FREE(buf);
        buf_ptr     = NULL;
        buf_end     = NULL;
    }
    
    int size() const{
        int data_size = get_data_size();
        return (data_size / sizeof(TENTRY));
    }
    
    int get_data_size() const{
        return buf_ptr - buf;
    }
    
    int get_buf_size() const{
        return buf_end - buf;
    }
    
    int search_key(int64_t wanted_key){
        if(buf == NULL) return -1;
        int res = -1;
        int a = -1;
        int b = -1;
        
        int data_size = 0;
        int buf_size  = 0;
        
        b = size();
        
        if(b && ((TENTRY*)buf)[b - 1].key < wanted_key){
            a = b - 1;
        }
        
        int         m = 0;
        int64_t     key_value = 0;
        while(b - a > 1){
            m = a + b >> 1;
            key_value = ((TENTRY*)buf)[m].key;
            if(wanted_key <= key_value)
                b = m;
            if(wanted_key >= key_value)
                a = m;
        }
        
        res = b;
        
        return res ;
    }
    
    T& operator [](int64_t key){
        TENTRY*  pe = NULL;
        int data_size = get_data_size();
        int buf_size  = get_buf_size();
        
        if(buf_ptr + sizeof(TENTRY) > buf_end){
            buf_size    += MVECT_INIT_ALLOC_SIZE;
            buf = (uint8_t*)realloc(buf, buf_size);
            buf_ptr = buf + data_size;
            buf_end = buf + buf_size;
        }
        
        
        int pos = search_key(key);
        if( pos > -1){
            pe = &((TENTRY*)buf)[pos];
            
            memmove((char*)pe + sizeof(TENTRY),pe,(size() - pos) * sizeof(TENTRY));
            pe->key = key;
            
            buf_ptr += sizeof(TENTRY);
            return pe->entry;
        }else{
            ((TENTRY*)buf_ptr)->key = key;
            T* t = &((TENTRY*)buf_ptr)->entry;
            buf_ptr += sizeof(TENTRY);
            return *t;
        }
    }
    
    T& operator [](int index){
        return ((TENTRY*)buf)[index].entry;
    }
    
    void operator =(TENTRY& e){
        TENTRY* pe  = (TENTRY*)buf_ptr;
        pe->key     = e.key;
        pe->entry   = e.entry;
        buf_ptr    += sizeof(TENTRY);
    }
    
private:
    uint8_t*    buf;
    uint8_t*    buf_ptr;
    uint8_t*    buf_end;
};


class subtitle_srt
{
    typedef struct{
        int64_t     start_time;
        int64_t     stop_time;
        int         off_pos;
        int         style;
    }SRT_ENTRY;

public:
    subtitle_srt(void);
    ~subtitle_srt(void);

public:
    int     detect(uint8_t* filename);
    int     load(char* filename);

    int     read_srt_data();
    int     fill_buffer();
    int     get_read_buf_size()     const;
    int     get_read_buf_remain()   const;
    int     get_text_data_size()    const;
    int     get_text_buf_size()     const;

    int     find_first_char(const char* src,int len,char ch);
    int     find_next_str_entry(const char* src,int len);
    int     get_next_line_off(const char* src,int len);
    int     analysis_text(char* src,int& style_len);

    int     string_trim(char** src,int& len);
    int     append_str(char* data,int len);

    int     sort_subtitle();

    int     get_next_data(char*& buffer,int64_t& start_time,int64_t& stop_time);
    int     find_best_data(int64_t wanted_time,char*& buffer,int64_t& start_time,int64_t& stop_time);
    

public:
    mvect<SRT_ENTRY> m_srt; 


private:
    FILE*       m_fp;
    int         file_size;

    char*    read_buf;
    char*    read_buf_ptr;
    char*    read_buf_end;

    char*    text_buf;
    char*    text_buf_ptr;
    char*    text_buf_end;

private:
    int      m_index;
};

#endif