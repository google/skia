/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/effects/GrPorterDuffXferProcessor.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkRandom.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/Blend.h"
#include "src/gpu/BlendFormula.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrProcessorAnalysis.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/glsl/GrGLSLBlend.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

#include <cstring>
#include <memory>
#include <string>

using skgpu::BlendFormula;

class PorterDuffXferProcessor : public GrXferProcessor {
public:
    PorterDuffXferProcessor(BlendFormula blendFormula, GrProcessorAnalysisCoverage coverage)
            : INHERITED(kPorterDuffXferProcessor_ClassID, /*willReadDstColor=*/false, coverage)
            , fBlendFormula(blendFormula) {
    }

    const char* name() const override { return "Porter Duff"; }

    std::unique_ptr<ProgramImpl> makeProgramImpl() const override;

    BlendFormula getBlendFormula() const { return fBlendFormula; }

private:
    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override;

    bool onHasSecondaryOutput() const override { return fBlendFormula.hasSecondaryOutput(); }

    void onGetBlendInfo(skgpu::BlendInfo* blendInfo) const override {
        blendInfo->fEquation = fBlendFormula.equation();
        blendInfo->fSrcBlend = fBlendFormula.srcCoeff();
        blendInfo->fDstBlend = fBlendFormula.dstCoeff();
        blendInfo->fWritesColor = fBlendFormula.modifiesDst();
    }

    bool onIsEqual(const GrXferProcessor& xpBase) const override {
        const PorterDuffXferProcessor& xp = xpBase.cast<PorterDuffXferProcessor>();
        return fBlendFormula == xp.fBlendFormula;
    }

    const BlendFormula fBlendFormula;

    using INHERITED = GrXferProcessor;
};

///////////////////////////////////////////////////////////////////////////////

static void append_color_output(const PorterDuffXferProcessor& xp,
                                GrGLSLXPFragmentBuilder* fragBuilder,
                                BlendFormula::OutputType outputType, const char* output,
                                const char* inColor, const char* inCoverage) {
    SkASSERT(inCoverage);
    SkASSERT(inColor);
    switch (outputType) {
        case BlendFormula::kNone_OutputType:
            fragBuilder->codeAppendf("%s = half4(0.0);", output);
            break;
        case BlendFormula::kCoverage_OutputType:
            fragBuilder->codeAppendf("%s = %s;", output, inCoverage);
            break;
        case BlendFormula::kModulate_OutputType:
            fragBuilder->codeAppendf("%s = %s * %s;", output, inColor, inCoverage);
            break;
        case BlendFormula::kSAModulate_OutputType:
            fragBuilder->codeAppendf("%s = %s.a * %s;", output, inColor, inCoverage);
            break;
        case BlendFormula::kISAModulate_OutputType:
            fragBuilder->codeAppendf("%s = (1.0 - %s.a) * %s;", output, inColor, inCoverage);
            break;
        case BlendFormula::kISCModulate_OutputType:
            fragBuilder->codeAppendf("%s = (half4(1.0) - %s) * %s;", output, inColor, inCoverage);
            break;
        default:
            SK_ABORT("Unsupported output type.");
            break;
    }
}

void PorterDuffXferProcessor::onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder* b) const {
    b->add32(fBlendFormula.primaryOutput() | (fBlendFormula.secondaryOutput() << 3));
    static_assert(BlendFormula::kLast_OutputType < 8);
}

std::unique_ptr<GrXferProcessor::ProgramImpl> PorterDuffXferProcessor::makeProgramImpl() const {
    class Impl : public ProgramImpl {
    private:
        void emitOutputsForBlendState(const EmitArgs& args) override {
            const PorterDuffXferProcessor& xp = args.fXP.cast<PorterDuffXferProcessor>();
            GrGLSLXPFragmentBuilder* fragBuilder = args.fXPFragBuilder;

            const BlendFormula& blendFormula = xp.fBlendFormula;
            if (blendFormula.hasSecondaryOutput()) {
                append_color_output(xp,
                                    fragBuilder,
                                    blendFormula.secondaryOutput(),
                                    args.fOutputSecondary,
                                    args.fInputColor,
                                    args.fInputCoverage);
            }
            append_color_output(xp,
                                fragBuilder,
                                blendFormula.primaryOutput(),
                                args.fOutputPrimary,
                                args.fInputColor,
                                args.fInputCoverage);
        }
    };

    return std::make_unique<Impl>();
}

