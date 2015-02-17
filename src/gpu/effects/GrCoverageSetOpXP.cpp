
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrCoverageSetOpXP.h"
#include "GrColor.h"
#include "GrDrawTargetCaps.h"
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

    ~CoverageSetOpXP() SK_OVERRIDE;

    const char* name() const SK_OVERRIDE { return "Coverage Set Op"; }

    GrGLXferProcessor* createGLInstance() const SK_OVERRIDE;

    bool hasSecondaryOutput() const SK_OVERRIDE { return false; }

    GrXferProcessor::OptFlags getOptimizations(const GrProcOptInfo& colorPOI,
                                               const GrProcOptInfo& coveragePOI,
                                               bool doesStencilWrite,
                                               GrColor* color,
                                               const GrDrawTargetCaps& caps) SK_OVERRIDE;

    void getBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const SK_OVERRIDE;

    bool invertCoverage() const { return fInvertCoverage; }

private:
    CoverageSetOpXP(SkRegion::Op regionOp, bool fInvertCoverage);

    void onGetGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const SK_OVERRIDE;

    bool onIsEqual(const GrXferProcessor& xpBase) const SK_OVERRIDE {
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

    ~GLCoverageSetOpXP() SK_OVERRIDE {}

    static void GenKey(const GrProcessor& processor, const GrGLCaps& caps,
                       GrProcessorKeyBuilder* b) {
        const CoverageSetOpXP& xp = processor.cast<CoverageSetOpXP>();
        uint32_t key = xp.invertCoverage() ?  0x0 : 0x1;
        b->add32(key);
    };

private:
    void onEmitCode(const EmitArgs& args) SK_OVERRIDE {
        const CoverageSetOpXP& xp = args.fXP.cast<CoverageSetOpXP>();
        GrGLFPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();

        if (xp.invertCoverage()) {
            fsBuilder->codeAppendf("%s = 1.0 - %s;", args.fOutputPrimary, args.fInputCoverage);
        } else {
            fsBuilder->codeAppendf("%s = %s;", args.fOutputPrimary, args.fInputCoverage);
        }
    }

    void onSetData(const GrGLProgramDataManager&, const GrXferProcessor&) SK_OVERRIDE {};

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

void CoverageSetOpXP::onGetGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLCoverageSetOpXP::GenKey(*this, caps, b);
}

GrGLXferProcessor* CoverageSetOpXP::createGLInstance() const {
    return SkNEW_ARGS(GLCoverageSetOpXP, (*this));
}

GrXferProcessor::OptFlags
CoverageSetOpXP::getOptimizations(const GrProcOptInfo& colorPOI,
                                  const GrProcOptInfo& coveragePOI,
                                  bool doesStencilWrite,
                                  GrColor* color,
                                  const GrDrawTargetCaps& caps) {
    // We never look at the color input
    return GrXferProcessor::kIgnoreColor_OptFlag; 
}

void CoverageSetOpXP::getBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const {
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
GrCoverageSetOpXPFactory::onCreateXferProcessor(const GrDrawTargetCaps& caps,
                                                const GrProcOptInfo& colorPOI,
                                                const GrProcOptInfo& covPOI,
                                                const GrDeviceCoordTexture* dstCopy) const {
    return CoverageSetOpXP::Create(fRegionOp, fInvertCoverage);
}

void GrCoverageSetOpXPFactory::getInvariantOutput(const GrProcOptInfo& colorPOI,
                                                  const GrProcOptInfo& coveragePOI,
                                                  GrXPFactory::InvariantOutput* output) const {
    if (SkRegion::kReplace_Op == fRegionOp) {
        if (coveragePOI.isSolidWhite()) {
            output->fBlendedColor = GrColor_WHITE;
            output->fBlendedColorFlags = kRGBA_GrColorComponentFlags;
        } else {
            output->fBlendedColorFlags = 0;
        }

        output->fWillBlendWithDst = false;
    } else {
        output->fBlendedColorFlags = 0;
        output->fWillBlendWithDst = true;
    }
}

GR_DEFINE_XP_FACTORY_TEST(GrCoverageSetOpXPFactory);

GrXPFactory* GrCoverageSetOpXPFactory::TestCreate(SkRandom* random,
                                                  GrContext*,
                                                  const GrDrawTargetCaps&,
                                                  GrTexture*[]) {
    SkRegion::Op regionOp = SkRegion::Op(random->nextULessThan(SkRegion::kLastOp + 1));
    bool invertCoverage = random->nextBool();
    return GrCoverageSetOpXPFactory::Create(regionOp, invertCoverage);
}

