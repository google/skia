#ifndef SkBlitRow_DEFINED
#define SkBlitRow_DEFINED

#include "SkBitmap.h"
#include "SkColor.h"

class SkBlitRow {
public:
    enum {
        kGlobalAlpha_Flag   = 0x01,
        kSrcPixelAlpha_Flag = 0x02,
        kDither_Flag        = 0x04
    };

    typedef void (*Proc)(uint16_t* SK_RESTRICT dst,
                         const SkPMColor* SK_RESTRICT src,
                         int count, U8CPU alpha, int x, int y);

    static Proc Factory(unsigned flags, SkBitmap::Config);
};

#endif
