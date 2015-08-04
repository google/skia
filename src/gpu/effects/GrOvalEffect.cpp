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
#include "gl/GrGLFragmentProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

//////////////////////////////////////////////////////////////////////////////

class CircleEffect : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create(GrPrimitiveEdgeType, const SkPoint& center, SkScalar radius);

    virtual ~CircleEffect() {};

    const char* name() const override { return "Circle"; }

    GrGLFragmentProcessor* createGLInstance() const override;

    const SkPoint& getCenter() const { return fCenter; }
    SkScalar getRadius() const { return fRadius; }

    GrPrimitiveEdgeType getEdgeType() const { return fEdgeType; }

private:
    CircleEffect(GrPrimitiveEdgeType, const SkPoint& center, SkScalar radius);

    void onGetGLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    SkPoint             fCenter;
    SkScalar            fRadius;
    GrPrimitiveEdgeType    fEdgeType;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

GrFragmentProcessor* CircleEffect::Create(GrPrimitiveEdgeType edgeType, const SkPoint& center,
                                          SkScalar radius) {
    SkASSERT(radius >= 0);
    return SkNEW_ARGS(CircleEffect, (edgeType, center, radius));
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

GrFragmentProcessor* CircleEffect::TestCreate(GrProcessorTestData* d) {
    SkPoint center;
    center.fX = d->fRandom->nextRangeScalar(0.f, 1000.f);
    center.fY = d->fRandom->nextRangeScalar(0.f, 1000.f);
    SkScalar radius = d->fRandom->nextRangeF(0.f, 1000.f);
    GrPrimitiveEdgeType et;
    do {
        et = (GrPrimitiveEdgeType)d->fRandom->nextULessThan(kGrProcessorEdgeTypeCnt);
    } while (kHairlineAA_GrProcessorEdgeType == et);
    return CircleEffect::Create(et, center, radius);
}

//////////////////////////////////////////////////////////////////////////////

class GLCircleEffect : public GrGLFragmentProcessor {
public:
    GLCircleEffect(const GrProcessor&);

    virtual void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*);

    void setData(const GrGLProgramDataManager&, const GrProcessor&) override;

private:
    GrGLProgramDataManager::UniformHandle fCircleUniform;
    SkPoint                               fPrevCenter;
    SkScalar                              fPrevRadius;

    typedef GrGLFragmentProcessor INHERITED;
};

GLCircleEffect::GLCircleEffect(const GrProcessor&) {
    fPrevRadius = -1.f;
}

void GLCircleEffect::emitCode(EmitArgs& args) {
    const CircleEffect& ce = args.fFp.cast<CircleEffect>();
    const char *circleName;
    // The circle uniform is (center.x, center.y, radius + 0.5, 1 / (radius + 0.5)) for regular
    // fills and (..., radius - 0.5, 1 / (radius - 0.5)) for inverse fills.
    fCircleUniform = args.fBuilder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                         kVec4f_GrSLType, kDefault_GrSLPrecision,
                                         "circle",
                                         &circleName);

    GrGLFragmentBuilder* fsBuilder = args.fBuilder->getFragmentShaderBuilder();
    const char* fragmentPos = fsBuilder->fragmentPosition();

    SkASSERT(kHairlineAA_GrProcessorEdgeType != ce.getEdgeType());
    // TODO: Right now the distance to circle caclulation is performed in a space normalized to the
    // radius and then denormalized. This is to prevent overflow on devices that have a "real"
    // mediump. It'd be nice to only to this on mediump devices but we currently don't have the
    // caps here.
    if (GrProcessorEdgeTypeIsInverseFill(ce.getEdgeType())) {
        fsBuilder->codeAppendf("\t\tfloat d = (length((%s.xy - %s.xy) * %s.w) - 1.0) * %s.z;\n",
                                circleName, fragmentPos, circleName, circleName);
    } else {
        fsBuilder->codeAppendf("\t\tfloat d = (1.0 - length((%s.xy - %s.xy) *  %s.w)) * %s.z;\n",
                               circleName, fragmentPos, circleName, circleName);
    }
    if (GrProcessorEdgeTypeIsAA(ce.getEdgeType())) {
        fsBuilder->codeAppend("\t\td = clamp(d, 0.0, 1.0);\n");
    } else {
        fsBuilder->codeAppend("\t\td = d > 0.5 ? 1.0 : 0.0;\n");
    }

    fsBuilder->codeAppendf("\t\t%s = %s;\n", args.fOutputColor,
                           (GrGLSLExpr4(args.fInputColor) * GrGLSLExpr1("d")).c_str());
}

