//
// GlobalDefs.h
//

#ifndef __H_GlobalDefs__
#define __H_GlobalDefs__

//-----------------------------------------------------------------------------
// Miscellaneous helper functions
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

typedef enum
{
	DT_DV = 0,
	DT_Analog_WDM,
	DT_Analog_VFW,
	DT_Analog_Audio,
	DT_Unknown
} Device_Type;

typedef enum 
{
	MD_Preview, 
	MD_Capture
} Work_Mode;

typedef enum 
{
	SR_NTSC, 
	SR_PAL,
	SR_SECAM,
	SR_UNKNOWN
} Signal_Resolution;

typedef enum 
{
	ET_AVI, 
	ET_DIVX
} Encoding_Type;

const long cVideoSourceChanged			= 'lcap' + 1;
const long cAudioSourceChanged			= 'lcap' + 2;
const long cVideoConnectorChanged		= 'lcap' + 3;
const long cAudioConnectorChanged		= 'lcap' + 4;
const long cVideoResolutionChanged		= 'lcap' + 5;
const long cAudioMixLevelChanged		= 'lcap' + 6;

const long msg_PnPDeviceAdded	= 88888;
const long msg_PnPDeviceRemoved	= 88889;

const long msg_FilterGraphError	= 88890;
const long msg_DShowDeviceLost  = 88891;

#endif // __H_GlobalDefs__