
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


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