void GLCircleEffect::GenKey(const GrProcessor& processor, const GrGLSLCaps&,
                            GrProcessorKeyBuilder* b) {
    const CircleEffect& ce = processor.cast<CircleEffect>();
    b->add32(ce.getEdgeType());
}

void GLCircleEffect::setData(const GrGLProgramDataManager& pdman, const GrProcessor& processor) {
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

void CircleEffect::onGetGLProcessorKey(const GrGLSLCaps& caps,
                                     GrProcessorKeyBuilder* b) const {
    GLCircleEffect::GenKey(*this, caps, b);
}

GrGLFragmentProcessor* CircleEffect::createGLInstance() const  {
    return SkNEW_ARGS(GLCircleEffect, (*this));
}

//////////////////////////////////////////////////////////////////////////////

class EllipseEffect : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create(GrPrimitiveEdgeType, const SkPoint& center, SkScalar rx,
                                       SkScalar ry);

    virtual ~EllipseEffect() {};

    const char* name() const override { return "Ellipse"; }

    GrGLFragmentProcessor* createGLInstance() const override;

    const SkPoint& getCenter() const { return fCenter; }
    SkVector getRadii() const { return fRadii; }

    GrPrimitiveEdgeType getEdgeType() const { return fEdgeType; }

private:
    EllipseEffect(GrPrimitiveEdgeType, const SkPoint& center, SkScalar rx, SkScalar ry);

    void onGetGLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    SkPoint             fCenter;
    SkVector            fRadii;
    GrPrimitiveEdgeType    fEdgeType;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

GrFragmentProcessor* EllipseEffect::Create(GrPrimitiveEdgeType edgeType,
                                           const SkPoint& center,
                                           SkScalar rx,
                                           SkScalar ry) {
    SkASSERT(rx >= 0 && ry >= 0);
    return SkNEW_ARGS(EllipseEffect, (edgeType, center, rx, ry));
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

GrFragmentProcessor* EllipseEffect::TestCreate(GrProcessorTestData* d) {
    SkPoint center;
    center.fX = d->fRandom->nextRangeScalar(0.f, 1000.f);
    center.fY = d->fRandom->nextRangeScalar(0.f, 1000.f);
    SkScalar rx = d->fRandom->nextRangeF(0.f, 1000.f);
    SkScalar ry = d->fRandom->nextRangeF(0.f, 1000.f);
    GrPrimitiveEdgeType et;
    do {
        et = (GrPrimitiveEdgeType)d->fRandom->nextULessThan(kGrProcessorEdgeTypeCnt);
    } while (kHairlineAA_GrProcessorEdgeType == et);
    return EllipseEffect::Create(et, center, rx, ry);
}

//////////////////////////////////////////////////////////////////////////////

class GLEllipseEffect : public GrGLFragmentProcessor {
public:
    GLEllipseEffect(const GrProcessor&);

    virtual void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*);

    void setData(const GrGLProgramDataManager&, const GrProcessor&) override;

private:
    GrGLProgramDataManager::UniformHandle fEllipseUniform;
    SkPoint                               fPrevCenter;
    SkVector                              fPrevRadii;

    typedef GrGLFragmentProcessor INHERITED;
};

GLEllipseEffect::GLEllipseEffect(const GrProcessor& effect) {
    fPrevRadii.fX = -1.f;
}

void GLEllipseEffect::emitCode(EmitArgs& args) {
    const EllipseEffect& ee = args.fFp.cast<EllipseEffect>();
    const char *ellipseName;
    // The ellipse uniform is (center.x, center.y, 1 / rx^2, 1 / ry^2)
    // The last two terms can underflow on mediump, so we use highp.
    fEllipseUniform = args.fBuilder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                         kVec4f_GrSLType, kHigh_GrSLPrecision,
                                         "ellipse",
                                         &ellipseName);

    GrGLFragmentBuilder* fsBuilder = args.fBuilder->getFragmentShaderBuilder();
    const char* fragmentPos = fsBuilder->fragmentPosition();

    // d is the offset to the ellipse center
    fsBuilder->codeAppendf("\t\tvec2 d = %s.xy - %s.xy;\n", fragmentPos, ellipseName);
    fsBuilder->codeAppendf("\t\tvec2 Z = d * %s.zw;\n", ellipseName);
    // implicit is the evaluation of (x/rx)^2 + (y/ry)^2 - 1.
    fsBuilder->codeAppend("\t\tfloat implicit = dot(Z, d) - 1.0;\n");
    // grad_dot is the squared length of the gradient of the implicit.
    fsBuilder->codeAppendf("\t\tfloat grad_dot = 4.0 * dot(Z, Z);\n");
    // avoid calling inversesqrt on zero.
    fsBuilder->codeAppend("\t\tgrad_dot = max(grad_dot, 1.0e-4);\n");
    fsBuilder->codeAppendf("\t\tfloat approx_dist = implicit * inversesqrt(grad_dot);\n");

    switch (ee.getEdgeType()) {
        case kFillAA_GrProcessorEdgeType:
            fsBuilder->codeAppend("\t\tfloat alpha = clamp(0.5 - approx_dist, 0.0, 1.0);\n");
            break;
        case kInverseFillAA_GrProcessorEdgeType:
            fsBuilder->codeAppend("\t\tfloat alpha = clamp(0.5 + approx_dist, 0.0, 1.0);\n");
            break;
        case kFillBW_GrProcessorEdgeType:
            fsBuilder->codeAppend("\t\tfloat alpha = approx_dist > 0.0 ? 0.0 : 1.0;\n");
            break;
        case kInverseFillBW_GrProcessorEdgeType:
            fsBuilder->codeAppend("\t\tfloat alpha = approx_dist > 0.0 ? 1.0 : 0.0;\n");
            break;
        case kHairlineAA_GrProcessorEdgeType:
            SkFAIL("Hairline not expected here.");
    }

    fsBuilder->codeAppendf("\t\t%s = %s;\n", args.fOutputColor,
                           (GrGLSLExpr4(args.fInputColor) * GrGLSLExpr1("alpha")).c_str());
}

