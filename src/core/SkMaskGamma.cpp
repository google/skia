/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#include "SkColor.h"
#include "SkFloatingPoint.h"
#include "SkMaskGamma.h"

SkScalar SkSRGBLuminance::toLuma(SkScalar luminance) const {
    //The magic numbers are derived from the sRGB specification.
    //See http://www.color.org/chardata/rgb/srgb.xalter .
    if (luminance <= SkFloatToScalar(0.04045f)) {
        return luminance / SkFloatToScalar(12.92f);
    }
    return SkScalarPow((luminance + SkFloatToScalar(0.055f)) / SkFloatToScalar(1.055f),
                       SkFloatToScalar(2.4f));
}

SkScalar SkSRGBLuminance::fromLuma(SkScalar luma) const {
    //The magic numbers are derived from the sRGB specification.
    //See http://www.color.org/chardata/rgb/srgb.xalter .
    if (luma <= SkFloatToScalar(0.0031308f)) {
        return luma * SkFloatToScalar(12.92f);
    }
    return SkFloatToScalar(1.055f) * SkScalarPow(luma, SkScalarInvert(SkFloatToScalar(2.4f)))
           - SkFloatToScalar(0.055f);
}

SkGammaLuminance::SkGammaLuminance(SkScalar gamma)
    : fGamma(gamma)
    , fGammaInverse(SkScalarInvert(gamma)) {
}

float SkGammaLuminance::toLuma(SkScalar luminance) const {
    return SkScalarPow(luminance, fGamma);
}

float SkGammaLuminance::fromLuma(SkScalar luma) const {
    return SkScalarPow(luma, fGammaInverse);
}

static float apply_contrast(float srca, float contrast) {
    return srca + ((1.0f - srca) * contrast * srca);
}

void SkTMaskGamma_build_correcting_lut(uint8_t table[256], U8CPU srcI, SkScalar contrast,
                                const SkColorSpaceLuminance& srcConvert,
                                const SkColorSpaceLuminance& dstConvert) {
    const float src = (float)srcI / 255.0f;
    const float linSrc = srcConvert.toLuma(src);
    //Guess at the dst.
    const float linDst = 1.0f - linSrc;
    const float dst = dstConvert.fromLuma(linDst);

    //Contrast value tapers off to 0 as the src luminance becomes white
    const float adjustedContrast = SkScalarToFloat(contrast) * linDst;
    const float step = 1.0f / 255.0f;

    //Remove discontinuity and instability when src is close to dst.
    //The value 1/256 is arbitrary and appears to contain the instability.
    if (fabs(src - dst) < (1.0f / 256.0f)) {
        float rawSrca = 0.0f;
        for (int i = 0; i < 256; ++i, rawSrca += step) {
            float srca = apply_contrast(rawSrca, adjustedContrast);
            table[i] = SkToU8(sk_float_round2int(255.0f * srca));
        }
    } else {
        float rawSrca = 0.0f;
        for (int i = 0; i < 256; ++i, rawSrca += step) {
            float srca = apply_contrast(rawSrca, adjustedContrast);
            SkASSERT(srca <= 1.0f);
            float dsta = 1.0f - srca;

            //Calculate the output we want.
            float linOut = (linSrc * srca + dsta * linDst);
            SkASSERT(linOut <= 1.0f);
            float out = dstConvert.fromLuma(linOut);

            //Undo what the blit blend will do.
            float result = (out - dst) / (src - dst);
            SkASSERT(sk_float_round2int(255.0f * result) <= 255);

            table[i] = SkToU8(sk_float_round2int(255.0f * result));
        }
    }
}
