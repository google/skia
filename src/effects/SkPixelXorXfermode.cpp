
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPixelXorXfermode.h"
#include "SkColorPriv.h"

// we always return an opaque color, 'cause I don't know what to do with
// the alpha-component and still return a valid premultiplied color.
SkPMColor SkPixelXorXfermode::xferColor(SkPMColor src, SkPMColor dst) {
    SkPMColor res = src ^ dst ^ fOpColor;
    res |= (SK_A32_MASK << SK_A32_SHIFT);   // force it to be opaque
    return res;
}

void SkPixelXorXfermode::flatten(SkFlattenableWriteBuffer& wb) const {
    this->INHERITED::flatten(wb);
    wb.write32(fOpColor);
}

SkPixelXorXfermode::SkPixelXorXfermode(SkFlattenableReadBuffer& rb)
        : INHERITED(rb) {
    fOpColor = rb.readU32();
}

SK_DEFINE_FLATTENABLE_REGISTRAR(SkPixelXorXfermode)