void GLEllipseEffect::GenKey(const GrProcessor& effect, const GrGLSLCaps&,
                             GrProcessorKeyBuilder* b) {
    const EllipseEffect& ee = effect.cast<EllipseEffect>();
    b->add32(ee.getEdgeType());
}

void GLEllipseEffect::setData(const GrGLProgramDataManager& pdman, const GrProcessor& effect) {
    const EllipseEffect& ee = effect.cast<EllipseEffect>();
    if (ee.getRadii() != fPrevRadii || ee.getCenter() != fPrevCenter) {
        SkScalar invRXSqd = 1.f / (ee.getRadii().fX * ee.getRadii().fX);
        SkScalar invRYSqd = 1.f / (ee.getRadii().fY * ee.getRadii().fY);
        pdman.set4f(fEllipseUniform, ee.getCenter().fX, ee.getCenter().fY, invRXSqd, invRYSqd);
        fPrevCenter = ee.getCenter();
        fPrevRadii = ee.getRadii();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EllipseEffect::onGetGLProcessorKey(const GrGLSLCaps& caps,
                                     GrProcessorKeyBuilder* b) const {
    GLEllipseEffect::GenKey(*this, caps, b);
}

GrGLFragmentProcessor* EllipseEffect::createGLInstance() const  {
    return SkNEW_ARGS(GLEllipseEffect, (*this));
}

//////////////////////////////////////////////////////////////////////////////

GrFragmentProcessor* GrOvalEffect::Create(GrPrimitiveEdgeType edgeType, const SkRect& oval) {
    if (kHairlineAA_GrProcessorEdgeType == edgeType) {
        return NULL;
    }
    SkScalar w = oval.width();
    SkScalar h = oval.height();
    if (SkScalarNearlyEqual(w, h)) {
        w /= 2;
        return CircleEffect::Create(edgeType, SkPoint::Make(oval.fLeft + w, oval.fTop + w), w);
    } else {
        w /= 2;
        h /= 2;
        return EllipseEffect::Create(edgeType, SkPoint::Make(oval.fLeft + w, oval.fTop + h), w, h);
    }

    return NULL;
}
