
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrCoverageSetOpXP.h"
#include "GrCaps.h"
#include "GrColor.h"
#include "GrProcessor.h"
#include "GrProcOptInfo.h"
#include "gl/GrGLXferProcessor.h"
#include "gl/builders/GrGLFragmentShaderBuilder.h"
#include "gl/builders/GrGLProgramBuilder.h"

class CoverageSetOpXP : public GrXferProcessor {
public:
    static GrXferProcessor* Create(SkRegion::Op regionOp, bool invertCoverage) {
        return SkNEW_ARGS(CoverageSetOpXP, (regionOp, invertCoverage));
    }

    ~CoverageSetOpXP() override;

    const char* name() const override { return "Coverage Set Op"; }

    GrGLXferProcessor* createGLInstance() const override;

    bool invertCoverage() const { return fInvertCoverage; }

private:
    CoverageSetOpXP(SkRegion::Op regionOp, bool fInvertCoverage);

    GrXferProcessor::OptFlags onGetOptimizations(const GrProcOptInfo& colorPOI,
                                                 const GrProcOptInfo& coveragePOI,
                                                 bool doesStencilWrite,
                                                 GrColor* color,
                                                 const GrCaps& caps) override;

    void onGetGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    void onGetBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const override;

    bool onIsEqual(const GrXferProcessor& xpBase) const override {
        const CoverageSetOpXP& xp = xpBase.cast<CoverageSetOpXP>();
        return (fRegionOp == xp.fRegionOp &&
                fInvertCoverage == xp.fInvertCoverage);
    }

