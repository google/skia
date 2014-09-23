/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/builders/GrGLProgramBuilder.h"
#include "GrConvexPolyEffect.h"

#include "gl/GrGLProcessor.h"
#include "gl/GrGLSL.h"
#include "GrTBackendProcessorFactory.h"

#include "SkPath.h"

//////////////////////////////////////////////////////////////////////////////
class GLAARectEffect;

class AARectEffect : public GrFragmentProcessor {
public:
    typedef GLAARectEffect GLProcessor;

    const SkRect& getRect() const { return fRect; }

    static const char* Name() { return "AARect"; }

    static GrFragmentProcessor* Create(GrPrimitiveEdgeType edgeType, const SkRect& rect) {
        return SkNEW_ARGS(AARectEffect, (edgeType, rect));
    }

    virtual void getConstantColorComponents(GrColor* color,
                                            uint32_t* validFlags) const SK_OVERRIDE {
        if (fRect.isEmpty()) {
            // An empty rect will have no coverage anywhere.
            *color = 0x00000000;
            *validFlags = kRGBA_GrColorComponentFlags;
        } else {
            *validFlags = 0;
        }
    }

    GrPrimitiveEdgeType getEdgeType() const { return fEdgeType; }

    virtual const GrBackendFragmentProcessorFactory& getFactory() const SK_OVERRIDE;

private:
    AARectEffect(GrPrimitiveEdgeType edgeType, const SkRect& rect) : fRect(rect), fEdgeType(edgeType) {
        this->setWillReadFragmentPosition();
    }

    virtual bool onIsEqual(const GrProcessor& other) const SK_OVERRIDE {
        const AARectEffect& aare = other.cast<AARectEffect>();
        return fRect == aare.fRect;
    }

    SkRect              fRect;
    GrPrimitiveEdgeType fEdgeType;

    typedef GrFragmentProcessor INHERITED;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

};

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(AARectEffect);

GrFragmentProcessor* AARectEffect::TestCreate(SkRandom* random,
                                              GrContext*,
                                              const GrDrawTargetCaps& caps,
                                              GrTexture*[]) {
    SkRect rect = SkRect::MakeLTRB(random->nextSScalar1(),
                                   random->nextSScalar1(),
                                   random->nextSScalar1(),
                                   random->nextSScalar1());
    GrFragmentProcessor* fp;
    do {
        GrPrimitiveEdgeType edgeType = static_cast<GrPrimitiveEdgeType>(random->nextULessThan(
                                                                    kGrProcessorEdgeTypeCnt));

        fp = AARectEffect::Create(edgeType, rect);
    } while (NULL == fp);
    return fp;
}

//////////////////////////////////////////////////////////////////////////////

class GLAARectEffect : public GrGLFragmentProcessor {
public:
    GLAARectEffect(const GrBackendProcessorFactory&, const GrProcessor&);

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
    GrGLProgramDataManager::UniformHandle fRectUniform;
    SkRect                                fPrevRect;
    typedef GrGLFragmentProcessor INHERITED;
};

GLAARectEffect::GLAARectEffect(const GrBackendProcessorFactory& factory,
                               const GrProcessor& effect)
    : INHERITED (factory) {
    fPrevRect.fLeft = SK_ScalarNaN;
}

void GLAARectEffect::emitCode(GrGLProgramBuilder* builder,
                              const GrFragmentProcessor& fp,
                              const GrProcessorKey& key,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray&,
                              const TextureSamplerArray& samplers) {
    const AARectEffect& aare = fp.cast<AARectEffect>();
    const char *rectName;
    // The rect uniform's xyzw refer to (left + 0.5, top + 0.5, right - 0.5, bottom - 0.5),
    // respectively.
    fRectUniform = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                       kVec4f_GrSLType,
                                       "rect",
                                       &rectName);

    GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    const char* fragmentPos = fsBuilder->fragmentPosition();
    if (GrProcessorEdgeTypeIsAA(aare.getEdgeType())) {
        // The amount of coverage removed in x and y by the edges is computed as a pair of negative
        // numbers, xSub and ySub.
        fsBuilder->codeAppend("\t\tfloat xSub, ySub;\n");
        fsBuilder->codeAppendf("\t\txSub = min(%s.x - %s.x, 0.0);\n", fragmentPos, rectName);
        fsBuilder->codeAppendf("\t\txSub += min(%s.z - %s.x, 0.0);\n", rectName, fragmentPos);
        fsBuilder->codeAppendf("\t\tySub = min(%s.y - %s.y, 0.0);\n", fragmentPos, rectName);
        fsBuilder->codeAppendf("\t\tySub += min(%s.w - %s.y, 0.0);\n", rectName, fragmentPos);
        // Now compute coverage in x and y and multiply them to get the fraction of the pixel
        // covered.
        fsBuilder->codeAppendf("\t\tfloat alpha = (1.0 + max(xSub, -1.0)) * (1.0 + max(ySub, -1.0));\n");
    } else {
        fsBuilder->codeAppendf("\t\tfloat alpha = 1.0;\n");
        fsBuilder->codeAppendf("\t\talpha *= (%s.x - %s.x) > -0.5 ? 1.0 : 0.0;\n", fragmentPos, rectName);
        fsBuilder->codeAppendf("\t\talpha *= (%s.z - %s.x) > -0.5 ? 1.0 : 0.0;\n", rectName, fragmentPos);
        fsBuilder->codeAppendf("\t\talpha *= (%s.y - %s.y) > -0.5 ? 1.0 : 0.0;\n", fragmentPos, rectName);
        fsBuilder->codeAppendf("\t\talpha *= (%s.w - %s.y) > -0.5 ? 1.0 : 0.0;\n", rectName, fragmentPos);
    }

    if (GrProcessorEdgeTypeIsInverseFill(aare.getEdgeType())) {
        fsBuilder->codeAppend("\t\talpha = 1.0 - alpha;\n");
    }
    fsBuilder->codeAppendf("\t\t%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("alpha")).c_str());
}

