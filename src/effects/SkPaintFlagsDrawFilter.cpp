#include "SkPaintFlagsDrawFilter.h"
#include "SkPaint.h"

SkPaintFlagsDrawFilter::SkPaintFlagsDrawFilter(uint32_t clearFlags,
                                               uint32_t setFlags) {
    fClearFlags = SkToU16(clearFlags & SkPaint::kAllFlags);
    fSetFlags = SkToU16(setFlags & SkPaint::kAllFlags);
}

void SkPaintFlagsDrawFilter::filter(SkPaint* paint, Type) {
    paint->setFlags((paint->getFlags() & ~fClearFlags) | fSetFlags);
}