///////////////////////////////////////////////////////////////////////////////

class ShaderPDXferProcessor : public GrXferProcessor {
public:
    ShaderPDXferProcessor(SkBlendMode xfermode, GrProcessorAnalysisCoverage coverage)
            : INHERITED(kShaderPDXferProcessor_ClassID, /*willReadDstColor=*/true, coverage)
            , fXfermode(xfermode) {
    }

    const char* name() const override { return "Porter Duff Shader"; }

    std::unique_ptr<ProgramImpl> makeProgramImpl() const override;

private:
    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override;

    bool onIsEqual(const GrXferProcessor& xpBase) const override {
        const ShaderPDXferProcessor& xp = xpBase.cast<ShaderPDXferProcessor>();
        return fXfermode == xp.fXfermode;
    }

    const SkBlendMode fXfermode;

    using INHERITED = GrXferProcessor;
};

///////////////////////////////////////////////////////////////////////////////


void ShaderPDXferProcessor::onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder* b) const {
    b->add32(GrGLSLBlend::BlendKey(fXfermode));
}

std::unique_ptr<GrXferProcessor::ProgramImpl> ShaderPDXferProcessor::makeProgramImpl() const {
    class Impl : public ProgramImpl {
    private:
        void emitBlendCodeForDstRead(GrGLSLXPFragmentBuilder* fragBuilder,
                                     GrGLSLUniformHandler* uniformHandler,
                                     const char* srcColor,
                                     const char* srcCoverage,
                                     const char* dstColor,
                                     const char* outColor,
                                     const char* outColorSecondary,
                                     const GrXferProcessor& proc) override {
            const ShaderPDXferProcessor& xp = proc.cast<ShaderPDXferProcessor>();

            std::string blendExpr = GrGLSLBlend::BlendExpression(
                    &xp, uniformHandler, &fBlendUniform, srcColor, dstColor, xp.fXfermode);
            fragBuilder->codeAppendf("%s = %s;", outColor, blendExpr.c_str());

            // Apply coverage.
            DefaultCoverageModulation(fragBuilder,
                                      srcCoverage,
                                      dstColor,
                                      outColor,
                                      outColorSecondary,
                                      xp);
        }

        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrXferProcessor& proc) override {
            if (fBlendUniform.isValid()) {
                const ShaderPDXferProcessor& xp = proc.cast<ShaderPDXferProcessor>();
                GrGLSLBlend::SetBlendModeUniformData(pdman, fBlendUniform, xp.fXfermode);
            }
        }

        GrGLSLUniformHandler::UniformHandle fBlendUniform;
    };

    return std::make_unique<Impl>();
}

///////////////////////////////////////////////////////////////////////////////

class PDLCDXferProcessor : public GrXferProcessor {
public:
    static sk_sp<const GrXferProcessor> Make(SkBlendMode mode,
                                             const GrProcessorAnalysisColor& inputColor);

    const char* name() const override { return "Porter Duff LCD"; }

    std::unique_ptr<ProgramImpl> makeProgramImpl() const override;

private:
    PDLCDXferProcessor(const SkPMColor4f& blendConstant, float alpha);

    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override {}

    void onGetBlendInfo(skgpu::BlendInfo* blendInfo) const override {
        blendInfo->fSrcBlend = skgpu::BlendCoeff::kConstC;
        blendInfo->fDstBlend = skgpu::BlendCoeff::kISC;
        blendInfo->fBlendConstant = fBlendConstant;
    }

    bool onIsEqual(const GrXferProcessor& xpBase) const override {
        const PDLCDXferProcessor& xp = xpBase.cast<PDLCDXferProcessor>();
        if (fBlendConstant != xp.fBlendConstant || fAlpha != xp.fAlpha) {
            return false;
        }
        return true;
    }

    SkPMColor4f fBlendConstant;
    float fAlpha;

    using INHERITED = GrXferProcessor;
};

