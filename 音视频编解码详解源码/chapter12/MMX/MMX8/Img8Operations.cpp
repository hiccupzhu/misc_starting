#include "StdAfx.h"
#include ".\img8operations.h"

CImg8Operations::CImg8Operations(void)
{
}

CImg8Operations::~CImg8Operations(void)
{
}


#pragma warning(disable : 4799)

// Helper function to convert __int64 to __m64
// Function is supposed to be called after _mm_empty(), as part of MMX code
__m64 CImg8Operations::Get_m64(__int64 n)
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
void CImg8Operations::InvertImageCPlusPlus(
    BYTE* pSource, 
    BYTE* pDest, 
    int nNumberOfPixels)
{
    for ( int i = 0; i < nNumberOfPixels; i++ )
    {
        *pDest++ = 255 - *pSource++;
    }
}

// Invert image using C++ with MMX
void CImg8Operations::InvertImageC_MMX(
    BYTE* pSource, 
    BYTE* pDest, 
    int nNumberOfPixels)
{
    __int64 i = 0;
    i = ~i;                                 // 0xffffffffffffffff    

    // 8 pixels are processed in one loop
    int nLoop = nNumberOfPixels/8;

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

        pIn++;                              // next 8 pixels
        pOut++;
    }

    _mm_empty();                            // emms
}

// Invert image using Assembly with MMX
void CImg8Operations::InvertImageAssembly_MMX(
    BYTE* pSource, 
    BYTE* pDest, 
    int nNumberOfPixels)
{
    __int64 i = 0;
    i = ~i;                                 // 0xffffffffffffffff    

    // 8 pixels are processed in one loop
    int nNumberOfLoops = nNumberOfPixels / 8;

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
                                            
        movq        [edi], mm0              // [edi] = mm0 (8 pixels)

        add         esi, 8                  // increment input pointer (next 64 bits)
        add         edi, 8                  // increment output pointer (next 64 bits)

        dec         ecx
        jnz         start_loop

        emms
    }
}

// Change brightness using C++
void CImg8Operations::ChangeBrightnessCPlusPlus(
    BYTE* pSource, 
    BYTE* pDest, 
    int nNumberOfPixels, 
    int nChange)
{
    if ( nChange > 255 )
        nChange = 255;
    else if ( nChange < -255 )
        nChange = -255;

    BYTE b = (BYTE) abs(nChange);

    int i, n;

    if ( nChange > 0 )
    {
        for ( i = 0; i < nNumberOfPixels; i++ )
        {
            n = (int)(*pSource++ + b);

            if ( n > 255 )
                n = 255;

            *pDest++ = (BYTE) n;
        }
    }
    else
    {
        for ( i = 0; i < nNumberOfPixels; i++ )
        {
            n = (int)(*pSource++ - b);

            if ( n < 0 )
                n = 0;
            *pDest++ = (BYTE) n;
        }
    }
}

// Change brightness using C++ with MMX
void CImg8Operations::ChangeBrightnessC_MMX(
    BYTE* pSource, 
    BYTE* pDest, 
    int nNumberOfPixels, 
    int nChange)
{
	 int i;
    if ( nChange > 255 )
        nChange = 255;
    else if ( nChange < -255 )
        nChange = -255;

    BYTE b = (BYTE) abs(nChange);

    // make 64 bits value with b in each byte
    __int64 c = b;

    for ( i = 1; i <= 7; i++ )
    {
        c = c << 8;
        c |= b;
    }

    // 8 pixels are processed in one loop
    int nNumberOfLoops = nNumberOfPixels / 8;

    __m64* pIn = (__m64*) pSource;          // input pointer
    __m64* pOut = (__m64*) pDest;           // output pointer

    __m64 tmp;                              // work variable


    _mm_empty();                            // emms

    __m64 nChange64 = Get_m64(c);

    if ( nChange > 0 )
    {
        for ( i = 0; i < nNumberOfLoops; i++ )
        {
            tmp = _mm_adds_pu8(*pIn, nChange64);       // Unsigned addition with saturation.
                                                       // tmp = *pIn + nChange64 for each byte

            *pOut = tmp;

            pIn++;                                      // next 8 pixels
            pOut++;
        }
    }
    else
    {
        for ( i = 0; i < nNumberOfLoops; i++ )
        {
            tmp = _mm_subs_pu8(*pIn, nChange64);       // Unsigned subtraction with saturation.
                                                       // tmp = *pIn - nChange64for each byte

            *pOut = tmp;

            pIn++;                                      // next 8 pixels
            pOut++;
        }
    }

    _mm_empty();                            // emms
}

// Change brightness using Assembly with MMX
void CImg8Operations::ChangeBrightnessAssembly_MMX(
    BYTE* pSource, 
    BYTE* pDest, 
    int nNumberOfPixels, 
    int nChange)
{
    // Convert abs(nChange) to BYTE
    BYTE b = (BYTE) abs(nChange);

    if ( b < 0 )
        b = 0;
    else if ( b > 255 )
        b = 255;

    // make 64 bits value with b in each byte
    __int64 nChange64 = b;

    for ( int i = 1; i <= 7; i++ )
    {
        nChange64 = nChange64 << 8;
        nChange64 |= b;
    }

    // 8 pixels are processed in one loop
    int nNumberOfLoops = nNumberOfPixels / 8;

    if ( nChange > 0 )
    {
        __asm
        {
            emms

            mov         esi, pSource            // input pointer
            mov         edi, pDest              // output pointer
            mov         ecx, nNumberOfLoops     // loop counter

            movq        mm1, nChange64          // mm1 = nChange64

start_loop:
            movq        mm0, [esi]              // mm0 = [esi]

            paddusb     mm0, mm1                // Unsigned addition with saturation.
                                                // mm0 = mm0 + mm1 for each byte

            movq        [edi], mm0              // [edi] = mm0 (8 pixels)

            add         esi, 8                  // increment input pointer (next 64 bits)
            add         edi, 8                  // increment output pointer (next 64 bits)

            dec         ecx
            jnz         start_loop

            emms
        }
    }
    else
    {
        __asm
        {
            emms

            mov         esi, pSource            // input pointer
            mov         edi, pDest              // output pointer
            mov         ecx, nNumberOfLoops     // loop counter

            movq        mm1, nChange64          // mm1 = nChange64

start_loop1:
            movq        mm0, [esi]              // mm0 = [esi]

            psubusb     mm0, mm1                // Unsigned subtraction with saturation.
                                                // mm0 = mm0 - mm1 for each byte

            movq        [edi], mm0              // [edi] = mm0 (8 pixels)

            add         esi, 8                  // increment input pointer (next 64 bits)
            add         edi, 8                  // increment output pointer (next 64 bits)

            dec         ecx
            jnz         start_loop1

            emms
        }
    }
}
