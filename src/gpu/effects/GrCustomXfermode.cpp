/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrCustomXfermode.h"

#include "GrCaps.h"
#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrPipeline.h"
#include "GrProcessor.h"
#include "GrShaderCaps.h"
#include "glsl/GrGLSLBlend.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLXferProcessor.h"

bool GrCustomXfermode::IsSupportedMode(SkBlendMode mode) {
    return (int)mode  > (int)SkBlendMode::kLastCoeffMode &&
           (int)mode <= (int)SkBlendMode::kLastMode;
}

///////////////////////////////////////////////////////////////////////////////
// Static helpers
///////////////////////////////////////////////////////////////////////////////

static constexpr GrBlendEquation hw_blend_equation(SkBlendMode mode) {
// In C++14 this could be a constexpr int variable.
#define EQ_OFFSET (kOverlay_GrBlendEquation - (int)SkBlendMode::kOverlay)
    GR_STATIC_ASSERT(kOverlay_GrBlendEquation == (int)SkBlendMode::kOverlay + EQ_OFFSET);
    GR_STATIC_ASSERT(kDarken_GrBlendEquation == (int)SkBlendMode::kDarken + EQ_OFFSET);
    GR_STATIC_ASSERT(kLighten_GrBlendEquation == (int)SkBlendMode::kLighten + EQ_OFFSET);
    GR_STATIC_ASSERT(kColorDodge_GrBlendEquation == (int)SkBlendMode::kColorDodge + EQ_OFFSET);
    GR_STATIC_ASSERT(kColorBurn_GrBlendEquation == (int)SkBlendMode::kColorBurn + EQ_OFFSET);
    GR_STATIC_ASSERT(kHardLight_GrBlendEquation == (int)SkBlendMode::kHardLight + EQ_OFFSET);
    GR_STATIC_ASSERT(kSoftLight_GrBlendEquation == (int)SkBlendMode::kSoftLight + EQ_OFFSET);
    GR_STATIC_ASSERT(kDifference_GrBlendEquation == (int)SkBlendMode::kDifference + EQ_OFFSET);
    GR_STATIC_ASSERT(kExclusion_GrBlendEquation == (int)SkBlendMode::kExclusion + EQ_OFFSET);
    GR_STATIC_ASSERT(kMultiply_GrBlendEquation == (int)SkBlendMode::kMultiply + EQ_OFFSET);
    GR_STATIC_ASSERT(kHSLHue_GrBlendEquation == (int)SkBlendMode::kHue + EQ_OFFSET);
    GR_STATIC_ASSERT(kHSLSaturation_GrBlendEquation == (int)SkBlendMode::kSaturation + EQ_OFFSET);
    GR_STATIC_ASSERT(kHSLColor_GrBlendEquation == (int)SkBlendMode::kColor + EQ_OFFSET);
    GR_STATIC_ASSERT(kHSLLuminosity_GrBlendEquation == (int)SkBlendMode::kLuminosity + EQ_OFFSET);

    // There's an illegal GrBlendEquation that corresponds to no SkBlendMode, hence the extra +1.
    GR_STATIC_ASSERT(kGrBlendEquationCnt == (int)SkBlendMode::kLastMode + 1 + 1 + EQ_OFFSET);

    return static_cast<GrBlendEquation>((int)mode + EQ_OFFSET);
#undef EQ_OFFSET
}

