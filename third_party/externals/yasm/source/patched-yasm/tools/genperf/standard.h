/*
------------------------------------------------------------------------------
Standard definitions and types, Bob Jenkins
------------------------------------------------------------------------------
*/
#ifndef STANDARD
#define STANDARD

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
#define UB4BITS 32
typedef  unsigned short int  ub2;
#define UB2MAXVAL 0xffff
typedef  unsigned       char ub1;
#define UB1MAXVAL 0xff
typedef                 int  word;  /* fastest type available */

#define bis(target,mask)  ((target) |=  (mask))
#define bic(target,mask)  ((target) &= ~(mask))
#define bit(target,mask)  ((target) &   (mask))
#ifndef align
# define align(a) (((ub4)a+(sizeof(void *)-1))&(~(sizeof(void *)-1)))
#endif /* align */

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#endif /* STANDARD */
