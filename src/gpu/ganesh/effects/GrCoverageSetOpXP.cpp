/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/effects/GrCoverageSetOpXP.h"

#include "include/private/base/SkAssert.h"
#include "src/base/SkRandom.h"
#include "src/core/SkColorData.h"
#include "src/gpu/Blend.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"

#include <memory>

enum class GrClampType;
struct GrShaderCaps;

class CoverageSetOpXP : public GrXferProcessor {
public:
    CoverageSetOpXP(SkRegion::Op regionOp, bool invertCoverage)
            : INHERITED(kCoverageSetOpXP_ClassID)
            , fRegionOp(regionOp)
            , fInvertCoverage(invertCoverage) {}

    const char* name() const override { return "Coverage Set Op"; }

    std::unique_ptr<ProgramImpl> makeProgramImpl() const override;

private:
    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override;

    void onGetBlendInfo(skgpu::BlendInfo* blendInfo) const override;

    bool onIsEqual(const GrXferProcessor& xpBase) const override {
        const CoverageSetOpXP& xp = xpBase.cast<CoverageSetOpXP>();
        return (fRegionOp == xp.fRegionOp &&
                fInvertCoverage == xp.fInvertCoverage);
    }

    SkRegion::Op fRegionOp;
    bool         fInvertCoverage;

    using INHERITED = GrXferProcessor;
};

void CoverageSetOpXP::onAddToKey(const GrShaderCaps& caps, skgpu::KeyBuilder* b) const {
    b->addBool(fInvertCoverage, "invert coverage");
}

std::unique_ptr<GrXferProcessor::ProgramImpl> CoverageSetOpXP::makeProgramImpl() const {
    class Impl : public ProgramImpl {
    private:
        void emitOutputsForBlendState(const EmitArgs& args) override {
            const CoverageSetOpXP& xp = args.fXP.cast<CoverageSetOpXP>();
            GrGLSLXPFragmentBuilder* fb = args.fXPFragBuilder;
            if (xp.fInvertCoverage) {
                fb->codeAppendf("%s = 1.0 - %s;", args.fOutputPrimary, args.fInputCoverage);
            } else {
                fb->codeAppendf("%s = %s;", args.fOutputPrimary, args.fInputCoverage);
            }
        }
    };
    return std::make_unique<Impl>();
}

void CoverageSetOpXP::onGetBlendInfo(skgpu::BlendInfo* blendInfo) const {
    switch (fRegionOp) {
        case SkRegion::kReplace_Op:
            blendInfo->fSrcBlend = skgpu::BlendCoeff::kOne;
            blendInfo->fDstBlend = skgpu::BlendCoeff::kZero;
            break;
        case SkRegion::kIntersect_Op:
            blendInfo->fSrcBlend = skgpu::BlendCoeff::kDC;
            blendInfo->fDstBlend = skgpu::BlendCoeff::kZero;
            break;
        case SkRegion::kUnion_Op:
            blendInfo->fSrcBlend = skgpu::BlendCoeff::kOne;
            blendInfo->fDstBlend = skgpu::BlendCoeff::kISC;
            break;
        case SkRegion::kXOR_Op:
            blendInfo->fSrcBlend = skgpu::BlendCoeff::kIDC;
            blendInfo->fDstBlend = skgpu::BlendCoeff::kISC;
            break;
        case SkRegion::kDifference_Op:
            blendInfo->fSrcBlend = skgpu::BlendCoeff::kZero;
            blendInfo->fDstBlend = skgpu::BlendCoeff::kISC;
            break;
        case SkRegion::kReverseDifference_Op:
            blendInfo->fSrcBlend = skgpu::BlendCoeff::kIDC;
            blendInfo->fDstBlend = skgpu::BlendCoeff::kZero;
            break;
    }
    blendInfo->fBlendConstant = SK_PMColor4fTRANSPARENT;
}

///////////////////////////////////////////////////////////////////////////////

