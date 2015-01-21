#ifndef SkMutex_DEFINED
#define SkMutex_DEFINED

// This file is not part of the public Skia API.
#include "SkTypes.h"

#if defined(SK_BUILD_FOR_WIN)
    #include "../ports/SkMutex_win.h"
#else
    #include "../ports/SkMutex_pthread.h"
#endif

#endif//SkMutex_DEFINED
