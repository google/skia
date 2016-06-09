/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRRectEffect.h"

#include "GrConvexPolyEffect.h"
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "GrOvalEffect.h"
#include "SkRRect.h"
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
    static sk_sp<GrFragmentProcessor> Make(GrPrimitiveEdgeType, uint32_t circularCornerFlags,
                                           const SkRRect&);

    virtual ~CircularRRectEffect() {};

    const char* name() const override { return "CircularRRect"; }

    const SkRRect& getRRect() const { return fRRect; }

    uint32_t getCircularCornerFlags() const { return fCircularCornerFlags; }

    GrPrimitiveEdgeType getEdgeType() const { return fEdgeType; }

private:
    CircularRRectEffect(GrPrimitiveEdgeType, uint32_t circularCornerFlags, const SkRRect&);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    SkRRect                fRRect;
    GrPrimitiveEdgeType    fEdgeType;
    uint32_t               fCircularCornerFlags;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

sk_sp<GrFragmentProcessor> CircularRRectEffect::Make(GrPrimitiveEdgeType edgeType,
                                                     uint32_t circularCornerFlags,
                                                     const SkRRect& rrect) {
    if (kFillAA_GrProcessorEdgeType != edgeType && kInverseFillAA_GrProcessorEdgeType != edgeType) {
        return nullptr;
    }
    return sk_sp<GrFragmentProcessor>(
        new CircularRRectEffect(edgeType, circularCornerFlags, rrect));
}

void CircularRRectEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    inout->mulByUnknownSingleComponent();
}

CircularRRectEffect::CircularRRectEffect(GrPrimitiveEdgeType edgeType, uint32_t circularCornerFlags,
                                         const SkRRect& rrect)
    : fRRect(rrect)
    , fEdgeType(edgeType)
    , fCircularCornerFlags(circularCornerFlags) {
    this->initClassID<CircularRRectEffect>();
    this->setWillReadFragmentPosition();
}

bool CircularRRectEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const CircularRRectEffect& crre = other.cast<CircularRRectEffect>();
    // The corner flags are derived from fRRect, so no need to check them.
    return fEdgeType == crre.fEdgeType && fRRect == crre.fRRect;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(CircularRRectEffect);

sk_sp<GrFragmentProcessor> CircularRRectEffect::TestCreate(GrProcessorTestData* d) {
    SkScalar w = d->fRandom->nextRangeScalar(20.f, 1000.f);
    SkScalar h = d->fRandom->nextRangeScalar(20.f, 1000.f);
    SkScalar r = d->fRandom->nextRangeF(kRadiusMin, 9.f);
    SkRRect rrect;
    rrect.setRectXY(SkRect::MakeWH(w, h), r, r);
    sk_sp<GrFragmentProcessor> fp;
    do {
        GrPrimitiveEdgeType et =
                (GrPrimitiveEdgeType)d->fRandom->nextULessThan(kGrProcessorEdgeTypeCnt);
        fp = GrRRectEffect::Make(et, rrect);
    } while (nullptr == fp);
    return fp;
}

//////////////////////////////////////////////////////////////////////////////

class GLCircularRRectEffect : public GrGLSLFragmentProcessor {
public:
    GLCircularRRectEffect() {
        fPrevRRect.setEmpty();
    }

