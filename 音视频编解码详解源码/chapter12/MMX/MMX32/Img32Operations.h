#pragma once


#include <emmintrin.h>		// MMX, SSE, SSE2 intrinsic support


// Image processing operations with 32 bits per pixel RGB image
//
// For each image processing operation there are 3 functions:
// C++, C++ with MMX, Assembly with MMX
class CImg32Operations
{
public:
    CImg32Operations(void);
    ~CImg32Operations(void);

    // Invert image
    typedef void (CImg32Operations:: *INVERT_IMAGE)(BYTE* pSource, BYTE* pDest, int nNumberOfPixels);

    void InvertImageCPlusPlus(BYTE* pSource, BYTE* pDest, int nNumberOfPixels);
    void InvertImageC_MMX(BYTE* pSource, BYTE* pDest, int nNumberOfPixels);
    void InvertImageAssembly_MMX(BYTE* pSource, BYTE* pDest, int nNumberOfPixels);


    // Change colors
    typedef void (CImg32Operations::* COMPUTE_COLORS)(BYTE* pSource, BYTE* pDest, int nNumberOfPixels, 
        float fRedCoefficient, float fGreenCoefficient, float fBlueCoefficient);

    void ColorsCPlusPlus(BYTE* pSource, BYTE* pDest, int nNumberOfPixels, 
        float fRedCoefficient, float fGreenCoefficient, float fBlueCoefficient);
    void ColorsC_MMX(BYTE* pSource, BYTE* pDest, int nNumberOfPixels, 
        float fRedCoefficient, float fGreenCoefficient, float fBlueCoefficient);
    void ColorsAssembly_MMX(BYTE* pSource, BYTE* pDest, int nNumberOfPixels, 
        float fRedCoefficient, float fGreenCoefficient, float fBlueCoefficient);

protected:
    __m64 Get_m64(__int64 n);


};
