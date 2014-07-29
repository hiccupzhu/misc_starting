#ifndef __H_IMKVSOURCE__
#define __H_IMKVSOURCE__

#ifdef __cplusplus
extern"C"{
#endif


DECLARE_INTERFACE_(IMKVSource,IUnknown)
{
	STDMETHOD(CanChangeSource) (THIS) PURE;
};



#ifdef __cplusplus
};
#endif

#endif