/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrTestUtils.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRRect.h"
#include "src/core/SkRectPriv.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrProcessorUnitTest.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/utils/SkDashPathPriv.h"

#if defined(GR_TEST_UTILS)

static const SkMatrix& test_matrix(SkRandom* random,
                                   bool includeNonPerspective,
                                   bool includePerspective) {
    static SkMatrix gMatrices[5];
    static const int kPerspectiveCount = 1;
    static bool gOnce;
    if (!gOnce) {
        gOnce = true;
        gMatrices[0].reset();
        gMatrices[1].setTranslate(SkIntToScalar(-100), SkIntToScalar(100));
        gMatrices[2].setRotate(SkIntToScalar(17));
        gMatrices[3].setRotate(SkIntToScalar(185));
        gMatrices[3].postTranslate(SkIntToScalar(66), SkIntToScalar(-33));
        gMatrices[3].postScale(SkIntToScalar(2), SK_ScalarHalf);

        // Perspective matrices
        gMatrices[4].setRotate(SkIntToScalar(215));
        gMatrices[4].set(SkMatrix::kMPersp0, 0.00013f);
        gMatrices[4].set(SkMatrix::kMPersp1, -0.000039f);
    }

    uint32_t count = static_cast<uint32_t>(std::size(gMatrices));
    if (includeNonPerspective && includePerspective) {
        return gMatrices[random->nextULessThan(count)];
    } else if (!includeNonPerspective) {
        return gMatrices[count - 1 - random->nextULessThan(kPerspectiveCount)];
    } else {
        SkASSERT(includeNonPerspective && !includePerspective);
        return gMatrices[random->nextULessThan(count - kPerspectiveCount)];
    }
}

