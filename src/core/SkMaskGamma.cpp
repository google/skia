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

class SkLinearColorSpaceLuminance : public SkColorSpaceLuminance {
    virtual SkScalar toLuma(SkScalar SkDEBUGCODE(gamma), SkScalar luminance) const SK_OVERRIDE {
        SkASSERT(SK_Scalar1 == gamma);
        return luminance;
    }
    virtual SkScalar fromLuma(SkScalar SkDEBUGCODE(gamma), SkScalar luma) const SK_OVERRIDE {
        SkASSERT(SK_Scalar1 == gamma);
        return luma;
    }
};

class SkGammaColorSpaceLuminance : public SkColorSpaceLuminance {
    virtual SkScalar toLuma(SkScalar gamma, SkScalar luminance) const SK_OVERRIDE {
        return SkScalarPow(luminance, gamma);
    }
    virtual SkScalar fromLuma(SkScalar gamma, SkScalar luma) const SK_OVERRIDE {
        return SkScalarPow(luma, SkScalarInvert(gamma));
    }
};

class SkSRGBColorSpaceLuminance : public SkColorSpaceLuminance {
    virtual SkScalar toLuma(SkScalar SkDEBUGCODE(gamma), SkScalar luminance) const SK_OVERRIDE {
        SkASSERT(0 == gamma);
        //The magic numbers are derived from the sRGB specification.
        //See http://www.color.org/chardata/rgb/srgb.xalter .
        if (luminance <= SkFloatToScalar(0.04045f)) {
            return luminance / SkFloatToScalar(12.92f);
        }
        return SkScalarPow((luminance + SkFloatToScalar(0.055f)) / SkFloatToScalar(1.055f),
                        SkFloatToScalar(2.4f));
    }
    virtual SkScalar fromLuma(SkScalar SkDEBUGCODE(gamma), SkScalar luma) const SK_OVERRIDE {
        SkASSERT(0 == gamma);
        //The magic numbers are derived from the sRGB specification.
        //See http://www.color.org/chardata/rgb/srgb.xalter .
        if (luma <= SkFloatToScalar(0.0031308f)) {
            return luma * SkFloatToScalar(12.92f);
        }
        return SkFloatToScalar(1.055f) * SkScalarPow(luma, SkScalarInvert(SkFloatToScalar(2.4f)))
               - SkFloatToScalar(0.055f);
    }
};

/*static*/ const SkColorSpaceLuminance& SkColorSpaceLuminance::Fetch(SkScalar gamma) {
    static SkLinearColorSpaceLuminance gSkLinearColorSpaceLuminance;
    static SkGammaColorSpaceLuminance gSkGammaColorSpaceLuminance;
    static SkSRGBColorSpaceLuminance gSkSRGBColorSpaceLuminance;

    if (0 == gamma) {
        return gSkSRGBColorSpaceLuminance;
    } else if (SK_Scalar1 == gamma) {
        return gSkLinearColorSpaceLuminance;
    } else {
        return gSkGammaColorSpaceLuminance;
    }
}

static float apply_contrast(float srca, float contrast) {
    return srca + ((1.0f - srca) * contrast * srca);
}

void SkTMaskGamma_build_correcting_lut(uint8_t table[256], U8CPU srcI, SkScalar contrast,
                                       const SkColorSpaceLuminance& srcConvert, SkScalar srcGamma,
                                       const SkColorSpaceLuminance& dstConvert, SkScalar dstGamma) {
    const float src = (float)srcI / 255.0f;
    const float linSrc = srcConvert.toLuma(srcGamma, src);
    //Guess at the dst. The perceptual inverse provides smaller visual
    //discontinuities when slight changes to desaturated colors cause a channel
    //to map to a different correcting lut with neighboring srcI.
    //See https://code.google.com/p/chromium/issues/detail?id=141425#c59 .
    const float dst = 1.0f - src;
    const float linDst = dstConvert.toLuma(dstGamma, dst);

    //Contrast value tapers off to 0 as the src luminance becomes white
    const float adjustedContrast = SkScalarToFloat(contrast) * linDst;

    //Remove discontinuity and instability when src is close to dst.
    //The value 1/256 is arbitrary and appears to contain the instability.
    if (fabs(src - dst) < (1.0f / 256.0f)) {
        float ii = 0.0f;
        for (int i = 0; i < 256; ++i, ii += 1.0f) {
            float rawSrca = ii / 255.0f;
            float srca = apply_contrast(rawSrca, adjustedContrast);
            table[i] = SkToU8(sk_float_round2int(255.0f * srca));
        }
    } else {
        // Avoid slow int to float conversion.
        float ii = 0.0f;
        for (int i = 0; i < 256; ++i, ii += 1.0f) {
            // 'rawSrca += 1.0f / 255.0f' and even
            // 'rawSrca = i * (1.0f / 255.0f)' can add up to more than 1.0f.
            // When this happens the table[255] == 0x0 instead of 0xff.
            // See http://code.google.com/p/chromium/issues/detail?id=146466
            float rawSrca = ii / 255.0f;
            float srca = apply_contrast(rawSrca, adjustedContrast);
            SkASSERT(srca <= 1.0f);
            float dsta = 1.0f - srca;

            //Calculate the output we want.
            float linOut = (linSrc * srca + dsta * linDst);
            SkASSERT(linOut <= 1.0f);
            float out = dstConvert.fromLuma(dstGamma, linOut);

            //Undo what the blit blend will do.
            float result = (out - dst) / (src - dst);
            SkASSERT(sk_float_round2int(255.0f * result) <= 255);

            table[i] = SkToU8(sk_float_round2int(255.0f * result));
        }
    }
}
