#include "SkBlitRow.h"

// Platform impl of Platform_procs with no overrides

SkBlitRow::Proc SkBlitRow::PlatformProcs4444(unsigned flags) {
    return NULL;
}

SkBlitRow::Proc SkBlitRow::PlatformProcs565(unsigned flags) {
    return NULL;
}

SkBlitRow::Proc32 SkBlitRow::PlatformProcs32(unsigned flags) {
    return NULL;
}

SkBlitRow::ColorProc SkBlitRow::PlatformColorProc() {
    return NULL;
}


SkBlitMask::Proc SkBlitMask::PlatformProcs(SkBitmap::Config dstConfig,
                                           SkColor color)
{
   return NULL;
}
