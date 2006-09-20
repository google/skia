#ifndef SkAvoidXfermode_DEFINED
#define SkAvoidXfermode_DEFINED

#include "SkXfermode.h"

/** \class SkAvoidXfermode

    This xfermode will draw the src everywhere except on top of the specified
    color.
*/
class SkAvoidXfermode : public SkXfermode {
public:
    /** This xfermode will draw the src everywhere except on top of the specified
        color.
        @param opColor  the color to avoid (or to target if reverse is true);
        @param tolerance    How closely we compare a pixel to the opColor.
                            0 - we only avoid on an exact match
                            255 - maximum gradation (blending) based on how similar
                            the pixel is to our opColor.
        @param reverse      true means we target the opColor rather than avoid it.
    */
    SkAvoidXfermode(SkColor opColor, U8CPU tolerance, bool reverse);

    virtual void xfer32(SkPMColor dst[], const SkPMColor src[], int count, const SkAlpha aa[]);
    virtual void xfer16(uint16_t dst[], const SkPMColor src[], int count, const SkAlpha aa[]);
    virtual void xferA8(SkAlpha dst[], const SkPMColor src[], int count, const SkAlpha aa[]);

private:
    SkColor     fOpColor;
    uint32_t    fDistMul;   // x.14
    bool        fReverse;
};

#endif