PDLCDXferProcessor::PDLCDXferProcessor(const SkPMColor4f& blendConstant, float alpha)
    : INHERITED(kPDLCDXferProcessor_ClassID, /*willReadDstColor=*/false,
                GrProcessorAnalysisCoverage::kLCD)
    , fBlendConstant(blendConstant)
    , fAlpha(alpha) {
}

sk_sp<const GrXferProcessor> PDLCDXferProcessor::Make(SkBlendMode mode,
                                                      const GrProcessorAnalysisColor& color) {
    if (SkBlendMode::kSrcOver != mode) {
        return nullptr;
    }
    SkPMColor4f blendConstantPM;
    if (!color.isConstant(&blendConstantPM)) {
        return nullptr;
    }
    SkColor4f blendConstantUPM = blendConstantPM.unpremul();
    float alpha = blendConstantUPM.fA;
    blendConstantPM = { blendConstantUPM.fR, blendConstantUPM.fG, blendConstantUPM.fB, 1 };
    return sk_sp<GrXferProcessor>(new PDLCDXferProcessor(blendConstantPM, alpha));
}

std::unique_ptr<GrXferProcessor::ProgramImpl> PDLCDXferProcessor::makeProgramImpl() const {
    class Impl : public ProgramImpl {
    private:
        void emitOutputsForBlendState(const EmitArgs& args) override {
            const char* alpha;
            fAlphaUniform = args.fUniformHandler->addUniform(nullptr,
                                                             kFragment_GrShaderFlag,
                                                             SkSLType::kHalf,
                                                             "alpha",
                                                             &alpha);
            GrGLSLXPFragmentBuilder* fragBuilder = args.fXPFragBuilder;
            // We want to force our primary output to be alpha * Coverage, where alpha is the alpha
            // value of the src color. We know that there are no color stages (or we wouldn't have
            // created this xp) and the r,g, and b channels of the op's input color are baked into
            // the blend constant.
            SkASSERT(args.fInputCoverage);
            fragBuilder->codeAppendf("%s = %s * %s;",
                                     args.fOutputPrimary,
                                     alpha, args.fInputCoverage);
        }

        void onSetData(const GrGLSLProgramDataManager& pdm, const GrXferProcessor& xp) override {
            float alpha = xp.cast<PDLCDXferProcessor>().fAlpha;
            if (fLastAlpha != alpha) {
                pdm.set1f(fAlphaUniform, alpha);
                fLastAlpha = alpha;
            }
        }

        GrGLSLUniformHandler::UniformHandle fAlphaUniform;
        float fLastAlpha = SK_FloatNaN;
    };

    return std::make_unique<Impl>();
}

///////////////////////////////////////////////////////////////////////////////

constexpr GrPorterDuffXPFactory::GrPorterDuffXPFactory(SkBlendMode xfermode)
        : fBlendMode(xfermode) {}

