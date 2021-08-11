/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrCoverageSetOpXP.h"

#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrXferProcessor.h"
#include "src/gpu/glsl/GrGLSLBlend.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

class CoverageSetOpXP : public GrXferProcessor {
public:
    CoverageSetOpXP(SkRegion::Op regionOp, bool invertCoverage)
            : INHERITED(kCoverageSetOpXP_ClassID)
            , fRegionOp(regionOp)
            , fInvertCoverage(invertCoverage) {}

    const char* name() const override { return "Coverage Set Op"; }

    std::unique_ptr<ProgramImpl> makeProgramImpl() const override;

private:
    void onAddToKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    void onGetBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const override;

    bool onIsEqual(const GrXferProcessor& xpBase) const override {
        const CoverageSetOpXP& xp = xpBase.cast<CoverageSetOpXP>();
        return (fRegionOp == xp.fRegionOp &&
                fInvertCoverage == xp.fInvertCoverage);
    }

    SkRegion::Op fRegionOp;
    bool         fInvertCoverage;

    using INHERITED = GrXferProcessor;
};

void CoverageSetOpXP::onAddToKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const {
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

void CoverageSetOpXP::onGetBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const {
    switch (fRegionOp) {
        case SkRegion::kReplace_Op:
            blendInfo->fSrcBlend = kOne_GrBlendCoeff;
            blendInfo->fDstBlend = kZero_GrBlendCoeff;
            break;
        case SkRegion::kIntersect_Op:
            blendInfo->fSrcBlend = kDC_GrBlendCoeff;
            blendInfo->fDstBlend = kZero_GrBlendCoeff;
            break;
        case SkRegion::kUnion_Op:
            blendInfo->fSrcBlend = kOne_GrBlendCoeff;
            blendInfo->fDstBlend = kISC_GrBlendCoeff;
            break;
        case SkRegion::kXOR_Op:
            blendInfo->fSrcBlend = kIDC_GrBlendCoeff;
            blendInfo->fDstBlend = kISC_GrBlendCoeff;
            break;
        case SkRegion::kDifference_Op:
            blendInfo->fSrcBlend = kZero_GrBlendCoeff;
            blendInfo->fDstBlend = kISC_GrBlendCoeff;
            break;
        case SkRegion::kReverseDifference_Op:
            blendInfo->fSrcBlend = kIDC_GrBlendCoeff;
            blendInfo->fDstBlend = kZero_GrBlendCoeff;
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

GR_DEFINE_XP_FACTORY_TEST(GrCoverageSetOpXPFactory);

#if GR_TEST_UTILS
const GrXPFactory* GrCoverageSetOpXPFactory::TestGet(GrProcessorTestData* d) {
    SkRegion::Op regionOp = SkRegion::Op(d->fRandom->nextULessThan(SkRegion::kLastOp + 1));
    bool invertCoverage = d->fRandom->nextBool();
    return GrCoverageSetOpXPFactory::Get(regionOp, invertCoverage);
}
#endif
