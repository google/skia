/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRRectEffect.h"

#include "GrConvexPolyEffect.h"
#include "GrFragmentProcessor.h"
#include "GrOvalEffect.h"
#include "GrShaderCaps.h"
#include "SkRRectPriv.h"
#include "SkTLazy.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

// The effects defined here only handle rrect radii >= kRadiusMin.
static const SkScalar kRadiusMin = SK_ScalarHalf;

//////////////////////////////////////////////////////////////////////////////

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
    static std::unique_ptr<GrFragmentProcessor> Make(GrClipEdgeType,
                                                     uint32_t circularCornerFlags, const SkRRect&);

    ~CircularRRectEffect() override {}

    const char* name() const override { return "CircularRRect"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override;

    const SkRRect& getRRect() const { return fRRect; }

    uint32_t getCircularCornerFlags() const { return fCircularCornerFlags; }

    GrClipEdgeType getEdgeType() const { return fEdgeType; }

private:
    CircularRRectEffect(GrClipEdgeType, uint32_t circularCornerFlags, const SkRRect&);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override;

    SkRRect                fRRect;
    GrClipEdgeType    fEdgeType;
    uint32_t               fCircularCornerFlags;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;
};

std::unique_ptr<GrFragmentProcessor> CircularRRectEffect::Make(GrClipEdgeType edgeType,
                                                               uint32_t circularCornerFlags,
                                                               const SkRRect& rrect) {
    if (GrClipEdgeType::kFillAA != edgeType && GrClipEdgeType::kInverseFillAA != edgeType) {
        return nullptr;
    }
    return std::unique_ptr<GrFragmentProcessor>(
            new CircularRRectEffect(edgeType, circularCornerFlags, rrect));
}

CircularRRectEffect::CircularRRectEffect(GrClipEdgeType edgeType, uint32_t circularCornerFlags,
                                         const SkRRect& rrect)
        : INHERITED(kCircularRRectEffect_ClassID, kCompatibleWithCoverageAsAlpha_OptimizationFlag)
        , fRRect(rrect)
        , fEdgeType(edgeType)
        , fCircularCornerFlags(circularCornerFlags) {
}

std::unique_ptr<GrFragmentProcessor> CircularRRectEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(
            new CircularRRectEffect(fEdgeType, fCircularCornerFlags, fRRect));
}

bool CircularRRectEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const CircularRRectEffect& crre = other.cast<CircularRRectEffect>();
    // The corner flags are derived from fRRect, so no need to check them.
    return fEdgeType == crre.fEdgeType && fRRect == crre.fRRect;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(CircularRRectEffect);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> CircularRRectEffect::TestCreate(GrProcessorTestData* d) {
    SkScalar w = d->fRandom->nextRangeScalar(20.f, 1000.f);
    SkScalar h = d->fRandom->nextRangeScalar(20.f, 1000.f);
    SkScalar r = d->fRandom->nextRangeF(kRadiusMin, 9.f);
    SkRRect rrect;
    rrect.setRectXY(SkRect::MakeWH(w, h), r, r);
    std::unique_ptr<GrFragmentProcessor> fp;
    do {
        GrClipEdgeType et =
                (GrClipEdgeType)d->fRandom->nextULessThan(kGrClipEdgeTypeCnt);
        fp = GrRRectEffect::Make(et, rrect, *d->caps()->shaderCaps());
    } while (nullptr == fp);
    return fp;
}
#endif

//////////////////////////////////////////////////////////////////////////////

class GLCircularRRectEffect : public GrGLSLFragmentProcessor {
public:
    GLCircularRRectEffect() = default;

    virtual void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

private:
    GrGLSLProgramDataManager::UniformHandle fInnerRectUniform;
    GrGLSLProgramDataManager::UniformHandle fRadiusPlusHalfUniform;
    SkRRect                                 fPrevRRect;
    typedef GrGLSLFragmentProcessor INHERITED;
};

