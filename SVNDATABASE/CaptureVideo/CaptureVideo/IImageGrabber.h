//
// IImageGrabber.h
// Desc: DirectShow sample code - custom interface
//

/*-----------------------------------------------------*\
			HQ Tech, Make Technology Easy!       
 More information, please go to http://hqtech.nease.net.
/*-----------------------------------------------------*/

#ifndef __H_IImageGrabber__
#define __H_IImageGrabber__

#ifdef __cplusplus
extern "C" {
#endif


//----------------------------------------------------------------------------
// IImageGrabber
//----------------------------------------------------------------------------
DECLARE_INTERFACE_(IImageGrabber, IUnknown)
{
	STDMETHOD(get_ImageSize) (THIS_
		long * outWidth, long * outHeight, long * outBitCount   
	) PURE;

	STDMETHOD(get_FrameSize) (THIS_
		long * outFrameSize
	) PURE;

	STDMETHOD(get_BitmapInfoHeader) (THIS_
		BITMAPINFOHEADER * outBitmapInfo
	) PURE;

	STDMETHOD(get_Is16BitsRGB) (THIS_
		long * outIsRGB565
	) PURE;

	STDMETHOD(put_IsFieldPicture) (THIS_
		BOOL inIsField
	) PURE;

	STDMETHOD(Snapshot) (THIS_ 
		BYTE * outBuffer, BOOL inIsSyncMode 
	) PURE;

	STDMETHOD(IsAsyncSnapshotFinished) (THIS_
		BOOL * outFinished
	) PURE;

	STDMETHOD(CancelAllPending) (THIS) PURE;
};
//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __H_IImageGrabber__