    virtual void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

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
    fInnerRectUniform = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                   kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                   "innerRect",
                                                   &rectName);
    // x is (r + .5) and y is 1/(r + .5)
    fRadiusPlusHalfUniform = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                        kVec2f_GrSLType, kDefault_GrSLPrecision,
                                                        "radiusPlusHalf",
                                                        &radiusPlusHalfName);

    // If we're on a device with a "real" mediump then the length calculation could overflow.
    SkString clampedCircleDistance;
    if (args.fGLSLCaps->floatPrecisionVaries()) {
        clampedCircleDistance.printf("clamp(%s.x * (1.0 - length(dxy * %s.y)), 0.0, 1.0);",
                                     radiusPlusHalfName, radiusPlusHalfName);
    } else {
        clampedCircleDistance.printf("clamp(%s.x - length(dxy), 0.0, 1.0);", radiusPlusHalfName);
    }

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    const char* fragmentPos = fragBuilder->fragmentPosition();
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
            fragBuilder->codeAppendf("vec2 dxy0 = %s.xy - %s.xy;", rectName, fragmentPos);
            fragBuilder->codeAppendf("vec2 dxy1 = %s.xy - %s.zw;", fragmentPos, rectName);
            fragBuilder->codeAppend("vec2 dxy = max(max(dxy0, dxy1), 0.0);");
            fragBuilder->codeAppendf("float alpha = %s;", clampedCircleDistance.c_str());
            break;
        case CircularRRectEffect::kTopLeft_CornerFlag:
            fragBuilder->codeAppendf("vec2 dxy = max(%s.xy - %s.xy, 0.0);",
                                     rectName, fragmentPos);
            fragBuilder->codeAppendf("float rightAlpha = clamp(%s.z - %s.x, 0.0, 1.0);",
                                     rectName, fragmentPos);
            fragBuilder->codeAppendf("float bottomAlpha = clamp(%s.w - %s.y, 0.0, 1.0);",
                                     rectName, fragmentPos);
            fragBuilder->codeAppendf("float alpha = bottomAlpha * rightAlpha * %s;",
                                     clampedCircleDistance.c_str());
            break;
        case CircularRRectEffect::kTopRight_CornerFlag:
            fragBuilder->codeAppendf("vec2 dxy = max(vec2(%s.x - %s.z, %s.y - %s.y), 0.0);",
                                     fragmentPos, rectName, rectName, fragmentPos);
            fragBuilder->codeAppendf("float leftAlpha = clamp(%s.x - %s.x, 0.0, 1.0);",
                                     fragmentPos, rectName);
            fragBuilder->codeAppendf("float bottomAlpha = clamp(%s.w - %s.y, 0.0, 1.0);",
                                     rectName, fragmentPos);
            fragBuilder->codeAppendf("float alpha = bottomAlpha * leftAlpha * %s;",
                                     clampedCircleDistance.c_str());
            break;
        case CircularRRectEffect::kBottomRight_CornerFlag:
            fragBuilder->codeAppendf("vec2 dxy = max(%s.xy - %s.zw, 0.0);",
                                     fragmentPos, rectName);
            fragBuilder->codeAppendf("float leftAlpha = clamp(%s.x - %s.x, 0.0, 1.0);",
                                     fragmentPos, rectName);
            fragBuilder->codeAppendf("float topAlpha = clamp(%s.y - %s.y, 0.0, 1.0);",
                                     fragmentPos, rectName);
            fragBuilder->codeAppendf("float alpha = topAlpha * leftAlpha * %s;",
                                     clampedCircleDistance.c_str());
            break;
        case CircularRRectEffect::kBottomLeft_CornerFlag:
            fragBuilder->codeAppendf("vec2 dxy = max(vec2(%s.x - %s.x, %s.y - %s.w), 0.0);",
                                     rectName, fragmentPos, fragmentPos, rectName);
            fragBuilder->codeAppendf("float rightAlpha = clamp(%s.z - %s.x, 0.0, 1.0);",
                                     rectName, fragmentPos);
            fragBuilder->codeAppendf("float topAlpha = clamp(%s.y - %s.y, 0.0, 1.0);",
                                     fragmentPos, rectName);
            fragBuilder->codeAppendf("float alpha = topAlpha * rightAlpha * %s;",
                                     clampedCircleDistance.c_str());
            break;
        case CircularRRectEffect::kLeft_CornerFlags:
            fragBuilder->codeAppendf("vec2 dxy0 = %s.xy - %s.xy;", rectName, fragmentPos);
            fragBuilder->codeAppendf("float dy1 = %s.y - %s.w;", fragmentPos, rectName);
            fragBuilder->codeAppend("vec2 dxy = max(vec2(dxy0.x, max(dxy0.y, dy1)), 0.0);");
            fragBuilder->codeAppendf("float rightAlpha = clamp(%s.z - %s.x, 0.0, 1.0);",
                                     rectName, fragmentPos);
            fragBuilder->codeAppendf("float alpha = rightAlpha * %s;",
                                     clampedCircleDistance.c_str());
            break;
        case CircularRRectEffect::kTop_CornerFlags:
            fragBuilder->codeAppendf("vec2 dxy0 = %s.xy - %s.xy;", rectName, fragmentPos);
            fragBuilder->codeAppendf("float dx1 = %s.x - %s.z;", fragmentPos, rectName);
            fragBuilder->codeAppend("vec2 dxy = max(vec2(max(dxy0.x, dx1), dxy0.y), 0.0);");
            fragBuilder->codeAppendf("float bottomAlpha = clamp(%s.w - %s.y, 0.0, 1.0);",
                                     rectName, fragmentPos);
            fragBuilder->codeAppendf("float alpha = bottomAlpha * %s;",
                                     clampedCircleDistance.c_str());
            break;
        case CircularRRectEffect::kRight_CornerFlags:
            fragBuilder->codeAppendf("float dy0 = %s.y - %s.y;", rectName, fragmentPos);
            fragBuilder->codeAppendf("vec2 dxy1 = %s.xy - %s.zw;", fragmentPos, rectName);
            fragBuilder->codeAppend("vec2 dxy = max(vec2(dxy1.x, max(dy0, dxy1.y)), 0.0);");
            fragBuilder->codeAppendf("float leftAlpha = clamp(%s.x - %s.x, 0.0, 1.0);",
                                     fragmentPos, rectName);
            fragBuilder->codeAppendf("float alpha = leftAlpha * %s;",
                                     clampedCircleDistance.c_str());
            break;
        case CircularRRectEffect::kBottom_CornerFlags:
            fragBuilder->codeAppendf("float dx0 = %s.x - %s.x;", rectName, fragmentPos);
            fragBuilder->codeAppendf("vec2 dxy1 = %s.xy - %s.zw;", fragmentPos, rectName);
            fragBuilder->codeAppend("vec2 dxy = max(vec2(max(dx0, dxy1.x), dxy1.y), 0.0);");
            fragBuilder->codeAppendf("float topAlpha = clamp(%s.y - %s.y, 0.0, 1.0);",
                                     fragmentPos, rectName);
            fragBuilder->codeAppendf("float alpha = topAlpha * %s;",
                                     clampedCircleDistance.c_str());
            break;
    }

    if (kInverseFillAA_GrProcessorEdgeType == crre.getEdgeType()) {
        fragBuilder->codeAppend("alpha = 1.0 - alpha;");
    }

    fragBuilder->codeAppendf("%s = %s;", args.fOutputColor,
                             (GrGLSLExpr4(args.fInputColor) * GrGLSLExpr1("alpha")).c_str());
}

