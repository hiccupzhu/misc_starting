#include "StdAfx.h"
#include ".\img32operations.h"

CImg32Operations::CImg32Operations(void)
{
}

CImg32Operations::~CImg32Operations(void)
{
}

#pragma warning(disable : 4799)

// Helper function to convert __int64 to __m64
// Function is supposed to be called after _mm_empty(), as part of MMX code
__m64 CImg32Operations::Get_m64(__int64 n)
{
    union __m64__m64
    {
        __m64 m;
        __int64 i;
    } mi;

    mi.i = n;
    return mi.m;
}

#pragma warning(default : 4799)


// Invert image using C++
void CImg32Operations::InvertImageCPlusPlus(
    BYTE* pSource, 
    BYTE* pDest, 
    int nNumberOfPixels)
{
    for ( int i = 0; i < nNumberOfPixels; i++ )
    {
        *pDest++ = 255 - *pSource++;
        *pDest++ = 255 - *pSource++;
        *pDest++ = 255 - *pSource++;

        pDest++;
        pSource++;
    }
}

// Invert image using C++ with MMX
void CImg32Operations::InvertImageC_MMX(
    BYTE* pSource, 
    BYTE* pDest, 
    int nNumberOfPixels)
{
    __int64 i = 0;
    i = ~i;                                 // 0xffffffffffffffff    

    // Two pixels are processed in one loop
    int nLoop = nNumberOfPixels/2;

    __m64* pIn = (__m64*) pSource;          // input pointer
    __m64* pOut = (__m64*) pDest;           // output pointer

    __m64 tmp;                              // work variable

    _mm_empty();                            // emms

    __m64 n1 = Get_m64(i);

    for ( int i = 0; i < nLoop; i++ )
    {
        tmp = _mm_subs_pu8 (n1 , *pIn);     // Unsigned subtraction with saturation.
                                            // tmp = n1 - *pIn  for each byte

        *pOut = tmp;

        pIn++;                              // next two pixels
        pOut++;
    }

    _mm_empty();                            // emms
}

// Invert image using Assembly with MMX
void CImg32Operations::InvertImageAssembly_MMX(
    BYTE* pSource, 
    BYTE* pDest, 
    int nNumberOfPixels)
{
    __int64 i = 0;
    i = ~i;                                 // 0xffffffffffffffff    

    // Two pixels are processed in one loop
    int nNumberOfLoops = nNumberOfPixels / 2;

    __asm
    {
        emms

        mov         esi, pSource            // input pointer
        mov         edi, pDest              // output pointer
        mov         ecx, nNumberOfLoops     // loop counter

start_loop:
        movq        mm0, i                  // mm0 = 0xffffffffffffffff 

        psubusb     mm0, [esi]              // Unsigned subtraction with saturation.
                                            // mm0 = mm0 - [esi] for each byte
                                            
        movq        [edi], mm0              // [edi] = mm0 (two pixels)

        add         esi, 8                  // increment input pointer (next 64 bits)
        add         edi, 8                  // increment output pointer (next 64 bits)

        dec         ecx
        jnz         start_loop

        emms
    }
}

// Convert 32-bits image colors scale using C++
void CImg32Operations::ColorsCPlusPlus(
    BYTE* pSource, 
    BYTE* pDest, 
    int nNumberOfPixels, 
    float fRedCoefficient, 
    float fGreenCoefficient, 
    float fBlueCoefficient)
{
    int nRed = (int)(fRedCoefficient * 256.0f);
    int nGreen = (int)(fGreenCoefficient * 256.0f);
    int nBlue = (int)(fBlueCoefficient * 256.0f);


    for ( int i = 0; i < nNumberOfPixels; i++ )
    {
        *pDest++ = (BYTE) ( ((int)(*pSource++ * nBlue)) >> 8 );
        *pDest++ = (BYTE) ( ((int)(*pSource++ * nGreen)) >> 8 );
        *pDest++ = (BYTE) ( ((int)(*pSource++ * nRed)) >> 8 );

        pDest++;
        pSource++;
    }
}

