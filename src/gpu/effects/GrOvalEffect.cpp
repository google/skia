/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOvalEffect.h"

#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "SkRect.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

//////////////////////////////////////////////////////////////////////////////

class CircleEffect : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make(GrPrimitiveEdgeType, const SkPoint& center,
                                           SkScalar radius);

    virtual ~CircleEffect() {};

    const char* name() const override { return "Circle"; }

    const SkPoint& getCenter() const { return fCenter; }
    SkScalar getRadius() const { return fRadius; }

    GrPrimitiveEdgeType getEdgeType() const { return fEdgeType; }

private:
    CircleEffect(GrPrimitiveEdgeType, const SkPoint& center, SkScalar radius);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    SkPoint             fCenter;
    SkScalar            fRadius;
    GrPrimitiveEdgeType    fEdgeType;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

sk_sp<GrFragmentProcessor> CircleEffect::Make(GrPrimitiveEdgeType edgeType, const SkPoint& center,
                                              SkScalar radius) {
    SkASSERT(radius >= 0);
    return sk_sp<GrFragmentProcessor>(new CircleEffect(edgeType, center, radius));
}

void CircleEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    inout->mulByUnknownSingleComponent();
}

CircleEffect::CircleEffect(GrPrimitiveEdgeType edgeType, const SkPoint& c, SkScalar r)
    : fCenter(c)
    , fRadius(r)
    , fEdgeType(edgeType) {
    this->initClassID<CircleEffect>();
    this->setWillReadFragmentPosition();
}

bool CircleEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const CircleEffect& ce = other.cast<CircleEffect>();
    return fEdgeType == ce.fEdgeType && fCenter == ce.fCenter && fRadius == ce.fRadius;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(CircleEffect);

sk_sp<GrFragmentProcessor> CircleEffect::TestCreate(GrProcessorTestData* d) {
    SkPoint center;
    center.fX = d->fRandom->nextRangeScalar(0.f, 1000.f);
    center.fY = d->fRandom->nextRangeScalar(0.f, 1000.f);
    SkScalar radius = d->fRandom->nextRangeF(0.f, 1000.f);
    GrPrimitiveEdgeType et;
    do {
        et = (GrPrimitiveEdgeType)d->fRandom->nextULessThan(kGrProcessorEdgeTypeCnt);
    } while (kHairlineAA_GrProcessorEdgeType == et);
    return CircleEffect::Make(et, center, radius);
}

//////////////////////////////////////////////////////////////////////////////

class GLCircleEffect : public GrGLSLFragmentProcessor {
public:
    GLCircleEffect() : fPrevRadius(-1.0f) { }

    virtual void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    GrGLSLProgramDataManager::UniformHandle fCircleUniform;
    SkPoint                                 fPrevCenter;
    SkScalar                                fPrevRadius;

    typedef GrGLSLFragmentProcessor INHERITED;
};

void GLCircleEffect::emitCode(EmitArgs& args) {
    const CircleEffect& ce = args.fFp.cast<CircleEffect>();
    const char *circleName;
    // The circle uniform is (center.x, center.y, radius + 0.5, 1 / (radius + 0.5)) for regular
    // fills and (..., radius - 0.5, 1 / (radius - 0.5)) for inverse fills.
    fCircleUniform = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                      kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                      "circle",
                                                      &circleName);

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    const char* fragmentPos = fragBuilder->fragmentPosition();

    SkASSERT(kHairlineAA_GrProcessorEdgeType != ce.getEdgeType());
    // TODO: Right now the distance to circle caclulation is performed in a space normalized to the
    // radius and then denormalized. This is to prevent overflow on devices that have a "real"
    // mediump. It'd be nice to only to this on mediump devices but we currently don't have the
    // caps here.
    if (GrProcessorEdgeTypeIsInverseFill(ce.getEdgeType())) {
        fragBuilder->codeAppendf("float d = (length((%s.xy - %s.xy) * %s.w) - 1.0) * %s.z;",
                                 circleName, fragmentPos, circleName, circleName);
    } else {
        fragBuilder->codeAppendf("float d = (1.0 - length((%s.xy - %s.xy) *  %s.w)) * %s.z;",
                                 circleName, fragmentPos, circleName, circleName);
    }
    if (GrProcessorEdgeTypeIsAA(ce.getEdgeType())) {
        fragBuilder->codeAppend("d = clamp(d, 0.0, 1.0);");
    } else {
        fragBuilder->codeAppend("d = d > 0.5 ? 1.0 : 0.0;");
    }

    fragBuilder->codeAppendf("%s = %s;", args.fOutputColor,
                             (GrGLSLExpr4(args.fInputColor) * GrGLSLExpr1("d")).c_str());
}

void GLCircleEffect::GenKey(const GrProcessor& processor, const GrGLSLCaps&,
                            GrProcessorKeyBuilder* b) {
    const CircleEffect& ce = processor.cast<CircleEffect>();
    b->add32(ce.getEdgeType());
}

void GLCircleEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                               const GrProcessor& processor) {
    const CircleEffect& ce = processor.cast<CircleEffect>();
    if (ce.getRadius() != fPrevRadius || ce.getCenter() != fPrevCenter) {
        SkScalar radius = ce.getRadius();
        if (GrProcessorEdgeTypeIsInverseFill(ce.getEdgeType())) {
            radius -= 0.5f;
        } else {
            radius += 0.5f;
        }
        pdman.set4f(fCircleUniform, ce.getCenter().fX, ce.getCenter().fY, radius,
                    SkScalarInvert(radius));
        fPrevCenter = ce.getCenter();
        fPrevRadius = ce.getRadius();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void CircleEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                         GrProcessorKeyBuilder* b) const {
    GLCircleEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* CircleEffect::onCreateGLSLInstance() const  {
    return new GLCircleEffect;
}

//////////////////////////////////////////////////////////////////////////////

class EllipseEffect : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make(GrPrimitiveEdgeType, const SkPoint& center,
                                           SkScalar rx, SkScalar ry);

    virtual ~EllipseEffect() {};

    const char* name() const override { return "Ellipse"; }

    const SkPoint& getCenter() const { return fCenter; }
    SkVector getRadii() const { return fRadii; }

    GrPrimitiveEdgeType getEdgeType() const { return fEdgeType; }

private:
    EllipseEffect(GrPrimitiveEdgeType, const SkPoint& center, SkScalar rx, SkScalar ry);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    SkPoint             fCenter;
    SkVector            fRadii;
    GrPrimitiveEdgeType    fEdgeType;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

sk_sp<GrFragmentProcessor> EllipseEffect::Make(GrPrimitiveEdgeType edgeType,
                                               const SkPoint& center,
                                               SkScalar rx,
                                               SkScalar ry) {
    SkASSERT(rx >= 0 && ry >= 0);
    return sk_sp<GrFragmentProcessor>(new EllipseEffect(edgeType, center, rx, ry));
}

void EllipseEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    inout->mulByUnknownSingleComponent();
}

EllipseEffect::EllipseEffect(GrPrimitiveEdgeType edgeType, const SkPoint& c, SkScalar rx, SkScalar ry)
    : fCenter(c)
    , fRadii(SkVector::Make(rx, ry))
    , fEdgeType(edgeType) {
    this->initClassID<EllipseEffect>();
    this->setWillReadFragmentPosition();
}

bool EllipseEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const EllipseEffect& ee = other.cast<EllipseEffect>();
    return fEdgeType == ee.fEdgeType && fCenter == ee.fCenter && fRadii == ee.fRadii;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(EllipseEffect);

sk_sp<GrFragmentProcessor> EllipseEffect::TestCreate(GrProcessorTestData* d) {
    SkPoint center;
    center.fX = d->fRandom->nextRangeScalar(0.f, 1000.f);
    center.fY = d->fRandom->nextRangeScalar(0.f, 1000.f);
    SkScalar rx = d->fRandom->nextRangeF(0.f, 1000.f);
    SkScalar ry = d->fRandom->nextRangeF(0.f, 1000.f);
    GrPrimitiveEdgeType et;
    do {
        et = (GrPrimitiveEdgeType)d->fRandom->nextULessThan(kGrProcessorEdgeTypeCnt);
    } while (kHairlineAA_GrProcessorEdgeType == et);
    return EllipseEffect::Make(et, center, rx, ry);
}

//////////////////////////////////////////////////////////////////////////////

class GLEllipseEffect : public GrGLSLFragmentProcessor {
public:
    GLEllipseEffect() {
        fPrevRadii.fX = -1.0f;
    }

    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    GrGLSLProgramDataManager::UniformHandle fEllipseUniform;
    GrGLSLProgramDataManager::UniformHandle fScaleUniform;
    SkPoint                                 fPrevCenter;
    SkVector                                fPrevRadii;

    typedef GrGLSLFragmentProcessor INHERITED;
};

void GLEllipseEffect::emitCode(EmitArgs& args) {
    const EllipseEffect& ee = args.fFp.cast<EllipseEffect>();
    const char *ellipseName;
    // The ellipse uniform is (center.x, center.y, 1 / rx^2, 1 / ry^2)
    // The last two terms can underflow on mediump, so we use highp.
    fEllipseUniform = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                       kVec4f_GrSLType, kHigh_GrSLPrecision,
                                                       "ellipse",
                                                       &ellipseName);
    // If we're on a device with a "real" mediump then we'll do the distance computation in a space
    // that is normalized by the larger radius. The scale uniform will be scale, 1/scale. The
    // inverse squared radii uniform values are already in this normalized space. The center is
    // not.
    const char* scaleName = nullptr;
    if (args.fGLSLCaps->floatPrecisionVaries()) {
        fScaleUniform = args.fUniformHandler->addUniform(
            kFragment_GrShaderFlag, kVec2f_GrSLType, kDefault_GrSLPrecision,
            "scale", &scaleName);
    }

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    const char* fragmentPos = fragBuilder->fragmentPosition();

    // d is the offset to the ellipse center
    fragBuilder->codeAppendf("vec2 d = %s.xy - %s.xy;", fragmentPos, ellipseName);
    if (scaleName) {
        fragBuilder->codeAppendf("d *= %s.y;", scaleName);
    }
    fragBuilder->codeAppendf("vec2 Z = d * %s.zw;", ellipseName);
    // implicit is the evaluation of (x/rx)^2 + (y/ry)^2 - 1.
    fragBuilder->codeAppend("float implicit = dot(Z, d) - 1.0;");
    // grad_dot is the squared length of the gradient of the implicit.
    fragBuilder->codeAppendf("float grad_dot = 4.0 * dot(Z, Z);");
    // Avoid calling inversesqrt on zero.
    fragBuilder->codeAppend("grad_dot = max(grad_dot, 1.0e-4);");
    fragBuilder->codeAppendf("float approx_dist = implicit * inversesqrt(grad_dot);");
    if (scaleName) {
        fragBuilder->codeAppendf("approx_dist *= %s.x;", scaleName);
    }

    switch (ee.getEdgeType()) {
        case kFillAA_GrProcessorEdgeType:
            fragBuilder->codeAppend("float alpha = clamp(0.5 - approx_dist, 0.0, 1.0);");
            break;
        case kInverseFillAA_GrProcessorEdgeType:
            fragBuilder->codeAppend("float alpha = clamp(0.5 + approx_dist, 0.0, 1.0);");
            break;
        case kFillBW_GrProcessorEdgeType:
            fragBuilder->codeAppend("float alpha = approx_dist > 0.0 ? 0.0 : 1.0;");
            break;
        case kInverseFillBW_GrProcessorEdgeType:
            fragBuilder->codeAppend("float alpha = approx_dist > 0.0 ? 1.0 : 0.0;");
            break;
        case kHairlineAA_GrProcessorEdgeType:
            SkFAIL("Hairline not expected here.");
    }

    fragBuilder->codeAppendf("%s = %s;", args.fOutputColor,
                             (GrGLSLExpr4(args.fInputColor) * GrGLSLExpr1("alpha")).c_str());
}

