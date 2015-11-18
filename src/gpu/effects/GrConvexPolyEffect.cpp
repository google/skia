/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrConvexPolyEffect.h"
#include "GrInvariantOutput.h"
#include "SkPathPriv.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"

//////////////////////////////////////////////////////////////////////////////
class AARectEffect : public GrFragmentProcessor {
public:
    const SkRect& getRect() const { return fRect; }

    static GrFragmentProcessor* Create(GrPrimitiveEdgeType edgeType, const SkRect& rect) {
        return new AARectEffect(edgeType, rect);
    }

    GrPrimitiveEdgeType getEdgeType() const { return fEdgeType; }

    const char* name() const override { return "AARect"; }

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

private:
    AARectEffect(GrPrimitiveEdgeType edgeType, const SkRect& rect)
        : fRect(rect), fEdgeType(edgeType) {
        this->initClassID<AARectEffect>();
        this->setWillReadFragmentPosition();
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override {
        const AARectEffect& aare = other.cast<AARectEffect>();
        return fRect == aare.fRect;
    }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        if (fRect.isEmpty()) {
            // An empty rect will have no coverage anywhere.
            inout->mulByKnownSingleComponent(0);
        } else {
            inout->mulByUnknownSingleComponent();
        }
    }

    SkRect              fRect;
    GrPrimitiveEdgeType fEdgeType;

    typedef GrFragmentProcessor INHERITED;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

};

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(AARectEffect);

const GrFragmentProcessor* AARectEffect::TestCreate(GrProcessorTestData* d) {
    SkRect rect = SkRect::MakeLTRB(d->fRandom->nextSScalar1(),
                                   d->fRandom->nextSScalar1(),
                                   d->fRandom->nextSScalar1(),
                                   d->fRandom->nextSScalar1());
    GrFragmentProcessor* fp;
    do {
        GrPrimitiveEdgeType edgeType = static_cast<GrPrimitiveEdgeType>(
                d->fRandom->nextULessThan(kGrProcessorEdgeTypeCnt));

        fp = AARectEffect::Create(edgeType, rect);
    } while (nullptr == fp);
    return fp;
}

//////////////////////////////////////////////////////////////////////////////

class GLAARectEffect : public GrGLSLFragmentProcessor {
public:
    GLAARectEffect(const GrProcessor&);

    virtual void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    GrGLSLProgramDataManager::UniformHandle fRectUniform;
    SkRect                                fPrevRect;
    typedef GrGLSLFragmentProcessor INHERITED;
};

GLAARectEffect::GLAARectEffect(const GrProcessor& effect) {
    fPrevRect.fLeft = SK_ScalarNaN;
}

