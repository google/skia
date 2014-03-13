
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkTransparentShader.h"
#include "SkColorPriv.h"
#include "SkString.h"

bool SkTransparentShader::setContext(const SkBitmap& device,
                                     const SkPaint& paint,
                                     const SkMatrix& matrix) {
    fDevice = &device;
    fAlpha = paint.getAlpha();

    return this->INHERITED::setContext(device, paint, matrix);
}

uint32_t SkTransparentShader::getFlags() {
    uint32_t flags = this->INHERITED::getFlags();

    switch (fDevice->colorType()) {
        case kRGB_565_SkColorType:
            flags |= kHasSpan16_Flag;
            if (fAlpha == 255)
                flags |= kOpaqueAlpha_Flag;
            break;
        case kPMColor_SkColorType:
            if (fAlpha == 255 && fDevice->isOpaque())
                flags |= kOpaqueAlpha_Flag;
            break;
        default:
            break;
    }
    return flags;
}

void SkTransparentShader::shadeSpan(int x, int y, SkPMColor span[], int count) {
    unsigned scale = SkAlpha255To256(fAlpha);

    switch (fDevice->colorType()) {
        case kPMColor_SkColorType:
            if (scale == 256) {
                SkPMColor* src = fDevice->getAddr32(x, y);
                if (src != span) {
                    memcpy(span, src, count * sizeof(SkPMColor));
                }
            } else {
                const SkPMColor* src = fDevice->getAddr32(x, y);
                for (int i = count - 1; i >= 0; --i) {
                    span[i] = SkAlphaMulQ(src[i], scale);
                }
            }
            break;
        case kRGB_565_SkColorType: {
            const uint16_t* src = fDevice->getAddr16(x, y);
            if (scale == 256) {
                for (int i = count - 1; i >= 0; --i) {
                    span[i] = SkPixel16ToPixel32(src[i]);
                }
            } else {
                unsigned alpha = fAlpha;
                for (int i = count - 1; i >= 0; --i) {
                    uint16_t c = src[i];
                    unsigned r = SkPacked16ToR32(c);
                    unsigned g = SkPacked16ToG32(c);
                    unsigned b = SkPacked16ToB32(c);

                    span[i] = SkPackARGB32( alpha,
                                            SkAlphaMul(r, scale),
                                            SkAlphaMul(g, scale),
                                            SkAlphaMul(b, scale));
                }
            }
            break;
        }
        case kAlpha_8_SkColorType: {
            const uint8_t* src = fDevice->getAddr8(x, y);
            if (scale == 256) {
                for (int i = count - 1; i >= 0; --i) {
                    span[i] = SkPackARGB32(src[i], 0, 0, 0);
                }
            } else {
                for (int i = count - 1; i >= 0; --i) {
                    span[i] = SkPackARGB32(SkAlphaMul(src[i], scale), 0, 0, 0);
                }
            }
            break;
        }
        default:
            SkDEBUGFAIL("colorType not supported as a destination device");
            break;
    }
}

void SkTransparentShader::shadeSpan16(int x, int y, uint16_t span[], int count) {
    SkASSERT(fDevice->colorType() == kRGB_565_SkColorType);

    uint16_t* src = fDevice->getAddr16(x, y);
    if (src != span) {
        memcpy(span, src, count << 1);
    }
}

#ifndef SK_IGNORE_TO_STRING
void SkTransparentShader::toString(SkString* str) const {
    str->append("SkTransparentShader: (");

    this->INHERITED::toString(str);

    str->append(")");
}
#endif
