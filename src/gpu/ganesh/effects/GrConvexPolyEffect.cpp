/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/effects/GrConvexPolyEffect.h"

#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkString.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkRandom.h"
#include "src/core/SkPathEnums.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <tuple>

struct GrShaderCaps;

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
        if (GrClipEdgeTypeIsInverseFill(type)) {
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
                // Non-linear segment so not a polygon.
                return GrFPFailure(std::move(inputFP));
        }
    }

    if (path.isInverseFillType()) {
        type = GrInvertClipEdgeType(type);
    }
    return GrConvexPolyEffect::Make(std::move(inputFP), type, n, edges);
}

GrConvexPolyEffect::~GrConvexPolyEffect() {}

void GrConvexPolyEffect::onAddToKey(const GrShaderCaps& caps, skgpu::KeyBuilder* b) const {
    static_assert(kGrClipEdgeTypeCnt <= 8);
    uint32_t key = (fEdgeCount << 3) | static_cast<int>(fEdgeType);
    b->add32(key);
}

std::unique_ptr<GrFragmentProcessor::ProgramImpl> GrConvexPolyEffect::onMakeProgramImpl() const {
    class Impl : public ProgramImpl {
    public:
        void emitCode(EmitArgs& args) override {
            const GrConvexPolyEffect& cpe = args.fFp.cast<GrConvexPolyEffect>();

            const char *edgeArrayName;
            fEdgeUniform = args.fUniformHandler->addUniformArray(&cpe,
                                                                 kFragment_GrShaderFlag,
                                                                 SkSLType::kHalf3,
                                                                 "edgeArray",
                                                                 cpe.fEdgeCount,
                                                                 &edgeArrayName);
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            fragBuilder->codeAppend("half alpha = 1.0;\n"
                                    "half edge;\n");
            for (int i = 0; i < cpe.fEdgeCount; ++i) {
                fragBuilder->codeAppendf("edge = dot(%s[%d], half3(sk_FragCoord.xy1));\n",
                                         edgeArrayName, i);
                if (GrClipEdgeTypeIsAA(cpe.fEdgeType)) {
                    fragBuilder->codeAppend("alpha *= saturate(edge);\n");
                } else {
                    fragBuilder->codeAppend("alpha *= step(0.5, edge);\n");
                }
            }

            if (GrClipEdgeTypeIsInverseFill(cpe.fEdgeType)) {
                fragBuilder->codeAppend("alpha = 1.0 - alpha;\n");
            }

            SkString inputSample = this->invokeChild(/*childIndex=*/0, args);

            fragBuilder->codeAppendf("return %s * alpha;\n", inputSample.c_str());
        }

    private:
        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& fp) override {
            const GrConvexPolyEffect& cpe = fp.cast<GrConvexPolyEffect>();
            size_t n = 3*cpe.fEdgeCount;
            if (!std::equal(fPrevEdges.begin(), fPrevEdges.begin() + n, cpe.fEdges.begin())) {
                pdman.set3fv(fEdgeUniform, cpe.fEdgeCount, cpe.fEdges.data());
                std::copy_n(cpe.fEdges.begin(), n, fPrevEdges.begin());
            }
        }

        GrGLSLProgramDataManager::UniformHandle              fEdgeUniform;
        std::array<float, 3 * GrConvexPolyEffect::kMaxEdges> fPrevEdges = {SK_FloatNaN};
    };

    return std::make_unique<Impl>();
}

GrConvexPolyEffect::GrConvexPolyEffect(std::unique_ptr<GrFragmentProcessor> inputFP,
                                       GrClipEdgeType edgeType,
                                       int n,
                                       const float edges[])
        : INHERITED(kGrConvexPolyEffect_ClassID,
                    ProcessorOptimizationFlags(inputFP.get()) &
                            kCompatibleWithCoverageAsAlpha_OptimizationFlag)
        , fEdgeType(edgeType)
        , fEdgeCount(n) {
    // Factory function should have already ensured this.
    SkASSERT(n <= kMaxEdges);
    std::copy_n(edges, 3*n, fEdges.begin());
    // Outset the edges by 0.5 so that a pixel with center on an edge is 50% covered in the AA case
    // and 100% covered in the non-AA case.
    for (int i = 0; i < n; ++i) {
        fEdges[3 * i + 2] += SK_ScalarHalf;
    }

    this->registerChild(std::move(inputFP));
}

GrConvexPolyEffect::GrConvexPolyEffect(const GrConvexPolyEffect& that)
        : INHERITED(that)
        , fEdgeType(that.fEdgeType)
        , fEdgeCount(that.fEdgeCount) {
    std::copy_n(that.fEdges.begin(), 3*that.fEdgeCount, fEdges.begin());
}

std::unique_ptr<GrFragmentProcessor> GrConvexPolyEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrConvexPolyEffect(*this));
}

bool GrConvexPolyEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrConvexPolyEffect& cpe = other.cast<GrConvexPolyEffect>();
    int n = 3*cpe.fEdgeCount;
    return cpe.fEdgeType == fEdgeType   &&
           cpe.fEdgeCount == fEdgeCount &&
           std::equal(cpe.fEdges.begin(), cpe.fEdges.begin() + n, fEdges.begin());
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrConvexPolyEffect)

#if defined(GR_TEST_UTILS)
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
