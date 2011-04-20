/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */

#include "SkTableMaskFilter.h"

SkTableMaskFilter::SkTableMaskFilter() {
    for (int i = 0; i < 256; i++) {
        fTable[i] = i;
    }
}

SkTableMaskFilter::SkTableMaskFilter(const uint8_t table[256]) {
    this->setTable(table);
}

SkTableMaskFilter::~SkTableMaskFilter() {}

void SkTableMaskFilter::setTable(const uint8_t table[256]) {
    memcpy(fTable, table, 256);
}

bool SkTableMaskFilter::filterMask(SkMask* dst, const SkMask& src,
                                 const SkMatrix&, SkIPoint* margin) {
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

SkMask::Format SkTableMaskFilter::getFormat() {
    return SkMask::kA8_Format;
}

void SkTableMaskFilter::flatten(SkFlattenableWriteBuffer& wb) {
    this->INHERITED::flatten(wb);
    wb.writePad(fTable, 256);
}

SkTableMaskFilter::SkTableMaskFilter(SkFlattenableReadBuffer& rb)
        : INHERITED(rb) {
    rb.read(fTable, 256);
}

SkFlattenable* SkTableMaskFilter::Factory(SkFlattenableReadBuffer& rb) {
    return SkNEW_ARGS(SkTableMaskFilter, (rb));
}

SkFlattenable::Factory SkTableMaskFilter::getFactory() {
    return SkTableMaskFilter::Factory;
}

///////////////////////////////////////////////////////////////////////////////

void SkTableMaskFilter::MakeGammaTable(uint8_t table[256], SkScalar gamma) {
    float x = 0;
    const float dx = 1 / 255.0f;
    for (int i = 0; i < 256; i++) {
        table[i] = SkPin32(SkScalarRound(powf(x, gamma) * 255), 0, 255);
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
        int value = SkFixedRound(scale * (i - min));
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