    SkRegion::Op fRegionOp;
    bool         fInvertCoverage;

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GLCoverageSetOpXP : public GrGLXferProcessor {
public:
    GLCoverageSetOpXP(const GrProcessor&) {}

    ~GLCoverageSetOpXP() override {}

    static void GenKey(const GrProcessor& processor, const GrGLSLCaps& caps,
                       GrProcessorKeyBuilder* b) {
        const CoverageSetOpXP& xp = processor.cast<CoverageSetOpXP>();
        uint32_t key = xp.invertCoverage() ?  0x0 : 0x1;
        b->add32(key);
    };

private:
    void emitOutputsForBlendState(const EmitArgs& args) override {
        const CoverageSetOpXP& xp = args.fXP.cast<CoverageSetOpXP>();
        GrGLXPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();

        if (xp.invertCoverage()) {
            fsBuilder->codeAppendf("%s = 1.0 - %s;", args.fOutputPrimary, args.fInputCoverage);
        } else {
            fsBuilder->codeAppendf("%s = %s;", args.fOutputPrimary, args.fInputCoverage);
        }
    }

    void onSetData(const GrGLProgramDataManager&, const GrXferProcessor&) override {};

    typedef GrGLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

CoverageSetOpXP::CoverageSetOpXP(SkRegion::Op regionOp, bool invertCoverage)
    : fRegionOp(regionOp)
    , fInvertCoverage(invertCoverage) {
    this->initClassID<CoverageSetOpXP>();
}

CoverageSetOpXP::~CoverageSetOpXP() {
}

void CoverageSetOpXP::onGetGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLCoverageSetOpXP::GenKey(*this, caps, b);
}

GrGLXferProcessor* CoverageSetOpXP::createGLInstance() const {
    return SkNEW_ARGS(GLCoverageSetOpXP, (*this));
}

GrXferProcessor::OptFlags
CoverageSetOpXP::onGetOptimizations(const GrProcOptInfo& colorPOI,
                                    const GrProcOptInfo& coveragePOI,
                                    bool doesStencilWrite,
                                    GrColor* color,
                                    const GrCaps& caps) {
    // We never look at the color input
    return GrXferProcessor::kIgnoreColor_OptFlag; 
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
    blendInfo->fBlendConstant = 0;
}

///////////////////////////////////////////////////////////////////////////////

GrCoverageSetOpXPFactory::GrCoverageSetOpXPFactory(SkRegion::Op regionOp, bool invertCoverage)
    : fRegionOp(regionOp)
    , fInvertCoverage(invertCoverage) {
    this->initClassID<GrCoverageSetOpXPFactory>();
}

GrXPFactory* GrCoverageSetOpXPFactory::Create(SkRegion::Op regionOp, bool invertCoverage) {
    switch (regionOp) {
        case SkRegion::kReplace_Op: {
            if (invertCoverage) {
                static GrCoverageSetOpXPFactory gReplaceCDXPFI(regionOp, invertCoverage);
                return SkRef(&gReplaceCDXPFI);
            } else {
                static GrCoverageSetOpXPFactory gReplaceCDXPF(regionOp, invertCoverage);
                return SkRef(&gReplaceCDXPF);
            }
            break;
        }
        case SkRegion::kIntersect_Op: {
            if (invertCoverage) {
                static GrCoverageSetOpXPFactory gIntersectCDXPFI(regionOp, invertCoverage);
                return SkRef(&gIntersectCDXPFI);
            } else {
                static GrCoverageSetOpXPFactory gIntersectCDXPF(regionOp, invertCoverage);
                return SkRef(&gIntersectCDXPF);
            }
            break;
        }
        case SkRegion::kUnion_Op: {
            if (invertCoverage) {
                static GrCoverageSetOpXPFactory gUnionCDXPFI(regionOp, invertCoverage);
                return SkRef(&gUnionCDXPFI);
            } else {
                static GrCoverageSetOpXPFactory gUnionCDXPF(regionOp, invertCoverage);
                return SkRef(&gUnionCDXPF);
            }
            break;
        }
        case SkRegion::kXOR_Op: {
            if (invertCoverage) {
                static GrCoverageSetOpXPFactory gXORCDXPFI(regionOp, invertCoverage);
                return SkRef(&gXORCDXPFI);
            } else {
                static GrCoverageSetOpXPFactory gXORCDXPF(regionOp, invertCoverage);
                return SkRef(&gXORCDXPF);
            }
            break;
        }
        case SkRegion::kDifference_Op: {
            if (invertCoverage) {
                static GrCoverageSetOpXPFactory gDifferenceCDXPFI(regionOp, invertCoverage);
                return SkRef(&gDifferenceCDXPFI);
            } else {
                static GrCoverageSetOpXPFactory gDifferenceCDXPF(regionOp, invertCoverage);
                return SkRef(&gDifferenceCDXPF);
            }
            break;
        }
        case SkRegion::kReverseDifference_Op: {
            if (invertCoverage) {
                static GrCoverageSetOpXPFactory gRevDiffCDXPFI(regionOp, invertCoverage);
                return SkRef(&gRevDiffCDXPFI);
            } else {
                static GrCoverageSetOpXPFactory gRevDiffCDXPF(regionOp, invertCoverage);
                return SkRef(&gRevDiffCDXPF);
            }
            break;
        }
        default:
            return NULL;
    }
}

GrXferProcessor*
GrCoverageSetOpXPFactory::onCreateXferProcessor(const GrCaps& caps,
                                                const GrProcOptInfo& colorPOI,
                                                const GrProcOptInfo& covPOI,
                                                bool hasMixedSamples,
                                                const DstTexture* dst) const {
    // We don't support inverting coverage with mixed samples. We don't expect to ever want this in
    // the future, however we could at some point make this work using an inverted coverage
    // modulation table. Note that an inverted table still won't work if there are coverage procs.
    if (fInvertCoverage && hasMixedSamples) {
        SkASSERT(false);
        return NULL;
    }

    return CoverageSetOpXP::Create(fRegionOp, fInvertCoverage);
}

void GrCoverageSetOpXPFactory::getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                                        InvariantBlendedColor* blendedColor) const {
    blendedColor->fWillBlendWithDst = SkRegion::kReplace_Op != fRegionOp;
    blendedColor->fKnownColorFlags = kNone_GrColorComponentFlags;
}

GR_DEFINE_XP_FACTORY_TEST(GrCoverageSetOpXPFactory);

GrXPFactory* GrCoverageSetOpXPFactory::TestCreate(GrProcessorTestData* d) {
    SkRegion::Op regionOp = SkRegion::Op(d->fRandom->nextULessThan(SkRegion::kLastOp + 1));
    bool invertCoverage = d->fRandom->nextBool();
    return GrCoverageSetOpXPFactory::Create(regionOp, invertCoverage);
}

