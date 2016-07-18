/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrCoverageSetOpXP.h"
#include "GrCaps.h"
#include "GrColor.h"
#include "GrDrawContext.h"
#include "GrPipeline.h"
#include "GrProcessor.h"
#include "GrProcOptInfo.h"
#include "glsl/GrGLSLBlend.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLXferProcessor.h"

class CoverageSetOpXP : public GrXferProcessor {
public:
    static GrXferProcessor* Create(SkRegion::Op regionOp, bool invertCoverage) {
        return new CoverageSetOpXP(regionOp, invertCoverage);
    }

    ~CoverageSetOpXP() override;

    const char* name() const override { return "Coverage Set Op"; }

    GrGLSLXferProcessor* createGLSLInstance() const override;

    bool invertCoverage() const { return fInvertCoverage; }

private:
    CoverageSetOpXP(SkRegion::Op regionOp, bool fInvertCoverage);

    GrXferProcessor::OptFlags onGetOptimizations(const GrPipelineOptimizations& optimizations,
                                                 bool doesStencilWrite,
                                                 GrColor* color,
                                                 const GrCaps& caps) const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

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

class GLCoverageSetOpXP : public GrGLSLXferProcessor {
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
        GrGLSLXPFragmentBuilder* fragBuilder = args.fXPFragBuilder;

        if (xp.invertCoverage()) {
            fragBuilder->codeAppendf("%s = 1.0 - %s;", args.fOutputPrimary, args.fInputCoverage);
        } else {
            fragBuilder->codeAppendf("%s = %s;", args.fOutputPrimary, args.fInputCoverage);
        }
    }

    void onSetData(const GrGLSLProgramDataManager&, const GrXferProcessor&) override {};

    typedef GrGLSLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

CoverageSetOpXP::CoverageSetOpXP(SkRegion::Op regionOp, bool invertCoverage)
    : fRegionOp(regionOp)
    , fInvertCoverage(invertCoverage) {
    this->initClassID<CoverageSetOpXP>();
}

CoverageSetOpXP::~CoverageSetOpXP() {
}

void CoverageSetOpXP::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                            GrProcessorKeyBuilder* b) const {
    GLCoverageSetOpXP::GenKey(*this, caps, b);
}

GrGLSLXferProcessor* CoverageSetOpXP::createGLSLInstance() const {
    return new GLCoverageSetOpXP(*this);
}