namespace GrTest {
const SkMatrix& TestMatrix(SkRandom* random) { return test_matrix(random, true, true); }

const SkMatrix& TestMatrixPreservesRightAngles(SkRandom* random) {
    static SkMatrix gMatrices[5];
    static bool gOnce;
    if (!gOnce) {
        gOnce = true;
        // identity
        gMatrices[0].reset();
        // translation
        gMatrices[1].setTranslate(SkIntToScalar(-100), SkIntToScalar(100));
        // scale
        gMatrices[2].setScale(SkIntToScalar(17), SkIntToScalar(17));
        // scale + translation
        gMatrices[3].setScale(SkIntToScalar(-17), SkIntToScalar(-17));
        gMatrices[3].postTranslate(SkIntToScalar(66), SkIntToScalar(-33));
        // orthogonal basis vectors
        gMatrices[4].reset();
        gMatrices[4].setScale(SkIntToScalar(-1), SkIntToScalar(-1));
        gMatrices[4].setRotate(47);

        for (size_t i = 0; i < std::size(gMatrices); i++) {
            SkASSERT(gMatrices[i].preservesRightAngles());
        }
    }
    return gMatrices[random->nextULessThan(static_cast<uint32_t>(std::size(gMatrices)))];
}

const SkMatrix& TestMatrixRectStaysRect(SkRandom* random) {
    static SkMatrix gMatrices[6];
    static bool gOnce;
    if (!gOnce) {
        gOnce = true;
        // identity
        gMatrices[0].reset();
        // translation
        gMatrices[1].setTranslate(SkIntToScalar(-100), SkIntToScalar(100));
        // scale
        gMatrices[2].setScale(SkIntToScalar(17), SkIntToScalar(17));
        // scale + translation
        gMatrices[3].setScale(SkIntToScalar(-17), SkIntToScalar(-17));
        gMatrices[3].postTranslate(SkIntToScalar(66), SkIntToScalar(-33));
        // reflection
        gMatrices[4].setScale(SkIntToScalar(-1), SkIntToScalar(-1));
        // 90 degress rotation
        gMatrices[5].setRotate(90);

        for (size_t i = 0; i < std::size(gMatrices); i++) {
            SkASSERT(gMatrices[i].rectStaysRect());
        }
    }
    return gMatrices[random->nextULessThan(static_cast<uint32_t>(std::size(gMatrices)))];
}

const SkMatrix& TestMatrixInvertible(SkRandom* random) { return test_matrix(random, true, false); }
const SkMatrix& TestMatrixPerspective(SkRandom* random) { return test_matrix(random, false, true); }

void TestWrapModes(SkRandom* random, GrSamplerState::WrapMode wrapModes[2]) {
    static const GrSamplerState::WrapMode kWrapModes[] = {
            GrSamplerState::WrapMode::kClamp,
            GrSamplerState::WrapMode::kRepeat,
            GrSamplerState::WrapMode::kMirrorRepeat,
    };
    wrapModes[0] = kWrapModes[random->nextULessThan(std::size(kWrapModes))];
    wrapModes[1] = kWrapModes[random->nextULessThan(std::size(kWrapModes))];
}
const SkRect& TestRect(SkRandom* random) {
    static SkRect gRects[7];
    static bool gOnce;
    if (!gOnce) {
        gOnce = true;
        gRects[0] = SkRect::MakeWH(1.f, 1.f);
        gRects[1] = SkRect::MakeWH(1.0f, 256.0f);
        gRects[2] = SkRect::MakeWH(256.0f, 1.0f);
        gRects[3] = SkRectPriv::MakeLargest();
        gRects[4] = SkRect::MakeLTRB(-65535.0f, -65535.0f, 65535.0f, 65535.0f);
        gRects[5] = SkRect::MakeLTRB(-10.0f, -10.0f, 10.0f, 10.0f);
    }
    return gRects[random->nextULessThan(static_cast<uint32_t>(std::size(gRects)))];
}

// Just some simple rects for code which expects its input very sanitized
const SkRect& TestSquare(SkRandom* random) {
    static SkRect gRects[2];
    static bool gOnce;
    if (!gOnce) {
        gOnce = true;
        gRects[0] = SkRect::MakeWH(128.f, 128.f);
        gRects[1] = SkRect::MakeWH(256.0f, 256.0f);
    }
    return gRects[random->nextULessThan(static_cast<uint32_t>(std::size(gRects)))];
}

const SkRRect& TestRRectSimple(SkRandom* random) {
    static SkRRect gRRect[2];
    static bool gOnce;
    if (!gOnce) {
        gOnce = true;
        SkRect rectangle = SkRect::MakeWH(10.f, 20.f);
        // true round rect with circular corners
        gRRect[0].setRectXY(rectangle, 1.f, 1.f);
        // true round rect with elliptical corners
        gRRect[1].setRectXY(rectangle, 2.0f, 1.0f);

        for (size_t i = 0; i < std::size(gRRect); i++) {
            SkASSERT(gRRect[i].isSimple());
        }
    }
    return gRRect[random->nextULessThan(static_cast<uint32_t>(std::size(gRRect)))];
}

const SkPath& TestPath(SkRandom* random) {
    static SkPath gPath[7];
    static bool gOnce;
    if (!gOnce) {
        gOnce = true;
        // line
        gPath[0] = SkPathBuilder().moveTo(0.f, 0.f)
                                  .lineTo(10.f, 10.f)
                                  .detach();
        // quad
        gPath[1] = SkPathBuilder().moveTo(0.f, 0.f)
                                  .quadTo(10.f, 10.f, 20.f, 20.f)
                                  .detach();
        // conic
        gPath[2] = SkPathBuilder().moveTo(0.f, 0.f)
                                  .conicTo(10.f, 10.f, 20.f, 20.f, 1.f)
                                  .detach();
        // cubic
        gPath[3] = SkPathBuilder().moveTo(0.f, 0.f)
                                  .cubicTo(10.f, 10.f, 20.f, 20.f, 30.f, 30.f)
                                  .detach();
        // all three
        gPath[4] = SkPathBuilder().moveTo(0.f, 0.f)
                                  .lineTo(10.f, 10.f)
                                  .quadTo(10.f, 10.f, 20.f, 20.f)
                                  .conicTo(10.f, 10.f, 20.f, 20.f, 1.f)
                                  .cubicTo(10.f, 10.f, 20.f, 20.f, 30.f, 30.f)
                                  .detach();
        // convex
        gPath[5] = SkPathBuilder().moveTo(0.0f, 0.0f)
                                  .lineTo(10.0f, 0.0f)
                                  .lineTo(10.0f, 10.0f)
                                  .lineTo(0.0f, 10.0f)
                                  .close()
                                  .detach();
        // concave
        gPath[6] = SkPathBuilder().moveTo(0.0f, 0.0f)
                                  .lineTo(5.0f, 5.0f)
                                  .lineTo(10.0f, 0.0f)
                                  .lineTo(10.0f, 10.0f)
                                  .lineTo(0.0f, 10.0f)
                                  .close()
                                  .detach();
    }

    return gPath[random->nextULessThan(static_cast<uint32_t>(std::size(gPath)))];
}

const SkPath& TestPathConvex(SkRandom* random) {
    static SkPath gPath[3];
    static bool gOnce;
    if (!gOnce) {
        gOnce = true;
        // narrow rect
        gPath[0] = SkPath::Polygon({{-1.5f, -50.0f},
                                    {-1.5f, -50.0f},
                                    { 1.5f, -50.0f},
                                    { 1.5f,  50.0f},
                                    {-1.5f,  50.0f}}, false);
        // degenerate
        gPath[1] = SkPath::Polygon({{-0.025f, -0.025f},
                                    {-0.025f, -0.025f},
                                    { 0.025f, -0.025f},
                                    { 0.025f,  0.025f},
                                    {-0.025f,  0.025f}}, false);
        // clipped triangle
        gPath[2] = SkPath::Polygon({{-10.0f, -50.0f},
                                    {-10.0f, -50.0f},
                                    { 10.0f, -50.0f},
                                    { 50.0f,  31.0f},
                                    { 40.0f,  50.0f},
                                    {-40.0f,  50.0f},
                                    {-50.0f,  31.0f}}, false);

        for (size_t i = 0; i < std::size(gPath); i++) {
            SkASSERT(gPath[i].isConvex());
        }
    }

    return gPath[random->nextULessThan(static_cast<uint32_t>(std::size(gPath)))];
}

static void randomize_stroke_rec(SkStrokeRec* rec, SkRandom* random) {
    bool strokeAndFill = random->nextBool();
    SkScalar strokeWidth = random->nextBool() ? 0.f : 1.f;
    rec->setStrokeStyle(strokeWidth, strokeAndFill);

    SkPaint::Cap cap = SkPaint::Cap(random->nextULessThan(SkPaint::kCapCount));
    SkPaint::Join join = SkPaint::Join(random->nextULessThan(SkPaint::kJoinCount));
    SkScalar miterLimit = random->nextRangeScalar(1.f, 5.f);
    rec->setStrokeParams(cap, join, miterLimit);
}

SkStrokeRec TestStrokeRec(SkRandom* random) {
    SkStrokeRec::InitStyle style =
            SkStrokeRec::InitStyle(random->nextULessThan(SkStrokeRec::kFill_InitStyle + 1));
    SkStrokeRec rec(style);
    randomize_stroke_rec(&rec, random);
    return rec;
}

void TestStyle(SkRandom* random, GrStyle* style) {
    SkStrokeRec::InitStyle initStyle =
            SkStrokeRec::InitStyle(random->nextULessThan(SkStrokeRec::kFill_InitStyle + 1));
    SkStrokeRec stroke(initStyle);
    randomize_stroke_rec(&stroke, random);
    sk_sp<SkPathEffect> pe;
    if (random->nextBool()) {
        int cnt = random->nextRangeU(1, 50) * 2;
        std::unique_ptr<SkScalar[]> intervals(new SkScalar[cnt]);
        SkScalar sum = 0;
        for (int i = 0; i < cnt; i++) {
            intervals[i] = random->nextRangeScalar(SkDoubleToScalar(0.01),
                                                   SkDoubleToScalar(10.0));
            sum += intervals[i];
        }
        SkScalar phase = random->nextRangeScalar(0, sum);
        pe = TestDashPathEffect::Make(intervals.get(), cnt, phase);
    }
    *style = GrStyle(stroke, std::move(pe));
}

TestDashPathEffect::TestDashPathEffect(const SkScalar* intervals, int count, SkScalar phase) {
    fCount = count;
    fIntervals.reset(count);
    memcpy(fIntervals.get(), intervals, count * sizeof(SkScalar));
    SkDashPath::CalcDashParameters(phase, intervals, count, &fInitialDashLength,
                                   &fInitialDashIndex, &fIntervalLength, &fPhase);
}

bool TestDashPathEffect::onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                                      const SkRect* cullRect, const SkMatrix&) const {
    return SkDashPath::InternalFilter(dst, src, rec, cullRect, fIntervals.get(), fCount,
                                      fInitialDashLength, fInitialDashIndex, fIntervalLength,
                                      fPhase);
}

