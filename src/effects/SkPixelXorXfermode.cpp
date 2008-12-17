#include "SkPixelXorXfermode.h"
#include "SkColorPriv.h"

// we always return an opaque color, 'cause I don't know what to do with
// the alpha-component and still return a valid premultiplied color.
SkPMColor SkPixelXorXfermode::xferColor(SkPMColor src, SkPMColor dst)
{
    SkPMColor res = src ^ dst ^ fOpColor;
    res |= (SK_A32_MASK << SK_A32_SHIFT);   // force it to be opaque
    return res;
}

void SkPixelXorXfermode::flatten(SkFlattenableWriteBuffer& wb)
{
    this->INHERITED::flatten(wb);
    wb.write32(fOpColor);
}

SkPixelXorXfermode::SkPixelXorXfermode(SkFlattenableReadBuffer& rb)
    : SkXfermode(rb)
{
    fOpColor = rb.readU32();
}

SkFlattenable::Factory SkPixelXorXfermode::getFactory()
{
    return Create;
}

SkFlattenable* SkPixelXorXfermode::Create(SkFlattenableReadBuffer& rb)
{
    return SkNEW_ARGS(SkPixelXorXfermode, (rb));
}



