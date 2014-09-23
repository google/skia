/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/builders/GrGLProgramBuilder.h"
#include "GrOvalEffect.h"

#include "gl/GrGLProcessor.h"
#include "gl/GrGLSL.h"
#include "GrTBackendProcessorFactory.h"

#include "SkRect.h"

//////////////////////////////////////////////////////////////////////////////

class GLCircleEffect;

class CircleEffect : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create(GrPrimitiveEdgeType, const SkPoint& center, SkScalar radius);

    virtual ~CircleEffect() {};
    static const char* Name() { return "Circle"; }

    const SkPoint& getCenter() const { return fCenter; }
    SkScalar getRadius() const { return fRadius; }

    GrPrimitiveEdgeType getEdgeType() const { return fEdgeType; }

    typedef GLCircleEffect GLProcessor;

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    virtual const GrBackendFragmentProcessorFactory& getFactory() const SK_OVERRIDE;

private:
    CircleEffect(GrPrimitiveEdgeType, const SkPoint& center, SkScalar radius);

    virtual bool onIsEqual(const GrProcessor&) const SK_OVERRIDE;

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

void CircleEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
}

const GrBackendFragmentProcessorFactory& CircleEffect::getFactory() const {
    return GrTBackendFragmentProcessorFactory<CircleEffect>::getInstance();
}

CircleEffect::CircleEffect(GrPrimitiveEdgeType edgeType, const SkPoint& c, SkScalar r)
    : fCenter(c)
    , fRadius(r)
    , fEdgeType(edgeType) {
    this->setWillReadFragmentPosition();
}

bool CircleEffect::onIsEqual(const GrProcessor& other) const {
    const CircleEffect& ce = other.cast<CircleEffect>();
    return fEdgeType == ce.fEdgeType && fCenter == ce.fCenter && fRadius == ce.fRadius;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(CircleEffect);

GrFragmentProcessor* CircleEffect::TestCreate(SkRandom* random,
                                              GrContext*,
                                              const GrDrawTargetCaps& caps,
                                              GrTexture*[]) {
    SkPoint center;
    center.fX = random->nextRangeScalar(0.f, 1000.f);
    center.fY = random->nextRangeScalar(0.f, 1000.f);
    SkScalar radius = random->nextRangeF(0.f, 1000.f);
    GrPrimitiveEdgeType et;
    do {
        et = (GrPrimitiveEdgeType)random->nextULessThan(kGrProcessorEdgeTypeCnt);
    } while (kHairlineAA_GrProcessorEdgeType == et);
    return CircleEffect::Create(et, center, radius);
}

//////////////////////////////////////////////////////////////////////////////

class GLCircleEffect : public GrGLFragmentProcessor {
public:
    GLCircleEffect(const GrBackendProcessorFactory&, const GrProcessor&);

    virtual void emitCode(GrGLProgramBuilder* builder,
                          const GrFragmentProcessor& fp,
                          const GrProcessorKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrProcessor&, const GrGLCaps&, GrProcessorKeyBuilder*);

    virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE;

private:
    GrGLProgramDataManager::UniformHandle fCircleUniform;
    SkPoint                               fPrevCenter;
    SkScalar                              fPrevRadius;

    typedef GrGLFragmentProcessor INHERITED;
};

GLCircleEffect::GLCircleEffect(const GrBackendProcessorFactory& factory,
                               const GrProcessor&)
    : INHERITED (factory) {
    fPrevRadius = -1.f;
}

void GLCircleEffect::emitCode(GrGLProgramBuilder* builder,
                              const GrFragmentProcessor& fp,
                              const GrProcessorKey& key,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray&,
                              const TextureSamplerArray& samplers) {
    const CircleEffect& ce = fp.cast<CircleEffect>();
    const char *circleName;
    // The circle uniform is (center.x, center.y, radius + 0.5) for regular fills and
    // (... ,radius - 0.5) for inverse fills.
    fCircleUniform = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                         kVec3f_GrSLType,
                                         "circle",
                                         &circleName);

    GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    const char* fragmentPos = fsBuilder->fragmentPosition();

    SkASSERT(kHairlineAA_GrProcessorEdgeType != ce.getEdgeType());
    if (GrProcessorEdgeTypeIsInverseFill(ce.getEdgeType())) {
        fsBuilder->codeAppendf("\t\tfloat d = length(%s.xy - %s.xy) - %s.z;\n",
                                circleName, fragmentPos, circleName);
    } else {
        fsBuilder->codeAppendf("\t\tfloat d = %s.z - length(%s.xy - %s.xy);\n",
                               circleName, fragmentPos, circleName);
    }
    if (GrProcessorEdgeTypeIsAA(ce.getEdgeType())) {
        fsBuilder->codeAppend("\t\td = clamp(d, 0.0, 1.0);\n");
    } else {
        fsBuilder->codeAppend("\t\td = d > 0.5 ? 1.0 : 0.0;\n");
    }

    fsBuilder->codeAppendf("\t\t%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("d")).c_str());
}

void GLCircleEffect::GenKey(const GrProcessor& processor, const GrGLCaps&,
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
        pdman.set3f(fCircleUniform, ce.getCenter().fX, ce.getCenter().fY, radius);
        fPrevCenter = ce.getCenter();
        fPrevRadius = ce.getRadius();
    }
}