const GrXPFactory* GrPorterDuffXPFactory::Get(SkBlendMode blendMode) {
    SkASSERT((unsigned)blendMode <= (unsigned)SkBlendMode::kLastCoeffMode);

    static constexpr const GrPorterDuffXPFactory gClearPDXPF(SkBlendMode::kClear);
    static constexpr const GrPorterDuffXPFactory gSrcPDXPF(SkBlendMode::kSrc);
    static constexpr const GrPorterDuffXPFactory gDstPDXPF(SkBlendMode::kDst);
    static constexpr const GrPorterDuffXPFactory gSrcOverPDXPF(SkBlendMode::kSrcOver);
    static constexpr const GrPorterDuffXPFactory gDstOverPDXPF(SkBlendMode::kDstOver);
    static constexpr const GrPorterDuffXPFactory gSrcInPDXPF(SkBlendMode::kSrcIn);
    static constexpr const GrPorterDuffXPFactory gDstInPDXPF(SkBlendMode::kDstIn);
    static constexpr const GrPorterDuffXPFactory gSrcOutPDXPF(SkBlendMode::kSrcOut);
    static constexpr const GrPorterDuffXPFactory gDstOutPDXPF(SkBlendMode::kDstOut);
    static constexpr const GrPorterDuffXPFactory gSrcATopPDXPF(SkBlendMode::kSrcATop);
    static constexpr const GrPorterDuffXPFactory gDstATopPDXPF(SkBlendMode::kDstATop);
    static constexpr const GrPorterDuffXPFactory gXorPDXPF(SkBlendMode::kXor);
    static constexpr const GrPorterDuffXPFactory gPlusPDXPF(SkBlendMode::kPlus);
    static constexpr const GrPorterDuffXPFactory gModulatePDXPF(SkBlendMode::kModulate);
    static constexpr const GrPorterDuffXPFactory gScreenPDXPF(SkBlendMode::kScreen);

    switch (blendMode) {
        case SkBlendMode::kClear:
            return &gClearPDXPF;
        case SkBlendMode::kSrc:
            return &gSrcPDXPF;
        case SkBlendMode::kDst:
            return &gDstPDXPF;
        case SkBlendMode::kSrcOver:
            return &gSrcOverPDXPF;
        case SkBlendMode::kDstOver:
            return &gDstOverPDXPF;
        case SkBlendMode::kSrcIn:
            return &gSrcInPDXPF;
        case SkBlendMode::kDstIn:
            return &gDstInPDXPF;
        case SkBlendMode::kSrcOut:
            return &gSrcOutPDXPF;
        case SkBlendMode::kDstOut:
            return &gDstOutPDXPF;
        case SkBlendMode::kSrcATop:
            return &gSrcATopPDXPF;
        case SkBlendMode::kDstATop:
            return &gDstATopPDXPF;
        case SkBlendMode::kXor:
            return &gXorPDXPF;
        case SkBlendMode::kPlus:
            return &gPlusPDXPF;
        case SkBlendMode::kModulate:
            return &gModulatePDXPF;
        case SkBlendMode::kScreen:
            return &gScreenPDXPF;
        default:
            SK_ABORT("Unexpected blend mode.");
    }
}

sk_sp<const GrXferProcessor> GrPorterDuffXPFactory::makeXferProcessor(
        const GrProcessorAnalysisColor& color, GrProcessorAnalysisCoverage coverage,
        const GrCaps& caps, GrClampType clampType) const {
    bool isLCD = coverage == GrProcessorAnalysisCoverage::kLCD;
    // See comment in MakeSrcOverXferProcessor about color.isOpaque here
    if (isLCD &&
        SkBlendMode::kSrcOver == fBlendMode && color.isConstant() && /*color.isOpaque() &&*/
        !caps.shaderCaps()->fDualSourceBlendingSupport &&
        !caps.shaderCaps()->fDstReadInShaderSupport) {
        // If we don't have dual source blending or in shader dst reads, we fall back to this
        // trick for rendering SrcOver LCD text instead of doing a dst copy.
        return PDLCDXferProcessor::Make(fBlendMode, color);
    }
    BlendFormula blendFormula = [&](){
        if (isLCD) {
            return skgpu::GetLCDBlendFormula(fBlendMode);
        }
        if (fBlendMode == SkBlendMode::kSrcOver && color.isOpaque() &&
            coverage == GrProcessorAnalysisCoverage::kNone &&
            caps.shouldCollapseSrcOverToSrcWhenAble())
        {
            return skgpu::GetBlendFormula(true, false, SkBlendMode::kSrc);
        }
        return skgpu::GetBlendFormula(
                color.isOpaque(), GrProcessorAnalysisCoverage::kNone != coverage, fBlendMode);
    }();

    // Skia always saturates after the kPlus blend mode, so it requires shader-based blending when
    // pixels aren't guaranteed to automatically be normalized (i.e. any floating point config).
    if ((blendFormula.hasSecondaryOutput() && !caps.shaderCaps()->fDualSourceBlendingSupport) ||
        (isLCD && (SkBlendMode::kSrcOver != fBlendMode /*|| !color.isOpaque()*/)) ||
        (GrClampType::kAuto != clampType && SkBlendMode::kPlus == fBlendMode)) {
        return sk_sp<const GrXferProcessor>(new ShaderPDXferProcessor(fBlendMode, coverage));
    }
    return sk_sp<const GrXferProcessor>(new PorterDuffXferProcessor(blendFormula, coverage));
}

