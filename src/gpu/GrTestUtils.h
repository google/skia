/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTestUtils_DEFINED
#define GrTestUtils_DEFINED

#include "SkTypes.h"

#if GR_TEST_UTILS

#include "GrColor.h"
#include "GrColorSpaceXform.h"
#include "SkPathEffect.h"
#include "SkRandom.h"
#include "SkShaderBase.h"
#include "SkStrokeRec.h"
#include "../private/SkTemplates.h"

struct GrProcessorTestData;
class GrStyle;
class SkMatrix;
class SkPath;
class SkRRect;
struct SkRect;

namespace GrTest {
/**
 * Helpers for use in Test functions.
 */
const SkMatrix& TestMatrix(SkRandom*);
const SkMatrix& TestMatrixPreservesRightAngles(SkRandom*);
const SkMatrix& TestMatrixRectStaysRect(SkRandom*);
const SkMatrix& TestMatrixInvertible(SkRandom*);
const SkMatrix& TestMatrixPerspective(SkRandom*);
const SkRect& TestRect(SkRandom*);
const SkRect& TestSquare(SkRandom*);
const SkRRect& TestRRectSimple(SkRandom*);
const SkPath& TestPath(SkRandom*);
const SkPath& TestPathConvex(SkRandom*);
SkStrokeRec TestStrokeRec(SkRandom*);
/** Creates styles with dash path effects and null path effects */
void TestStyle(SkRandom*, GrStyle*);
sk_sp<SkColorSpace> TestColorSpace(SkRandom*);
sk_sp<GrColorSpaceXform> TestColorXform(SkRandom*);

class TestAsFPArgs {
public:
    TestAsFPArgs(GrProcessorTestData*);
    const SkShaderBase::AsFPArgs& args() const { return fArgs; }

private:
    SkShaderBase::AsFPArgs fArgs;
    SkMatrix fViewMatrixStorage;
    sk_sp<SkColorSpace> fColorSpaceStorage;
};

// We have a simplified dash path effect here to avoid relying on SkDashPathEffect which
// is in the optional build target effects.
class TestDashPathEffect : public SkPathEffect {
public:
    static sk_sp<SkPathEffect> Make(const SkScalar* intervals, int count, SkScalar phase) {
        return sk_sp<SkPathEffect>(new TestDashPathEffect(intervals, count, phase));
    }

    bool filterPath(SkPath* dst, const SkPath&, SkStrokeRec* , const SkRect*) const override;
    DashType asADash(DashInfo* info) const override;
    Factory getFactory() const override { return nullptr; }
    void toString(SkString*) const override {}

private:
    TestDashPathEffect(const SkScalar* intervals, int count, SkScalar phase);

    int                     fCount;
    SkAutoTArray<SkScalar>  fIntervals;
    SkScalar                fPhase;
    SkScalar                fInitialDashLength;
    int                     fInitialDashIndex;
    SkScalar                fIntervalLength;
};

}  // namespace GrTest

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
    GrColor color SK_INIT_TO_AVOID_WARNING;
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
    uint8_t coverage SK_INIT_TO_AVOID_WARNING;
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