void GLAARectEffect::setData(const GrGLProgramDataManager& pdman, const GrProcessor& processor) {
    const AARectEffect& aare = processor.cast<AARectEffect>();
    const SkRect& rect = aare.getRect();
    if (rect != fPrevRect) {
        pdman.set4f(fRectUniform, rect.fLeft + 0.5f, rect.fTop + 0.5f,
                   rect.fRight - 0.5f, rect.fBottom - 0.5f);
        fPrevRect = rect;
    }
}

void GLAARectEffect::GenKey(const GrProcessor& processor, const GrGLCaps&,
                            GrProcessorKeyBuilder* b) {
    const AARectEffect& aare = processor.cast<AARectEffect>();
    b->add32(aare.getEdgeType());
}

const GrBackendFragmentProcessorFactory& AARectEffect::getFactory() const {
    return GrTBackendFragmentProcessorFactory<AARectEffect>::getInstance();
}

//////////////////////////////////////////////////////////////////////////////

class GrGLConvexPolyEffect : public GrGLFragmentProcessor {
public:
    GrGLConvexPolyEffect(const GrBackendProcessorFactory&, const GrProcessor&);

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
    GrGLProgramDataManager::UniformHandle fEdgeUniform;
    SkScalar                              fPrevEdges[3 * GrConvexPolyEffect::kMaxEdges];
    typedef GrGLFragmentProcessor INHERITED;
};

GrGLConvexPolyEffect::GrGLConvexPolyEffect(const GrBackendProcessorFactory& factory,
                                           const GrProcessor&)
    : INHERITED (factory) {
    fPrevEdges[0] = SK_ScalarNaN;
}