static inline GrXPFactory::AnalysisProperties analysis_properties(
        const GrProcessorAnalysisColor& color, const GrProcessorAnalysisCoverage& coverage,
        const GrCaps& caps, GrClampType clampType, SkBlendMode mode) {
    using AnalysisProperties = GrXPFactory::AnalysisProperties;
    AnalysisProperties props = AnalysisProperties::kNone;
    bool hasCoverage = GrProcessorAnalysisCoverage::kNone != coverage;
    bool isLCD = GrProcessorAnalysisCoverage::kLCD == coverage;
    BlendFormula formula = [&](){
        if (isLCD) {
            return skgpu::GetLCDBlendFormula(mode);
        }
        return skgpu::GetBlendFormula(color.isOpaque(), hasCoverage, mode);
    }();

    if (formula.canTweakAlphaForCoverage() && !isLCD) {
        props |= AnalysisProperties::kCompatibleWithCoverageAsAlpha;
    }

    if (isLCD) {
        // See comment in MakeSrcOverXferProcessor about color.isOpaque here
        if (SkBlendMode::kSrcOver == mode && color.isConstant() && /*color.isOpaque() &&*/
            !caps.shaderCaps()->fDualSourceBlendingSupport &&
            !caps.shaderCaps()->fDstReadInShaderSupport) {
            props |= AnalysisProperties::kIgnoresInputColor;
        } else {
            // For LCD blending, if the color is not opaque we must read the dst in shader even if
            // we have dual source blending. The opaqueness check must be done after blending so for
            // simplicity we only allow src-over to not take the dst read path (though src, src-in,
            // and DstATop would also work). We also fall into the dst read case for src-over if we
            // do not have dual source blending.
            if (SkBlendMode::kSrcOver != mode ||
                /*!color.isOpaque() ||*/ // See comment in MakeSrcOverXferProcessor about isOpaque.
                (formula.hasSecondaryOutput() && !caps.shaderCaps()->fDualSourceBlendingSupport)) {
                props |= AnalysisProperties::kReadsDstInShader;
            }
        }
    } else {
        // With dual-source blending we never need the destination color in the shader.
        if (!caps.shaderCaps()->fDualSourceBlendingSupport) {
            if (formula.hasSecondaryOutput()) {
                props |= AnalysisProperties::kReadsDstInShader;
            }
        }
    }

    if (GrClampType::kAuto != clampType && SkBlendMode::kPlus == mode) {
        props |= AnalysisProperties::kReadsDstInShader;
    }

    if (!formula.modifiesDst() || !formula.usesInputColor()) {
        props |= AnalysisProperties::kIgnoresInputColor;
    }
    if (formula.unaffectedByDst() || (formula.unaffectedByDstIfOpaque() && color.isOpaque() &&
                                      !hasCoverage)) {
        props |= AnalysisProperties::kUnaffectedByDstValue;
    }
    return props;
}

GrXPFactory::AnalysisProperties GrPorterDuffXPFactory::analysisProperties(
        const GrProcessorAnalysisColor& color,
        const GrProcessorAnalysisCoverage& coverage,
        const GrCaps& caps,
        GrClampType clampType) const {
    return analysis_properties(color, coverage, caps, clampType, fBlendMode);
}

GR_DEFINE_XP_FACTORY_TEST(GrPorterDuffXPFactory)

#if defined(GR_TEST_UTILS)
const GrXPFactory* GrPorterDuffXPFactory::TestGet(GrProcessorTestData* d) {
    SkBlendMode mode = SkBlendMode(d->fRandom->nextULessThan((int)SkBlendMode::kLastCoeffMode));
    return GrPorterDuffXPFactory::Get(mode);
}
#endif

