
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkTransparentShader.h"
#include "SkColorPriv.h"
#include "SkString.h"

SkShader::Context* SkTransparentShader::onCreateContext(const ContextRec& rec,
                                                        void* storage) const {
    return SkNEW_PLACEMENT_ARGS(storage, TransparentShaderContext, (*this, rec));
}

size_t SkTransparentShader::contextSize() const {
    return sizeof(TransparentShaderContext);
}

SkTransparentShader::TransparentShaderContext::TransparentShaderContext(
        const SkTransparentShader& shader, const ContextRec& rec)
    : INHERITED(shader, rec)
    , fDevice(rec.fDevice) {}

SkTransparentShader::TransparentShaderContext::~TransparentShaderContext() {}

uint32_t SkTransparentShader::TransparentShaderContext::getFlags() const {
    uint32_t flags = this->INHERITED::getFlags();

    switch (fDevice->colorType()) {
        case kRGB_565_SkColorType:
            flags |= kHasSpan16_Flag;
            if (this->getPaintAlpha() == 255)
                flags |= kOpaqueAlpha_Flag;
            break;
        case kN32_SkColorType:
            if (this->getPaintAlpha() == 255 && fDevice->isOpaque())
                flags |= kOpaqueAlpha_Flag;
            break;
        default:
            break;
    }
    return flags;
}

void SkTransparentShader::TransparentShaderContext::shadeSpan(int x, int y, SkPMColor span[],
                                                              int count) {
    unsigned scale = SkAlpha255To256(this->getPaintAlpha());

    switch (fDevice->colorType()) {
        case kN32_SkColorType:
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
                unsigned alpha = this->getPaintAlpha();
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

void SkTransparentShader::TransparentShaderContext::shadeSpan16(int x, int y, uint16_t span[],
                                                                int count) {
    SkASSERT(fDevice->colorType() == kRGB_565_SkColorType);

    uint16_t* src = fDevice->getAddr16(x, y);
    if (src != span) {
        memcpy(span, src, count << 1);
    }
}

SkFlattenable* SkTransparentShader::CreateProc(SkReadBuffer& buffer) {
    return SkNEW(SkTransparentShader);
}

#ifndef SK_IGNORE_TO_STRING
void SkTransparentShader::toString(SkString* str) const {
    str->append("SkTransparentShader: (");

    this->INHERITED::toString(str);

    str->append(")");
}
#endif