void GLCircularRRectEffect::emitCode(EmitArgs& args) {
    const CircularRRectEffect& crre = args.fFp.cast<CircularRRectEffect>();
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    const char *rectName;
    const char *radiusPlusHalfName;
    // The inner rect is the rrect bounds inset by the radius. Its left, top, right, and bottom
    // edges correspond to components x, y, z, and w, respectively. When a side of the rrect has
    // only rectangular corners, that side's value corresponds to the rect edge's value outset by
    // half a pixel.
    fInnerRectUniform = uniformHandler->addUniform(kFragment_GrShaderFlag, kFloat4_GrSLType,
                                                   "innerRect", &rectName);
    // x is (r + .5) and y is 1/(r + .5)
    fRadiusPlusHalfUniform = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf2_GrSLType,
                                                        "radiusPlusHalf", &radiusPlusHalfName);

    // If we're on a device where float != fp32 then the length calculation could overflow.
    SkString clampedCircleDistance;
    if (!args.fShaderCaps->floatIs32Bits()) {
        clampedCircleDistance.printf("saturate(%s.x * (1.0 - length(dxy * %s.y)))",
                                     radiusPlusHalfName, radiusPlusHalfName);
    } else {
        clampedCircleDistance.printf("saturate(%s.x - length(dxy))", radiusPlusHalfName);
    }

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
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
    // need be computed to determine the min alpha.
    //
    // For the cases where one half of the rrect is rectangular we drop one of the x or y
    // computations, compute a separate rect edge alpha for the rect side, and mul the two computed
    // alphas together.
    switch (crre.getCircularCornerFlags()) {
        case CircularRRectEffect::kAll_CornerFlags:
            fragBuilder->codeAppendf("float2 dxy0 = %s.xy - sk_FragCoord.xy;", rectName);
            fragBuilder->codeAppendf("float2 dxy1 = sk_FragCoord.xy - %s.zw;", rectName);
            fragBuilder->codeAppend("float2 dxy = max(max(dxy0, dxy1), 0.0);");
            fragBuilder->codeAppendf("half alpha = half(%s);", clampedCircleDistance.c_str());
            break;
        case CircularRRectEffect::kTopLeft_CornerFlag:
            fragBuilder->codeAppendf("float2 dxy = max(%s.xy - sk_FragCoord.xy, 0.0);",
                                     rectName);
            fragBuilder->codeAppendf("half rightAlpha = half(saturate(%s.z - sk_FragCoord.x));",
                                     rectName);
            fragBuilder->codeAppendf("half bottomAlpha = half(saturate(%s.w - sk_FragCoord.y));",
                                     rectName);
            fragBuilder->codeAppendf("half alpha = bottomAlpha * rightAlpha * half(%s);",
                                     clampedCircleDistance.c_str());
            break;
        case CircularRRectEffect::kTopRight_CornerFlag:
            fragBuilder->codeAppendf("float2 dxy = max(float2(sk_FragCoord.x - %s.z, "
                                                             "%s.y - sk_FragCoord.y), 0.0);",
                                     rectName, rectName);
            fragBuilder->codeAppendf("half leftAlpha = half(saturate(sk_FragCoord.x - %s.x));",
                                     rectName);
            fragBuilder->codeAppendf("half bottomAlpha = half(saturate(%s.w - sk_FragCoord.y));",
                                     rectName);
            fragBuilder->codeAppendf("half alpha = bottomAlpha * leftAlpha * half(%s);",
                                     clampedCircleDistance.c_str());
            break;
        case CircularRRectEffect::kBottomRight_CornerFlag:
            fragBuilder->codeAppendf("float2 dxy = max(sk_FragCoord.xy - %s.zw, 0.0);",
                                     rectName);
            fragBuilder->codeAppendf("half leftAlpha = half(saturate(sk_FragCoord.x - %s.x));",
                                     rectName);
            fragBuilder->codeAppendf("half topAlpha = half(saturate(sk_FragCoord.y - %s.y));",
                                     rectName);
            fragBuilder->codeAppendf("half alpha = topAlpha * leftAlpha * half(%s);",
                                     clampedCircleDistance.c_str());
            break;
        case CircularRRectEffect::kBottomLeft_CornerFlag:
            fragBuilder->codeAppendf("float2 dxy = max(float2(%s.x - sk_FragCoord.x, "
                                                             "sk_FragCoord.y - %s.w), 0.0);",
                                     rectName, rectName);
            fragBuilder->codeAppendf("half rightAlpha = half(saturate(%s.z - sk_FragCoord.x));",
                                     rectName);
            fragBuilder->codeAppendf("half topAlpha = half(saturate(sk_FragCoord.y - %s.y));",
                                     rectName);
            fragBuilder->codeAppendf("half alpha = topAlpha * rightAlpha * half(%s);",
                                     clampedCircleDistance.c_str());
            break;
        case CircularRRectEffect::kLeft_CornerFlags:
            fragBuilder->codeAppendf("float2 dxy0 = %s.xy - sk_FragCoord.xy;", rectName);
            fragBuilder->codeAppendf("float dy1 = sk_FragCoord.y - %s.w;", rectName);
            fragBuilder->codeAppend("float2 dxy = max(float2(dxy0.x, max(dxy0.y, dy1)), 0.0);");
            fragBuilder->codeAppendf("half rightAlpha = half(saturate(%s.z - sk_FragCoord.x));",
                                     rectName);
            fragBuilder->codeAppendf("half alpha = rightAlpha * half(%s);",
                                     clampedCircleDistance.c_str());
            break;
        case CircularRRectEffect::kTop_CornerFlags:
            fragBuilder->codeAppendf("float2 dxy0 = %s.xy - sk_FragCoord.xy;", rectName);
            fragBuilder->codeAppendf("float dx1 = sk_FragCoord.x - %s.z;", rectName);
            fragBuilder->codeAppend("float2 dxy = max(float2(max(dxy0.x, dx1), dxy0.y), 0.0);");
            fragBuilder->codeAppendf("half bottomAlpha = half(saturate(%s.w - sk_FragCoord.y));",
                                     rectName);
            fragBuilder->codeAppendf("half alpha = bottomAlpha * half(%s);",
                                     clampedCircleDistance.c_str());
            break;
        case CircularRRectEffect::kRight_CornerFlags:
            fragBuilder->codeAppendf("float dy0 = %s.y - sk_FragCoord.y;", rectName);
            fragBuilder->codeAppendf("float2 dxy1 = sk_FragCoord.xy - %s.zw;", rectName);
            fragBuilder->codeAppend("float2 dxy = max(float2(dxy1.x, max(dy0, dxy1.y)), 0.0);");
            fragBuilder->codeAppendf("half leftAlpha = half(saturate(sk_FragCoord.x - %s.x));",
                                     rectName);
            fragBuilder->codeAppendf("half alpha = leftAlpha * half(%s);",
                                     clampedCircleDistance.c_str());
            break;
        case CircularRRectEffect::kBottom_CornerFlags:
            fragBuilder->codeAppendf("float dx0 = %s.x - sk_FragCoord.x;", rectName);
            fragBuilder->codeAppendf("float2 dxy1 = sk_FragCoord.xy - %s.zw;", rectName);
            fragBuilder->codeAppend("float2 dxy = max(float2(max(dx0, dxy1.x), dxy1.y), 0.0);");
            fragBuilder->codeAppendf("half topAlpha = half(saturate(sk_FragCoord.y - %s.y));",
                                     rectName);
            fragBuilder->codeAppendf("half alpha = topAlpha * half(%s);",
                                     clampedCircleDistance.c_str());
            break;
    }

    if (GrClipEdgeType::kInverseFillAA == crre.getEdgeType()) {
        fragBuilder->codeAppend("alpha = 1.0 - alpha;");
    }

    fragBuilder->codeAppendf("%s = %s * alpha;", args.fOutputColor, args.fInputColor);
}

