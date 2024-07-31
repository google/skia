/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/effects/GrRRectEffect.h"

#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkRandom.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkRRectPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrProcessorUnitTest.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/effects/GrOvalEffect.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

#include <algorithm>
#include <cstdint>
#include <tuple>
#include <utility>

// The effects defined here only handle rrect radii >= kRadiusMin.
static const SkScalar kRadiusMin = SK_ScalarHalf;

//////////////////////////////////////////////////////////////////////////////

namespace {
class CircularRRectEffect : public GrFragmentProcessor {
public:
    enum CornerFlags {
        kTopLeft_CornerFlag     = (1 << SkRRect::kUpperLeft_Corner),
        kTopRight_CornerFlag    = (1 << SkRRect::kUpperRight_Corner),
        kBottomRight_CornerFlag = (1 << SkRRect::kLowerRight_Corner),
        kBottomLeft_CornerFlag  = (1 << SkRRect::kLowerLeft_Corner),

        kLeft_CornerFlags   = kTopLeft_CornerFlag    | kBottomLeft_CornerFlag,
        kTop_CornerFlags    = kTopLeft_CornerFlag    | kTopRight_CornerFlag,
        kRight_CornerFlags  = kTopRight_CornerFlag   | kBottomRight_CornerFlag,
        kBottom_CornerFlags = kBottomLeft_CornerFlag | kBottomRight_CornerFlag,

        kAll_CornerFlags = kTopLeft_CornerFlag    | kTopRight_CornerFlag |
                           kBottomLeft_CornerFlag | kBottomRight_CornerFlag,

        kNone_CornerFlags = 0
    };

    // The flags are used to indicate which corners are circluar (unflagged corners are assumed to
    // be square).
    static GrFPResult Make(std::unique_ptr<GrFragmentProcessor>, GrClipEdgeType,
                           uint32_t circularCornerFlags, const SkRRect&);

    ~CircularRRectEffect() override {}

    const char* name() const override { return "CircularRRect"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override;

private:
    class Impl;

    CircularRRectEffect(std::unique_ptr<GrFragmentProcessor> inputFP,
                        GrClipEdgeType, uint32_t circularCornerFlags, const SkRRect&);
    CircularRRectEffect(const CircularRRectEffect& that);

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override;

    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override;

    SkRRect           fRRect;
    GrClipEdgeType    fEdgeType;
    uint32_t          fCircularCornerFlags;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    using INHERITED = GrFragmentProcessor;
};
}  // anonymous namespace

GrFPResult CircularRRectEffect::Make(std::unique_ptr<GrFragmentProcessor> inputFP,
                                     GrClipEdgeType edgeType,
                                     uint32_t circularCornerFlags, const SkRRect& rrect) {
    if (GrClipEdgeType::kFillAA != edgeType && GrClipEdgeType::kInverseFillAA != edgeType) {
        return GrFPFailure(std::move(inputFP));
    }
    return GrFPSuccess(std::unique_ptr<GrFragmentProcessor>(
                new CircularRRectEffect(std::move(inputFP), edgeType, circularCornerFlags, rrect)));
}

CircularRRectEffect::CircularRRectEffect(std::unique_ptr<GrFragmentProcessor> inputFP,
                                         GrClipEdgeType edgeType,
                                         uint32_t circularCornerFlags,
                                         const SkRRect& rrect)
        : INHERITED(kCircularRRectEffect_ClassID,
                    ProcessorOptimizationFlags(inputFP.get()) &
                            kCompatibleWithCoverageAsAlpha_OptimizationFlag)
        , fRRect(rrect)
        , fEdgeType(edgeType)
        , fCircularCornerFlags(circularCornerFlags) {
    this->registerChild(std::move(inputFP));
}

CircularRRectEffect::CircularRRectEffect(const CircularRRectEffect& that)
        : INHERITED(that)
        , fRRect(that.fRRect)
        , fEdgeType(that.fEdgeType)
        , fCircularCornerFlags(that.fCircularCornerFlags) {}

std::unique_ptr<GrFragmentProcessor> CircularRRectEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new CircularRRectEffect(*this));
}

bool CircularRRectEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const CircularRRectEffect& crre = other.cast<CircularRRectEffect>();
    // The corner flags are derived from fRRect, so no need to check them.
    return fEdgeType == crre.fEdgeType && fRRect == crre.fRRect;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(CircularRRectEffect)

