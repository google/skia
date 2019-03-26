/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "GrBackendSurface.h"
#include "GrContextFactory.h"
#include "GrContextOptions.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrProxyProvider.h"
#include "GrXferProcessor.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "gl/GrGLCaps.h"
#include "ops/GrMeshDrawOp.h"

////////////////////////////////////////////////////////////////////////////////

static void test_color_unknown_with_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_color_not_opaque_no_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_color_opaque_with_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_color_opaque_no_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_lcd_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_lcd_coverage_fallback_case(skiatest::Reporter* reporter, const GrCaps& caps);

DEF_GPUTEST(GrPorterDuff, reporter, /*ctxInfo*/) {
    GrMockOptions mockOptions;
    mockOptions.fDualSourceBlendingSupport = true;
    auto context = GrContext::MakeMock(&mockOptions, GrContextOptions());
    const GrCaps& caps = *context->priv().getGpu()->caps();

    if (!caps.shaderCaps()->dualSourceBlendingSupport()) {
        SK_ABORT("Null context does not support dual source blending.");
        return;
    }

    test_color_unknown_with_coverage(reporter, caps);
    test_color_not_opaque_no_coverage(reporter, caps);
    test_color_opaque_with_coverage(reporter, caps);
    test_color_opaque_no_coverage(reporter, caps);
    test_lcd_coverage(reporter, caps);
    test_lcd_coverage_fallback_case(reporter, caps);
}

////////////////////////////////////////////////////////////////////////////////

#define TEST_ASSERT(...) REPORTER_ASSERT(reporter, __VA_ARGS__)

enum {
    kNone_OutputType,
    kCoverage_OutputType,
    kModulate_OutputType,
    kSAModulate_OutputType,
    kISAModulate_OutputType,
    kISCModulate_OutputType
};
static const int kInvalid_OutputType = -1;

static GrProcessorSet::Analysis do_analysis(const GrXPFactory* xpf,
                                            const GrProcessorAnalysisColor& colorInput,
                                            GrProcessorAnalysisCoverage coverageInput,
                                            const GrCaps& caps) {
    GrPaint paint;
    paint.setXPFactory(xpf);
    GrProcessorSet procs(std::move(paint));
    SkPMColor4f overrideColor;
    GrProcessorSet::Analysis analysis = procs.finalize(
            colorInput, coverageInput, nullptr, &GrUserStencilSettings::kUnused, GrFSAAType::kNone,
            caps, GrClampType::kAuto, &overrideColor);
    return analysis;
}

class GrPorterDuffTest {
public:
    struct XPInfo {
        XPInfo(skiatest::Reporter* reporter, SkBlendMode xfermode, const GrCaps& caps,
               GrProcessorAnalysisColor inputColor, GrProcessorAnalysisCoverage inputCoverage) {
            const GrXPFactory* xpf = GrPorterDuffXPFactory::Get(xfermode);

            bool isLCD = GrProcessorAnalysisCoverage::kLCD == inputCoverage;

            GrProcessorSet::Analysis analysis = do_analysis(xpf, inputColor, inputCoverage, caps);
            fCompatibleWithCoverageAsAlpha = analysis.isCompatibleWithCoverageAsAlpha();
            fIgnoresInputColor = analysis.inputColorIsIgnored();
            sk_sp<const GrXferProcessor> xp(
                    GrXPFactory::MakeXferProcessor(xpf, inputColor, inputCoverage, false, caps,
                                                   GrClampType::kAuto));
            TEST_ASSERT(!analysis.requiresDstTexture() ||
                        (isLCD &&
                         !caps.shaderCaps()->dstReadInShaderSupport() &&
                         (SkBlendMode::kSrcOver != xfermode ||
                          !inputColor.isOpaque())));
            // Porter Duff modes currently only use fixed-function or shader blending, and Ganesh
            // doesn't yet make use of framebuffer fetches that require a barrier
            // (e.g., QCOM_shader_framebuffer_fetch_noncoherent). So dst textures and xfer barriers
            // should always go hand in hand for Porter Duff modes.
            TEST_ASSERT(analysis.requiresDstTexture() == analysis.requiresNonOverlappingDraws());
            GetXPOutputTypes(xp.get(), &fPrimaryOutputType, &fSecondaryOutputType);
            xp->getBlendInfo(&fBlendInfo);
            TEST_ASSERT(!xp->willReadDstColor() ||
                        (isLCD && (SkBlendMode::kSrcOver != xfermode ||
                                   !inputColor.isOpaque())));
            TEST_ASSERT(xp->hasSecondaryOutput() == GrBlendCoeffRefsSrc2(fBlendInfo.fDstBlend));
        }

