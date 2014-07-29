// -------------------------  FILE HEADER  --------------------------------
//
// CM Ident:  $Header: 
//
// COPYRIGHT  AmBow Inc., SuZhou. 2002 All Rights Reserved
//
// File:      GENRS_SyncObject.h
//
// Project:   AmBow's Reusability Project
//
// Purpose:   Inline header file for the class GENRS_SyncObject
//
// ------------------------------------------------------------------------
#ifndef _GENRS_SyncObject_H_
#define _GENRS_SyncObject_H_


// --------------------------  System Includes  ---------------------------



// --------------------------  AmBow  Includes  ---------------------------



// -----------------------------  Constants  ------------------------------



// -------------------------  Forward References  -------------------------



// --------------------------  Type Definitions  --------------------------



// ------------------------------------------------------  GENRS_SyncObject
// Description:
//  Synchronization Object class.this class used to lock some variables,
//  they will be used in multiple thread.
//  when a thread want access these variables ,call the lock method;
//  and when end accessing these variables,call the unlock method.
// ------------------------------------------------------------------------    
class GENRS_SyncObject
{
// ------------------------------------------------------------------------
// Public Methods
// ------------------------------------------------------------------------
//make SyncObject can not be called Copy-Constuctor and Assign-Constuctor
    GENRS_SyncObject(const GENRS_SyncObject & rSynObj);
    GENRS_SyncObject & operator=(const GENRS_SyncObject & rSynObj);
public:
    GENRS_SyncObject()
    {
#ifdef _WINDOWS_
        InitializeCriticalSection(&m_KernelObject);
#endif
    }

    ~GENRS_SyncObject()
    {
#ifdef _WINDOWS_
        DeleteCriticalSection(&m_KernelObject);
#endif
    }

// ------------------------------------------------------------------- Lock
// Description:
//  before accessing the data in vary thread,call this function to lock 
//    the sync object.
//   Note:on vary platform ,implementation may be diff.
// ------------------------------------------------------------------------
    void Lock()
    {
#ifdef _WINDOWS_
        EnterCriticalSection(&m_KernelObject);
#endif
    }

// ----------------------------------------------------------------- Unlock
// Description:
//  When accessing finished the data in vary thread,call this function to 
// unlock the sync object.
//   Note:on vary platform ,implementation may be diff.
// ------------------------------------------------------------------------
    void Unlock()
    {
#ifdef _WINDOWS_
        LeaveCriticalSection(&m_KernelObject);
#endif
    }
// ------------------------------------------------------------------------
// Private Methods & Data Members
// ------------------------------------------------------------------------
private:        
// --------------------------  Private Data Members  ----------------------
private:
// --------------------------------------------------------- m_KernelObject
// Description:
//   this object manage the kernel sync object.define same name with vary 
//  platfroms.
// ------------------------------------------------------------------------
#ifdef _WINDOWS_
    CRITICAL_SECTION m_KernelObject;
#elif defined (_UNIX)
    
#endif
};


#endif