void GLCircularRRectEffect::GenKey(const GrProcessor& processor, const GrShaderCaps&,
                                   GrProcessorKeyBuilder* b) {
    const CircularRRectEffect& crre = processor.cast<CircularRRectEffect>();
    GR_STATIC_ASSERT(kGrClipEdgeTypeCnt <= 8);
    b->add32((crre.getCircularCornerFlags() << 3) | (int) crre.getEdgeType());
}

void GLCircularRRectEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                      const GrFragmentProcessor& processor) {
    const CircularRRectEffect& crre = processor.cast<CircularRRectEffect>();
    const SkRRect& rrect = crre.getRRect();
    if (rrect != fPrevRRect) {
        SkRect rect = rrect.getBounds();
        SkScalar radius = 0;
        switch (crre.getCircularCornerFlags()) {
            case CircularRRectEffect::kAll_CornerFlags:
                SkASSERT(SkRRectPriv::IsSimpleCircular(rrect));
                radius = SkRRectPriv::GetSimpleRadii(rrect).fX;
                SkASSERT(radius >= kRadiusMin);
                rect.inset(radius, radius);
                break;
            case CircularRRectEffect::kTopLeft_CornerFlag:
                radius = rrect.radii(SkRRect::kUpperLeft_Corner).fX;
                rect.fLeft += radius;
                rect.fTop += radius;
                rect.fRight += 0.5f;
                rect.fBottom += 0.5f;
                break;
            case CircularRRectEffect::kTopRight_CornerFlag:
                radius = rrect.radii(SkRRect::kUpperRight_Corner).fX;
                rect.fLeft -= 0.5f;
                rect.fTop += radius;
                rect.fRight -= radius;
                rect.fBottom += 0.5f;
                break;
            case CircularRRectEffect::kBottomRight_CornerFlag:
                radius = rrect.radii(SkRRect::kLowerRight_Corner).fX;
                rect.fLeft -= 0.5f;
                rect.fTop -= 0.5f;
                rect.fRight -= radius;
                rect.fBottom -= radius;
                break;
            case CircularRRectEffect::kBottomLeft_CornerFlag:
                radius = rrect.radii(SkRRect::kLowerLeft_Corner).fX;
                rect.fLeft += radius;
                rect.fTop -= 0.5f;
                rect.fRight += 0.5f;
                rect.fBottom -= radius;
                break;
            case CircularRRectEffect::kLeft_CornerFlags:
                radius = rrect.radii(SkRRect::kUpperLeft_Corner).fX;
                rect.fLeft += radius;
                rect.fTop += radius;
                rect.fRight += 0.5f;
                rect.fBottom -= radius;
                break;
            case CircularRRectEffect::kTop_CornerFlags:
                radius = rrect.radii(SkRRect::kUpperLeft_Corner).fX;
                rect.fLeft += radius;
                rect.fTop += radius;
                rect.fRight -= radius;
                rect.fBottom += 0.5f;
                break;
            case CircularRRectEffect::kRight_CornerFlags:
                radius = rrect.radii(SkRRect::kUpperRight_Corner).fX;
                rect.fLeft -= 0.5f;
                rect.fTop += radius;
                rect.fRight -= radius;
                rect.fBottom -= radius;
                break;
            case CircularRRectEffect::kBottom_CornerFlags:
                radius = rrect.radii(SkRRect::kLowerLeft_Corner).fX;
                rect.fLeft += radius;
                rect.fTop -= 0.5f;
                rect.fRight -= radius;
                rect.fBottom -= radius;
                break;
            default:
                SK_ABORT("Should have been one of the above cases.");
        }
        pdman.set4f(fInnerRectUniform, rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
        radius += 0.5f;
        pdman.set2f(fRadiusPlusHalfUniform, radius, 1.f / radius);
        fPrevRRect = rrect;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CircularRRectEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                GrProcessorKeyBuilder* b) const {
    GLCircularRRectEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* CircularRRectEffect::onCreateGLSLInstance() const  {
    return new GLCircularRRectEffect;
}

//////////////////////////////////////////////////////////////////////////////

class EllipticalRRectEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(GrClipEdgeType, const SkRRect&);

    ~EllipticalRRectEffect() override {}

    const char* name() const override { return "EllipticalRRect"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override;

    const SkRRect& getRRect() const { return fRRect; }

    GrClipEdgeType getEdgeType() const { return fEdgeType; }

private:
    EllipticalRRectEffect(GrClipEdgeType, const SkRRect&);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override;

    SkRRect fRRect;
    GrClipEdgeType fEdgeType;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;
};

std::unique_ptr<GrFragmentProcessor> EllipticalRRectEffect::Make(GrClipEdgeType edgeType,
                                                                 const SkRRect& rrect) {
    if (GrClipEdgeType::kFillAA != edgeType && GrClipEdgeType::kInverseFillAA != edgeType) {
        return nullptr;
    }
    return std::unique_ptr<GrFragmentProcessor>(new EllipticalRRectEffect(edgeType, rrect));
}

EllipticalRRectEffect::EllipticalRRectEffect(GrClipEdgeType edgeType, const SkRRect& rrect)
        : INHERITED(kEllipticalRRectEffect_ClassID, kCompatibleWithCoverageAsAlpha_OptimizationFlag)
        , fRRect(rrect)
        , fEdgeType(edgeType) {
}

std::unique_ptr<GrFragmentProcessor> EllipticalRRectEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new EllipticalRRectEffect(fEdgeType, fRRect));
}

bool EllipticalRRectEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const EllipticalRRectEffect& erre = other.cast<EllipticalRRectEffect>();
    return fEdgeType == erre.fEdgeType && fRRect == erre.fRRect;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(EllipticalRRectEffect);

#if GR_TEST_UTILS
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
    std::unique_ptr<GrFragmentProcessor> fp;
    do {
        GrClipEdgeType et = (GrClipEdgeType)d->fRandom->nextULessThan(kGrClipEdgeTypeCnt);
        fp = GrRRectEffect::Make(et, rrect, *d->caps()->shaderCaps());
    } while (nullptr == fp);
    return fp;
}
#endif

