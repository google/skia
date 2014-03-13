
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkTableMaskFilter.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkString.h"

SkTableMaskFilter::SkTableMaskFilter() {
    for (int i = 0; i < 256; i++) {
        fTable[i] = i;
    }
}

SkTableMaskFilter::SkTableMaskFilter(const uint8_t table[256]) {
    memcpy(fTable, table, sizeof(fTable));
}

SkTableMaskFilter::~SkTableMaskFilter() {}

bool SkTableMaskFilter::filterMask(SkMask* dst, const SkMask& src,
                                 const SkMatrix&, SkIPoint* margin) const {
    if (src.fFormat != SkMask::kA8_Format) {
        return false;
    }

    dst->fBounds = src.fBounds;
    dst->fRowBytes = SkAlign4(dst->fBounds.width());
    dst->fFormat = SkMask::kA8_Format;
    dst->fImage = NULL;

    if (src.fImage) {
        dst->fImage = SkMask::AllocImage(dst->computeImageSize());

        const uint8_t* srcP = src.fImage;
        uint8_t* dstP = dst->fImage;
        const uint8_t* table = fTable;
        int dstWidth = dst->fBounds.width();
        int extraZeros = dst->fRowBytes - dstWidth;

        for (int y = dst->fBounds.height() - 1; y >= 0; --y) {
            for (int x = dstWidth - 1; x >= 0; --x) {
                dstP[x] = table[srcP[x]];
            }
            srcP += src.fRowBytes;
            // we can't just inc dstP by rowbytes, because if it has any
            // padding between its width and its rowbytes, we need to zero those
            // so that the bitters can read those safely if that is faster for
            // them
            dstP += dstWidth;
            for (int i = extraZeros - 1; i >= 0; --i) {
                *dstP++ = 0;
            }
        }
    }

    if (margin) {
        margin->set(0, 0);
    }
    return true;
}

SkMask::Format SkTableMaskFilter::getFormat() const {
    return SkMask::kA8_Format;
}

void SkTableMaskFilter::flatten(SkWriteBuffer& wb) const {
    this->INHERITED::flatten(wb);
    wb.writeByteArray(fTable, 256);
}

SkTableMaskFilter::SkTableMaskFilter(SkReadBuffer& rb)
        : INHERITED(rb) {
    SkASSERT(256 == rb.getArrayCount());
    rb.readByteArray(fTable, 256);
}

///////////////////////////////////////////////////////////////////////////////

void SkTableMaskFilter::MakeGammaTable(uint8_t table[256], SkScalar gamma) {
    const float dx = 1 / 255.0f;
    const float g = SkScalarToFloat(gamma);

    float x = 0;
    for (int i = 0; i < 256; i++) {
     // float ee = powf(x, g) * 255;
        table[i] = SkPin32(sk_float_round2int(powf(x, g) * 255), 0, 255);
        x += dx;
    }
}

void SkTableMaskFilter::MakeClipTable(uint8_t table[256], uint8_t min,
                                      uint8_t max) {
    if (0 == max) {
        max = 1;
    }
    if (min >= max) {
        min = max - 1;
    }
    SkASSERT(min < max);

    SkFixed scale = (1 << 16) * 255 / (max - min);
    memset(table, 0, min + 1);
    for (int i = min + 1; i < max; i++) {
        int value = SkFixedRoundToInt(scale * (i - min));
        SkASSERT(value <= 255);
        table[i] = value;
    }
    memset(table + max, 255, 256 - max);

#if 0
    int j;
    for (j = 0; j < 256; j++) {
        if (table[j]) {
            break;
        }
    }
    SkDebugf("%d %d start [%d]", min, max, j);
    for (; j < 256; j++) {
        SkDebugf(" %d", table[j]);
    }
    SkDebugf("\n\n");
#endif
}

#ifndef SK_IGNORE_TO_STRING
void SkTableMaskFilter::toString(SkString* str) const {
    str->append("SkTableMaskFilter: (");

    str->append("table: ");
    for (int i = 0; i < 255; ++i) {
        str->appendf("%d, ", fTable[i]);
    }
    str->appendf("%d", fTable[255]);

    str->append(")");
}
#endif