        bool fCompatibleWithCoverageAsAlpha;
        bool fIgnoresInputColor;
        int fPrimaryOutputType;
        int fSecondaryOutputType;
        GrXferProcessor::BlendInfo fBlendInfo;
    };

    static void GetXPOutputTypes(const GrXferProcessor* xp, int* outPrimary, int* outSecondary) {
        GrPorterDuffXPFactory::TestGetXPOutputTypes(xp, outPrimary, outSecondary);
    }
};

static void test_lcd_coverage(skiatest::Reporter* reporter, const GrCaps& caps) {
    GrProcessorAnalysisColor inputColor = GrProcessorAnalysisColor::Opaque::kYes;
    GrProcessorAnalysisCoverage inputCoverage = GrProcessorAnalysisCoverage::kLCD;

    for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
        SkBlendMode xfermode = static_cast<SkBlendMode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, inputColor, inputCoverage);
        switch (xfermode) {
            case SkBlendMode::kClear:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOver:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kSAModulate_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            default:
                ERRORF(reporter, "Invalid xfermode.");
                break;
        }
    }
}
static void test_color_unknown_with_coverage(skiatest::Reporter* reporter, const GrCaps& caps) {
    GrProcessorAnalysisColor inputColor = GrProcessorAnalysisColor::Opaque::kNo;
    GrProcessorAnalysisCoverage inputCoverage = GrProcessorAnalysisCoverage::kSingleChannel;

    for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
        SkBlendMode xfermode = static_cast<SkBlendMode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, inputColor, inputCoverage);
        switch (xfermode) {
            case SkBlendMode::kClear:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kCoverage_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2A_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOver:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2A_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kISAModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2A_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kISAModulate_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kISCModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISC_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            default:
                ERRORF(reporter, "Invalid xfermode.");
                break;
        }
    }
}

static void test_color_not_opaque_no_coverage(skiatest::Reporter* reporter, const GrCaps& caps) {
    GrProcessorAnalysisColor inputColor(
            SkPMColor4f::FromBytes_RGBA(GrColorPackRGBA(229, 0, 154, 240)));
    GrProcessorAnalysisCoverage inputCoverage = GrProcessorAnalysisCoverage::kNone;

    for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
        SkBlendMode xfermode = static_cast<SkBlendMode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, inputColor, inputCoverage);
        switch (xfermode) {
            case SkBlendMode::kClear:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOver:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSC_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISC_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            default:
                ERRORF(reporter, "Invalid xfermode.");
                break;
        }
    }
}

static void test_color_opaque_with_coverage(skiatest::Reporter* reporter, const GrCaps& caps) {
    GrProcessorAnalysisColor inputColor = GrProcessorAnalysisColor::Opaque::kYes;
    GrProcessorAnalysisCoverage inputCoverage = GrProcessorAnalysisCoverage::kSingleChannel;

    for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
        SkBlendMode xfermode = static_cast<SkBlendMode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, inputColor, inputCoverage);
        switch (xfermode) {
            case SkBlendMode::kClear:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kCoverage_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOver:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kCoverage_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(kISCModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISC_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            default:
                ERRORF(reporter, "Invalid xfermode.");
                break;
        }
    }
}

static void test_color_opaque_no_coverage(skiatest::Reporter* reporter, const GrCaps& caps) {
    GrProcessorAnalysisColor inputColor = GrProcessorAnalysisColor::Opaque::kYes;
    GrProcessorAnalysisCoverage inputCoverage = GrProcessorAnalysisCoverage::kNone;

    for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
        SkBlendMode xfermode = static_cast<SkBlendMode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, inputColor, inputCoverage);

        switch (xfermode) {
            case SkBlendMode::kClear:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOver:
                // We don't specialize opaque src-over. See note in GrPorterDuffXferProcessor.cpp
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSC_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISC_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            default:
                ERRORF(reporter, "Invalid xfermode.");
                break;
        }
    }
}

