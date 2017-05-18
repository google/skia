/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrConvexPolyEffect.h"
#include "SkPathPriv.h"
#include "effects/GrConstColorProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "../private/GrGLSL.h"

//////////////////////////////////////////////////////////////////////////////
class AARectEffect : public GrFragmentProcessor {
public:
    const SkRect& getRect() const { return fRect; }

    static sk_sp<GrFragmentProcessor> Make(GrPrimitiveEdgeType edgeType, const SkRect& rect) {
        return sk_sp<GrFragmentProcessor>(new AARectEffect(edgeType, rect));
    }

    GrPrimitiveEdgeType getEdgeType() const { return fEdgeType; }

    const char* name() const override { return "AARect"; }

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

private:
    AARectEffect(GrPrimitiveEdgeType edgeType, const SkRect& rect)
            : INHERITED(kCompatibleWithCoverageAsAlpha_OptimizationFlag)
            , fRect(rect)
            , fEdgeType(edgeType) {
        this->initClassID<AARectEffect>();
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override {
        const AARectEffect& aare = other.cast<AARectEffect>();
        return fRect == aare.fRect;
    }

    SkRect              fRect;
    GrPrimitiveEdgeType fEdgeType;

    typedef GrFragmentProcessor INHERITED;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

};

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(AARectEffect);

#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> AARectEffect::TestCreate(GrProcessorTestData* d) {
    SkRect rect = SkRect::MakeLTRB(d->fRandom->nextSScalar1(),
                                   d->fRandom->nextSScalar1(),
                                   d->fRandom->nextSScalar1(),
                                   d->fRandom->nextSScalar1());
    sk_sp<GrFragmentProcessor> fp;
    do {
        GrPrimitiveEdgeType edgeType = static_cast<GrPrimitiveEdgeType>(
                d->fRandom->nextULessThan(kGrProcessorEdgeTypeCnt));

        fp = AARectEffect::Make(edgeType, rect);
    } while (nullptr == fp);
    return fp;
}
#endif

//////////////////////////////////////////////////////////////////////////////

class GLAARectEffect : public GrGLSLFragmentProcessor {
public:
    GLAARectEffect() {
        fPrevRect.fLeft = SK_ScalarNaN;
    }

    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

private:
    GrGLSLProgramDataManager::UniformHandle fRectUniform;
    SkRect                                  fPrevRect;

    typedef GrGLSLFragmentProcessor INHERITED;
};

void GLAARectEffect::emitCode(EmitArgs& args) {
    const AARectEffect& aare = args.fFp.cast<AARectEffect>();
    const char *rectName;
    // The rect uniform's xyzw refer to (left + 0.5, top + 0.5, right - 0.5, bottom - 0.5),
    // respectively.
    fRectUniform = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                    kVec4f_GrSLType,
                                                    kDefault_GrSLPrecision,
                                                    "rect",
                                                    &rectName);

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    if (GrProcessorEdgeTypeIsAA(aare.getEdgeType())) {
        // The amount of coverage removed in x and y by the edges is computed as a pair of negative
        // numbers, xSub and ySub.
        fragBuilder->codeAppend("\t\tfloat xSub, ySub;\n");
        fragBuilder->codeAppendf("\t\txSub = min(sk_FragCoord.x - %s.x, 0.0);\n", rectName);
        fragBuilder->codeAppendf("\t\txSub += min(%s.z - sk_FragCoord.x, 0.0);\n", rectName);
        fragBuilder->codeAppendf("\t\tySub = min(sk_FragCoord.y - %s.y, 0.0);\n", rectName);
        fragBuilder->codeAppendf("\t\tySub += min(%s.w - sk_FragCoord.y, 0.0);\n", rectName);
        // Now compute coverage in x and y and multiply them to get the fraction of the pixel
        // covered.
        fragBuilder->codeAppendf("\t\tfloat alpha = (1.0 + max(xSub, -1.0)) * (1.0 + max(ySub, -1.0));\n");
    } else {
        fragBuilder->codeAppendf("\t\tfloat alpha = 1.0;\n");
        fragBuilder->codeAppendf("\t\talpha *= (sk_FragCoord.x - %s.x) > -0.5 ? 1.0 : 0.0;\n",
                                 rectName);
        fragBuilder->codeAppendf("\t\talpha *= (%s.z - sk_FragCoord.x) > -0.5 ? 1.0 : 0.0;\n",
                                 rectName);
        fragBuilder->codeAppendf("\t\talpha *= (sk_FragCoord.y - %s.y) > -0.5 ? 1.0 : 0.0;\n",
                                 rectName);
        fragBuilder->codeAppendf("\t\talpha *= (%s.w - sk_FragCoord.y) > -0.5 ? 1.0 : 0.0;\n",
                                 rectName);
    }

    if (GrProcessorEdgeTypeIsInverseFill(aare.getEdgeType())) {
        fragBuilder->codeAppend("\t\talpha = 1.0 - alpha;\n");
    }
    fragBuilder->codeAppendf("\t\t%s = %s * alpha;\n", args.fOutputColor, args.fInputColor);
}

void GLAARectEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                               const GrFragmentProcessor& processor) {
    const AARectEffect& aare = processor.cast<AARectEffect>();
    const SkRect& rect = aare.getRect();
    if (rect != fPrevRect) {
        pdman.set4f(fRectUniform, rect.fLeft + 0.5f, rect.fTop + 0.5f,
                   rect.fRight - 0.5f, rect.fBottom - 0.5f);
        fPrevRect = rect;
    }
}

void GLAARectEffect::GenKey(const GrProcessor& processor, const GrShaderCaps&,
                            GrProcessorKeyBuilder* b) {
    const AARectEffect& aare = processor.cast<AARectEffect>();
    b->add32(aare.getEdgeType());
}

void AARectEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const {
    GLAARectEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* AARectEffect::onCreateGLSLInstance() const  {
    return new GLAARectEffect;
}

//////////////////////////////////////////////////////////////////////////////

class GrGLConvexPolyEffect : public GrGLSLFragmentProcessor {
public:
    GrGLConvexPolyEffect() {
        for (size_t i = 0; i < SK_ARRAY_COUNT(fPrevEdges); ++i) {
            fPrevEdges[i] = SK_ScalarNaN;
        }
    }

    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

private:
    GrGLSLProgramDataManager::UniformHandle fEdgeUniform;
    SkScalar                                fPrevEdges[3 * GrConvexPolyEffect::kMaxEdges];
    typedef GrGLSLFragmentProcessor INHERITED;
};

void GrGLConvexPolyEffect::emitCode(EmitArgs& args) {
    const GrConvexPolyEffect& cpe = args.fFp.cast<GrConvexPolyEffect>();

    const char *edgeArrayName;
    fEdgeUniform = args.fUniformHandler->addUniformArray(kFragment_GrShaderFlag,
                                                         kVec3f_GrSLType,
                                                         kDefault_GrSLPrecision,
                                                         "edges",
                                                         cpe.getEdgeCount(),
                                                         &edgeArrayName);
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    fragBuilder->codeAppend("\t\tfloat alpha = 1.0;\n");
    fragBuilder->codeAppend("\t\tfloat edge;\n");
    for (int i = 0; i < cpe.getEdgeCount(); ++i) {
        fragBuilder->codeAppendf("\t\tedge = dot(%s[%d], vec3(sk_FragCoord.x, sk_FragCoord.y, "
                                                             "1));\n",
                                 edgeArrayName, i);
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
    fragBuilder->codeAppendf("\t%s = %s * alpha;\n", args.fOutputColor, args.fInputColor);
}

void GrGLConvexPolyEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                     const GrFragmentProcessor& effect) {
    const GrConvexPolyEffect& cpe = effect.cast<GrConvexPolyEffect>();
    size_t byteSize = 3 * cpe.getEdgeCount() * sizeof(SkScalar);
    if (0 != memcmp(fPrevEdges, cpe.getEdges(), byteSize)) {
        pdman.set3fv(fEdgeUniform, cpe.getEdgeCount(), cpe.getEdges());
        memcpy(fPrevEdges, cpe.getEdges(), byteSize);
    }
}

void GrGLConvexPolyEffect::GenKey(const GrProcessor& processor, const GrShaderCaps&,
                                  GrProcessorKeyBuilder* b) {
    const GrConvexPolyEffect& cpe = processor.cast<GrConvexPolyEffect>();
    GR_STATIC_ASSERT(kGrProcessorEdgeTypeCnt <= 8);
    uint32_t key = (cpe.getEdgeCount() << 3) | cpe.getEdgeType();
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> GrConvexPolyEffect::Make(GrPrimitiveEdgeType type, const SkPath& path) {
    if (kHairlineAA_GrProcessorEdgeType == type) {
        return nullptr;
    }
    if (path.getSegmentMasks() != SkPath::kLine_SegmentMask ||
        !path.isConvex()) {
        return nullptr;
    }

    SkPathPriv::FirstDirection dir;
    // The only way this should fail is if the clip is effectively a infinitely thin line. In that
    // case nothing is inside the clip. It'd be nice to detect this at a higher level and either
    // skip the draw or omit the clip element.
    if (!SkPathPriv::CheapComputeFirstDirection(path, &dir)) {
        if (GrProcessorEdgeTypeIsInverseFill(type)) {
            return GrConstColorProcessor::Make(GrColor4f::OpaqueWhite(),
                                               GrConstColorProcessor::kModulateRGBA_InputMode);
        }
        // This could use kIgnore instead of kModulateRGBA but it would trigger a debug print
        // about a coverage processor not being compatible with the alpha-as-coverage optimization.
        // We don't really care about this unlikely case so we just use kModulateRGBA to suppress
        // the print.
        return GrConstColorProcessor::Make(GrColor4f::TransparentBlack(),
                                           GrConstColorProcessor::kModulateRGBA_InputMode);
    }

    SkScalar        edges[3 * kMaxEdges];
    SkPoint         pts[4];
    SkPath::Verb    verb;
    SkPath::Iter    iter(path, true);

    // SkPath considers itself convex so long as there is a convex contour within it,
    // regardless of any degenerate contours such as a string of moveTos before it.
    // Iterate here to consume any degenerate contours and only process the points
    // on the actual convex contour.
    int n = 0;
    while ((verb = iter.next(pts, true, true)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                SkASSERT(n == 0);
            case SkPath::kClose_Verb:
                break;
            case SkPath::kLine_Verb: {
                if (n >= kMaxEdges) {
                    return nullptr;
                }
                SkVector v = pts[1] - pts[0];
                v.normalize();
                if (SkPathPriv::kCCW_FirstDirection == dir) {
                    edges[3 * n] = v.fY;
                    edges[3 * n + 1] = -v.fX;
                } else {
                    edges[3 * n] = -v.fY;
                    edges[3 * n + 1] = v.fX;
                }
                edges[3 * n + 2] = -(edges[3 * n] * pts[1].fX + edges[3 * n + 1] * pts[1].fY);
                ++n;
                break;
            }
            default:
                return nullptr;
        }
    }

    if (path.isInverseFillType()) {
        type = GrInvertProcessorEdgeType(type);
    }
    return Make(type, n, edges);
}

sk_sp<GrFragmentProcessor> GrConvexPolyEffect::Make(GrPrimitiveEdgeType edgeType,
                                                    const SkRect& rect) {
    if (kHairlineAA_GrProcessorEdgeType == edgeType){
        return nullptr;
    }
    return AARectEffect::Make(edgeType, rect);
}

GrConvexPolyEffect::~GrConvexPolyEffect() {}

void GrConvexPolyEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                               GrProcessorKeyBuilder* b) const {
    GrGLConvexPolyEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrConvexPolyEffect::onCreateGLSLInstance() const  {
    return new GrGLConvexPolyEffect;
}

GrConvexPolyEffect::GrConvexPolyEffect(GrPrimitiveEdgeType edgeType, int n, const SkScalar edges[])
        : INHERITED(kCompatibleWithCoverageAsAlpha_OptimizationFlag)
        , fEdgeType(edgeType)
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
}

bool GrConvexPolyEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrConvexPolyEffect& cpe = other.cast<GrConvexPolyEffect>();
    // ignore the fact that 0 == -0 and just use memcmp.
    return (cpe.fEdgeType == fEdgeType && cpe.fEdgeCount == fEdgeCount &&
            0 == memcmp(cpe.fEdges, fEdges, 3 * fEdgeCount * sizeof(SkScalar)));
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrConvexPolyEffect);

#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> GrConvexPolyEffect::TestCreate(GrProcessorTestData* d) {
    int count = d->fRandom->nextULessThan(kMaxEdges) + 1;
    SkScalar edges[kMaxEdges * 3];
    for (int i = 0; i < 3 * count; ++i) {
        edges[i] = d->fRandom->nextSScalar1();
    }

    sk_sp<GrFragmentProcessor> fp;
    do {
        GrPrimitiveEdgeType edgeType = static_cast<GrPrimitiveEdgeType>(
                d->fRandom->nextULessThan(kGrProcessorEdgeTypeCnt));
        fp = GrConvexPolyEffect::Make(edgeType, count, edges);
    } while (nullptr == fp);
    return fp;
}
#endif
