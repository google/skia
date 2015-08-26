/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLerpXfermode.h"
#include "SkColorPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkString.h"

SkXfermode* SkLerpXfermode::Create(SkScalar scale) {
    int scale256 = SkScalarRoundToInt(scale * 256);
    if (scale256 >= 256) {
        return SkXfermode::Create(SkXfermode::kSrc_Mode);
    } else if (scale256 <= 0) {
        return SkXfermode::Create(SkXfermode::kDst_Mode);
    }
    return new SkLerpXfermode(scale256);
}

SkLerpXfermode::SkLerpXfermode(unsigned scale256) : fScale256(scale256) {}

void SkLerpXfermode::flatten(SkWriteBuffer& buffer) const {
    buffer.writeUInt(fScale256);
}

SkFlattenable* SkLerpXfermode::CreateProc(SkReadBuffer& buffer) {
    return new SkLerpXfermode(buffer.readUInt());
}

void SkLerpXfermode::xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                             const SkAlpha aa[]) const {
    const int scale = fScale256;

    if (aa) {
        for (int i = 0; i < count; ++i) {
            unsigned a = aa[i];
            if (a) {
                SkPMColor dstC = dst[i];
                SkPMColor resC = SkFastFourByteInterp256(src[i], dstC, scale);
                if (a < 255) {
                    resC = SkFastFourByteInterp256(resC, dstC, a + (a >> 7));
                }
                dst[i] = resC;
            }
        }
    } else {
        for (int i = 0; i < count; ++i) {
            dst[i] = SkFastFourByteInterp256(src[i], dst[i], scale);
        }
    }
}

void SkLerpXfermode::xfer16(uint16_t dst[], const SkPMColor src[], int count,
                             const SkAlpha aa[]) const {
    const int scale = fScale256;

    if (aa) {
        for (int i = 0; i < count; ++i) {
            unsigned a = aa[i];
            if (a) {
                SkPMColor dstC = SkPixel16ToPixel32(dst[i]);
                SkPMColor resC = SkFastFourByteInterp256(src[i], dstC, scale);
                if (a < 255) {
                    resC = SkFastFourByteInterp256(resC, dstC, a + (a >> 7));
                }
                dst[i] = SkPixel32ToPixel16(resC);
            }
        }
    } else {
        for (int i = 0; i < count; ++i) {
            SkPMColor dstC = SkPixel16ToPixel32(dst[i]);
            SkPMColor resC = SkFastFourByteInterp256(src[i], dstC, scale);
            dst[i] = SkPixel32ToPixel16(resC);
        }
    }
}

void SkLerpXfermode::xferA8(SkAlpha dst[], const SkPMColor src[], int count,
                             const SkAlpha aa[]) const {
    const int scale = fScale256;

    if (aa) {
        for (int i = 0; i < count; ++i) {
            unsigned a = aa[i];
            if (a) {
                unsigned dstA = dst[i];
                unsigned resA = SkAlphaBlend(SkGetPackedA32(src[i]), dstA, scale);
                if (a < 255) {
                    resA = SkAlphaBlend(resA, dstA, a + (a >> 7));
                }
                dst[i] = resA;
            }
        }
    } else {
        for (int i = 0; i < count; ++i) {
            dst[i] = SkAlphaBlend(SkGetPackedA32(src[i]), dst[i], scale);
        }
    }
}

#ifndef SK_IGNORE_TO_STRING
void SkLerpXfermode::toString(SkString* str) const {
    str->printf("SkLerpXfermode: scale: %g", fScale256 / 256.0);
}
#endif
