
#if !defined(GENRS_Asserter_H_)
#define GENRS_Asserter_H_

// ----------  System Includes  ----------------- 
#include <crtdbg.h>
#include <fstream>
#include <stdarg.h>
#include <time.h>
using namespace std;

class GENRS_Asserter 
{

// -----------------------------------------------------------------------
// Public Methods
// -----------------------------------------------------------------------

public:

	CRITICAL_SECTION m_lock;
	GENRS_Asserter ()
	{
		InitializeCriticalSection(&m_lock);
	};
	~GENRS_Asserter ()
	{
		DeleteCriticalSection(&m_lock);
	}

    void operator ()(
        const char* testName,
        const char* fileName,
        const int& lineNum)
    {
        
            
//#ifdef _DEBUG     
			EnterCriticalSection(&m_lock);
            static ofstream m_genrsErr;
            
            if(0==m_genrsErr.is_open())
            {
                m_genrsErr.open("c:\\core", ios::out|ios::ate|ios::app);
            }

			char time[15];
			char date[15];
			//Get current time and data.
			_strtime(time);
			_strdate(date);

            m_genrsErr << testName <<", " << fileName<< ", " << lineNum << 
				", TIME:" << time << ", DATE:" << date << endl;
            m_genrsErr.close();
			LeaveCriticalSection(&m_lock);
//#endif            

          
        
    };
	void operator()(const char* fileName,
		const int& lineNum,
		char* pFmt,
		...)
	{
//#ifdef _DEBUG		
		EnterCriticalSection(&m_lock);
		 static ofstream m_genrsErr;
            
            if(0==m_genrsErr.is_open())
            {
                m_genrsErr.open("c:\\core", ios::out|ios::ate|ios::app);
            }
			
			char Msg[500];
			va_list pArg;
			va_start(pArg, pFmt);
			vsprintf(Msg, pFmt, pArg);
			va_end(pArg);
			//strcat(Msg,"\n");

			char time[15];
			char date[15];
			//Get current time and data.
			_strtime(time);
			_strdate(date);
			
            m_genrsErr << Msg <<", " << fileName<< ", " << lineNum << 
				", TIME:" << time << ", DATE:" << date << endl;

            m_genrsErr.close();
			LeaveCriticalSection(&m_lock);
		
//#endif
		  }
		
	}; 


inline void TKL_Trace(char * pFormat,...)
{
#ifdef _DEBUG
    char Msg[500];
    va_list pArg;
    va_start(pArg, pFormat);
    vsprintf(Msg, pFormat, pArg);
    va_end(pArg);
    strcat(Msg,"\n");
    OutputDebugString(Msg);
#endif
}

#define NOTE(name)\
        GENRS_Asserter()(name, __FILE__, __LINE__)
#define NOTE1(name,nval)\
		GENRS_Asserter()( __FILE__, __LINE__,name,nval)
#define NOTE2(name,nval1,nval2)\
		GENRS_Asserter()(__FILE__, __LINE__,name,nval1,nval2)
#define NOTE3(name,nval1,nval2,nval3)\
		GENRS_Asserter()(__FILE__, __LINE__,name,nval1,nval2,nval3)

#endif //GENRS_Asserter_H_



