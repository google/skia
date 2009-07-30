#ifndef SkBlitRow_DEFINED
#define SkBlitRow_DEFINED

#include "SkBitmap.h"
#include "SkColor.h"

class SkBlitRow {
public:
    enum {
        //! If set, the alpha parameter will be != 255
        kGlobalAlpha_Flag   = 0x01,
        //! If set, the src colors may have alpha != 255
        kSrcPixelAlpha_Flag = 0x02,
        //! If set, the resulting 16bit colors should be dithered
        kDither_Flag        = 0x04
    };

    /** Function pointer that reads a scanline of src SkPMColors, and writes
        a corresponding scanline of 16bit colors (specific format based on the
        config passed to the Factory.
     
        The x,y params are useful just for dithering
     
        @param alpha A global alpha to be applied to all of the src colors
        @param x The x coordinate of the beginning of the scanline
        @param y THe y coordinate of the scanline
     */
    typedef void (*Proc)(uint16_t* SK_RESTRICT dst,
                         const SkPMColor* SK_RESTRICT src,
                         int count, U8CPU alpha, int x, int y);

    //! Public entry-point to return a blit function ptr
    static Proc Factory(unsigned flags, SkBitmap::Config);

private:
    /** These global arrays are indexed using the flags parameterr to Factory,
        and contain either NULL, or a platform-specific function-ptr to be used
        in place of the system default.
     */
    static const Proc gPlatform_565_Procs[];
    static const Proc gPlatform_4444_Procs[];
};

#endif
