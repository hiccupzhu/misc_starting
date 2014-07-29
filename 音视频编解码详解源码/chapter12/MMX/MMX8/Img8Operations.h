#pragma once

#include <emmintrin.h>		// MMX, SSE, SSE2 intrinsic support


// Image processing operations with 8 bits per pixel image
//
// For each image processing operation there are 3 functions:
// C++, C++ with MMX, Assembly with MMX
//
// Note about emms instruction:
// This program doesn't use float operations. However, it uses
// emms in the start and end of each MMX block.  I think it is 
// a good idea to use emms always if this instruction is not part
// of some program loop.
//
class CImg8Operations
{
public:
    CImg8Operations(void);
    ~CImg8Operations(void);

    // Invert image
    typedef void (CImg8Operations:: *INVERT_IMAGE)(BYTE* pSource, BYTE* pDest, int nNumberOfPixels);

    void InvertImageCPlusPlus(BYTE* pSource, BYTE* pDest, int nNumberOfPixels);
    void InvertImageC_MMX(BYTE* pSource, BYTE* pDest, int nNumberOfPixels);
    void InvertImageAssembly_MMX(BYTE* pSource, BYTE* pDest, int nNumberOfPixels);

    // Reduce brightness
    typedef void (CImg8Operations:: *CHANGE_BRIGHTNESS)(BYTE* pSource, BYTE* pDest, int nNumberOfPixels,
                                                        int nChange);

    void ChangeBrightnessCPlusPlus(BYTE* pSource, BYTE* pDest, int nNumberOfPixels, int nChange);
    void ChangeBrightnessC_MMX(BYTE* pSource, BYTE* pDest, int nNumberOfPixels, int nChange);
    void ChangeBrightnessAssembly_MMX(BYTE* pSource, BYTE* pDest, int nNumberOfPixels, int nChange);



protected:
    __m64 Get_m64(__int64 n);

};
