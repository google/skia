/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkStippleMaskFilter.h"


bool SkStippleMaskFilter::filterMask(SkMask* dst,
                                     const SkMask& src,
                                     const SkMatrix& matrix,
                                     SkIPoint* margin) {

    if (src.fFormat != SkMask::kA8_Format) {
        return false;
    }

    dst->fBounds = src.fBounds;
    dst->fRowBytes = dst->fBounds.width();
    dst->fFormat = SkMask::kA8_Format;
    dst->fImage = NULL;

    if (NULL != src.fImage) {
        size_t dstSize = dst->computeImageSize();
        if (0 == dstSize) {
            return false;   // too big to allocate, abort
        }

        dst->fImage = SkMask::AllocImage(dstSize);

        uint8_t* srcScanLine = src.fImage;
        uint8_t* scanline = dst->fImage;

        for (int y = 0; y < src.fBounds.height(); ++y) {
            for (int x = 0; x < src.fBounds.width(); ++x) {
                scanline[x] = srcScanLine[x] && ((x+y) & 0x1) ? 0xFF : 0x00;
            }
            scanline += dst->fRowBytes;
            srcScanLine += src.fRowBytes;
        }
    }

    return true;
}

SK_DEFINE_FLATTENABLE_REGISTRAR(SkStippleMaskFilter)
