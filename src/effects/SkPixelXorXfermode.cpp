
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPixelXorXfermode.h"
#include "SkColorPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkString.h"

// we always return an opaque color, 'cause I don't know what to do with
// the alpha-component and still return a valid premultiplied color.
SkPMColor SkPixelXorXfermode::xferColor(SkPMColor src, SkPMColor dst) const {
    SkPMColor res = src ^ dst ^ fOpColor;
    res |= (SK_A32_MASK << SK_A32_SHIFT);   // force it to be opaque
    return res;
}

void SkPixelXorXfermode::flatten(SkWriteBuffer& wb) const {
    this->INHERITED::flatten(wb);
    wb.writeColor(fOpColor);
}

SkPixelXorXfermode::SkPixelXorXfermode(SkReadBuffer& rb)
        : INHERITED(rb) {
    fOpColor = rb.readColor();
}

#ifndef SK_IGNORE_TO_STRING
void SkPixelXorXfermode::toString(SkString* str) const {
    str->append("SkPixelXorXfermode: ");
    str->appendHex(fOpColor);
}
#endif