void GLCircularRRectEffect::GenKey(const GrProcessor& processor, const GrGLSLCaps&,
                                   GrProcessorKeyBuilder* b) {
    const CircularRRectEffect& crre = processor.cast<CircularRRectEffect>();
    GR_STATIC_ASSERT(kGrProcessorEdgeTypeCnt <= 8);
    b->add32((crre.getCircularCornerFlags() << 3) | crre.getEdgeType());
}

void GLCircularRRectEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                      const GrProcessor& processor) {
    const CircularRRectEffect& crre = processor.cast<CircularRRectEffect>();
    const SkRRect& rrect = crre.getRRect();
    if (rrect != fPrevRRect) {
        SkRect rect = rrect.getBounds();
        SkScalar radius = 0;
        switch (crre.getCircularCornerFlags()) {
            case CircularRRectEffect::kAll_CornerFlags:
                SkASSERT(rrect.isSimpleCircular());
                radius = rrect.getSimpleRadii().fX;
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
                SkFAIL("Should have been one of the above cases.");
        }
        pdman.set4f(fInnerRectUniform, rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
        radius += 0.5f;
        pdman.set2f(fRadiusPlusHalfUniform, radius, 1.f / radius);
        fPrevRRect = rrect;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CircularRRectEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                                GrProcessorKeyBuilder* b) const {
    GLCircularRRectEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* CircularRRectEffect::onCreateGLSLInstance() const  {
    return new GLCircularRRectEffect;
}

//////////////////////////////////////////////////////////////////////////////

class EllipticalRRectEffect : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make(GrPrimitiveEdgeType, const SkRRect&);

    virtual ~EllipticalRRectEffect() {};

    const char* name() const override { return "EllipticalRRect"; }

    const SkRRect& getRRect() const { return fRRect; }

    GrPrimitiveEdgeType getEdgeType() const { return fEdgeType; }

private:
    EllipticalRRectEffect(GrPrimitiveEdgeType, const SkRRect&);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    SkRRect             fRRect;
    GrPrimitiveEdgeType    fEdgeType;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

sk_sp<GrFragmentProcessor>
EllipticalRRectEffect::Make(GrPrimitiveEdgeType edgeType, const SkRRect& rrect) {
    if (kFillAA_GrProcessorEdgeType != edgeType && kInverseFillAA_GrProcessorEdgeType != edgeType) {
        return nullptr;
    }
    return sk_sp<GrFragmentProcessor>(new EllipticalRRectEffect(edgeType, rrect));
}

void EllipticalRRectEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    inout->mulByUnknownSingleComponent();
}

EllipticalRRectEffect::EllipticalRRectEffect(GrPrimitiveEdgeType edgeType, const SkRRect& rrect)
    : fRRect(rrect)
    , fEdgeType(edgeType) {
    this->initClassID<EllipticalRRectEffect>();
    this->setWillReadFragmentPosition();
}

bool EllipticalRRectEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const EllipticalRRectEffect& erre = other.cast<EllipticalRRectEffect>();
    return fEdgeType == erre.fEdgeType && fRRect == erre.fRRect;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(EllipticalRRectEffect);

sk_sp<GrFragmentProcessor> EllipticalRRectEffect::TestCreate(GrProcessorTestData* d) {
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
    sk_sp<GrFragmentProcessor> fp;
    do {
        GrPrimitiveEdgeType et =
                (GrPrimitiveEdgeType)d->fRandom->nextULessThan(kGrProcessorEdgeTypeCnt);
        fp = GrRRectEffect::Make(et, rrect);
    } while (nullptr == fp);
    return fp;
}

//////////////////////////////////////////////////////////////////////////////

class GLEllipticalRRectEffect : public GrGLSLFragmentProcessor {
public:
    GLEllipticalRRectEffect() {
        fPrevRRect.setEmpty();
    }

    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

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
    fInnerRectUniform = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                   kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                   "innerRect",
                                                   &rectName);

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    const char* fragmentPos = fragBuilder->fragmentPosition();
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
    fragBuilder->codeAppendf("vec2 dxy0 = %s.xy - %s.xy;", rectName, fragmentPos);
    fragBuilder->codeAppendf("vec2 dxy1 = %s.xy - %s.zw;", fragmentPos, rectName);

    // If we're on a device with a "real" mediump then we'll do the distance computation in a space
    // that is normalized by the largest radius. The scale uniform will be scale, 1/scale. The
    // radii uniform values are already in this normalized space.
    const char* scaleName = nullptr;
    if (args.fGLSLCaps->floatPrecisionVaries()) {
        fScaleUniform = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                   kVec2f_GrSLType, kDefault_GrSLPrecision,
                                                   "scale", &scaleName);
    }

    // The uniforms with the inv squared radii are highp to prevent underflow.
    switch (erre.getRRect().getType()) {
        case SkRRect::kSimple_Type: {
            const char *invRadiiXYSqdName;
            fInvRadiiSqdUniform = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                             kVec2f_GrSLType,
                                                             kDefault_GrSLPrecision,
                                                             "invRadiiXY",
                                                             &invRadiiXYSqdName);
            fragBuilder->codeAppend("vec2 dxy = max(max(dxy0, dxy1), 0.0);");
            if (scaleName) {
                fragBuilder->codeAppendf("dxy *= %s.y;", scaleName);
            }
            // Z is the x/y offsets divided by squared radii.
            fragBuilder->codeAppendf("vec2 Z = dxy * %s.xy;", invRadiiXYSqdName);
            break;
        }
        case SkRRect::kNinePatch_Type: {
            const char *invRadiiLTRBSqdName;
            fInvRadiiSqdUniform = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                             kVec4f_GrSLType,
                                                             kDefault_GrSLPrecision,
                                                             "invRadiiLTRB",
                                                             &invRadiiLTRBSqdName);
            if (scaleName) {
                fragBuilder->codeAppendf("dxy0 *= %s.y;", scaleName);
                fragBuilder->codeAppendf("dxy1 *= %s.y;", scaleName);
            }
            fragBuilder->codeAppend("vec2 dxy = max(max(dxy0, dxy1), 0.0);");
            // Z is the x/y offsets divided by squared radii. We only care about the (at most) one
            // corner where both the x and y offsets are positive, hence the maxes. (The inverse
            // squared radii will always be positive.)
            fragBuilder->codeAppendf("vec2 Z = max(max(dxy0 * %s.xy, dxy1 * %s.zw), 0.0);",
                                     invRadiiLTRBSqdName, invRadiiLTRBSqdName);

            break;
        }
        default:
            SkFAIL("RRect should always be simple or nine-patch.");
    }
    // implicit is the evaluation of (x/a)^2 + (y/b)^2 - 1.
    fragBuilder->codeAppend("float implicit = dot(Z, dxy) - 1.0;");
    // grad_dot is the squared length of the gradient of the implicit.
    fragBuilder->codeAppend("float grad_dot = 4.0 * dot(Z, Z);");
    // avoid calling inversesqrt on zero.
    fragBuilder->codeAppend("grad_dot = max(grad_dot, 1.0e-4);");
    fragBuilder->codeAppend("float approx_dist = implicit * inversesqrt(grad_dot);");
    if (scaleName) {
        fragBuilder->codeAppendf("approx_dist *= %s.x;", scaleName);
    }

    if (kFillAA_GrProcessorEdgeType == erre.getEdgeType()) {
        fragBuilder->codeAppend("float alpha = clamp(0.5 - approx_dist, 0.0, 1.0);");
    } else {
        fragBuilder->codeAppend("float alpha = clamp(0.5 + approx_dist, 0.0, 1.0);");
    }

    fragBuilder->codeAppendf("%s = %s;", args.fOutputColor,
                             (GrGLSLExpr4(args.fInputColor) * GrGLSLExpr1("alpha")).c_str());
}