// Convert 32-bits image colors using C++ with MMX.
// Performs with the same speed like C++ code.
// In the real program I would not use this way to change the
// color balance - C++ code without MMX is simple and performs well. 
// However, SSE2 may give the better results.
void CImg32Operations::ColorsC_MMX(
    BYTE* pSource, 
    BYTE* pDest, 
    int nNumberOfPixels, 
    float fRedCoefficient, 
    float fGreenCoefficient, 
    float fBlueCoefficient)
{
    int nRed = (int)(fRedCoefficient * 256.0f);
    int nGreen = (int)(fGreenCoefficient * 256.0f);
    int nBlue = (int)(fBlueCoefficient * 256.0f);

    // make miltiplication coefficient
    __int64 c = 0;
    c = nRed;
    c = c << 16;
    c |= nGreen;
    c = c << 16;
    c |= nBlue;

    __m64 nNull = _m_from_int(0);           // null
    __m64 tmp = _m_from_int(0);             // work variable

    _mm_empty();                            // emms

    __m64 nCoeff = Get_m64(c);

    DWORD* pIn = (DWORD*) pSource;          // input pointer
    DWORD* pOut = (DWORD*) pDest;           // output pointer


    for ( int i = 0; i < nNumberOfPixels; i++ )
    {
#if 1
        tmp = _m_from_int(*pIn);                // tmp = *pIn (write to low 32 bits)

        tmp = _mm_unpacklo_pi8(tmp, nNull );    // convert low 4 bytes of tmp to 4 words
                                                // high byte for each word is takem from nNull

        tmp =  _mm_mullo_pi16 (tmp , nCoeff);   // multiply each word in tmp to word in nCoeff
                                                // get low word of each result

        tmp = _mm_srli_pi16 (tmp , 8);          // shift each word in tmp right to 8 bits (/256)

        tmp = _mm_packs_pu16 (tmp, nNull);      // Pack with unsigned saturation.
                                                // Convert 4 words from tmp to 4 bytes and write them
                                                // to low 32 bits of tmp.
                                                // Convert 4 words from nNull to 4 bytes and write them
                                                // to high 32 bits of tmp.

        *pOut = _m_to_int(tmp);                 // *pOut = tmp (low 32 bits)
#else
        // The same code, but hardly understandable.
        // Performs exactly as previous version in the Release configuration.
        *pOut = _m_to_int( 
            _mm_packs_pu16 (
            _mm_srli_pi16 (
            _mm_mullo_pi16 (
            _mm_unpacklo_pi8(
            _m_from_int(*pIn), 
            nNull ), nCoeff) , 8), nNull) );
#endif

        pIn++;
        pOut++;

    }

    _mm_empty();                          // emms
}

// Convert 32-bits image colors using Assembly with MMX
// (5-10% faster than C++ code)
void CImg32Operations::ColorsAssembly_MMX(
    BYTE* pSource, 
    BYTE* pDest, 
    int nNumberOfPixels, 
    float fRedCoefficient, 
    float fGreenCoefficient, 
    float fBlueCoefficient)
{
    int nRed = (int)(fRedCoefficient * 256.0f);
    int nGreen = (int)(fGreenCoefficient * 256.0f);
    int nBlue = (int)(fBlueCoefficient * 256.0f);

    __int64 nCoeff = 0;
    __int64 nNull = 0;

    nCoeff = nRed;
    nCoeff = nCoeff << 16;
    nCoeff |= nGreen;
    nCoeff = nCoeff << 16;
    nCoeff |= nBlue;

    __asm
    {
        emms

        mov         esi, pSource            // input pointer
        mov         edi, pDest              // output pointer
        mov         ecx, nNumberOfPixels    // loop counter

        // movq instruction copies quadword from the source operand (second operand) 
        // to the destination operand (first operand)
        movq        mm1, nNull              // null
        movq        mm2, nCoeff             // multiplication coefficient


start_loop:
        movd        mm0, [esi]          // load pixel RGBW to low 32 bits of MMX register

        punpcklbw   mm0, mm1            // convert 4 bytes in low 32 bits of mm0
                                        // to 4 words
                                        // high byte for each word is taken from mm1 (0)

        pmullw      mm0, mm2            // multiply each word in mm0 to word from mm2
                                        // and place low word of result to mm0

        psrlw       mm0, 8               // shift each word in mm0 right to 8 bits (/256)

        packuswb    mm0, mm1            // Pack with unsigned saturation.
                                        // Convert 4 words from mm0 to 4 bytes and write them
                                        // to low 32 bits of mm0
                                        // (high 32 bits of mm0 are filled by the same way
                                        // from mm1 - doesn't matter)

        movd        [edi], mm0          // write pixel from low 32 bits of mm0
                                        // to [edi]


        add         esi, 4             // increment input pointer
        add         edi, 4             // increment output pointer

        dec         ecx
        jnz         start_loop

        emms
    }
}