void GLAARectEffect::emitCode(EmitArgs& args) {
    const AARectEffect& aare = args.fFp.cast<AARectEffect>();
    const char *rectName;
    // The rect uniform's xyzw refer to (left + 0.5, top + 0.5, right - 0.5, bottom - 0.5),
    // respectively.
    fRectUniform = args.fBuilder->addUniform(GrGLSLProgramBuilder::kFragment_Visibility,
                                       kVec4f_GrSLType,
                                       kDefault_GrSLPrecision,
                                       "rect",
                                       &rectName);

    GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
    const char* fragmentPos = fragBuilder->fragmentPosition();
    if (GrProcessorEdgeTypeIsAA(aare.getEdgeType())) {
        // The amount of coverage removed in x and y by the edges is computed as a pair of negative
        // numbers, xSub and ySub.
        fragBuilder->codeAppend("\t\tfloat xSub, ySub;\n");
        fragBuilder->codeAppendf("\t\txSub = min(%s.x - %s.x, 0.0);\n", fragmentPos, rectName);
        fragBuilder->codeAppendf("\t\txSub += min(%s.z - %s.x, 0.0);\n", rectName, fragmentPos);
        fragBuilder->codeAppendf("\t\tySub = min(%s.y - %s.y, 0.0);\n", fragmentPos, rectName);
        fragBuilder->codeAppendf("\t\tySub += min(%s.w - %s.y, 0.0);\n", rectName, fragmentPos);
        // Now compute coverage in x and y and multiply them to get the fraction of the pixel
        // covered.
        fragBuilder->codeAppendf("\t\tfloat alpha = (1.0 + max(xSub, -1.0)) * (1.0 + max(ySub, -1.0));\n");
    } else {
        fragBuilder->codeAppendf("\t\tfloat alpha = 1.0;\n");
        fragBuilder->codeAppendf("\t\talpha *= (%s.x - %s.x) > -0.5 ? 1.0 : 0.0;\n", fragmentPos, rectName);
        fragBuilder->codeAppendf("\t\talpha *= (%s.z - %s.x) > -0.5 ? 1.0 : 0.0;\n", rectName, fragmentPos);
        fragBuilder->codeAppendf("\t\talpha *= (%s.y - %s.y) > -0.5 ? 1.0 : 0.0;\n", fragmentPos, rectName);
        fragBuilder->codeAppendf("\t\talpha *= (%s.w - %s.y) > -0.5 ? 1.0 : 0.0;\n", rectName, fragmentPos);
    }

    if (GrProcessorEdgeTypeIsInverseFill(aare.getEdgeType())) {
        fragBuilder->codeAppend("\t\talpha = 1.0 - alpha;\n");
    }
    fragBuilder->codeAppendf("\t\t%s = %s;\n", args.fOutputColor,
                             (GrGLSLExpr4(args.fInputColor) * GrGLSLExpr1("alpha")).c_str());
}

void GLAARectEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                               const GrProcessor& processor) {
    const AARectEffect& aare = processor.cast<AARectEffect>();
    const SkRect& rect = aare.getRect();
    if (rect != fPrevRect) {
        pdman.set4f(fRectUniform, rect.fLeft + 0.5f, rect.fTop + 0.5f,
                   rect.fRight - 0.5f, rect.fBottom - 0.5f);
        fPrevRect = rect;
    }
}

void GLAARectEffect::GenKey(const GrProcessor& processor, const GrGLSLCaps&,
                            GrProcessorKeyBuilder* b) {
    const AARectEffect& aare = processor.cast<AARectEffect>();
    b->add32(aare.getEdgeType());
}

void AARectEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLAARectEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* AARectEffect::onCreateGLSLInstance() const  {
    return new GLAARectEffect(*this);
}

//////////////////////////////////////////////////////////////////////////////

class GrGLConvexPolyEffect : public GrGLSLFragmentProcessor {
public:
    GrGLConvexPolyEffect(const GrProcessor&);

    virtual void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    GrGLSLProgramDataManager::UniformHandle fEdgeUniform;
    SkScalar                              fPrevEdges[3 * GrConvexPolyEffect::kMaxEdges];
    typedef GrGLSLFragmentProcessor INHERITED;
};

GrGLConvexPolyEffect::GrGLConvexPolyEffect(const GrProcessor&) {
    fPrevEdges[0] = SK_ScalarNaN;
}

void GrGLConvexPolyEffect::emitCode(EmitArgs& args) {
    const GrConvexPolyEffect& cpe = args.fFp.cast<GrConvexPolyEffect>();

    const char *edgeArrayName;
    fEdgeUniform = args.fBuilder->addUniformArray(GrGLSLProgramBuilder::kFragment_Visibility,
                                            kVec3f_GrSLType,
                                             kDefault_GrSLPrecision,
                                             "edges",
                                            cpe.getEdgeCount(),
                                            &edgeArrayName);
    GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
    fragBuilder->codeAppend("\t\tfloat alpha = 1.0;\n");
    fragBuilder->codeAppend("\t\tfloat edge;\n");
    const char* fragmentPos = fragBuilder->fragmentPosition();
    for (int i = 0; i < cpe.getEdgeCount(); ++i) {
        fragBuilder->codeAppendf("\t\tedge = dot(%s[%d], vec3(%s.x, %s.y, 1));\n",
                                 edgeArrayName, i, fragmentPos, fragmentPos);
        if (GrProcessorEdgeTypeIsAA(cpe.getEdgeType())) {
            fragBuilder->codeAppend("\t\tedge = clamp(edge, 0.0, 1.0);\n");
        } else {
            fragBuilder->codeAppend("\t\tedge = edge >= 0.5 ? 1.0 : 0.0;\n");
        }
        fragBuilder->codeAppend("\t\talpha *= edge;\n");
    }

    if (GrProcessorEdgeTypeIsInverseFill(cpe.getEdgeType())) {
        fragBuilder->codeAppend("\talpha = 1.0 - alpha;\n");
    }
    fragBuilder->codeAppendf("\t%s = %s;\n", args.fOutputColor,
                             (GrGLSLExpr4(args.fInputColor) * GrGLSLExpr1("alpha")).c_str());
}

void GrGLConvexPolyEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                     const GrProcessor& effect) {
    const GrConvexPolyEffect& cpe = effect.cast<GrConvexPolyEffect>();
    size_t byteSize = 3 * cpe.getEdgeCount() * sizeof(SkScalar);
    if (0 != memcmp(fPrevEdges, cpe.getEdges(), byteSize)) {
        pdman.set3fv(fEdgeUniform, cpe.getEdgeCount(), cpe.getEdges());
        memcpy(fPrevEdges, cpe.getEdges(), byteSize);
    }
}

void GrGLConvexPolyEffect::GenKey(const GrProcessor& processor, const GrGLSLCaps&,
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
        return nullptr;
    }
    if (path.getSegmentMasks() != SkPath::kLine_SegmentMask ||
        !path.isConvex()) {
        return nullptr;
    }

    if (path.countPoints() > kMaxEdges) {
        return nullptr;
    }

    SkPoint pts[kMaxEdges];
    SkScalar edges[3 * kMaxEdges];

    SkPathPriv::FirstDirection dir;
    SkAssertResult(SkPathPriv::CheapComputeFirstDirection(path, &dir));

    SkVector t;
    if (nullptr == offset) {
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
            if (SkPathPriv::kCCW_FirstDirection == dir) {
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
        return nullptr;
    }
    return AARectEffect::Create(edgeType, rect);
}

GrConvexPolyEffect::~GrConvexPolyEffect() {}

void GrConvexPolyEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    inout->mulByUnknownSingleComponent();
}

void GrConvexPolyEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                               GrProcessorKeyBuilder* b) const {
    GrGLConvexPolyEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrConvexPolyEffect::onCreateGLSLInstance() const  {
    return new GrGLConvexPolyEffect(*this);
}

GrConvexPolyEffect::GrConvexPolyEffect(GrPrimitiveEdgeType edgeType, int n, const SkScalar edges[])
    : fEdgeType(edgeType)
    , fEdgeCount(n) {
    this->initClassID<GrConvexPolyEffect>();
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

bool GrConvexPolyEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrConvexPolyEffect& cpe = other.cast<GrConvexPolyEffect>();
    // ignore the fact that 0 == -0 and just use memcmp.
    return (cpe.fEdgeType == fEdgeType && cpe.fEdgeCount == fEdgeCount &&
            0 == memcmp(cpe.fEdges, fEdges, 3 * fEdgeCount * sizeof(SkScalar)));
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrConvexPolyEffect);

const GrFragmentProcessor* GrConvexPolyEffect::TestCreate(GrProcessorTestData* d) {
    int count = d->fRandom->nextULessThan(kMaxEdges) + 1;
    SkScalar edges[kMaxEdges * 3];
    for (int i = 0; i < 3 * count; ++i) {
        edges[i] = d->fRandom->nextSScalar1();
    }

    GrFragmentProcessor* fp;
    do {
        GrPrimitiveEdgeType edgeType = static_cast<GrPrimitiveEdgeType>(
                d->fRandom->nextULessThan(kGrProcessorEdgeTypeCnt));
        fp = GrConvexPolyEffect::Create(edgeType, count, edges);
    } while (nullptr == fp);
    return fp;
}
