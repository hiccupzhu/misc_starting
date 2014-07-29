// -------------------------  FILE HEADER  --------------------------------
//
// CM Ident:  $Header: 
//
// COPYRIGHT  AmBow Inc., SuZhou. 2002 All Rights Reserved
//
// File:      GENRS_AutoLock.h
//
// Project:   AmBow's Reusability Project
//
// Purpose:   Inline header file for the class GENRS_SyncObject
//
// ------------------------------------------------------------------------
#ifndef _GENRIC_AUTOLOCK_H_
#define _GENRIC_AUTOLOCK_H_
// --------------------------  System Includes  ---------------------------



// --------------------------  AmBow  Includes  ---------------------------
#include "GENRS_SyncObject.h"


// -----------------------------  Constants  ------------------------------



// -------------------------  Forward References  -------------------------



// --------------------------  Type Definitions  --------------------------

// --------------------------------------------------------  GENRS_AutoLock
// Description: 
//  The Synchronization object helper class.
//  by the lifeline control the lock and unlock.
//  when enter an code block, use this class access multiple thread.
//  sample code:
//  //some place will keep the class GENRS_SyncObject instance such as:
//  GENRS_SyncObject g_accXXSyncObject;
//
//  foo()
//  {
//      GENRS_AutoLock lock(g_accXXSyncObject)
//      
//  }
//  
//  in this function object will auto lock and unlock the variables ,even 
// if the foo will throw some exception.
// ------------------------------------------------------------------------    
class GENRS_AutoLock
{
// ------------------------------------------------------------------------
// Public Methods
// ------------------------------------------------------------------------
//make GENRS_AutoLock can not be called Copy-Constuctor and Assign-Constuctor
    GENRS_AutoLock(const GENRS_SyncObject & rSynObj);
    GENRS_AutoLock & operator=(const GENRS_SyncObject & rSynObj);
public :
    GENRS_AutoLock(GENRS_SyncObject &SyncObj):m_SyncObj(SyncObj)
    {
        m_SyncObj.Lock();
    }
    ~GENRS_AutoLock()
    {
        m_SyncObj.Unlock();
    }
// ------------------------------------------------------------------------
// Private Methods & Data Members
// ------------------------------------------------------------------------
private:        
// --------------------------  Private Data Members  ----------------------
// -------------------------------------------------------------- m_SyncObj
// Description:
//   this object want to lock which sync object.
// ------------------------------------------------------------------------
    GENRS_SyncObject& m_SyncObj;
};

#endif