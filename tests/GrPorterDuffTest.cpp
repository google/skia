/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if SK_SUPPORT_GPU

#include "GrContextFactory.h"
#include "GrContextOptions.h"
#include "GrGpu.h"
#include "GrResourceProvider.h"
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

DEF_GPUTEST_FOR_NULLGL_CONTEXT(GrPorterDuff, reporter, ctxInfo) {
    const GrCaps& caps = *ctxInfo.grContext()->getGpu()->caps();
    if (!caps.shaderCaps()->dualSourceBlendingSupport()) {
        SkFAIL("Null context does not support dual source blending.");
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

class GrPorterDuffTest {
public:
    struct XPInfo {
        XPInfo(skiatest::Reporter* reporter, SkBlendMode xfermode, const GrCaps& caps,
               GrProcessorAnalysisColor inputColor, GrProcessorAnalysisCoverage inputCoverage) {
            const GrXPFactory* xpf = GrPorterDuffXPFactory::Get(xfermode);
            GrProcessorSet::Analysis analysis(inputColor, inputCoverage, xpf, caps);
            fCompatibleWithCoverageAsAlpha = analysis.isCompatibleWithCoverageAsAlpha();
            fCanCombineOverlappedStencilAndCover = analysis.canCombineOverlappedStencilAndCover();
            fIgnoresInputColor = analysis.isInputColorIgnored();
            sk_sp<GrXferProcessor> xp(
                    GrXPFactory::MakeXferProcessor(xpf, inputColor, inputCoverage, false, caps));
            TEST_ASSERT(!analysis.requiresDstTexture());
            GetXPOutputTypes(xp.get(), &fPrimaryOutputType, &fSecondaryOutputType);
            xp->getBlendInfo(&fBlendInfo);
            TEST_ASSERT(!xp->willReadDstColor());
            TEST_ASSERT(xp->hasSecondaryOutput() == GrBlendCoeffRefsSrc2(fBlendInfo.fDstBlend));
        }

        bool fCanCombineOverlappedStencilAndCover;
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
    GrProcessorAnalysisColor inputColor = GrProcessorAnalysisColor::Opaque::kNo;
    GrProcessorAnalysisCoverage inputCoverage = GrProcessorAnalysisCoverage::kLCD;

    for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
        SkBlendMode xfermode = static_cast<SkBlendMode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, inputColor, inputCoverage);
        switch (xfermode) {
            case SkBlendMode::kClear:
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOver:
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kSAModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISC_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kSAModulate_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kSAModulate_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
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
static void test_color_unknown_with_coverage(skiatest::Reporter* reporter, const GrCaps& caps) {
    GrProcessorAnalysisColor inputColor = GrProcessorAnalysisColor::Opaque::kNo;
    GrProcessorAnalysisCoverage inputCoverage = GrProcessorAnalysisCoverage::kSingleChannel;

    for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
        SkBlendMode xfermode = static_cast<SkBlendMode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, inputColor, inputCoverage);
        switch (xfermode) {
            case SkBlendMode::kClear:
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
    GrProcessorAnalysisColor inputColor(GrColorPackRGBA(229, 0, 154, 240));
    GrProcessorAnalysisCoverage inputCoverage = GrProcessorAnalysisCoverage::kNone;

    for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
        SkBlendMode xfermode = static_cast<SkBlendMode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, inputColor, inputCoverage);
        switch (xfermode) {
            case SkBlendMode::kClear:
                TEST_ASSERT(xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(kISCModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(xpi.fCanCombineOverlappedStencilAndCover);
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
                TEST_ASSERT(!xpi.fCanCombineOverlappedStencilAndCover);
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
    const GrXPFactory* xpf = GrPorterDuffXPFactory::Get(SkBlendMode::kSrcOver);
    GrProcessorAnalysisColor color = GrColorPackRGBA(123, 45, 67, 221);
    GrProcessorAnalysisCoverage coverage = GrProcessorAnalysisCoverage::kLCD;
    SkASSERT(!(GrXPFactory::GetAnalysisProperties(xpf, color, coverage, caps) &
               GrXPFactory::AnalysisProperties::kRequiresDstTexture));
    sk_sp<GrXferProcessor> xp(GrXPFactory::MakeXferProcessor(xpf, color, coverage, false, caps));
    if (!xp) {
        ERRORF(reporter, "Failed to create an XP with LCD coverage.");
        return;
    }

    GrXferProcessor::BlendInfo blendInfo;
    xp->getBlendInfo(&blendInfo);
    TEST_ASSERT(blendInfo.fWriteColor);
}

DEF_GPUTEST(PorterDuffNoDualSourceBlending, reporter, /*factory*/) {
    GrContextOptions opts;
    opts.fSuppressDualSourceBlending = true;
    sk_gpu_test::GrContextFactory mockFactory(opts);
    GrContext* ctx = mockFactory.get(sk_gpu_test::GrContextFactory::kNullGL_ContextType);
    if (!ctx) {
        SkFAIL("Failed to create null context without ARB_blend_func_extended.");
        return;
    }

    const GrCaps& caps = *ctx->caps();
    if (caps.shaderCaps()->dualSourceBlendingSupport()) {
        SkFAIL("Null context failed to honor request for no ARB_blend_func_extended.");
        return;
    }

    GrBackendObject backendTex =
        ctx->getGpu()->createTestingOnlyBackendTexture(nullptr, 100, 100, kRGBA_8888_GrPixelConfig);
    GrBackendTextureDesc fakeDesc;
    fakeDesc.fConfig = kRGBA_8888_GrPixelConfig;
    fakeDesc.fWidth = fakeDesc.fHeight = 100;
    fakeDesc.fTextureHandle = backendTex;
    GrXferProcessor::DstTexture fakeDstTexture;
    fakeDstTexture.setTexture(
        ctx->resourceProvider()->wrapBackendTexture(fakeDesc, kBorrow_GrWrapOwnership));

    static const GrProcessorAnalysisColor colorInputs[] = {
            GrProcessorAnalysisColor::Opaque::kNo, GrProcessorAnalysisColor::Opaque::kYes,
            GrProcessorAnalysisColor(GrColorPackRGBA(0, 82, 17, 100)),
            GrProcessorAnalysisColor(GrColorPackRGBA(0, 82, 17, 255))};

    for (const auto& colorInput : colorInputs) {
        for (GrProcessorAnalysisCoverage coverageType :
             {GrProcessorAnalysisCoverage::kSingleChannel, GrProcessorAnalysisCoverage::kNone}) {
            for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
                SkBlendMode xfermode = static_cast<SkBlendMode>(m);
                const GrXPFactory* xpf = GrPorterDuffXPFactory::Get(xfermode);
                GrProcessorSet::Analysis analysis;
                analysis = GrProcessorSet::Analysis(colorInput, coverageType, xpf, caps);
                sk_sp<GrXferProcessor> xp(
                        GrXPFactory::MakeXferProcessor(xpf, colorInput, coverageType, false, caps));
                if (!xp) {
                    ERRORF(reporter, "Failed to create an XP without dual source blending.");
                    return;
                }
                TEST_ASSERT(!xp->hasSecondaryOutput());
            }
        }
    }
    ctx->getGpu()->deleteTestingOnlyBackendTexture(backendTex);
}

#endif