constexpr GrCoverageSetOpXPFactory::GrCoverageSetOpXPFactory(SkRegion::Op regionOp,
                                                             bool invertCoverage)
        : fRegionOp(regionOp), fInvertCoverage(invertCoverage) {}

const GrXPFactory* GrCoverageSetOpXPFactory::Get(SkRegion::Op regionOp, bool invertCoverage) {
    switch (regionOp) {
        case SkRegion::kReplace_Op: {
            if (invertCoverage) {
                static constexpr const GrCoverageSetOpXPFactory gReplaceCDXPFI(
                        SkRegion::kReplace_Op, true);
                return &gReplaceCDXPFI;
            } else {
                static constexpr const GrCoverageSetOpXPFactory gReplaceCDXPF(SkRegion::kReplace_Op,
                                                                              false);
                return &gReplaceCDXPF;
            }
        }
        case SkRegion::kIntersect_Op: {
            if (invertCoverage) {
                static constexpr const GrCoverageSetOpXPFactory gIntersectCDXPFI(
                        SkRegion::kIntersect_Op, true);
                return &gIntersectCDXPFI;
            } else {
                static constexpr const GrCoverageSetOpXPFactory gIntersectCDXPF(
                        SkRegion::kIntersect_Op, false);
                return &gIntersectCDXPF;
            }
        }
        case SkRegion::kUnion_Op: {
            if (invertCoverage) {
                static constexpr const GrCoverageSetOpXPFactory gUnionCDXPFI(SkRegion::kUnion_Op,
                                                                             true);
                return &gUnionCDXPFI;
            } else {
                static constexpr const GrCoverageSetOpXPFactory gUnionCDXPF(SkRegion::kUnion_Op,
                                                                            false);
                return &gUnionCDXPF;
            }
        }
        case SkRegion::kXOR_Op: {
            if (invertCoverage) {
                static constexpr const GrCoverageSetOpXPFactory gXORCDXPFI(SkRegion::kXOR_Op, true);
                return &gXORCDXPFI;
            } else {
                static constexpr const GrCoverageSetOpXPFactory gXORCDXPF(SkRegion::kXOR_Op, false);
                return &gXORCDXPF;
            }
        }
        case SkRegion::kDifference_Op: {
            if (invertCoverage) {
                static constexpr const GrCoverageSetOpXPFactory gDifferenceCDXPFI(
                        SkRegion::kDifference_Op, true);
                return &gDifferenceCDXPFI;
            } else {
                static constexpr const GrCoverageSetOpXPFactory gDifferenceCDXPF(
                        SkRegion::kDifference_Op, false);
                return &gDifferenceCDXPF;
            }
        }
        case SkRegion::kReverseDifference_Op: {
            if (invertCoverage) {
                static constexpr const GrCoverageSetOpXPFactory gRevDiffCDXPFI(
                        SkRegion::kReverseDifference_Op, true);
                return &gRevDiffCDXPFI;
            } else {
                static constexpr const GrCoverageSetOpXPFactory gRevDiffCDXPF(
                        SkRegion::kReverseDifference_Op, false);
                return &gRevDiffCDXPF;
            }
        }
    }
    SK_ABORT("Unknown region op.");
}

sk_sp<const GrXferProcessor> GrCoverageSetOpXPFactory::makeXferProcessor(
        const GrProcessorAnalysisColor&,
        GrProcessorAnalysisCoverage,
        const GrCaps& caps,
        GrClampType) const {
    return sk_sp<GrXferProcessor>(new CoverageSetOpXP(fRegionOp, fInvertCoverage));
}

GR_DEFINE_XP_FACTORY_TEST(GrCoverageSetOpXPFactory)

#if defined(GPU_TEST_UTILS)
const GrXPFactory* GrCoverageSetOpXPFactory::TestGet(GrProcessorTestData* d) {
    SkRegion::Op regionOp = SkRegion::Op(d->fRandom->nextULessThan(SkRegion::kLastOp + 1));
    bool invertCoverage = d->fRandom->nextBool();
    return GrCoverageSetOpXPFactory::Get(regionOp, invertCoverage);
}
#endif
