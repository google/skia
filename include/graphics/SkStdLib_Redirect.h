#ifndef SkStdLib_Redirect_DEFINED
#define SkStdLib_Redirect_DEFINED

#error

#include "SkTypes.h"

#define fread(buffer, count, size, file)	sk_stdlib_fread(buffer, count, size, file)
#define qsort
#define tolower
#define setjmp
#define longjmp
#define memmove
#define malloc
#define realloc
#endif

