#ifndef __BIT_STREAM_DEF__
#define __BIT_STREAM_DEF__

#ifndef AV_WB32
#   define AV_WB32(p, d) do {                   \
    ((uint8_t*)(p))[3] = (d);               \
    ((uint8_t*)(p))[2] = (d)>>8;            \
    ((uint8_t*)(p))[1] = (d)>>16;           \
    ((uint8_t*)(p))[0] = (d)>>24;           \
    } while(0)
#endif

#ifndef av_bswap32
const uint32_t av_bswap32(uint32_t x)
{
    x= ((x<<8)&0xFF00FF00) | ((x>>8)&0x00FF00FF);
    x= (x>>16) | (x<<16);
    return x;
}
#endif
#define av_be2ne32(x) av_bswap32(x)

#ifndef AV_RB32
#   define AV_RB32(x)                           \
    ((((const uint8_t*)(x))[0] << 24) |         \
    (((const uint8_t*)(x))[1] << 16) |         \
    (((const uint8_t*)(x))[2] <<  8) |         \
    ((const uint8_t*)(x))[3])
#endif

#ifndef NEG_USR32
#   define NEG_USR32(a,s) (((uint32_t)(a))>>(32-(s)))
#endif
#ifndef NEG_SSR32
#   define NEG_SSR32(a,s) ((( int32_t)(a))>>(32-(s)))
#endif

#ifndef sign_extend
static const int sign_extend(int val, unsigned bits)
{
    return (val << ((8 * sizeof(int)) - bits)) >> ((8 * sizeof(int)) - bits);
}
#endif

#endif