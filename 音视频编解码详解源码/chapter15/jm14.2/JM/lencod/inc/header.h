
/*!
 *************************************************************************************
 * \file header.h
 *
 * \brief
 *    Prototypes for header.c
 *************************************************************************************
 */

#ifndef _HEADER_H_
#define _HEADER_H_

int SliceHeader(void);
int Partition_BC_Header(int PartNo);

int  writeERPS(SyntaxElement *sym, DataPartition *partition);

#endif

