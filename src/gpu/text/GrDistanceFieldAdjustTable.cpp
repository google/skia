/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/text/GrDistanceFieldAdjustTable.h"

#include "src/core/SkScalerContext.h"

SkDEBUGCODE(static const int kExpectedDistanceAdjustTableSize = 8;)

SkScalar* build_distance_adjust_table(SkScalar paintGamma, SkScalar deviceGamma) {
    // This is used for an approximation of the mask gamma hack, used by raster and bitmap
    // text. The mask gamma hack is based off of guessing what the blend color is going to
    // be, and adjusting the mask so that when run through the linear blend will
    // produce the value closest to the desired result. However, in practice this means
    // that the 'adjusted' mask is just increasing or decreasing the coverage of
    // the mask depending on what it is thought it will blit against. For black (on
    // assumed white) this means that coverages are decreased (on a curve). For white (on
    // assumed black) this means that coverages are increased (on a a curve). At
    // middle (perceptual) gray (which could be blit against anything) the coverages
    // remain the same.
    //
    // The idea here is that instead of determining the initial (real) coverage and
    // then adjusting that coverage, we determine an adjusted coverage directly by
    // essentially manipulating the geometry (in this case, the distance to the glyph
    // edge). So for black (on assumed white) this thins a bit; for white (on
    // assumed black) this fake bolds the geometry a bit.
    //
    // The distance adjustment is calculated by determining the actual coverage value which
    // when fed into in the mask gamma table gives us an 'adjusted coverage' value of 0.5. This
    // actual coverage value (assuming it's between 0 and 1) corresponds to a distance from the
    // actual edge. So by subtracting this distance adjustment and computing without the
    // the coverage adjustment we should get 0.5 coverage at the same point.
    //
    // This has several implications:
    //     For non-gray lcd smoothed text, each subpixel essentially is using a
    //     slightly different geometry.
    //
    //     For black (on assumed white) this may not cover some pixels which were
    //     previously covered; however those pixels would have been only slightly
    //     covered and that slight coverage would have been decreased anyway. Also, some pixels
    //     which were previously fully covered may no longer be fully covered.
    //
    //     For white (on assumed black) this may cover some pixels which weren't
    //     previously covered at all.

    int width, height;
    size_t size;

#ifdef SK_GAMMA_CONTRAST
    SkScalar contrast = SK_GAMMA_CONTRAST;
#else
    SkScalar contrast = 0.5f;
#endif

    size = SkScalerContext::GetGammaLUTSize(contrast, paintGamma, deviceGamma,
        &width, &height);

    SkASSERT(kExpectedDistanceAdjustTableSize == height);
    SkScalar* table = new SkScalar[height];

    SkAutoTArray<uint8_t> data((int)size);
    if (!SkScalerContext::GetGammaLUTData(contrast, paintGamma, deviceGamma, data.get())) {
        // if no valid data is available simply do no adjustment
        for (int row = 0; row < height; ++row) {
            table[row] = 0;
        }
        return table;
    }

    // find the inverse points where we cross 0.5
    // binsearch might be better, but we only need to do this once on creation
    for (int row = 0; row < height; ++row) {
        uint8_t* rowPtr = data.get() + row*width;
        for (int col = 0; col < width - 1; ++col) {
            if (rowPtr[col] <= 127 && rowPtr[col + 1] >= 128) {
                // compute point where a mask value will give us a result of 0.5
                float interp = (127.5f - rowPtr[col]) / (rowPtr[col + 1] - rowPtr[col]);
                float borderAlpha = (col + interp) / 255.f;

                // compute t value for that alpha
                // this is an approximate inverse for smoothstep()
                float t = borderAlpha*(borderAlpha*(4.0f*borderAlpha - 6.0f) + 5.0f) / 3.0f;

                // compute distance which gives us that t value
                const float kDistanceFieldAAFactor = 0.65f; // should match SK_DistanceFieldAAFactor
                float d = 2.0f*kDistanceFieldAAFactor*t - kDistanceFieldAAFactor;

                table[row] = d;
                break;
            }
        }
    }

    return table;
}

const GrDistanceFieldAdjustTable* GrDistanceFieldAdjustTable::Get() {
    static const GrDistanceFieldAdjustTable* dfat = new GrDistanceFieldAdjustTable;
    return dfat;
}

GrDistanceFieldAdjustTable::GrDistanceFieldAdjustTable() {
    fTable = build_distance_adjust_table(SK_GAMMA_EXPONENT, SK_GAMMA_EXPONENT);
    fGammaCorrectTable = build_distance_adjust_table(SK_Scalar1, SK_Scalar1);
}