#if defined(GR_TEST_UTILS)
std::unique_ptr<GrFragmentProcessor> CircularRRectEffect::TestCreate(GrProcessorTestData* d) {
    SkScalar w = d->fRandom->nextRangeScalar(20.f, 1000.f);
    SkScalar h = d->fRandom->nextRangeScalar(20.f, 1000.f);
    SkScalar r = d->fRandom->nextRangeF(kRadiusMin, 9.f);
    SkRRect rrect;
    rrect.setRectXY(SkRect::MakeWH(w, h), r, r);
    std::unique_ptr<GrFragmentProcessor> fp = d->inputFP();
    bool success;
    do {
        GrClipEdgeType et =
                (GrClipEdgeType)d->fRandom->nextULessThan(kGrClipEdgeTypeCnt);
        std::tie(success, fp) = GrRRectEffect::Make(std::move(fp), et, rrect,
                                                    *d->caps()->shaderCaps());
    } while (!success);
    return fp;
}
#endif

//////////////////////////////////////////////////////////////////////////////

class CircularRRectEffect::Impl : public ProgramImpl {
public:
    void emitCode(EmitArgs&) override;

private:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

    GrGLSLProgramDataManager::UniformHandle fRectUniform;
    GrGLSLProgramDataManager::UniformHandle fRadiusPlusHalfUniform;
    GrGLSLProgramDataManager::UniformHandle fEdgeSelectUniform;
    SkRRect                                 fPrevRRect;
};

void CircularRRectEffect::Impl::emitCode(EmitArgs& args) {
    const CircularRRectEffect& crre = args.fFp.cast<CircularRRectEffect>();
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    const char *rectName;
    const char *radiusPlusHalfName;
    const char *edgeSelectName;
    // The inner rect is the rrect bounds inset by the radius. Its left, top, right, and bottom
    // edges correspond to components x, y, z, and w, respectively. When a side of the rrect has
    // only rectangular corners, that side's value corresponds to the rect edge's value outset by
    // half a pixel.
    fRectUniform = uniformHandler->addUniform(&crre, kFragment_GrShaderFlag, SkSLType::kFloat4,
                                              "rect", &rectName);
    // x is (r + .5) and y is 1/(r + .5)
    fRadiusPlusHalfUniform = uniformHandler->addUniform(&crre, kFragment_GrShaderFlag,
                                                        SkSLType::kHalf2, "radiusPlusHalf",
                                                        &radiusPlusHalfName);
    // Selects whether the edge is adjacent to a rounded corner or not. 1 == rounded, 0 == 90°.
    fEdgeSelectUniform = uniformHandler->addUniform(&crre, kFragment_GrShaderFlag,
                                                    SkSLType::kHalf4, "edgeSelect",
                                                    &edgeSelectName);

    // If we're on a device where float != fp32 then the length calculation could overflow.
    SkString clampedCircleDistance;
    if (!args.fShaderCaps->fFloatIs32Bits) {
        clampedCircleDistance.printf("saturate(%s.x * (1.0 - length(dxy * %s.y)))",
                                     radiusPlusHalfName, radiusPlusHalfName);
    } else {
        clampedCircleDistance.printf("saturate(%s.x - length(dxy))", radiusPlusHalfName);
    }

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    // We copy fragCoord here to avoid re-evaluation of the RTFlip math.
    fragBuilder->codeAppend("float2 fragCoord = sk_FragCoord.xy;");

    // At each quarter-circle corner we compute a vector that is the offset of the fragment position
    // from the circle center. The vector is pinned in x and y to be in the quarter-plane relevant
    // to that corner. This means that points near the interior near the rrect top edge will have
    // a vector that points straight up for both the TL left and TR corners. Computing an
    // alpha from this vector at either the TR or TL corner will give the correct result. Similarly,
    // fragments near the other three edges will get the correct AA. Fragments in the interior of
    // the rrect will have a (0,0) vector at all four corners. So long as the radius > 0.5 they will
    // correctly produce an alpha value of 1 at all four corners. We take the min of all the alphas.
    // The code below is a simplified version of the above that performs maxs on the vector
    // components before computing distances and alpha values so that only one distance computation
    // need be computed to determine the min alpha. The edgeSelect variable is used to
    // only compute these values for edges adjacent to rounded corners, so fragCoords near
    // rectangular corners will be treated as if they're inside the rrect.
    fragBuilder->codeAppendf("float2 radius = float2(%s.x);", radiusPlusHalfName);
    fragBuilder->codeAppendf("float2 dxy0 = %s.LT*((%s.LT + radius) - fragCoord);",
                             edgeSelectName, rectName);
    fragBuilder->codeAppendf("float2 dxy1 = %s.RB*(fragCoord - (%s.RB - radius));",
                             edgeSelectName, rectName);
    fragBuilder->codeAppend("float2 dxy = max(max(dxy0, dxy1), 0.0);");
    fragBuilder->codeAppendf("half circleCornerAlpha = half(%s);", clampedCircleDistance.c_str());

    // We also compute corner values assuming that the rrect is rectangular by computing a separate
    // rect edge alpha for each side, and multiply the computed edge alphas together to
    // get the rectangular corner alpha value. At most only two edge alphas will be < 1, and that
    // will be the corner the fragCoord is closest to. The others will be 1. We use edgeSelect to
    // force any edges next to a rounded corner to have a value of 1 to avoid affecting the
    // circular corner alpha.
    fragBuilder->codeAppendf("half4 rectEdgeAlphas = saturate(half4(fragCoord - %s.LT,"
                                                                   "%s.RB - fragCoord));",
                             rectName, rectName);
    fragBuilder->codeAppendf("rectEdgeAlphas = mix(rectEdgeAlphas, half4(1), %s);",
                             edgeSelectName);

    // The final step is to multiply all of these alphas together.
    fragBuilder->codeAppend("half alpha = circleCornerAlpha * rectEdgeAlphas.L *"
                                         "rectEdgeAlphas.T * rectEdgeAlphas.R *"
                                         "rectEdgeAlphas.B;");

    if (GrClipEdgeType::kInverseFillAA == crre.fEdgeType) {
        fragBuilder->codeAppend("alpha = 1.0 - alpha;");
    }

    SkString inputSample = this->invokeChild(/*childIndex=*/0, args);

    fragBuilder->codeAppendf("return %s * alpha;", inputSample.c_str());
}

