
////////////////////////////////////////////
// Generated by BRCC 1.4
// BRCC Compiled on: Mar  2 2009 13:08:08
////////////////////////////////////////////

#include "brook/brook.h"
#include "test_gpu.h"
#include "test.h"

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
void  __sub255_cpu_inner(const __BrtUChar1  &a,
                        __BrtUChar1  &b)

#line 2 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
{

#line 3 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
  b = (__BrtUChar1 ) (__BrtInt1((int)255)) - a;
}
void  __sub255_cpu(::brt::KernelC *__k, int __brt_idxstart, int __brt_idxend, bool __brt_isreduce)

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
{

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
  ::brt::StreamInterface *arg_a = (::brt::StreamInterface *) __k->getVectorElement(0);

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
  ::brt::StreamInterface *arg_b = (::brt::StreamInterface *) __k->getVectorElement(1);

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
  

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
    for(int __brt_idx=__brt_idxstart; __brt_idx<__brt_idxend; __brt_idx++) {
  if(!(__k->isValidAddress(__brt_idx))){ continue; }

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
    Addressable <__BrtUChar1  > __out_arg_b((__BrtUChar1 *) __k->FetchElem(arg_b, __brt_idx));

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
    __sub255_cpu_inner (

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
                        Addressable <__BrtUChar1 >((__BrtUChar1 *) __k->FetchElem(arg_a, __brt_idx)),

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
                        __out_arg_b);

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
    *reinterpret_cast<__BrtUChar1 *>(__out_arg_b.address) = __out_arg_b.castToArg(*reinterpret_cast<__BrtUChar1 *>(__out_arg_b.address));
  }
}

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
void __sub255::operator()(const ::brook::Stream< uchar >& a,

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
		const ::brook::Stream<  uchar >& b)
{

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
  static const void *__sub255_fp[] = {

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
     "cal", __sub255_cal,

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
     "cpu", (void *) __sub255_cpu,

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
     NULL, NULL };


#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
  ::brook::Kernel  __k(__sub255_fp, brook::KERNEL_MAP);
  ::brook::ArgumentInfo __argumentInfo;


#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
  __k.PushStream(a);

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
  __k.PushOutput(b);

#line 0 "e:/VCCODE2/CaptureVideo/CaptureVideo/test.br"
  __argumentInfo.startExecDomain = _domainOffset;
  __argumentInfo.domainDimension = _domainSize;


  __k.run(&__argumentInfo);
  DESTROYPARAM();

}

__THREAD__ __sub255 sub255;