static bool can_use_hw_blend_equation(GrBlendEquation equation,
                                      GrProcessorAnalysisCoverage coverage, const GrCaps& caps) {
    if (!caps.advancedBlendEquationSupport()) {
        return false;
    }
    if (GrProcessorAnalysisCoverage::kLCD == coverage) {
        return false; // LCD coverage must be applied after the blend equation.
    }
    if (caps.isAdvancedBlendEquationBlacklisted(equation)) {
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Xfer Processor
///////////////////////////////////////////////////////////////////////////////

class CustomXP : public GrXferProcessor {
public:
    CustomXP(SkBlendMode mode, GrBlendEquation hwBlendEquation)
        : INHERITED(kCustomXP_ClassID)
        , fMode(mode)
        , fHWBlendEquation(hwBlendEquation) {}

    CustomXP(bool hasMixedSamples, SkBlendMode mode, GrProcessorAnalysisCoverage coverage)
            : INHERITED(kCustomXP_ClassID, true, hasMixedSamples, coverage)
            , fMode(mode)
            , fHWBlendEquation(kIllegal_GrBlendEquation) {
    }

    const char* name() const override { return "Custom Xfermode"; }

    GrGLSLXferProcessor* createGLSLInstance() const override;

    SkBlendMode mode() const { return fMode; }
    bool hasHWBlendEquation() const { return kIllegal_GrBlendEquation != fHWBlendEquation; }

    GrBlendEquation hwBlendEquation() const {
        SkASSERT(this->hasHWBlendEquation());
        return fHWBlendEquation;
    }

    GrXferBarrierType xferBarrierType(const GrCaps&) const override;

private:
    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override;

    void onGetBlendInfo(BlendInfo*) const override;

    bool onIsEqual(const GrXferProcessor& xpBase) const override;

    const SkBlendMode      fMode;
    const GrBlendEquation  fHWBlendEquation;

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GLCustomXP : public GrGLSLXferProcessor {
public:
    GLCustomXP(const GrXferProcessor&) {}
    ~GLCustomXP() override {}

    static void GenKey(const GrXferProcessor& p, const GrShaderCaps& caps,
                       GrProcessorKeyBuilder* b) {
        const CustomXP& xp = p.cast<CustomXP>();
        uint32_t key = 0;
        if (xp.hasHWBlendEquation()) {
            SkASSERT(caps.advBlendEqInteraction() > 0);  // 0 will mean !xp.hasHWBlendEquation().
            key |= caps.advBlendEqInteraction();
            GR_STATIC_ASSERT(GrShaderCaps::kLast_AdvBlendEqInteraction < 4);
        }
        if (!xp.hasHWBlendEquation() || caps.mustEnableSpecificAdvBlendEqs()) {
            key |= (int)xp.mode() << 3;
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
        fragBuilder->codeAppendf("%s = %s * %s;", args.fOutputPrimary, args.fInputCoverage,
                                 args.fInputColor);
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
        INHERITED::DefaultCoverageModulation(fragBuilder, srcCoverage, dstColor, outColor,
                                             outColorSecondary, xp);
    }

    void onSetData(const GrGLSLProgramDataManager&, const GrXferProcessor&) override {}

    typedef GrGLSLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

void CustomXP::onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const {
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

GrXferBarrierType CustomXP::xferBarrierType(const GrCaps& caps) const {
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

// See the comment above GrXPFactory's definition about this warning suppression.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif
class CustomXPFactory : public GrXPFactory {
public:
    constexpr CustomXPFactory(SkBlendMode mode)
            : fMode(mode), fHWBlendEquation(hw_blend_equation(mode)) {}

private:
    sk_sp<const GrXferProcessor> makeXferProcessor(const GrProcessorAnalysisColor&,
                                                   GrProcessorAnalysisCoverage,
                                                   bool hasMixedSamples,
                                                   const GrCaps&,
                                                   GrClampType) const override;

    AnalysisProperties analysisProperties(const GrProcessorAnalysisColor&,
                                          const GrProcessorAnalysisCoverage&,
                                          const GrCaps&,
                                          GrClampType) const override;

    GR_DECLARE_XP_FACTORY_TEST

    SkBlendMode fMode;
    GrBlendEquation fHWBlendEquation;

    typedef GrXPFactory INHERITED;
};
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

sk_sp<const GrXferProcessor> CustomXPFactory::makeXferProcessor(
        const GrProcessorAnalysisColor&,
        GrProcessorAnalysisCoverage coverage,
        bool hasMixedSamples,
        const GrCaps& caps,
        GrClampType clampType) const {
    SkASSERT(GrCustomXfermode::IsSupportedMode(fMode));
    if (can_use_hw_blend_equation(fHWBlendEquation, coverage, caps)) {
        return sk_sp<GrXferProcessor>(new CustomXP(fMode, fHWBlendEquation));
    }
    return sk_sp<GrXferProcessor>(new CustomXP(hasMixedSamples, fMode, coverage));
}

GrXPFactory::AnalysisProperties CustomXPFactory::analysisProperties(
        const GrProcessorAnalysisColor&, const GrProcessorAnalysisCoverage& coverage,
        const GrCaps& caps, GrClampType clampType) const {
    /*
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
                                   B(Sca/Sa, Dca/Da) * Sa * Da + Sca * (1-Da) + Dca * (1-Sa) : if
      Sa,Da != 0}

      And for coverage modulation, we use a post blend src-over model:

        Dca'' = f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca

      (Where f is the fractional coverage.)

      Next we show that canTweakAlphaForCoverage() is true by proving the
      following relationship:

        blend(f*Sca, Dca, f*Sa, Da) == f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca

      General case (f,Sa,Da != 0):

        f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca
          = f * (B(Sca/Sa, Dca/Da) * Sa * Da + Sca * (1-Da) + Dca * (1-Sa)) + (1-f) * Dca  [Sa,Da !=
      0, definition of blend()]
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
    if (can_use_hw_blend_equation(fHWBlendEquation, coverage, caps)) {
        if (caps.blendEquationSupport() == GrCaps::kAdvancedCoherent_BlendEquationSupport) {
            return AnalysisProperties::kCompatibleWithAlphaAsCoverage;
        } else {
            return AnalysisProperties::kCompatibleWithAlphaAsCoverage |
                   AnalysisProperties::kRequiresNonOverlappingDraws;
        }
    }
    return AnalysisProperties::kCompatibleWithAlphaAsCoverage |
           AnalysisProperties::kReadsDstInShader;
}

GR_DEFINE_XP_FACTORY_TEST(CustomXPFactory);
#if GR_TEST_UTILS
const GrXPFactory* CustomXPFactory::TestGet(GrProcessorTestData* d) {
    int mode = d->fRandom->nextRangeU((int)SkBlendMode::kLastCoeffMode + 1,
                                      (int)SkBlendMode::kLastSeparableMode);

    return GrCustomXfermode::Get((SkBlendMode)mode);
}
#endif

///////////////////////////////////////////////////////////////////////////////

const GrXPFactory* GrCustomXfermode::Get(SkBlendMode mode) {
    // If these objects are constructed as static constexpr by cl.exe (2015 SP2) the vtables are
    // null.
#ifdef SK_BUILD_FOR_WIN
#define _CONSTEXPR_
#else
#define _CONSTEXPR_ constexpr
#endif
    static _CONSTEXPR_ const CustomXPFactory gOverlay(SkBlendMode::kOverlay);
    static _CONSTEXPR_ const CustomXPFactory gDarken(SkBlendMode::kDarken);
    static _CONSTEXPR_ const CustomXPFactory gLighten(SkBlendMode::kLighten);
    static _CONSTEXPR_ const CustomXPFactory gColorDodge(SkBlendMode::kColorDodge);
    static _CONSTEXPR_ const CustomXPFactory gColorBurn(SkBlendMode::kColorBurn);
    static _CONSTEXPR_ const CustomXPFactory gHardLight(SkBlendMode::kHardLight);
    static _CONSTEXPR_ const CustomXPFactory gSoftLight(SkBlendMode::kSoftLight);
    static _CONSTEXPR_ const CustomXPFactory gDifference(SkBlendMode::kDifference);
    static _CONSTEXPR_ const CustomXPFactory gExclusion(SkBlendMode::kExclusion);
    static _CONSTEXPR_ const CustomXPFactory gMultiply(SkBlendMode::kMultiply);
    static _CONSTEXPR_ const CustomXPFactory gHue(SkBlendMode::kHue);
    static _CONSTEXPR_ const CustomXPFactory gSaturation(SkBlendMode::kSaturation);
    static _CONSTEXPR_ const CustomXPFactory gColor(SkBlendMode::kColor);
    static _CONSTEXPR_ const CustomXPFactory gLuminosity(SkBlendMode::kLuminosity);
#undef _CONSTEXPR_
    switch (mode) {
        case SkBlendMode::kOverlay:
            return &gOverlay;
        case SkBlendMode::kDarken:
            return &gDarken;
        case SkBlendMode::kLighten:
            return &gLighten;
        case SkBlendMode::kColorDodge:
            return &gColorDodge;
        case SkBlendMode::kColorBurn:
            return &gColorBurn;
        case SkBlendMode::kHardLight:
            return &gHardLight;
        case SkBlendMode::kSoftLight:
            return &gSoftLight;
        case SkBlendMode::kDifference:
            return &gDifference;
        case SkBlendMode::kExclusion:
            return &gExclusion;
        case SkBlendMode::kMultiply:
            return &gMultiply;
        case SkBlendMode::kHue:
            return &gHue;
        case SkBlendMode::kSaturation:
            return &gSaturation;
        case SkBlendMode::kColor:
            return &gColor;
        case SkBlendMode::kLuminosity:
            return &gLuminosity;
        default:
            SkASSERT(!GrCustomXfermode::IsSupportedMode(mode));
            return nullptr;
    }
}