void GrPorterDuffXPFactory::TestGetXPOutputTypes(const GrXferProcessor* xp,
                                                 int* outPrimary,
                                                 int* outSecondary) {
    if (!!strcmp(xp->name(), "Porter Duff")) {
        *outPrimary = *outSecondary = -1;
        return;
    }
    BlendFormula blendFormula = static_cast<const PorterDuffXferProcessor*>(xp)->getBlendFormula();
    *outPrimary = blendFormula.primaryOutput();
    *outSecondary = blendFormula.secondaryOutput();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// SrcOver Global functions
////////////////////////////////////////////////////////////////////////////////////////////////
const GrXferProcessor& GrPorterDuffXPFactory::SimpleSrcOverXP() {
    static BlendFormula kSrcOverBlendFormula = skgpu::GetBlendFormula(
            /*isOpaque=*/false, /*hasCoverage=*/false, SkBlendMode::kSrcOver);
    static PorterDuffXferProcessor gSrcOverXP(kSrcOverBlendFormula,
                                              GrProcessorAnalysisCoverage::kSingleChannel);
    return gSrcOverXP;
}

sk_sp<const GrXferProcessor> GrPorterDuffXPFactory::MakeSrcOverXferProcessor(
        const GrProcessorAnalysisColor& color, GrProcessorAnalysisCoverage coverage,
        const GrCaps& caps) {
    // We want to not make an xfer processor if possible. Thus for the simple case where we are not
    // doing lcd blending we will just use our global SimpleSrcOverXP. This slightly differs from
    // the general case where we convert a src-over blend that has solid coverage and an opaque
    // color to src-mode, which allows disabling of blending.
    if (coverage != GrProcessorAnalysisCoverage::kLCD) {
        if (color.isOpaque() && coverage == GrProcessorAnalysisCoverage::kNone &&
            caps.shouldCollapseSrcOverToSrcWhenAble()) {
            BlendFormula blendFormula = skgpu::GetBlendFormula(true, false, SkBlendMode::kSrc);
            return sk_sp<GrXferProcessor>(new PorterDuffXferProcessor(blendFormula, coverage));
        }
        // We return nullptr here, which our caller interprets as meaning "use SimpleSrcOverXP".
        // We don't simply return the address of that XP here because our caller would have to unref
        // it and since it is a global object and GrProgramElement's ref-cnting system is not thread
        // safe.
        return nullptr;
    }

    // Currently up the stack Skia is requiring that the dst is opaque or that the client has said
    // the opaqueness doesn't matter. Thus for src-over we don't need to worry about the src color
    // being opaque or not. This allows us to use faster code paths as well as avoid various bugs
    // that occur with dst reads in the shader blending. For now we disable the check for
    // opaqueness, but in the future we should pass down the knowledge about dst opaqueness and make
    // the correct decision here.
    //
    // This also fixes a chrome bug on macs where we are getting random fuzziness when doing
    // blending in the shader for non opaque sources.
    if (color.isConstant() && /*color.isOpaque() &&*/
        !caps.shaderCaps()->fDualSourceBlendingSupport &&
        !caps.shaderCaps()->fDstReadInShaderSupport) {
        // If we don't have dual source blending or in shader dst reads, we fall
        // back to this trick for rendering SrcOver LCD text instead of doing a
        // dst copy.
        return PDLCDXferProcessor::Make(SkBlendMode::kSrcOver, color);
    }

    BlendFormula blendFormula = skgpu::GetLCDBlendFormula(SkBlendMode::kSrcOver);
    // See comment above regarding why the opaque check is commented out here.
    if (/*!color.isOpaque() ||*/
        (blendFormula.hasSecondaryOutput() && !caps.shaderCaps()->fDualSourceBlendingSupport)) {
        return sk_sp<GrXferProcessor>(new ShaderPDXferProcessor(SkBlendMode::kSrcOver, coverage));
    }
    return sk_sp<GrXferProcessor>(new PorterDuffXferProcessor(blendFormula, coverage));
}

sk_sp<const GrXferProcessor> GrPorterDuffXPFactory::MakeNoCoverageXP(SkBlendMode blendmode) {
    BlendFormula formula = skgpu::GetBlendFormula(false, false, blendmode);
    return sk_make_sp<PorterDuffXferProcessor>(formula, GrProcessorAnalysisCoverage::kNone);
}

GrXPFactory::AnalysisProperties GrPorterDuffXPFactory::SrcOverAnalysisProperties(
        const GrProcessorAnalysisColor& color,
        const GrProcessorAnalysisCoverage& coverage,
        const GrCaps& caps,
        GrClampType clampType) {
    return analysis_properties(color, coverage, caps, clampType, SkBlendMode::kSrcOver);
}
