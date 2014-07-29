

/*!
 ****************************************************************************
 * \file errorconcealment.h
 *
 * \brief
 *    Header file for errorconcealment.c
 *
 ****************************************************************************
 */

#ifndef _ERRORCONCEALMENT_H_
#define _ERRORCONCEALMENT_H_

int set_ec_flag(int se);
void reset_ec_flags(void);
int get_concealed_element(SyntaxElement *sym);

#endif