static void test_lcd_coverage_fallback_case(skiatest::Reporter* reporter, const GrCaps& caps) {
    constexpr GrClampType autoClamp = GrClampType::kAuto;
    const GrXPFactory* xpf = GrPorterDuffXPFactory::Get(SkBlendMode::kSrcOver);
    GrProcessorAnalysisColor color = SkPMColor4f::FromBytes_RGBA(GrColorPackRGBA(123, 45, 67, 255));
    GrProcessorAnalysisCoverage coverage = GrProcessorAnalysisCoverage::kLCD;
    TEST_ASSERT(!(GrXPFactory::GetAnalysisProperties(xpf, color, coverage, caps, autoClamp) &
                  GrXPFactory::AnalysisProperties::kRequiresDstTexture));
    sk_sp<const GrXferProcessor> xp_opaque(
            GrXPFactory::MakeXferProcessor(xpf, color, coverage, false, caps, autoClamp));
    if (!xp_opaque) {
        ERRORF(reporter, "Failed to create an XP with LCD coverage.");
        return;
    }

    GrXferProcessor::BlendInfo blendInfo;
    xp_opaque->getBlendInfo(&blendInfo);
    TEST_ASSERT(blendInfo.fWriteColor);

    // Test with non-opaque alpha
    color = SkPMColor4f::FromBytes_RGBA(GrColorPackRGBA(123, 45, 67, 221));
    coverage = GrProcessorAnalysisCoverage::kLCD;
    TEST_ASSERT(!(GrXPFactory::GetAnalysisProperties(xpf, color, coverage, caps, autoClamp) &
                GrXPFactory::AnalysisProperties::kRequiresDstTexture));
    sk_sp<const GrXferProcessor> xp(
            GrXPFactory::MakeXferProcessor(xpf, color, coverage, false, caps, autoClamp));
    if (!xp) {
        ERRORF(reporter, "Failed to create an XP with LCD coverage.");
        return;
    }

    xp->getBlendInfo(&blendInfo);
    TEST_ASSERT(blendInfo.fWriteColor);
}

DEF_GPUTEST(PorterDuffNoDualSourceBlending, reporter, options) {
    GrContextOptions opts = options;
    opts.fSuppressDualSourceBlending = true;
    sk_gpu_test::GrContextFactory mockFactory(opts);
    GrContext* ctx = mockFactory.get(sk_gpu_test::GrContextFactory::kMock_ContextType);
    if (!ctx) {
        SK_ABORT("Failed to create mock context without ARB_blend_func_extended.");
        return;
    }

    GrGpu* gpu = ctx->priv().getGpu();
    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
    const GrCaps& caps = *ctx->priv().caps();
    if (caps.shaderCaps()->dualSourceBlendingSupport()) {
        SK_ABORT("Mock context failed to honor request for no ARB_blend_func_extended.");
        return;
    }

    GrBackendTexture backendTex =
        gpu->createTestingOnlyBackendTexture(nullptr, 100, 100, GrColorType::kRGBA_8888,
                                             false, GrMipMapped::kNo);

    GrXferProcessor::DstProxy fakeDstProxy;
    {
        sk_sp<GrTextureProxy> proxy = proxyProvider->wrapBackendTexture(
                backendTex, kTopLeft_GrSurfaceOrigin, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
                kRead_GrIOType);
        fakeDstProxy.setProxy(std::move(proxy));
    }

    static const GrProcessorAnalysisColor colorInputs[] = {
            GrProcessorAnalysisColor::Opaque::kNo, GrProcessorAnalysisColor::Opaque::kYes,
            GrProcessorAnalysisColor(SkPMColor4f::FromBytes_RGBA(GrColorPackRGBA(0, 82, 17, 100))),
            GrProcessorAnalysisColor(SkPMColor4f::FromBytes_RGBA(GrColorPackRGBA(0, 82, 17, 255)))};

    for (const auto& colorInput : colorInputs) {
        for (GrProcessorAnalysisCoverage coverageType :
             {GrProcessorAnalysisCoverage::kSingleChannel, GrProcessorAnalysisCoverage::kNone}) {
            for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
                SkBlendMode xfermode = static_cast<SkBlendMode>(m);
                const GrXPFactory* xpf = GrPorterDuffXPFactory::Get(xfermode);
                sk_sp<const GrXferProcessor> xp(
                        GrXPFactory::MakeXferProcessor(xpf, colorInput, coverageType, false, caps,
                                                       GrClampType::kAuto));
                if (!xp) {
                    ERRORF(reporter, "Failed to create an XP without dual source blending.");
                    return;
                }
                TEST_ASSERT(!xp->hasSecondaryOutput());
            }
        }
    }
    gpu->deleteTestingOnlyBackendTexture(backendTex);
}
