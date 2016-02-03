/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTestUtils_DEFINED
#define GrTestUtils_DEFINED

#include "SkTypes.h"

#ifdef GR_TEST_UTILS

#include "GrColor.h"
#include "SkRandom.h"
#include "SkStrokeRec.h"

class GrStrokeInfo;
class SkMatrix;
class SkPath;
class SkRRect;
struct SkRect;

namespace GrTest {
/**
 * A helper for use in Test functions.
 */
const SkMatrix& TestMatrix(SkRandom*);
const SkMatrix& TestMatrixPreservesRightAngles(SkRandom*);
const SkMatrix& TestMatrixRectStaysRect(SkRandom*);
const SkMatrix& TestMatrixInvertible(SkRandom*);
const SkRect& TestRect(SkRandom*);
const SkRect& TestSquare(SkRandom*);
const SkRRect& TestRRectSimple(SkRandom*);
const SkPath& TestPath(SkRandom*);
const SkPath& TestPathConvex(SkRandom*);
SkStrokeRec TestStrokeRec(SkRandom*);
GrStrokeInfo TestStrokeInfo(SkRandom*);

}

static inline GrColor GrRandomColor(SkRandom* random) {
    // There are only a few cases of random colors which interest us
    enum ColorMode {
        kAllOnes_ColorMode,
        kAllZeros_ColorMode,
        kAlphaOne_ColorMode,
        kRandom_ColorMode,
        kLast_ColorMode = kRandom_ColorMode
    };

    ColorMode colorMode = ColorMode(random->nextULessThan(kLast_ColorMode + 1));
    GrColor color;
    switch (colorMode) {
        case kAllOnes_ColorMode:
            color = GrColorPackRGBA(0xFF, 0xFF, 0xFF, 0xFF);
            break;
        case kAllZeros_ColorMode:
            color = GrColorPackRGBA(0, 0, 0, 0);
            break;
        case kAlphaOne_ColorMode:
            color = GrColorPackRGBA(random->nextULessThan(256),
                                    random->nextULessThan(256),
                                    random->nextULessThan(256),
                                    0xFF);
            break;
        case kRandom_ColorMode: {
                uint8_t alpha = random->nextULessThan(256);
                color = GrColorPackRGBA(random->nextRangeU(0, alpha),
                                        random->nextRangeU(0, alpha),
                                        random->nextRangeU(0, alpha),
                                        alpha);
            break;
        }
    }
    GrColorIsPMAssert(color);
    return color;
}

static inline uint8_t GrRandomCoverage(SkRandom* random) {
    enum CoverageMode {
        kZero_CoverageMode,
        kAllOnes_CoverageMode,
        kRandom_CoverageMode,
        kLast_CoverageMode = kRandom_CoverageMode
    };

    CoverageMode colorMode = CoverageMode(random->nextULessThan(kLast_CoverageMode + 1));
    uint8_t coverage;
    switch (colorMode) {
        case kZero_CoverageMode:
            coverage = 0;
            break;
        case kAllOnes_CoverageMode:
            coverage = 0xff;
            break;
        case kRandom_CoverageMode:
            coverage = random->nextULessThan(256);
            break;
    }
    return coverage;
}

#endif
#endif
