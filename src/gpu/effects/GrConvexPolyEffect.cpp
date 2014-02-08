/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrConvexPolyEffect.h"

#include "gl/GrGLEffect.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLVertexEffect.h"
#include "GrTBackendEffectFactory.h"

#include "SkPath.h"

//////////////////////////////////////////////////////////////////////////////
class GLAARectEffect;

class AARectEffect : public GrEffect {
public:
    typedef GLAARectEffect GLEffect;

    const SkRect& getRect() const { return fRect; }

    static const char* Name() { return "AARect"; }

    static GrEffectRef* Create(const SkRect& rect) {
        return CreateEffectRef(AutoEffectUnref(SkNEW_ARGS(AARectEffect, (rect))));
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

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    AARectEffect(const SkRect& rect) : fRect(rect) {
        this->setWillReadFragmentPosition();
    }

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE {
        const AARectEffect& aare = CastEffect<AARectEffect>(other);
        return fRect == aare.fRect;
    }

    SkRect fRect;
    typedef GrEffect INHERITED;

    GR_DECLARE_EFFECT_TEST;

};

GR_DEFINE_EFFECT_TEST(AARectEffect);

GrEffectRef* AARectEffect::TestCreate(SkRandom* random,
                                      GrContext*,
                                      const GrDrawTargetCaps& caps,
                                      GrTexture*[]) {
    SkRect rect = SkRect::MakeLTRB(random->nextSScalar1(),
                                   random->nextSScalar1(),
                                   random->nextSScalar1(),
                                   random->nextSScalar1());
    return AARectEffect::Create(rect);
}

//////////////////////////////////////////////////////////////////////////////

class GLAARectEffect : public GrGLEffect {
public:
    GLAARectEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          EffectKey key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&) { return 0; }

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    GrGLUniformManager::UniformHandle   fRectUniform;
    SkRect                              fPrevRect;
    typedef GrGLEffect INHERITED;
};

GLAARectEffect::GLAARectEffect(const GrBackendEffectFactory& factory,
                               const GrDrawEffect& drawEffect)
    : INHERITED (factory) {
    fPrevRect.fLeft = SK_ScalarNaN;
}

void GLAARectEffect::emitCode(GrGLShaderBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              EffectKey key,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray&,
                              const TextureSamplerArray& samplers) {
    const char *rectName;
    fRectUniform = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                       kVec4f_GrSLType,
                                       "rect",
                                       &rectName);
    const char* fragmentPos = builder->fragmentPosition();
    // The rect uniform's xyzw refer to (left + 0.5, top + 0.5, right - 0.5, bottom - 0.5),
    // respectively. The amount of coverage removed in x and y by the edges is computed as a pair of
    // negative numbers, xSub and ySub.
    builder->fsCodeAppend("\t\tfloat xSub, ySub;\n");
    builder->fsCodeAppendf("\t\txSub = min(%s.x - %s.x, 0.0);\n", fragmentPos, rectName);
    builder->fsCodeAppendf("\t\txSub += min(%s.z - %s.x, 0.0);\n", rectName, fragmentPos);
    builder->fsCodeAppendf("\t\tySub = min(%s.y - %s.y, 0.0);\n", fragmentPos, rectName);
    builder->fsCodeAppendf("\t\tySub += min(%s.w - %s.y, 0.0);\n", rectName, fragmentPos);
    // Now compute coverage in x and y and multiply them to get the fraction of the pixel covered.
    builder->fsCodeAppendf("\t\tfloat alpha = (1.0 + max(xSub, -1.0)) * (1.0 + max(ySub, -1.0));\n");

    builder->fsCodeAppendf("\t\t%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("alpha")).c_str());
}

void GLAARectEffect::setData(const GrGLUniformManager& uman, const GrDrawEffect& drawEffect) {
    const AARectEffect& aare = drawEffect.castEffect<AARectEffect>();
    const SkRect& rect = aare.getRect();
    if (rect != fPrevRect) {
        uman.set4f(fRectUniform, rect.fLeft + 0.5f, rect.fTop + 0.5f,
                   rect.fRight - 0.5f, rect.fBottom - 0.5f);
        fPrevRect = rect;
    }
}

const GrBackendEffectFactory& AARectEffect::getFactory() const {
    return GrTBackendEffectFactory<AARectEffect>::getInstance();
}

//////////////////////////////////////////////////////////////////////////////

class GrGLConvexPolyEffect : public GrGLEffect {
public:
    GrGLConvexPolyEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          EffectKey key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&);

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    GrGLUniformManager::UniformHandle   fEdgeUniform;
    SkScalar                            fPrevEdges[3 * GrConvexPolyEffect::kMaxEdges];
    typedef GrGLEffect INHERITED;
};

GrGLConvexPolyEffect::GrGLConvexPolyEffect(const GrBackendEffectFactory& factory,
                                           const GrDrawEffect& drawEffect)
    : INHERITED (factory) {
    fPrevEdges[0] = SK_ScalarNaN;
}

