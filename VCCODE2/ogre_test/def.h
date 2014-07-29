#ifndef __DEF_H__
#define __DEF_H__

#define CHECK(x)    do{if((int)(x) < 0){ return (x);}}while(0)

#define SAFE_DELETE(x) do{if((x)){\
    delete (x);\
    (x) = NULL;}\
}while(0)



#endif