void GrGLConvexPolyEffect::emitCode(GrGLProgramBuilder* builder,
                                    const GrFragmentProcessor& fp,
                                    const GrProcessorKey& key,
                                    const char* outputColor,
                                    const char* inputColor,
                                    const TransformedCoordsArray&,
                                    const TextureSamplerArray& samplers) {
    const GrConvexPolyEffect& cpe = fp.cast<GrConvexPolyEffect>();

    const char *edgeArrayName;
    fEdgeUniform = builder->addUniformArray(GrGLProgramBuilder::kFragment_Visibility,
                                            kVec3f_GrSLType,
                                            "edges",
                                            cpe.getEdgeCount(),
                                            &edgeArrayName);
    GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    fsBuilder->codeAppend("\t\tfloat alpha = 1.0;\n");
    fsBuilder->codeAppend("\t\tfloat edge;\n");
    const char* fragmentPos = fsBuilder->fragmentPosition();
    for (int i = 0; i < cpe.getEdgeCount(); ++i) {
        fsBuilder->codeAppendf("\t\tedge = dot(%s[%d], vec3(%s.x, %s.y, 1));\n",
                               edgeArrayName, i, fragmentPos, fragmentPos);
        if (GrProcessorEdgeTypeIsAA(cpe.getEdgeType())) {
            fsBuilder->codeAppend("\t\tedge = clamp(edge, 0.0, 1.0);\n");
        } else {
            fsBuilder->codeAppend("\t\tedge = edge >= 0.5 ? 1.0 : 0.0;\n");
        }
        fsBuilder->codeAppend("\t\talpha *= edge;\n");
    }

    // Woe is me. See skbug.com/2149.
    if (kTegra2_GrGLRenderer == builder->ctxInfo().renderer()) {
        fsBuilder->codeAppend("\t\tif (-1.0 == alpha) {\n\t\t\tdiscard;\n\t\t}\n");
    }

    if (GrProcessorEdgeTypeIsInverseFill(cpe.getEdgeType())) {
        fsBuilder->codeAppend("\talpha = 1.0 - alpha;\n");
    }
    fsBuilder->codeAppendf("\t%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("alpha")).c_str());
}

void GrGLConvexPolyEffect::setData(const GrGLProgramDataManager& pdman, const GrProcessor& effect) {
    const GrConvexPolyEffect& cpe = effect.cast<GrConvexPolyEffect>();
    size_t byteSize = 3 * cpe.getEdgeCount() * sizeof(SkScalar);
    if (0 != memcmp(fPrevEdges, cpe.getEdges(), byteSize)) {
        pdman.set3fv(fEdgeUniform, cpe.getEdgeCount(), cpe.getEdges());
        memcpy(fPrevEdges, cpe.getEdges(), byteSize);
    }
}

void GrGLConvexPolyEffect::GenKey(const GrProcessor& processor, const GrGLCaps&,
                                  GrProcessorKeyBuilder* b) {
    const GrConvexPolyEffect& cpe = processor.cast<GrConvexPolyEffect>();
    GR_STATIC_ASSERT(kGrProcessorEdgeTypeCnt <= 8);
    uint32_t key = (cpe.getEdgeCount() << 3) | cpe.getEdgeType();
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

GrFragmentProcessor* GrConvexPolyEffect::Create(GrPrimitiveEdgeType type, const SkPath& path,
                                                const SkVector* offset) {
    if (kHairlineAA_GrProcessorEdgeType == type) {
        return NULL;
    }
    if (path.getSegmentMasks() != SkPath::kLine_SegmentMask ||
        !path.isConvex()) {
        return NULL;
    }

    if (path.countPoints() > kMaxEdges) {
        return NULL;
    }

    SkPoint pts[kMaxEdges];
    SkScalar edges[3 * kMaxEdges];

    SkPath::Direction dir;
    SkAssertResult(path.cheapComputeDirection(&dir));

    SkVector t;
    if (NULL == offset) {
        t.set(0, 0);
    } else {
        t = *offset;
    }

    int count = path.getPoints(pts, kMaxEdges);
    int n = 0;
    for (int lastPt = count - 1, i = 0; i < count; lastPt = i++) {
        if (pts[lastPt] != pts[i]) {
            SkVector v = pts[i] - pts[lastPt];
            v.normalize();
            if (SkPath::kCCW_Direction == dir) {
                edges[3 * n] = v.fY;
                edges[3 * n + 1] = -v.fX;
            } else {
                edges[3 * n] = -v.fY;
                edges[3 * n + 1] = v.fX;
            }
            SkPoint p = pts[i] + t;
            edges[3 * n + 2] = -(edges[3 * n] * p.fX + edges[3 * n + 1] * p.fY);
            ++n;
        }
    }
    if (path.isInverseFillType()) {
        type = GrInvertProcessorEdgeType(type);
    }
    return Create(type, n, edges);
}

GrFragmentProcessor* GrConvexPolyEffect::Create(GrPrimitiveEdgeType edgeType, const SkRect& rect) {
    if (kHairlineAA_GrProcessorEdgeType == edgeType){
        return NULL;
    }
    return AARectEffect::Create(edgeType, rect);
}

GrConvexPolyEffect::~GrConvexPolyEffect() {}

void GrConvexPolyEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
}

const GrBackendFragmentProcessorFactory& GrConvexPolyEffect::getFactory() const {
    return GrTBackendFragmentProcessorFactory<GrConvexPolyEffect>::getInstance();
}

GrConvexPolyEffect::GrConvexPolyEffect(GrPrimitiveEdgeType edgeType, int n, const SkScalar edges[])
    : fEdgeType(edgeType)
    , fEdgeCount(n) {
    // Factory function should have already ensured this.
    SkASSERT(n <= kMaxEdges);
    memcpy(fEdges, edges, 3 * n * sizeof(SkScalar));
    // Outset the edges by 0.5 so that a pixel with center on an edge is 50% covered in the AA case
    // and 100% covered in the non-AA case.
    for (int i = 0; i < n; ++i) {
        fEdges[3 * i + 2] += SK_ScalarHalf;
    }
    this->setWillReadFragmentPosition();
}

bool GrConvexPolyEffect::onIsEqual(const GrProcessor& other) const {
    const GrConvexPolyEffect& cpe = other.cast<GrConvexPolyEffect>();
    // ignore the fact that 0 == -0 and just use memcmp.
    return (cpe.fEdgeType == fEdgeType && cpe.fEdgeCount == fEdgeCount &&
            0 == memcmp(cpe.fEdges, fEdges, 3 * fEdgeCount * sizeof(SkScalar)));
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrConvexPolyEffect);

GrFragmentProcessor* GrConvexPolyEffect::TestCreate(SkRandom* random,
                                                    GrContext*,
                                                    const GrDrawTargetCaps& caps,
                                                    GrTexture*[]) {
    int count = random->nextULessThan(kMaxEdges) + 1;
    SkScalar edges[kMaxEdges * 3];
    for (int i = 0; i < 3 * count; ++i) {
        edges[i] = random->nextSScalar1();
    }

    GrFragmentProcessor* fp;
    do {
        GrPrimitiveEdgeType edgeType = static_cast<GrPrimitiveEdgeType>(
                                        random->nextULessThan(kGrProcessorEdgeTypeCnt));
        fp = GrConvexPolyEffect::Create(edgeType, count, edges);
    } while (NULL == fp);
    return fp;
}
