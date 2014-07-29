#ifndef _TEST_AUTO_GENERATED_H_
#define _TEST_AUTO_GENERATED_H_

/**************************************************************************** 
                                                                              
Copyright (c) 2003, Stanford University                                       
All rights reserved.                                                          
                                                                              
Copyright (c) 2008, Advanced Micro Devices, Inc.                              
All rights reserved.                                                          
                                                                              
                                                                              
The BRCC portion of BrookGPU is derived from the cTool project                
(http://ctool.sourceforge.net) and distributed under the GNU Public License.  
                                                                              
Additionally, see LICENSE.ctool for information on redistributing the         
contents of this directory.                                                   
                                                                              
****************************************************************************/ 

#include "brook/Stream.h" 
#include "brook/KernelInterface.h" 

//! Kernel declarations
class __sub255
{
    public:
        void operator()(const ::brook::Stream< uchar >& a, const ::brook::Stream<  uchar >& b);
        EXTENDCLASS();
};
extern __THREAD__ __sub255 sub255;

#endif // _TEST_AUTO_GENERATED_H_

