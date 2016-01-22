/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrCustomXfermode.h"

#include "GrCoordTransform.h"
#include "GrContext.h"
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "GrPipeline.h"
#include "GrProcessor.h"
#include "GrTexture.h"
#include "GrTextureAccess.h"
#include "SkXfermode.h"
#include "glsl/GrGLSLBlend.h"
#include "glsl/GrGLSLCaps.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLXferProcessor.h"

bool GrCustomXfermode::IsSupportedMode(SkXfermode::Mode mode) {
    return mode > SkXfermode::kLastCoeffMode && mode <= SkXfermode::kLastMode;
}

///////////////////////////////////////////////////////////////////////////////
// Static helpers
///////////////////////////////////////////////////////////////////////////////

static GrBlendEquation hw_blend_equation(SkXfermode::Mode mode) {
    enum { kOffset = kOverlay_GrBlendEquation - SkXfermode::kOverlay_Mode };
    return static_cast<GrBlendEquation>(mode + kOffset);

    GR_STATIC_ASSERT(kOverlay_GrBlendEquation == SkXfermode::kOverlay_Mode + kOffset);
    GR_STATIC_ASSERT(kDarken_GrBlendEquation == SkXfermode::kDarken_Mode + kOffset);
    GR_STATIC_ASSERT(kLighten_GrBlendEquation == SkXfermode::kLighten_Mode + kOffset);
    GR_STATIC_ASSERT(kColorDodge_GrBlendEquation == SkXfermode::kColorDodge_Mode + kOffset);
    GR_STATIC_ASSERT(kColorBurn_GrBlendEquation == SkXfermode::kColorBurn_Mode + kOffset);
    GR_STATIC_ASSERT(kHardLight_GrBlendEquation == SkXfermode::kHardLight_Mode + kOffset);
    GR_STATIC_ASSERT(kSoftLight_GrBlendEquation == SkXfermode::kSoftLight_Mode + kOffset);
    GR_STATIC_ASSERT(kDifference_GrBlendEquation == SkXfermode::kDifference_Mode + kOffset);
    GR_STATIC_ASSERT(kExclusion_GrBlendEquation == SkXfermode::kExclusion_Mode + kOffset);
    GR_STATIC_ASSERT(kMultiply_GrBlendEquation == SkXfermode::kMultiply_Mode + kOffset);
    GR_STATIC_ASSERT(kHSLHue_GrBlendEquation == SkXfermode::kHue_Mode + kOffset);
    GR_STATIC_ASSERT(kHSLSaturation_GrBlendEquation == SkXfermode::kSaturation_Mode + kOffset);
    GR_STATIC_ASSERT(kHSLColor_GrBlendEquation == SkXfermode::kColor_Mode + kOffset);
    GR_STATIC_ASSERT(kHSLLuminosity_GrBlendEquation == SkXfermode::kLuminosity_Mode + kOffset);
    GR_STATIC_ASSERT(kGrBlendEquationCnt == SkXfermode::kLastMode + 1 + kOffset);
}

