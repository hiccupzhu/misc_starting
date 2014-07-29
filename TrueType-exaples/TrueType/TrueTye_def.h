#ifndef __H_TRUETYPE_DEF__
#define __H_TRUETYPE_DEF__
#include <malloc.h>
#include <memory.h>

#define SAFE_FREE(x)    if((x)){ \
                            free((x));\
                            (x) = 0;\
                        }
#define SAFE_DELETE(x)  if((x)){\
                            delete (x);\
                            (x) = 0;\
                        }
#define array_size(x)   (sizeof((x)) / sizeof((x)[0]))


#endif