GrXferProcessor::OptFlags
CoverageSetOpXP::onGetOptimizations(const GrPipelineOptimizations& optimizations,
                                    bool doesStencilWrite,
                                    GrColor* color,
                                    const GrCaps& caps) const {
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

class ShaderCSOXferProcessor : public GrXferProcessor {
public:
    ShaderCSOXferProcessor(const DstTexture* dstTexture,
                           bool hasMixedSamples,
                           SkRegion::Op regionOp,
                           bool invertCoverage)
        : INHERITED(dstTexture, true, hasMixedSamples)
        , fRegionOp(regionOp)
        , fInvertCoverage(invertCoverage) {
        this->initClassID<ShaderCSOXferProcessor>();
    }

    const char* name() const override { return "Coverage Set Op Shader"; }

    GrGLSLXferProcessor* createGLSLInstance() const override;

    SkRegion::Op regionOp() const { return fRegionOp; }
    bool invertCoverage() const { return fInvertCoverage; }

private:
    GrXferProcessor::OptFlags onGetOptimizations(const GrPipelineOptimizations&, bool, GrColor*,
                                                 const GrCaps&) const override {
        // We never look at the color input
        return GrXferProcessor::kIgnoreColor_OptFlag;
    }

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrXferProcessor& xpBase) const override {
        const ShaderCSOXferProcessor& xp = xpBase.cast<ShaderCSOXferProcessor>();
        return (fRegionOp == xp.fRegionOp &&
                fInvertCoverage == xp.fInvertCoverage);
    }

    SkRegion::Op fRegionOp;
    bool         fInvertCoverage;

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GLShaderCSOXferProcessor : public GrGLSLXferProcessor {
public:
    static void GenKey(const GrProcessor& processor, GrProcessorKeyBuilder* b) {
        const ShaderCSOXferProcessor& xp = processor.cast<ShaderCSOXferProcessor>();
        b->add32(xp.regionOp());
        uint32_t key = xp.invertCoverage() ?  0x0 : 0x1;
        b->add32(key);
    }

private:
    void emitBlendCodeForDstRead(GrGLSLXPFragmentBuilder* fragBuilder,
                                 GrGLSLUniformHandler* uniformHandler,
                                 const char* srcColor,
                                 const char* srcCoverage,
                                 const char* dstColor,
                                 const char* outColor,
                                 const char* outColorSecondary,
                                 const GrXferProcessor& proc) override {
        const ShaderCSOXferProcessor& xp = proc.cast<ShaderCSOXferProcessor>();

        if (xp.invertCoverage()) {
            fragBuilder->codeAppendf("%s = 1.0 - %s;", outColor, srcCoverage);
        } else {
            fragBuilder->codeAppendf("%s = %s;", outColor, srcCoverage);
        }

        GrGLSLBlend::AppendRegionOp(fragBuilder, outColor, dstColor, outColor, xp.regionOp());
    }

    void onSetData(const GrGLSLProgramDataManager&, const GrXferProcessor&) override {}

    typedef GrGLSLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

void ShaderCSOXferProcessor::onGetGLSLProcessorKey(const GrGLSLCaps&,
                                                   GrProcessorKeyBuilder* b) const {
    GLShaderCSOXferProcessor::GenKey(*this, b);
}

GrGLSLXferProcessor* ShaderCSOXferProcessor::createGLSLInstance() const {
    return new GLShaderCSOXferProcessor;
}

///////////////////////////////////////////////////////////////////////////////
//
GrCoverageSetOpXPFactory::GrCoverageSetOpXPFactory(SkRegion::Op regionOp, bool invertCoverage)
    : fRegionOp(regionOp)
    , fInvertCoverage(invertCoverage) {
    this->initClassID<GrCoverageSetOpXPFactory>();
}

sk_sp<GrXPFactory> GrCoverageSetOpXPFactory::Make(SkRegion::Op regionOp, bool invertCoverage) {
    switch (regionOp) {
        case SkRegion::kReplace_Op: {
            if (invertCoverage) {
                static GrCoverageSetOpXPFactory gReplaceCDXPFI(regionOp, invertCoverage);
                return sk_sp<GrXPFactory>(SkRef(&gReplaceCDXPFI));
            } else {
                static GrCoverageSetOpXPFactory gReplaceCDXPF(regionOp, invertCoverage);
                return sk_sp<GrXPFactory>(SkRef(&gReplaceCDXPF));
            }
            break;
        }
        case SkRegion::kIntersect_Op: {
            if (invertCoverage) {
                static GrCoverageSetOpXPFactory gIntersectCDXPFI(regionOp, invertCoverage);
                return sk_sp<GrXPFactory>(SkRef(&gIntersectCDXPFI));
            } else {
                static GrCoverageSetOpXPFactory gIntersectCDXPF(regionOp, invertCoverage);
                return sk_sp<GrXPFactory>(SkRef(&gIntersectCDXPF));
            }
            break;
        }
        case SkRegion::kUnion_Op: {
            if (invertCoverage) {
                static GrCoverageSetOpXPFactory gUnionCDXPFI(regionOp, invertCoverage);
                return sk_sp<GrXPFactory>(SkRef(&gUnionCDXPFI));
            } else {
                static GrCoverageSetOpXPFactory gUnionCDXPF(regionOp, invertCoverage);
                return sk_sp<GrXPFactory>(SkRef(&gUnionCDXPF));
            }
            break;
        }
        case SkRegion::kXOR_Op: {
            if (invertCoverage) {
                static GrCoverageSetOpXPFactory gXORCDXPFI(regionOp, invertCoverage);
                return sk_sp<GrXPFactory>(SkRef(&gXORCDXPFI));
            } else {
                static GrCoverageSetOpXPFactory gXORCDXPF(regionOp, invertCoverage);
                return sk_sp<GrXPFactory>(SkRef(&gXORCDXPF));
            }
            break;
        }
        case SkRegion::kDifference_Op: {
            if (invertCoverage) {
                static GrCoverageSetOpXPFactory gDifferenceCDXPFI(regionOp, invertCoverage);
                return sk_sp<GrXPFactory>(SkRef(&gDifferenceCDXPFI));
            } else {
                static GrCoverageSetOpXPFactory gDifferenceCDXPF(regionOp, invertCoverage);
                return sk_sp<GrXPFactory>(SkRef(&gDifferenceCDXPF));
            }
            break;
        }
        case SkRegion::kReverseDifference_Op: {
            if (invertCoverage) {
                static GrCoverageSetOpXPFactory gRevDiffCDXPFI(regionOp, invertCoverage);
                return sk_sp<GrXPFactory>(SkRef(&gRevDiffCDXPFI));
            } else {
                static GrCoverageSetOpXPFactory gRevDiffCDXPF(regionOp, invertCoverage);
                return sk_sp<GrXPFactory>(SkRef(&gRevDiffCDXPF));
            }
            break;
        }
        default:
            return nullptr;
    }
}

GrXferProcessor*
GrCoverageSetOpXPFactory::onCreateXferProcessor(const GrCaps& caps,
                                                const GrPipelineOptimizations& optimizations,
                                                bool hasMixedSamples,
                                                const DstTexture* dst) const {
    // We don't support inverting coverage with mixed samples. We don't expect to ever want this in
    // the future, however we could at some point make this work using an inverted coverage
    // modulation table. Note that an inverted table still won't work if there are coverage procs.
    if (fInvertCoverage && hasMixedSamples) {
        SkASSERT(false);
        return nullptr;
    }

    if (optimizations.fOverrides.fUsePLSDstRead) {
        return new ShaderCSOXferProcessor(dst, hasMixedSamples, fRegionOp, fInvertCoverage);
    }
    return CoverageSetOpXP::Create(fRegionOp, fInvertCoverage);
}

void GrCoverageSetOpXPFactory::getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                                        InvariantBlendedColor* blendedColor) const {
    blendedColor->fWillBlendWithDst = SkRegion::kReplace_Op != fRegionOp;
    blendedColor->fKnownColorFlags = kNone_GrColorComponentFlags;
}

GR_DEFINE_XP_FACTORY_TEST(GrCoverageSetOpXPFactory);

sk_sp<GrXPFactory> GrCoverageSetOpXPFactory::TestCreate(GrProcessorTestData* d) {
    SkRegion::Op regionOp = SkRegion::Op(d->fRandom->nextULessThan(SkRegion::kLastOp + 1));
    bool invertCoverage = !d->fDrawContext->hasMixedSamples() && d->fRandom->nextBool();
    return GrCoverageSetOpXPFactory::Make(regionOp, invertCoverage);
}
