#ifndef SkCGUtils_DEFINED
#define SkCGUtils_DEFINED

#include "SkTypes.h"

#ifdef SK_BUILD_FOR_MAC
    #include <Carbon/Carbon.h>
#else
    #include <CoreGraphics/CoreGraphics.h>
#endif

class SkBitmap;

CGImageRef SkCreateCGImageRef(const SkBitmap&);

void SkCGDrawBitmap(CGContextRef, const SkBitmap&, float x, float y);

#endif
