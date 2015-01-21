#ifndef SkAtomics_DEFINED
#define SkAtomics_DEFINED

// This file is not part of the public Skia API.
#include "SkTypes.h"

#if defined(_MSC_VER)
    #include "../ports/SkAtomics_win.h"
#else
    #include "../ports/SkAtomics_sync.h"
#endif

#endif//SkAtomics_DEFINED
