#include "SkPaintFlagsDrawFilter.h"
#include "SkPaint.h"

SkPaintFlagsDrawFilter::SkPaintFlagsDrawFilter(uint32_t clearFlags,
                                               uint32_t setFlags)
{
    fClearFlags = SkToU16(clearFlags & SkPaint::kAllFlags);
    fSetFlags = SkToU16(setFlags & SkPaint::kAllFlags);
}

bool SkPaintFlagsDrawFilter::filter(SkCanvas*, SkPaint* paint, Type)
{
    fPrevFlags = paint->getFlags();
    paint->setFlags((fPrevFlags & ~fClearFlags) | fSetFlags);
    return true;
}

void SkPaintFlagsDrawFilter::restore(SkCanvas*, SkPaint* paint, Type)
{
    paint->setFlags(fPrevFlags);
}
    