static bool can_use_hw_blend_equation(GrBlendEquation equation,
                                      const GrProcOptInfo& coveragePOI,
                                      const GrCaps& caps) {
    if (!caps.advancedBlendEquationSupport()) {
        return false;
    }
    if (coveragePOI.isFourChannelOutput()) {
        return false; // LCD coverage must be applied after the blend equation.
    }
    if (caps.canUseAdvancedBlendEquation(equation)) {
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Xfer Processor
///////////////////////////////////////////////////////////////////////////////

class CustomXP : public GrXferProcessor {
public:
    CustomXP(SkXfermode::Mode mode, GrBlendEquation hwBlendEquation)
        : fMode(mode),
          fHWBlendEquation(hwBlendEquation) {
        this->initClassID<CustomXP>();
    }

    CustomXP(const DstTexture* dstTexture, bool hasMixedSamples, SkXfermode::Mode mode)
        : INHERITED(dstTexture, true, hasMixedSamples),
          fMode(mode),
          fHWBlendEquation(static_cast<GrBlendEquation>(-1)) {
        this->initClassID<CustomXP>();
    }

    const char* name() const override { return "Custom Xfermode"; }

    GrGLSLXferProcessor* createGLSLInstance() const override;

    SkXfermode::Mode mode() const { return fMode; }
    bool hasHWBlendEquation() const { return -1 != static_cast<int>(fHWBlendEquation); }

    GrBlendEquation hwBlendEquation() const {
        SkASSERT(this->hasHWBlendEquation());
        return fHWBlendEquation;
    }

private:
    GrXferProcessor::OptFlags onGetOptimizations(const GrPipelineOptimizations& optimizations,
                                                 bool doesStencilWrite,
                                                 GrColor* overrideColor,
                                                 const GrCaps& caps) const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    GrXferBarrierType onXferBarrier(const GrRenderTarget*, const GrCaps&) const override;

    void onGetBlendInfo(BlendInfo*) const override;

    bool onIsEqual(const GrXferProcessor& xpBase) const override;

    const SkXfermode::Mode fMode;
    const GrBlendEquation  fHWBlendEquation;

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GLCustomXP : public GrGLSLXferProcessor {
public:
    GLCustomXP(const GrXferProcessor&) {}
    ~GLCustomXP() override {}

    static void GenKey(const GrXferProcessor& p, const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) {
        const CustomXP& xp = p.cast<CustomXP>();
        uint32_t key = 0;
        if (xp.hasHWBlendEquation()) {
            SkASSERT(caps.advBlendEqInteraction() > 0);  // 0 will mean !xp.hasHWBlendEquation().
            key |= caps.advBlendEqInteraction();
            GR_STATIC_ASSERT(GrGLSLCaps::kLast_AdvBlendEqInteraction < 4);
        }
        if (!xp.hasHWBlendEquation() || caps.mustEnableSpecificAdvBlendEqs()) {
            key |= xp.mode() << 3;
        }
        b->add32(key);
    }

private:
    void emitOutputsForBlendState(const EmitArgs& args) override {
        const CustomXP& xp = args.fXP.cast<CustomXP>();
        SkASSERT(xp.hasHWBlendEquation());

        GrGLSLXPFragmentBuilder* fragBuilder = args.fXPFragBuilder;
        fragBuilder->enableAdvancedBlendEquationIfNeeded(xp.hwBlendEquation());

        // Apply coverage by multiplying it into the src color before blending. Mixed samples will
        // "just work" automatically. (See onGetOptimizations())
        if (args.fInputCoverage) {
            fragBuilder->codeAppendf("%s = %s * %s;",
                                     args.fOutputPrimary, args.fInputCoverage, args.fInputColor);
        } else {
            fragBuilder->codeAppendf("%s = %s;", args.fOutputPrimary, args.fInputColor);
        }
    }

    void emitBlendCodeForDstRead(GrGLSLXPFragmentBuilder* fragBuilder,
                                 GrGLSLUniformHandler* uniformHandler,
                                 const char* srcColor,
                                 const char* srcCoverage,
                                 const char* dstColor,
                                 const char* outColor,
                                 const char* outColorSecondary,
                                 const GrXferProcessor& proc) override {
        const CustomXP& xp = proc.cast<CustomXP>();
        SkASSERT(!xp.hasHWBlendEquation());

        GrGLSLBlend::AppendMode(fragBuilder, srcColor, dstColor, outColor, xp.mode());

        // Apply coverage.
        if (xp.dstReadUsesMixedSamples()) {
            if (srcCoverage) {
                fragBuilder->codeAppendf("%s *= %s;", outColor, srcCoverage);
                fragBuilder->codeAppendf("%s = %s;", outColorSecondary, srcCoverage);
            } else {
                fragBuilder->codeAppendf("%s = vec4(1.0);", outColorSecondary);
            }
        } else if (srcCoverage) {
            fragBuilder->codeAppendf("%s = %s * %s + (vec4(1.0) - %s) * %s;",
                                     outColor, srcCoverage, outColor, srcCoverage, dstColor);
        }
    }

    void onSetData(const GrGLSLProgramDataManager&, const GrXferProcessor&) override {}

    typedef GrGLSLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

void CustomXP::onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLCustomXP::GenKey(*this, caps, b);
}

GrGLSLXferProcessor* CustomXP::createGLSLInstance() const {
    SkASSERT(this->willReadDstColor() != this->hasHWBlendEquation());
    return new GLCustomXP(*this);
}

bool CustomXP::onIsEqual(const GrXferProcessor& other) const {
    const CustomXP& s = other.cast<CustomXP>();
    return fMode == s.fMode && fHWBlendEquation == s.fHWBlendEquation;
}

GrXferProcessor::OptFlags CustomXP::onGetOptimizations(const GrPipelineOptimizations& optimizations,
                                                       bool doesStencilWrite,
                                                       GrColor* overrideColor,
                                                       const GrCaps& caps) const {
  /*
    Most the optimizations we do here are based on tweaking alpha for coverage.

    The general SVG blend equation is defined in the spec as follows:

      Dca' = B(Sc, Dc) * Sa * Da + Y * Sca * (1-Da) + Z * Dca * (1-Sa)
      Da'  = X * Sa * Da + Y * Sa * (1-Da) + Z * Da * (1-Sa)

    (Note that Sca, Dca indicate RGB vectors that are premultiplied by alpha,
     and that B(Sc, Dc) is a mode-specific function that accepts non-multiplied
     RGB colors.)

    For every blend mode supported by this class, i.e. the "advanced" blend
    modes, X=Y=Z=1 and this equation reduces to the PDF blend equation.

    It can be shown that when X=Y=Z=1, these equations can modulate alpha for
    coverage.


    == Color ==

    We substitute Y=Z=1 and define a blend() function that calculates Dca' in
    terms of premultiplied alpha only:

      blend(Sca, Dca, Sa, Da) = {Dca : if Sa == 0,
                                 Sca : if Da == 0,
                                 B(Sca/Sa, Dca/Da) * Sa * Da + Sca * (1-Da) + Dca * (1-Sa) : if Sa,Da != 0}

    And for coverage modulation, we use a post blend src-over model:

      Dca'' = f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca

    (Where f is the fractional coverage.)

    Next we show that canTweakAlphaForCoverage() is true by proving the
    following relationship:

      blend(f*Sca, Dca, f*Sa, Da) == f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca

    General case (f,Sa,Da != 0):

      f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca
        = f * (B(Sca/Sa, Dca/Da) * Sa * Da + Sca * (1-Da) + Dca * (1-Sa)) + (1-f) * Dca  [Sa,Da != 0, definition of blend()]
        = B(Sca/Sa, Dca/Da) * f*Sa * Da + f*Sca * (1-Da) + f*Dca * (1-Sa) + Dca - f*Dca
        = B(Sca/Sa, Dca/Da) * f*Sa * Da + f*Sca - f*Sca * Da + f*Dca - f*Dca * Sa + Dca - f*Dca
        = B(Sca/Sa, Dca/Da) * f*Sa * Da + f*Sca - f*Sca * Da - f*Dca * Sa + Dca
        = B(Sca/Sa, Dca/Da) * f*Sa * Da + f*Sca * (1-Da) - f*Dca * Sa + Dca
        = B(Sca/Sa, Dca/Da) * f*Sa * Da + f*Sca * (1-Da) + Dca * (1 - f*Sa)
        = B(f*Sca/f*Sa, Dca/Da) * f*Sa * Da + f*Sca * (1-Da) + Dca * (1 - f*Sa)  [f!=0]
        = blend(f*Sca, Dca, f*Sa, Da)  [definition of blend()]

    Corner cases (Sa=0, Da=0, and f=0):

      Sa=0: f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca
              = f * Dca + (1-f) * Dca  [Sa=0, definition of blend()]
              = Dca
              = blend(0, Dca, 0, Da)  [definition of blend()]
              = blend(f*Sca, Dca, f*Sa, Da)  [Sa=0]

      Da=0: f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca
              = f * Sca + (1-f) * Dca  [Da=0, definition of blend()]
              = f * Sca  [Da=0]
              = blend(f*Sca, 0, f*Sa, 0)  [definition of blend()]
              = blend(f*Sca, Dca, f*Sa, Da)  [Da=0]

      f=0: f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca
             = Dca  [f=0]
             = blend(0, Dca, 0, Da)  [definition of blend()]
             = blend(f*Sca, Dca, f*Sa, Da)  [f=0]

    == Alpha ==

    We substitute X=Y=Z=1 and define a blend() function that calculates Da':

      blend(Sa, Da) = Sa * Da + Sa * (1-Da) + Da * (1-Sa)
                    = Sa * Da + Sa - Sa * Da + Da - Da * Sa
                    = Sa + Da - Sa * Da

    We use the same model for coverage modulation as we did with color:

      Da'' = f * blend(Sa, Da) + (1-f) * Da

    And show that canTweakAlphaForCoverage() is true by proving the following
    relationship:

      blend(f*Sa, Da) == f * blend(Sa, Da) + (1-f) * Da


      f * blend(Sa, Da) + (1-f) * Da
        = f * (Sa + Da - Sa * Da) + (1-f) * Da
        = f*Sa + f*Da - f*Sa * Da + Da - f*Da
        = f*Sa - f*Sa * Da + Da
        = f*Sa + Da - f*Sa * Da
        = blend(f*Sa, Da)
   */

    OptFlags flags = kNone_OptFlags;
    if (optimizations.fColorPOI.allStagesMultiplyInput()) {
        flags |= kCanTweakAlphaForCoverage_OptFlag;
    }
    if (this->hasHWBlendEquation() && optimizations.fCoveragePOI.isSolidWhite()) {
        flags |= kIgnoreCoverage_OptFlag;
    }
    return flags;
}

GrXferBarrierType CustomXP::onXferBarrier(const GrRenderTarget* rt, const GrCaps& caps) const {
    if (this->hasHWBlendEquation() && !caps.advancedCoherentBlendEquationSupport()) {
        return kBlend_GrXferBarrierType;
    }
    return kNone_GrXferBarrierType;
}

void CustomXP::onGetBlendInfo(BlendInfo* blendInfo) const {
    if (this->hasHWBlendEquation()) {
        blendInfo->fEquation = this->hwBlendEquation();
    }
}

///////////////////////////////////////////////////////////////////////////////
class CustomXPFactory : public GrXPFactory {
public:
    CustomXPFactory(SkXfermode::Mode mode);

    void getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                  GrXPFactory::InvariantBlendedColor*) const override;

private:
    GrXferProcessor* onCreateXferProcessor(const GrCaps& caps,
                                           const GrPipelineOptimizations& optimizations,
                                           bool hasMixedSamples,
                                           const DstTexture*) const override;

    bool willReadDstColor(const GrCaps& caps,
                          const GrPipelineOptimizations& optimizations,
                          bool hasMixedSamples) const override;

    bool onIsEqual(const GrXPFactory& xpfBase) const override {
        const CustomXPFactory& xpf = xpfBase.cast<CustomXPFactory>();
        return fMode == xpf.fMode;
    }

    GR_DECLARE_XP_FACTORY_TEST;

    SkXfermode::Mode fMode;
    GrBlendEquation  fHWBlendEquation;

    typedef GrXPFactory INHERITED;
};

CustomXPFactory::CustomXPFactory(SkXfermode::Mode mode)
    : fMode(mode),
      fHWBlendEquation(hw_blend_equation(mode)) {
    SkASSERT(GrCustomXfermode::IsSupportedMode(fMode));
    this->initClassID<CustomXPFactory>();
}

GrXferProcessor* CustomXPFactory::onCreateXferProcessor(const GrCaps& caps,
                                                        const GrPipelineOptimizations& opt,
                                                        bool hasMixedSamples,
                                                        const DstTexture* dstTexture) const {
    if (can_use_hw_blend_equation(fHWBlendEquation, opt.fCoveragePOI, caps)) {
        SkASSERT(!dstTexture || !dstTexture->texture());
        return new CustomXP(fMode, fHWBlendEquation);
    }
    return new CustomXP(dstTexture, hasMixedSamples, fMode);
}

bool CustomXPFactory::willReadDstColor(const GrCaps& caps,
                                       const GrPipelineOptimizations& optimizations,
                                       bool hasMixedSamples) const {
    return !can_use_hw_blend_equation(fHWBlendEquation, optimizations.fCoveragePOI, caps);
}

void CustomXPFactory::getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                               InvariantBlendedColor* blendedColor) const {
    blendedColor->fWillBlendWithDst = true;
    blendedColor->fKnownColorFlags = kNone_GrColorComponentFlags;
}

GR_DEFINE_XP_FACTORY_TEST(CustomXPFactory);
const GrXPFactory* CustomXPFactory::TestCreate(GrProcessorTestData* d) {
    int mode = d->fRandom->nextRangeU(SkXfermode::kLastCoeffMode + 1,
                                      SkXfermode::kLastSeparableMode);

    return new CustomXPFactory(static_cast<SkXfermode::Mode>(mode));
}

///////////////////////////////////////////////////////////////////////////////

GrXPFactory* GrCustomXfermode::CreateXPFactory(SkXfermode::Mode mode) {
    if (!GrCustomXfermode::IsSupportedMode(mode)) {
        return nullptr;
    } else {
        return new CustomXPFactory(mode);
    }
}