void GLEllipseEffect::GenKey(const GrProcessor& effect, const GrGLSLCaps&,
                             GrProcessorKeyBuilder* b) {
    const EllipseEffect& ee = effect.cast<EllipseEffect>();
    b->add32(ee.getEdgeType());
}

void GLEllipseEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                const GrProcessor& effect) {
    const EllipseEffect& ee = effect.cast<EllipseEffect>();
    if (ee.getRadii() != fPrevRadii || ee.getCenter() != fPrevCenter) {
        float invRXSqd;
        float invRYSqd;
        // If we're using a scale factor to work around precision issues, choose the larger radius
        // as the scale factor. The inv radii need to be pre-adjusted by the scale factor.
        if (fScaleUniform.isValid()) {
            if (ee.getRadii().fX > ee.getRadii().fY) {
                invRXSqd = 1.f;
                invRYSqd = (ee.getRadii().fX * ee.getRadii().fX) /
                           (ee.getRadii().fY * ee.getRadii().fY);
                pdman.set2f(fScaleUniform, ee.getRadii().fX, 1.f / ee.getRadii().fX);
            } else {
                invRXSqd = (ee.getRadii().fY * ee.getRadii().fY) /
                           (ee.getRadii().fX * ee.getRadii().fX);
                invRYSqd = 1.f;
                pdman.set2f(fScaleUniform, ee.getRadii().fY, 1.f / ee.getRadii().fY);
            }
        } else {
            invRXSqd = 1.f / (ee.getRadii().fX * ee.getRadii().fX);
            invRYSqd = 1.f / (ee.getRadii().fY * ee.getRadii().fY);
        }
        pdman.set4f(fEllipseUniform, ee.getCenter().fX, ee.getCenter().fY, invRXSqd, invRYSqd);
        fPrevCenter = ee.getCenter();
        fPrevRadii = ee.getRadii();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EllipseEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                          GrProcessorKeyBuilder* b) const {
    GLEllipseEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* EllipseEffect::onCreateGLSLInstance() const  {
    return new GLEllipseEffect;
}

//////////////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> GrOvalEffect::Make(GrPrimitiveEdgeType edgeType, const SkRect& oval) {
    if (kHairlineAA_GrProcessorEdgeType == edgeType) {
        return nullptr;
    }
    SkScalar w = oval.width();
    SkScalar h = oval.height();
    if (SkScalarNearlyEqual(w, h)) {
        w /= 2;
        return CircleEffect::Make(edgeType, SkPoint::Make(oval.fLeft + w, oval.fTop + w), w);
    } else {
        w /= 2;
        h /= 2;
        return EllipseEffect::Make(edgeType, SkPoint::Make(oval.fLeft + w, oval.fTop + h), w, h);
    }

    return nullptr;
}
