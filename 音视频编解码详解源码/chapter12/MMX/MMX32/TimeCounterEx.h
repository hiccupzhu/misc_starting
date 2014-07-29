// TimeCounterEx.h


#ifndef TIME_COUNTER_H
#define TIME_COUNTER_H

#pragma warning( disable : 4710 )

// Helper class used to show execution time
class CTimeCounter
{
public:
    CTimeCounter()
    {
        QueryPerformanceFrequency(&m_nFreq);
        QueryPerformanceCounter(&m_nBeginTime);
    }

    __int64 GetExecutionTime()
    {
            LARGE_INTEGER nEndTime;
            __int64 nCalcTime;

        	QueryPerformanceCounter(&nEndTime);
            nCalcTime = (nEndTime.QuadPart - m_nBeginTime.QuadPart) *
                1000/m_nFreq.QuadPart;

            delete this;

            return nCalcTime;
    }

    ~CTimeCounter()
    {
    }


protected:
    LARGE_INTEGER m_nFreq;
    LARGE_INTEGER m_nBeginTime;

};



#endif