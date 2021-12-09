/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTestUtils_DEFINED
#define GrTestUtils_DEFINED

#include "include/core/SkTypes.h"

#if GR_TEST_UTILS

#include "include/core/SkStrokeRec.h"
#include "include/private/SkMacros.h"
#include "include/private/SkTemplates.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkPathEffectBase.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrFPArgs.h"
#include "src/gpu/GrSamplerState.h"
#include "src/shaders/SkShaderBase.h"

class GrColorInfo;
class GrColorSpaceXform;
class GrProcessorTestData;
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
void TestWrapModes(SkRandom*, GrSamplerState::WrapMode[2]);
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

GrColor RandomColor(SkRandom* random);
uint8_t RandomCoverage(SkRandom* random);

class TestAsFPArgs {
public:
    TestAsFPArgs(GrProcessorTestData*);
    ~TestAsFPArgs();
    const GrFPArgs& args() const { return fArgs; }

private:
    SkMatrixProvider fMatrixProvider;
    std::unique_ptr<GrColorInfo> fColorInfoStorage;
    GrFPArgs fArgs;
};

// We have a simplified dash path effect here to avoid relying on SkDashPathEffect which
// is in the optional build target effects.
class TestDashPathEffect : public SkPathEffectBase {
public:
    static sk_sp<SkPathEffect> Make(const SkScalar* intervals, int count, SkScalar phase) {
        return sk_sp<SkPathEffect>(new TestDashPathEffect(intervals, count, phase));
    }

    Factory getFactory() const override { return nullptr; }
    const char* getTypeName() const override { return nullptr; }

protected:
    bool onFilterPath(SkPath* dst, const SkPath&, SkStrokeRec* , const SkRect*,
                      const SkMatrix&) const override;
    DashType onAsADash(DashInfo* info) const override;

private:
    TestDashPathEffect(const SkScalar* intervals, int count, SkScalar phase);

    bool computeFastBounds(SkRect* bounds) const override { return true; }

    int                     fCount;
    SkAutoTArray<SkScalar>  fIntervals;
    SkScalar                fPhase;
    SkScalar                fInitialDashLength;
    int                     fInitialDashIndex;
    SkScalar                fIntervalLength;
};

}  // namespace GrTest

#endif
#endif