//////////////////////////////////////////////////////////////////////////////

class GLEllipticalRRectEffect : public GrGLSLFragmentProcessor {
public:
    GLEllipticalRRectEffect() = default;

    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

private:
    GrGLSLProgramDataManager::UniformHandle fInnerRectUniform;
    GrGLSLProgramDataManager::UniformHandle fInvRadiiSqdUniform;
    GrGLSLProgramDataManager::UniformHandle fScaleUniform;
    SkRRect                                 fPrevRRect;
    typedef GrGLSLFragmentProcessor INHERITED;
};

void GLEllipticalRRectEffect::emitCode(EmitArgs& args) {
    const EllipticalRRectEffect& erre = args.fFp.cast<EllipticalRRectEffect>();
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    const char *rectName;
    // The inner rect is the rrect bounds inset by the x/y radii
    fInnerRectUniform = uniformHandler->addUniform(kFragment_GrShaderFlag, kFloat4_GrSLType,
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
    fragBuilder->codeAppendf("float2 dxy0 = %s.xy - sk_FragCoord.xy;", rectName);
    fragBuilder->codeAppendf("float2 dxy1 = sk_FragCoord.xy - %s.zw;", rectName);

    // If we're on a device where float != fp32 then we'll do the distance computation in a space
    // that is normalized by the largest radius. The scale uniform will be scale, 1/scale. The
    // radii uniform values are already in this normalized space.
    const char* scaleName = nullptr;
    if (!args.fShaderCaps->floatIs32Bits()) {
        fScaleUniform = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf2_GrSLType, "scale",
                                                   &scaleName);
    }

    // The uniforms with the inv squared radii are highp to prevent underflow.
    switch (erre.getRRect().getType()) {
        case SkRRect::kSimple_Type: {
            const char *invRadiiXYSqdName;
            fInvRadiiSqdUniform = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                             kHalf2_GrSLType,
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
            fInvRadiiSqdUniform = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                             kHalf4_GrSLType,
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

    if (GrClipEdgeType::kFillAA == erre.getEdgeType()) {
        fragBuilder->codeAppend("half alpha = clamp(0.5 - approx_dist, 0.0, 1.0);");
    } else {
        fragBuilder->codeAppend("half alpha = clamp(0.5 + approx_dist, 0.0, 1.0);");
    }

    fragBuilder->codeAppendf("%s = %s * alpha;", args.fOutputColor, args.fInputColor);
}

void GLEllipticalRRectEffect::GenKey(const GrProcessor& effect, const GrShaderCaps&,
                                     GrProcessorKeyBuilder* b) {
    const EllipticalRRectEffect& erre = effect.cast<EllipticalRRectEffect>();
    GR_STATIC_ASSERT((int) GrClipEdgeType::kLast < (1 << 3));
    b->add32(erre.getRRect().getType() | (int) erre.getEdgeType() << 3);
}

void GLEllipticalRRectEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                        const GrFragmentProcessor& effect) {
    const EllipticalRRectEffect& erre = effect.cast<EllipticalRRectEffect>();
    const SkRRect& rrect = erre.getRRect();
    // If we're using a scale factor to work around precision issues, choose the largest radius
    // as the scale factor. The inv radii need to be pre-adjusted by the scale factor.
    if (rrect != fPrevRRect) {
        SkRect rect = rrect.getBounds();
        const SkVector& r0 = rrect.radii(SkRRect::kUpperLeft_Corner);
        SkASSERT(r0.fX >= kRadiusMin);
        SkASSERT(r0.fY >= kRadiusMin);
        switch (erre.getRRect().getType()) {
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
                    float scale = SkTMax(SkTMax(r0.fX, r0.fY), SkTMax(r1.fX, r1.fY));
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

void EllipticalRRectEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                  GrProcessorKeyBuilder* b) const {
    GLEllipticalRRectEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* EllipticalRRectEffect::onCreateGLSLInstance() const  {
    return new GLEllipticalRRectEffect;
}

//////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> GrRRectEffect::Make(GrClipEdgeType edgeType,
                                                         const SkRRect& rrect,
                                                         const GrShaderCaps& caps) {
    if (rrect.isRect()) {
        return GrConvexPolyEffect::Make(edgeType, rrect.getBounds());
    }

    if (rrect.isOval()) {
        return GrOvalEffect::Make(edgeType, rrect.getBounds(), caps);
    }

    if (rrect.isSimple()) {
        if (SkRRectPriv::GetSimpleRadii(rrect).fX < kRadiusMin ||
            SkRRectPriv::GetSimpleRadii(rrect).fY < kRadiusMin) {
            // In this case the corners are extremely close to rectangular and we collapse the
            // clip to a rectangular clip.
            return GrConvexPolyEffect::Make(edgeType, rrect.getBounds());
        }
        if (SkRRectPriv::GetSimpleRadii(rrect).fX == SkRRectPriv::GetSimpleRadii(rrect).fY) {
            return CircularRRectEffect::Make(edgeType, CircularRRectEffect::kAll_CornerFlags,
                                               rrect);
        } else {
            return EllipticalRRectEffect::Make(edgeType, rrect);
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
            SkASSERT((0 == radii[c].fX) == (0 == radii[c].fY));
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
                return CircularRRectEffect::Make(edgeType, cornerFlags, *rr);
            }
            case CircularRRectEffect::kNone_CornerFlags:
                return GrConvexPolyEffect::Make(edgeType, rrect.getBounds());
            default: {
                if (squashedRadii) {
                    // If we got here then we squashed some but not all the radii to zero. (If all
                    // had been squashed cornerFlags would be 0.) The elliptical effect doesn't
                    // support some rounded and some square corners.
                    return nullptr;
                }
                if (rrect.isNinePatch()) {
                    return EllipticalRRectEffect::Make(edgeType, rrect);
                }
                return nullptr;
            }
        }
    }

    return nullptr;
}