void CircularRRectEffect::Impl::onSetData(const GrGLSLProgramDataManager& pdman,
                                          const GrFragmentProcessor& processor) {
    const CircularRRectEffect& crre = processor.cast<CircularRRectEffect>();
    const SkRRect& rrect = crre.fRRect;
    if (rrect != fPrevRRect) {
        SkScalar radius = 0;
        switch (crre.fCircularCornerFlags) {
            case CircularRRectEffect::kAll_CornerFlags:
                SkASSERT(SkRRectPriv::IsSimpleCircular(rrect));
                radius = SkRRectPriv::GetSimpleRadii(rrect).fX;
                SkASSERT(radius >= kRadiusMin);
                pdman.set4f(fEdgeSelectUniform, 1, 1, 1, 1);
                break;
            case CircularRRectEffect::kTopLeft_CornerFlag:
                radius = rrect.radii(SkRRect::kUpperLeft_Corner).fX;
                pdman.set4f(fEdgeSelectUniform, 1, 1, 0, 0);
                break;
            case CircularRRectEffect::kTopRight_CornerFlag:
                radius = rrect.radii(SkRRect::kUpperRight_Corner).fX;
                pdman.set4f(fEdgeSelectUniform, 0, 1, 1, 0);
                break;
            case CircularRRectEffect::kBottomRight_CornerFlag:
                radius = rrect.radii(SkRRect::kLowerRight_Corner).fX;
                pdman.set4f(fEdgeSelectUniform, 0, 0, 1, 1);
                break;
            case CircularRRectEffect::kBottomLeft_CornerFlag:
                radius = rrect.radii(SkRRect::kLowerLeft_Corner).fX;
                pdman.set4f(fEdgeSelectUniform, 1, 0, 0, 1);
                break;
            case CircularRRectEffect::kLeft_CornerFlags:
                radius = rrect.radii(SkRRect::kUpperLeft_Corner).fX;
                pdman.set4f(fEdgeSelectUniform, 1, 1, 0, 1);
                break;
            case CircularRRectEffect::kTop_CornerFlags:
                radius = rrect.radii(SkRRect::kUpperLeft_Corner).fX;
                pdman.set4f(fEdgeSelectUniform, 1, 1, 1, 0);
                break;
            case CircularRRectEffect::kRight_CornerFlags:
                radius = rrect.radii(SkRRect::kUpperRight_Corner).fX;
                pdman.set4f(fEdgeSelectUniform, 0, 1, 1, 1);
                break;
            case CircularRRectEffect::kBottom_CornerFlags:
                radius = rrect.radii(SkRRect::kLowerLeft_Corner).fX;
                pdman.set4f(fEdgeSelectUniform, 1, 0, 1, 1);
                break;
            default:
                SK_ABORT("Should have been one of the above cases.");
        }
        SkRect rect = rrect.getBounds();
        rect.outset(0.5f, 0.5f);
        pdman.set4f(fRectUniform, rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
        radius += 0.5f;
        pdman.set2f(fRadiusPlusHalfUniform, radius, 1.f / radius);
        fPrevRRect = rrect;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CircularRRectEffect::onAddToKey(const GrShaderCaps& caps, skgpu::KeyBuilder* b) const {
    b->add32(static_cast<int>(fEdgeType));
}

std::unique_ptr<GrFragmentProcessor::ProgramImpl> CircularRRectEffect::onMakeProgramImpl() const {
    return std::make_unique<Impl>();
}

//////////////////////////////////////////////////////////////////////////////

namespace {
class EllipticalRRectEffect : public GrFragmentProcessor {
public:
    static GrFPResult Make(std::unique_ptr<GrFragmentProcessor>, GrClipEdgeType, const SkRRect&);

    ~EllipticalRRectEffect() override {}

    const char* name() const override { return "EllipticalRRect"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override;

private:
    class Impl;

    EllipticalRRectEffect(std::unique_ptr<GrFragmentProcessor>, GrClipEdgeType, const SkRRect&);
    EllipticalRRectEffect(const EllipticalRRectEffect& that);

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override;

    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override;

    SkRRect fRRect;
    GrClipEdgeType fEdgeType;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    using INHERITED = GrFragmentProcessor;
};

GrFPResult EllipticalRRectEffect::Make(std::unique_ptr<GrFragmentProcessor> inputFP,
                                       GrClipEdgeType edgeType,
                                       const SkRRect& rrect) {
    if (GrClipEdgeType::kFillAA != edgeType && GrClipEdgeType::kInverseFillAA != edgeType) {
        return GrFPFailure(std::move(inputFP));
    }
    return GrFPSuccess(std::unique_ptr<GrFragmentProcessor>(
            new EllipticalRRectEffect(std::move(inputFP), edgeType, rrect)));
}

EllipticalRRectEffect::EllipticalRRectEffect(std::unique_ptr<GrFragmentProcessor> inputFP,
                                             GrClipEdgeType edgeType,
                                             const SkRRect& rrect)
        : INHERITED(kEllipticalRRectEffect_ClassID,
                    ProcessorOptimizationFlags(inputFP.get()) &
                            kCompatibleWithCoverageAsAlpha_OptimizationFlag)
        , fRRect(rrect)
        , fEdgeType(edgeType) {
    this->registerChild(std::move(inputFP));
}

EllipticalRRectEffect::EllipticalRRectEffect(const EllipticalRRectEffect& that)
        : INHERITED(that)
        , fRRect(that.fRRect)
        , fEdgeType(that.fEdgeType) {}

std::unique_ptr<GrFragmentProcessor> EllipticalRRectEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new EllipticalRRectEffect(*this));
}

bool EllipticalRRectEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const EllipticalRRectEffect& erre = other.cast<EllipticalRRectEffect>();
    return fEdgeType == erre.fEdgeType && fRRect == erre.fRRect;
}
}  // anonymous namespace

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(EllipticalRRectEffect)

#if defined(GR_TEST_UTILS)
std::unique_ptr<GrFragmentProcessor> EllipticalRRectEffect::TestCreate(GrProcessorTestData* d) {
    SkScalar w = d->fRandom->nextRangeScalar(20.f, 1000.f);
    SkScalar h = d->fRandom->nextRangeScalar(20.f, 1000.f);
    SkVector r[4];
    r[SkRRect::kUpperLeft_Corner].fX = d->fRandom->nextRangeF(kRadiusMin, 9.f);
    // ensure at least one corner really is elliptical
    do {
        r[SkRRect::kUpperLeft_Corner].fY = d->fRandom->nextRangeF(kRadiusMin, 9.f);
    } while (r[SkRRect::kUpperLeft_Corner].fY == r[SkRRect::kUpperLeft_Corner].fX);

    SkRRect rrect;
    if (d->fRandom->nextBool()) {
        // half the time create a four-radii rrect.
        r[SkRRect::kLowerRight_Corner].fX = d->fRandom->nextRangeF(kRadiusMin, 9.f);
        r[SkRRect::kLowerRight_Corner].fY = d->fRandom->nextRangeF(kRadiusMin, 9.f);

        r[SkRRect::kUpperRight_Corner].fX = r[SkRRect::kLowerRight_Corner].fX;
        r[SkRRect::kUpperRight_Corner].fY = r[SkRRect::kUpperLeft_Corner].fY;

        r[SkRRect::kLowerLeft_Corner].fX = r[SkRRect::kUpperLeft_Corner].fX;
        r[SkRRect::kLowerLeft_Corner].fY = r[SkRRect::kLowerRight_Corner].fY;

        rrect.setRectRadii(SkRect::MakeWH(w, h), r);
    } else {
        rrect.setRectXY(SkRect::MakeWH(w, h), r[SkRRect::kUpperLeft_Corner].fX,
                                              r[SkRRect::kUpperLeft_Corner].fY);
    }
    std::unique_ptr<GrFragmentProcessor> fp = d->inputFP();
    bool success;
    do {
        GrClipEdgeType et = (GrClipEdgeType)d->fRandom->nextULessThan(kGrClipEdgeTypeCnt);
        std::tie(success, fp) = GrRRectEffect::Make(std::move(fp), et, rrect,
                                                    *d->caps()->shaderCaps());
    } while (!success);
    return fp;
}
#endif

//////////////////////////////////////////////////////////////////////////////

static bool elliptical_effect_uses_scale(const GrShaderCaps& caps, const SkRRect& rrect) {
    // Keep shaders consistent across varying radii when in reduced shader mode.
    if (caps.fReducedShaderMode) {
        return true;
    }
    // If we're on a device where float != fp32 then we'll do the distance computation in a space
    // that is normalized by the largest radius. The scale uniform will be scale, 1/scale. The
    // radii uniform values are already in this normalized space.
    if (!caps.fFloatIs32Bits) {
        return true;
    }
    // Additionally, even if we have fp32, large radii can underflow 1/radii^2 terms leading to
    // blurry coverage. This effect applies to simple and nine-patch, so only need to check TL+BR
    const SkVector& r0 = rrect.radii(SkRRect::kUpperLeft_Corner);
    const SkVector& r1 = rrect.radii(SkRRect::kLowerRight_Corner);
    float maxRadius = std::max(std::max(r0.fX, r0.fY), std::max(r1.fX, r1.fY));
    return SkScalarNearlyZero(1.f / (maxRadius * maxRadius));
}

class EllipticalRRectEffect::Impl : public ProgramImpl {
public:
    void emitCode(EmitArgs&) override;

private:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

    GrGLSLProgramDataManager::UniformHandle fInnerRectUniform;
    GrGLSLProgramDataManager::UniformHandle fInvRadiiSqdUniform;
    GrGLSLProgramDataManager::UniformHandle fScaleUniform;
    SkRRect                                 fPrevRRect;
};

void EllipticalRRectEffect::Impl::emitCode(EmitArgs& args) {
    const EllipticalRRectEffect& erre = args.fFp.cast<EllipticalRRectEffect>();
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    const char *rectName;
    // The inner rect is the rrect bounds inset by the x/y radii
    fInnerRectUniform = uniformHandler->addUniform(&erre, kFragment_GrShaderFlag, SkSLType::kFloat4,
                                                   "innerRect", &rectName);

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    // At each quarter-ellipse corner we compute a vector that is the offset of the fragment pos
    // to the ellipse center. The vector is pinned in x and y to be in the quarter-plane relevant
    // to that corner. This means that points near the interior near the rrect top edge will have
    // a vector that points straight up for both the TL left and TR corners. Computing an
    // alpha from this vector at either the TR or TL corner will give the correct result. Similarly,
    // fragments near the other three edges will get the correct AA. Fragments in the interior of
    // the rrect will have a (0,0) vector at all four corners. So long as the radii > 0.5 they will
    // correctly produce an alpha value of 1 at all four corners. We take the min of all the alphas.
    //
    // The code below is a simplified version of the above that performs maxs on the vector
    // components before computing distances and alpha values so that only one distance computation
    // need be computed to determine the min alpha.
    fragBuilder->codeAppendf("float2 dxy0 = %s.LT - sk_FragCoord.xy;", rectName);
    fragBuilder->codeAppendf("float2 dxy1 = sk_FragCoord.xy - %s.RB;", rectName);


    const char* scaleName = nullptr;
    if (elliptical_effect_uses_scale(*args.fShaderCaps, erre.fRRect)) {
        fScaleUniform = uniformHandler->addUniform(&erre, kFragment_GrShaderFlag, SkSLType::kHalf2,
                                                   "scale", &scaleName);
    }

    // The uniforms with the inv squared radii are highp to prevent underflow.
    switch (erre.fRRect.getType()) {
        case SkRRect::kSimple_Type: {
            const char *invRadiiXYSqdName;
            fInvRadiiSqdUniform = uniformHandler->addUniform(&erre,
                                                             kFragment_GrShaderFlag,
                                                             SkSLType::kFloat2,
                                                             "invRadiiXY",
                                                             &invRadiiXYSqdName);
            fragBuilder->codeAppend("float2 dxy = max(max(dxy0, dxy1), 0.0);");
            if (scaleName) {
                fragBuilder->codeAppendf("dxy *= %s.y;", scaleName);
            }
            // Z is the x/y offsets divided by squared radii.
            fragBuilder->codeAppendf("float2 Z = dxy * %s.xy;", invRadiiXYSqdName);
            break;
        }
        case SkRRect::kNinePatch_Type: {
            const char *invRadiiLTRBSqdName;
            fInvRadiiSqdUniform = uniformHandler->addUniform(&erre,
                                                             kFragment_GrShaderFlag,
                                                             SkSLType::kFloat4,
                                                             "invRadiiLTRB",
                                                             &invRadiiLTRBSqdName);
            if (scaleName) {
                fragBuilder->codeAppendf("dxy0 *= %s.y;", scaleName);
                fragBuilder->codeAppendf("dxy1 *= %s.y;", scaleName);
            }
            fragBuilder->codeAppend("float2 dxy = max(max(dxy0, dxy1), 0.0);");
            // Z is the x/y offsets divided by squared radii. We only care about the (at most) one
            // corner where both the x and y offsets are positive, hence the maxes. (The inverse
            // squared radii will always be positive.)
            fragBuilder->codeAppendf("float2 Z = max(max(dxy0 * %s.xy, dxy1 * %s.zw), 0.0);",
                                     invRadiiLTRBSqdName, invRadiiLTRBSqdName);

            break;
        }
        default:
            SK_ABORT("RRect should always be simple or nine-patch.");
    }
    // implicit is the evaluation of (x/a)^2 + (y/b)^2 - 1.
    fragBuilder->codeAppend("half implicit = half(dot(Z, dxy) - 1.0);");
    // grad_dot is the squared length of the gradient of the implicit.
    fragBuilder->codeAppend("half grad_dot = half(4.0 * dot(Z, Z));");
    // avoid calling inversesqrt on zero.
    fragBuilder->codeAppend("grad_dot = max(grad_dot, 1.0e-4);");
    fragBuilder->codeAppend("half approx_dist = implicit * half(inversesqrt(grad_dot));");
    if (scaleName) {
        fragBuilder->codeAppendf("approx_dist *= %s.x;", scaleName);
    }

    if (erre.fEdgeType == GrClipEdgeType::kFillAA) {
        fragBuilder->codeAppend("half alpha = clamp(0.5 - approx_dist, 0.0, 1.0);");
    } else {
        fragBuilder->codeAppend("half alpha = clamp(0.5 + approx_dist, 0.0, 1.0);");
    }

    SkString inputSample = this->invokeChild(/*childIndex=*/0, args);

    fragBuilder->codeAppendf("return %s * alpha;", inputSample.c_str());
}

void EllipticalRRectEffect::Impl::onSetData(const GrGLSLProgramDataManager& pdman,
                                            const GrFragmentProcessor& effect) {
    const EllipticalRRectEffect& erre = effect.cast<EllipticalRRectEffect>();
    const SkRRect& rrect = erre.fRRect;
    // If we're using a scale factor to work around precision issues, choose the largest radius
    // as the scale factor. The inv radii need to be pre-adjusted by the scale factor.
    if (rrect != fPrevRRect) {
        SkRect rect = rrect.getBounds();
        const SkVector& r0 = rrect.radii(SkRRect::kUpperLeft_Corner);
        SkASSERT(r0.fX >= kRadiusMin);
        SkASSERT(r0.fY >= kRadiusMin);
        switch (rrect.getType()) {
            case SkRRect::kSimple_Type:
                rect.inset(r0.fX, r0.fY);
                if (fScaleUniform.isValid()) {
                    if (r0.fX > r0.fY) {
                        pdman.set2f(fInvRadiiSqdUniform, 1.f, (r0.fX * r0.fX) / (r0.fY * r0.fY));
                        pdman.set2f(fScaleUniform, r0.fX, 1.f / r0.fX);
                    } else {
                        pdman.set2f(fInvRadiiSqdUniform, (r0.fY * r0.fY) / (r0.fX * r0.fX), 1.f);
                        pdman.set2f(fScaleUniform, r0.fY, 1.f / r0.fY);
                    }
                } else {
                    pdman.set2f(fInvRadiiSqdUniform, 1.f / (r0.fX * r0.fX),
                                                     1.f / (r0.fY * r0.fY));
                }
                break;
            case SkRRect::kNinePatch_Type: {
                const SkVector& r1 = rrect.radii(SkRRect::kLowerRight_Corner);
                SkASSERT(r1.fX >= kRadiusMin);
                SkASSERT(r1.fY >= kRadiusMin);
                rect.fLeft += r0.fX;
                rect.fTop += r0.fY;
                rect.fRight -= r1.fX;
                rect.fBottom -= r1.fY;
                if (fScaleUniform.isValid()) {
                    float scale = std::max(std::max(r0.fX, r0.fY), std::max(r1.fX, r1.fY));
                    float scaleSqd = scale * scale;
                    pdman.set4f(fInvRadiiSqdUniform, scaleSqd / (r0.fX * r0.fX),
                                                     scaleSqd / (r0.fY * r0.fY),
                                                     scaleSqd / (r1.fX * r1.fX),
                                                     scaleSqd / (r1.fY * r1.fY));
                    pdman.set2f(fScaleUniform, scale, 1.f / scale);
                } else {
                    pdman.set4f(fInvRadiiSqdUniform, 1.f / (r0.fX * r0.fX),
                                                     1.f / (r0.fY * r0.fY),
                                                     1.f / (r1.fX * r1.fX),
                                                     1.f / (r1.fY * r1.fY));
                }
                break;
            }
        default:
            SK_ABORT("RRect should always be simple or nine-patch.");
        }
        pdman.set4f(fInnerRectUniform, rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
        fPrevRRect = rrect;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void EllipticalRRectEffect::onAddToKey(const GrShaderCaps& caps, skgpu::KeyBuilder* b) const {
    static_assert(kGrClipEdgeTypeCnt <= 4); // 2 bits
    static_assert((int)SkRRect::kLastType + 1 <= 8); // 3 bits
    b->addBits(2, static_cast<int>(fEdgeType), "edge_type");
    b->addBits(3, static_cast<int>(fRRect.getType()), "rrect_type");
    b->addBool(elliptical_effect_uses_scale(caps, fRRect), "scale_radii");
}

std::unique_ptr<GrFragmentProcessor::ProgramImpl> EllipticalRRectEffect::onMakeProgramImpl() const {
    return std::make_unique<Impl>();
}

//////////////////////////////////////////////////////////////////////////////

GrFPResult GrRRectEffect::Make(std::unique_ptr<GrFragmentProcessor> inputFP,
                               GrClipEdgeType edgeType, const SkRRect& rrect,
                               const GrShaderCaps& caps) {
    if (rrect.isRect()) {
        auto fp = GrFragmentProcessor::Rect(std::move(inputFP), edgeType, rrect.getBounds());
        return GrFPSuccess(std::move(fp));
    }

    if (rrect.isOval()) {
        return GrOvalEffect::Make(std::move(inputFP), edgeType, rrect.getBounds(), caps);
    }

    if (rrect.isSimple()) {
        if (SkRRectPriv::GetSimpleRadii(rrect).fX < kRadiusMin ||
            SkRRectPriv::GetSimpleRadii(rrect).fY < kRadiusMin) {
            // In this case the corners are extremely close to rectangular and we collapse the
            // clip to a rectangular clip.
            auto fp = GrFragmentProcessor::Rect(std::move(inputFP), edgeType, rrect.getBounds());
            return GrFPSuccess(std::move(fp));
        }
        if (SkRRectPriv::IsSimpleCircular(rrect)) {
            return CircularRRectEffect::Make(std::move(inputFP), edgeType,
                                             CircularRRectEffect::kAll_CornerFlags, rrect);
        } else {
            return EllipticalRRectEffect::Make(std::move(inputFP), edgeType, rrect);
        }
    }

    if (rrect.isComplex() || rrect.isNinePatch()) {
        // Check for the "tab" cases - two adjacent circular corners and two square corners.
        SkScalar circularRadius = 0;
        uint32_t cornerFlags  = 0;

        SkVector radii[4];
        bool squashedRadii = false;
        for (int c = 0; c < 4; ++c) {
            radii[c] = rrect.radii((SkRRect::Corner)c);
            // This effect can't handle a corner with both zero and non-zero radii
            if ((0 == radii[c].fX) != (0 == radii[c].fY)) {
                return GrFPFailure(std::move(inputFP));
            }
            if (0 == radii[c].fX) {
                // The corner is square, so no need to squash or flag as circular.
                continue;
            }
            if (radii[c].fX < kRadiusMin || radii[c].fY < kRadiusMin) {
                radii[c].set(0, 0);
                squashedRadii = true;
                continue;
            }
            if (radii[c].fX != radii[c].fY) {
                cornerFlags = ~0U;
                break;
            }
            if (!cornerFlags) {
                circularRadius = radii[c].fX;
                cornerFlags = 1 << c;
            } else {
                if (radii[c].fX != circularRadius) {
                   cornerFlags = ~0U;
                   break;
                }
                cornerFlags |= 1 << c;
            }
        }

        switch (cornerFlags) {
            case CircularRRectEffect::kAll_CornerFlags:
                // This rrect should have been caught in the simple case above. Though, it would
                // be correctly handled in the fallthrough code.
                SkASSERT(false);
                [[fallthrough]];
            case CircularRRectEffect::kTopLeft_CornerFlag:
            case CircularRRectEffect::kTopRight_CornerFlag:
            case CircularRRectEffect::kBottomRight_CornerFlag:
            case CircularRRectEffect::kBottomLeft_CornerFlag:
            case CircularRRectEffect::kLeft_CornerFlags:
            case CircularRRectEffect::kTop_CornerFlags:
            case CircularRRectEffect::kRight_CornerFlags:
            case CircularRRectEffect::kBottom_CornerFlags: {
                SkTCopyOnFirstWrite<SkRRect> rr(rrect);
                if (squashedRadii) {
                    rr.writable()->setRectRadii(rrect.getBounds(), radii);
                }
                return CircularRRectEffect::Make(std::move(inputFP), edgeType, cornerFlags, *rr);
            }
            case CircularRRectEffect::kNone_CornerFlags: {
                auto fp =
                        GrFragmentProcessor::Rect(std::move(inputFP), edgeType, rrect.getBounds());
                return GrFPSuccess(std::move(fp));
            }
            default: {
                const SkVector ul = rrect.radii(SkRRect::kUpperLeft_Corner);
                const SkVector lr = rrect.radii(SkRRect::kLowerRight_Corner);
                if (rrect.isNinePatch()  &&
                    ul.fX >= kRadiusMin  &&
                    ul.fY >= kRadiusMin  &&
                    lr.fX >= kRadiusMin  &&
                    lr.fY >= kRadiusMin) {
                    return EllipticalRRectEffect::Make(std::move(inputFP), edgeType, rrect);
                }
                return GrFPFailure(std::move(inputFP));
            }
        }
    }
    return GrFPFailure(std::move(inputFP));
}