void GrGLConvexPolyEffect::emitCode(GrGLShaderBuilder* builder,
                                    const GrDrawEffect& drawEffect,
                                    EffectKey key,
                                    const char* outputColor,
                                    const char* inputColor,
                                    const TransformedCoordsArray&,
                                    const TextureSamplerArray& samplers) {
    const GrConvexPolyEffect& cpe = drawEffect.castEffect<GrConvexPolyEffect>();

    const char *edgeArrayName;
    fEdgeUniform = builder->addUniformArray(GrGLShaderBuilder::kFragment_Visibility,
                                            kVec3f_GrSLType,
                                            "edges",
                                            cpe.getEdgeCount(),
                                            &edgeArrayName);
    builder->fsCodeAppend("\t\tfloat alpha = 1.0;\n");
    builder->fsCodeAppend("\t\tfloat edge;\n");
    const char* fragmentPos = builder->fragmentPosition();
    for (int i = 0; i < cpe.getEdgeCount(); ++i) {
        builder->fsCodeAppendf("\t\tedge = dot(%s[%d], vec3(%s.x, %s.y, 1));\n",
                               edgeArrayName, i, fragmentPos, fragmentPos);
        switch (cpe.getEdgeType()) {
            case GrConvexPolyEffect::kFillAA_EdgeType:
                builder->fsCodeAppend("\t\tedge = clamp(edge, 0.0, 1.0);\n");
                builder->fsCodeAppend("\t\talpha *= edge;\n");
                break;
            case GrConvexPolyEffect::kFillNoAA_EdgeType:
                builder->fsCodeAppend("\t\tedge = edge >= 0.5 ? 1.0 : 0.0;\n");
                builder->fsCodeAppend("\t\talpha *= edge;\n");
                break;
        }
    }

    // Woe is me. See skbug.com/2149.
    if (kTegra2_GrGLRenderer == builder->ctxInfo().renderer()) {
        builder->fsCodeAppend("\t\tif (-1.0 == alpha) {\n\t\t\tdiscard;\n\t\t}\n");
    }
    builder->fsCodeAppendf("\t%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("alpha")).c_str());
}

void GrGLConvexPolyEffect::setData(const GrGLUniformManager& uman, const GrDrawEffect& drawEffect) {
    const GrConvexPolyEffect& cpe = drawEffect.castEffect<GrConvexPolyEffect>();
    size_t byteSize = 3 * cpe.getEdgeCount() * sizeof(SkScalar);
    if (0 != memcmp(fPrevEdges, cpe.getEdges(), byteSize)) {
        uman.set3fv(fEdgeUniform, cpe.getEdgeCount(), cpe.getEdges());
        memcpy(fPrevEdges, cpe.getEdges(), byteSize);
    }
}

GrGLEffect::EffectKey GrGLConvexPolyEffect::GenKey(const GrDrawEffect& drawEffect,
                                                   const GrGLCaps&) {
    const GrConvexPolyEffect& cpe = drawEffect.castEffect<GrConvexPolyEffect>();
    GR_STATIC_ASSERT(GrConvexPolyEffect::kEdgeTypeCnt <= 4);
    return (cpe.getEdgeCount() << 2) | cpe.getEdgeType();
}

//////////////////////////////////////////////////////////////////////////////

GrEffectRef* GrConvexPolyEffect::Create(EdgeType type, const SkPath& path, const SkVector* offset) {
    if (path.getSegmentMasks() != SkPath::kLine_SegmentMask ||
        !path.isConvex() ||
        path.isInverseFillType()) {
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
    return Create(type, n, edges);
}

GrEffectRef* GrConvexPolyEffect::CreateForAAFillRect(const SkRect& rect) {
    return AARectEffect::Create(rect);
}

GrConvexPolyEffect::~GrConvexPolyEffect() {}

void GrConvexPolyEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
}

const GrBackendEffectFactory& GrConvexPolyEffect::getFactory() const {
    return GrTBackendEffectFactory<GrConvexPolyEffect>::getInstance();
}

GrConvexPolyEffect::GrConvexPolyEffect(EdgeType edgeType, int n, const SkScalar edges[])
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

bool GrConvexPolyEffect::onIsEqual(const GrEffect& other) const {
    const GrConvexPolyEffect& cpe = CastEffect<GrConvexPolyEffect>(other);
    // ignore the fact that 0 == -0 and just use memcmp.
    return (cpe.fEdgeType == fEdgeType && cpe.fEdgeCount == fEdgeCount &&
            0 == memcmp(cpe.fEdges, fEdges, 3 * fEdgeCount * sizeof(SkScalar)));
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrConvexPolyEffect);

GrEffectRef* GrConvexPolyEffect::TestCreate(SkRandom* random,
                                            GrContext*,
                                            const GrDrawTargetCaps& caps,
                                            GrTexture*[]) {
    EdgeType edgeType = static_cast<EdgeType>(random->nextULessThan(kEdgeTypeCnt));
    int count = random->nextULessThan(kMaxEdges) + 1;
    SkScalar edges[kMaxEdges * 3];
    for (int i = 0; i < 3 * count; ++i) {
        edges[i] = random->nextSScalar1();
    }

    return GrConvexPolyEffect::Create(edgeType, count, edges);
}
