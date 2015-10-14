#ifndef _UINTPTR_T_DEFINED
#ifdef _WIN64
#include <vadefs.h>
#else
typedef unsigned long uintptr_t;
#endif
#define _UINTPTR_T_DEFINED
#endif