SkPathEffect::DashType TestDashPathEffect::onAsADash(DashInfo* info) const {
    if (info) {
        if (info->fCount >= fCount && info->fIntervals) {
            memcpy(info->fIntervals, fIntervals.get(), fCount * sizeof(SkScalar));
        }
        info->fCount = fCount;
        info->fPhase = fPhase;
    }
    return kDash_DashType;
}

sk_sp<SkColorSpace> TestColorSpace(SkRandom* random) {
    static sk_sp<SkColorSpace> gColorSpaces[3];
    static bool gOnce;
    if (!gOnce) {
        gOnce = true;
        // No color space (legacy mode)
        gColorSpaces[0] = nullptr;
        // sRGB or color-spin sRGB
        gColorSpaces[1] = SkColorSpace::MakeSRGB();
        gColorSpaces[2] = SkColorSpace::MakeSRGB()->makeColorSpin();
    }
    return gColorSpaces[random->nextULessThan(static_cast<uint32_t>(std::size(gColorSpaces)))];
}

sk_sp<GrColorSpaceXform> TestColorXform(SkRandom* random) {
    // TODO: Add many more kinds of xforms here
    static sk_sp<GrColorSpaceXform> gXforms[3];
    static bool gOnce;
    if (!gOnce) {
        gOnce = true;
        sk_sp<SkColorSpace> srgb = SkColorSpace::MakeSRGB();
        sk_sp<SkColorSpace> spin = SkColorSpace::MakeSRGB()->makeColorSpin();
        // No gamut change
        gXforms[0] = nullptr;
        gXforms[1] = GrColorSpaceXform::Make(srgb.get(), kPremul_SkAlphaType,
                                             spin.get(), kPremul_SkAlphaType);
        gXforms[2] = GrColorSpaceXform::Make(spin.get(), kPremul_SkAlphaType,
                                             srgb.get(), kPremul_SkAlphaType);
    }
    return gXforms[random->nextULessThan(static_cast<uint32_t>(std::size(gXforms)))];
}

TestAsFPArgs::TestAsFPArgs(GrProcessorTestData* d)
        : fColorInfoStorage(std::make_unique<GrColorInfo>(
                  GrColorType::kRGBA_8888, kPremul_SkAlphaType, TestColorSpace(d->fRandom)))
        , fArgs(d->context(), fColorInfoStorage.get(), fSurfaceProps, GrFPArgs::Scope::kDefault) {}

TestAsFPArgs::~TestAsFPArgs() {}

GrColor RandomColor(SkRandom* random) {
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
    return color;
}

uint8_t RandomCoverage(SkRandom* random) {
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

}  // namespace GrTest

#endif