void GLEllipticalRRectEffect::GenKey(const GrProcessor& effect, const GrGLSLCaps&,
                                     GrProcessorKeyBuilder* b) {
    const EllipticalRRectEffect& erre = effect.cast<EllipticalRRectEffect>();
    GR_STATIC_ASSERT(kLast_GrProcessorEdgeType < (1 << 3));
    b->add32(erre.getRRect().getType() | erre.getEdgeType() << 3);
}

void GLEllipticalRRectEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                        const GrProcessor& effect) {
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
            SkFAIL("RRect should always be simple or nine-patch.");
        }
        pdman.set4f(fInnerRectUniform, rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
        fPrevRRect = rrect;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void EllipticalRRectEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                                  GrProcessorKeyBuilder* b) const {
    GLEllipticalRRectEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* EllipticalRRectEffect::onCreateGLSLInstance() const  {
    return new GLEllipticalRRectEffect;
}

//////////////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> GrRRectEffect::Make(GrPrimitiveEdgeType edgeType, const SkRRect& rrect) {
    if (rrect.isRect()) {
        return GrConvexPolyEffect::Make(edgeType, rrect.getBounds());
    }

    if (rrect.isOval()) {
        return GrOvalEffect::Make(edgeType, rrect.getBounds());
    }

    if (rrect.isSimple()) {
        if (rrect.getSimpleRadii().fX < kRadiusMin || rrect.getSimpleRadii().fY < kRadiusMin) {
            // In this case the corners are extremely close to rectangular and we collapse the
            // clip to a rectangular clip.
            return GrConvexPolyEffect::Make(edgeType, rrect.getBounds());
        }
        if (rrect.getSimpleRadii().fX == rrect.getSimpleRadii().fY) {
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
