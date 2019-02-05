/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrConvexPolyEffect.h"
#include "SkPathPriv.h"
#include "effects/GrAARectEffect.h"
#include "effects/GrConstColorProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

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
                                                         kHalf3_GrSLType,
                                                         "edges",
                                                         cpe.getEdgeCount(),
                                                         &edgeArrayName);
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    fragBuilder->codeAppend("\t\thalf alpha = 1.0;\n");
    fragBuilder->codeAppend("\t\thalf edge;\n");
    for (int i = 0; i < cpe.getEdgeCount(); ++i) {
        fragBuilder->codeAppendf("\t\tedge = dot(%s[%d], half3(half(sk_FragCoord.x), "
                                                              "half(sk_FragCoord.y), "
                                                              "1));\n",
                                 edgeArrayName, i);
        if (GrProcessorEdgeTypeIsAA(cpe.getEdgeType())) {
            fragBuilder->codeAppend("\t\tedge = saturate(edge);\n");
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
    GR_STATIC_ASSERT(kGrClipEdgeTypeCnt <= 8);
    uint32_t key = (cpe.getEdgeCount() << 3) | (int) cpe.getEdgeType();
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> GrConvexPolyEffect::Make(GrClipEdgeType type,
                                                              const SkPath& path) {
    if (GrClipEdgeType::kHairlineAA == type) {
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
            return GrConstColorProcessor::Make(SK_PMColor4fWHITE,
                                               GrConstColorProcessor::InputMode::kModulateRGBA);
        }
        // This could use kIgnore instead of kModulateRGBA but it would trigger a debug print
        // about a coverage processor not being compatible with the alpha-as-coverage optimization.
        // We don't really care about this unlikely case so we just use kModulateRGBA to suppress
        // the print.
        return GrConstColorProcessor::Make(SK_PMColor4fTRANSPARENT,
                                           GrConstColorProcessor::InputMode::kModulateRGBA);
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

std::unique_ptr<GrFragmentProcessor> GrConvexPolyEffect::Make(GrClipEdgeType edgeType,
                                                              const SkRect& rect) {
    if (GrClipEdgeType::kHairlineAA == edgeType){
        return nullptr;
    }
    return GrAARectEffect::Make(edgeType, rect);
}

GrConvexPolyEffect::~GrConvexPolyEffect() {}

void GrConvexPolyEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                               GrProcessorKeyBuilder* b) const {
    GrGLConvexPolyEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrConvexPolyEffect::onCreateGLSLInstance() const  {
    return new GrGLConvexPolyEffect;
}

GrConvexPolyEffect::GrConvexPolyEffect(GrClipEdgeType edgeType, int n, const SkScalar edges[])
        : INHERITED(kGrConvexPolyEffect_ClassID, kCompatibleWithCoverageAsAlpha_OptimizationFlag)
        , fEdgeType(edgeType)
        , fEdgeCount(n) {
    // Factory function should have already ensured this.
    SkASSERT(n <= kMaxEdges);
    memcpy(fEdges, edges, 3 * n * sizeof(SkScalar));
    // Outset the edges by 0.5 so that a pixel with center on an edge is 50% covered in the AA case
    // and 100% covered in the non-AA case.
    for (int i = 0; i < n; ++i) {
        fEdges[3 * i + 2] += SK_ScalarHalf;
    }
}

GrConvexPolyEffect::GrConvexPolyEffect(const GrConvexPolyEffect& that)
        : INHERITED(kGrConvexPolyEffect_ClassID, kCompatibleWithCoverageAsAlpha_OptimizationFlag)
        , fEdgeType(that.fEdgeType)
        , fEdgeCount(that.fEdgeCount) {
    memcpy(fEdges, that.fEdges, 3 * that.fEdgeCount * sizeof(SkScalar));
}

std::unique_ptr<GrFragmentProcessor> GrConvexPolyEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrConvexPolyEffect(*this));
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
std::unique_ptr<GrFragmentProcessor> GrConvexPolyEffect::TestCreate(GrProcessorTestData* d) {
    int count = d->fRandom->nextULessThan(kMaxEdges) + 1;
    SkScalar edges[kMaxEdges * 3];
    for (int i = 0; i < 3 * count; ++i) {
        edges[i] = d->fRandom->nextSScalar1();
    }

    std::unique_ptr<GrFragmentProcessor> fp;
    do {
        GrClipEdgeType edgeType = static_cast<GrClipEdgeType>(
                d->fRandom->nextULessThan(kGrClipEdgeTypeCnt));
        fp = GrConvexPolyEffect::Make(edgeType, count, edges);
    } while (nullptr == fp);
    return fp;
}
#endif