//////////////////////////////////////////////////////////////////////////////

class GLEllipseEffect;

class EllipseEffect : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create(GrPrimitiveEdgeType, const SkPoint& center, SkScalar rx,
                                       SkScalar ry);

    virtual ~EllipseEffect() {};
    static const char* Name() { return "Ellipse"; }

    const SkPoint& getCenter() const { return fCenter; }
    SkVector getRadii() const { return fRadii; }

    GrPrimitiveEdgeType getEdgeType() const { return fEdgeType; }

    typedef GLEllipseEffect GLProcessor;

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    virtual const GrBackendFragmentProcessorFactory& getFactory() const SK_OVERRIDE;

private:
    EllipseEffect(GrPrimitiveEdgeType, const SkPoint& center, SkScalar rx, SkScalar ry);

    virtual bool onIsEqual(const GrProcessor&) const SK_OVERRIDE;

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

void EllipseEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
}

const GrBackendFragmentProcessorFactory& EllipseEffect::getFactory() const {
    return GrTBackendFragmentProcessorFactory<EllipseEffect>::getInstance();
}

EllipseEffect::EllipseEffect(GrPrimitiveEdgeType edgeType, const SkPoint& c, SkScalar rx, SkScalar ry)
    : fCenter(c)
    , fRadii(SkVector::Make(rx, ry))
    , fEdgeType(edgeType) {
    this->setWillReadFragmentPosition();
}

bool EllipseEffect::onIsEqual(const GrProcessor& other) const {
    const EllipseEffect& ee = other.cast<EllipseEffect>();
    return fEdgeType == ee.fEdgeType && fCenter == ee.fCenter && fRadii == ee.fRadii;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(EllipseEffect);

GrFragmentProcessor* EllipseEffect::TestCreate(SkRandom* random,
                                               GrContext*,
                                               const GrDrawTargetCaps& caps,
                                               GrTexture*[]) {
    SkPoint center;
    center.fX = random->nextRangeScalar(0.f, 1000.f);
    center.fY = random->nextRangeScalar(0.f, 1000.f);
    SkScalar rx = random->nextRangeF(0.f, 1000.f);
    SkScalar ry = random->nextRangeF(0.f, 1000.f);
    GrPrimitiveEdgeType et;
    do {
        et = (GrPrimitiveEdgeType)random->nextULessThan(kGrProcessorEdgeTypeCnt);
    } while (kHairlineAA_GrProcessorEdgeType == et);
    return EllipseEffect::Create(et, center, rx, ry);
}

//////////////////////////////////////////////////////////////////////////////

class GLEllipseEffect : public GrGLFragmentProcessor {
public:
    GLEllipseEffect(const GrBackendProcessorFactory&, const GrProcessor&);

    virtual void emitCode(GrGLProgramBuilder* builder,
                          const GrFragmentProcessor& fp,
                          const GrProcessorKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrProcessor&, const GrGLCaps&, GrProcessorKeyBuilder*);

    virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE;

private:
    GrGLProgramDataManager::UniformHandle fEllipseUniform;
    SkPoint                               fPrevCenter;
    SkVector                              fPrevRadii;

    typedef GrGLFragmentProcessor INHERITED;
};

GLEllipseEffect::GLEllipseEffect(const GrBackendProcessorFactory& factory,
                                 const GrProcessor& effect)
    : INHERITED (factory) {
    fPrevRadii.fX = -1.f;
}

void GLEllipseEffect::emitCode(GrGLProgramBuilder* builder,
                               const GrFragmentProcessor& fp,
                               const GrProcessorKey& key,
                               const char* outputColor,
                               const char* inputColor,
                               const TransformedCoordsArray&,
                               const TextureSamplerArray& samplers) {
    const EllipseEffect& ee = fp.cast<EllipseEffect>();
    const char *ellipseName;
    // The ellipse uniform is (center.x, center.y, 1 / rx^2, 1 / ry^2)
    fEllipseUniform = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                         kVec4f_GrSLType,
                                         "ellipse",
                                         &ellipseName);

    GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
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

    fsBuilder->codeAppendf("\t\t%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("alpha")).c_str());
}

void GLEllipseEffect::GenKey(const GrProcessor& effect, const GrGLCaps&,
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
