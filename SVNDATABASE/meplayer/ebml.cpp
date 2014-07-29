/***************************************************************************\
*                                                                           *
*   Copyright 2006 ViXS Systems Inc.  All Rights Reserved.                  *
*                                                                           *
*===========================================================================*
*                                                                           *
*   File Name: ebml.cpp                                                     *
*                                                                           *
*   Description:                                                            *
*       This file contains the implementation of the EBML file reader.      *
*                                                                           *
*===========================================================================*

\***************************************************************************/

//#ifdef ENABLE_MKV_SUPPORT

#include <memory.h>

#include "ebml.h"
//#include "xclog.h"

namespace DMP{
//#define DEBUG_PRINT(...)    xclog::printf(27, __VA_ARGS__);     // Borrow log #27

ebml_raw_t::ebml_raw_t()
    : m_Size(0), m_pData(NULL)
{
}
ebml_raw_t::~ebml_raw_t()
{
    if (m_pData)
    {
        free(m_pData);
        m_pData = NULL;
    }
}
void ebml_raw_t::set(const void *pData, size_t Size)
{
    if (m_pData)
    {
        free(m_pData);
        m_pData = NULL;
    }
    m_pData = (unsigned char *)malloc(Size);
    m_Size = Size;
    if (pData)
    {
        memcpy(m_pData, pData, Size);
    }
}
void ebml_raw_t::append(const void *pData, size_t Size)
{
    m_pData = (unsigned char *)realloc(m_pData, m_Size + Size);
    memcpy(m_pData + m_Size, pData, Size);
    m_Size += Size;
}
ebml_raw_t::ebml_raw_t(const void *pData, size_t Size)
    : m_Size(0), m_pData(NULL)
{
    set(pData, Size);
}

}


//#endif // ENABLE_MKV_SUPPORT
