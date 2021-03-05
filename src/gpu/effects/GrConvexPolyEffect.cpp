/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkPathPriv.h"
#include "src/gpu/effects/GrConvexPolyEffect.h"
#include "src/gpu/effects/generated/GrAARectEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/sksl/dsl/priv/DSLFPs.h"

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
    using INHERITED = GrGLSLFragmentProcessor;
};

void GrGLConvexPolyEffect::emitCode(EmitArgs& args) {
    const GrConvexPolyEffect& cpe = args.fFp.cast<GrConvexPolyEffect>();

    using namespace SkSL::dsl;
    StartFragmentProcessor(this, &args);
    Var edgeArray(kUniform_Modifier, Array(kHalf3, cpe.getEdgeCount()));
    fEdgeUniform = VarUniformHandle(edgeArray);
    Var alpha(kHalf, "alpha", 1);
    Declare(alpha);
    Var edge(kHalf, "edge");
    Declare(edge);
    for (int i = 0; i < cpe.getEdgeCount(); ++i) {
        edge = Dot(edgeArray[i], Half3(Swizzle(sk_FragCoord(), X, Y, ONE)));
        if (GrProcessorEdgeTypeIsAA(cpe.getEdgeType())) {
            edge = Saturate(edge);
        } else {
            edge = Select(edge >= 0.5, 1.0, 0.0);
        }
        alpha *= edge;
    }

    if (GrProcessorEdgeTypeIsInverseFill(cpe.getEdgeType())) {
        alpha = 1.0 - alpha;
    }

    Return(SampleChild(0) * alpha);
    EndFragmentProcessor();
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
    static_assert(kGrClipEdgeTypeCnt <= 8);
    uint32_t key = (cpe.getEdgeCount() << 3) | (int) cpe.getEdgeType();
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

GrFPResult GrConvexPolyEffect::Make(std::unique_ptr<GrFragmentProcessor> inputFP,
                                    GrClipEdgeType type, const SkPath& path) {
    if (path.getSegmentMasks() != SkPath::kLine_SegmentMask || !path.isConvex()) {
        return GrFPFailure(std::move(inputFP));
    }

    SkPathFirstDirection dir = SkPathPriv::ComputeFirstDirection(path);
    // The only way this should fail is if the clip is effectively a infinitely thin line. In that
    // case nothing is inside the clip. It'd be nice to detect this at a higher level and either
    // skip the draw or omit the clip element.
    if (dir == SkPathFirstDirection::kUnknown) {
        if (GrProcessorEdgeTypeIsInverseFill(type)) {
            return GrFPSuccess(
                    GrFragmentProcessor::ModulateRGBA(std::move(inputFP), SK_PMColor4fWHITE));
        }
        // This could use ConstColor instead of ModulateRGBA but it would trigger a debug print
        // about a coverage processor not being compatible with the alpha-as-coverage optimization.
        // We don't really care about this unlikely case so we just use ModulateRGBA to suppress
        // the print.
        return GrFPSuccess(
                GrFragmentProcessor::ModulateRGBA(std::move(inputFP), SK_PMColor4fTRANSPARENT));
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
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                SkASSERT(n == 0);
                break;
            case SkPath::kClose_Verb:
                break;
            case SkPath::kLine_Verb: {
                if (n >= kMaxEdges) {
                    return GrFPFailure(std::move(inputFP));
                }
                if (pts[0] != pts[1]) {
                    SkVector v = pts[1] - pts[0];
                    v.normalize();
                    if (SkPathFirstDirection::kCCW == dir) {
                        edges[3 * n] = v.fY;
                        edges[3 * n + 1] = -v.fX;
                    } else {
                        edges[3 * n] = -v.fY;
                        edges[3 * n + 1] = v.fX;
                    }
                    edges[3 * n + 2] = -(edges[3 * n] * pts[1].fX + edges[3 * n + 1] * pts[1].fY);
                    ++n;
                }
                break;
            }
            default:
                return GrFPFailure(std::move(inputFP));
        }
    }

    if (path.isInverseFillType()) {
        type = GrInvertProcessorEdgeType(type);
    }
    return GrConvexPolyEffect::Make(std::move(inputFP), type, n, edges);
}

GrConvexPolyEffect::~GrConvexPolyEffect() {}

void GrConvexPolyEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                               GrProcessorKeyBuilder* b) const {
    GrGLConvexPolyEffect::GenKey(*this, caps, b);
}

std::unique_ptr<GrGLSLFragmentProcessor> GrConvexPolyEffect::onMakeProgramImpl() const {
    return std::make_unique<GrGLConvexPolyEffect>();
}

GrConvexPolyEffect::GrConvexPolyEffect(std::unique_ptr<GrFragmentProcessor> inputFP,
                                       GrClipEdgeType edgeType, int n, const SkScalar edges[])
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

    this->registerChild(std::move(inputFP));
}

GrConvexPolyEffect::GrConvexPolyEffect(const GrConvexPolyEffect& that)
        : INHERITED(kGrConvexPolyEffect_ClassID, kCompatibleWithCoverageAsAlpha_OptimizationFlag)
        , fEdgeType(that.fEdgeType)
        , fEdgeCount(that.fEdgeCount) {
    this->cloneAndRegisterAllChildProcessors(that);
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

    bool success;
    std::unique_ptr<GrFragmentProcessor> fp = d->inputFP();
    do {
        GrClipEdgeType edgeType =
                static_cast<GrClipEdgeType>(d->fRandom->nextULessThan(kGrClipEdgeTypeCnt));
        std::tie(success, fp) = GrConvexPolyEffect::Make(std::move(fp), edgeType, count, edges);
    } while (!success);
    return fp;
}
#endif
