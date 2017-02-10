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
static void test_color_unknown_no_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
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
    test_color_unknown_no_coverage(reporter, caps);
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

enum {
    kNone_OptFlags                    = GrXferProcessor::kNone_OptFlags,
    kSkipDraw_OptFlag                 = GrXferProcessor::kSkipDraw_OptFlag,
    kIgnoreColor_OptFlag              = GrXferProcessor::kIgnoreColor_OptFlag,
    kCanTweakAlphaForCoverage_OptFlag = GrXferProcessor::kCanTweakAlphaForCoverage_OptFlag
};

class GrPorterDuffTest {
public:
    struct XPInfo {
        XPInfo(skiatest::Reporter* reporter, SkBlendMode xfermode, const GrCaps& caps,
               const GrPipelineAnalysis& analysis) {
            const GrXPFactory* xpf = GrPorterDuffXPFactory::Get(xfermode);
            sk_sp<GrXferProcessor> xp(xpf->createXferProcessor(analysis, false, nullptr, caps));
            TEST_ASSERT(!xpf->willNeedDstTexture(caps, analysis));
            xpf->getInvariantBlendedColor(analysis.fColorPOI, &fBlendedColor);
            GrColor ignoredOverrideColor;
            fOptFlags = xp->getOptimizations(analysis, false, &ignoredOverrideColor, caps);
            GetXPOutputTypes(xp.get(), &fPrimaryOutputType, &fSecondaryOutputType);
            xp->getBlendInfo(&fBlendInfo);
            TEST_ASSERT(!xp->willReadDstColor());
            TEST_ASSERT(xp->hasSecondaryOutput() == GrBlendCoeffRefsSrc2(fBlendInfo.fDstBlend));
        }

        GrXPFactory::InvariantBlendedColor fBlendedColor;
        int fOptFlags;
        int fPrimaryOutputType;
        int fSecondaryOutputType;
        GrXferProcessor::BlendInfo fBlendInfo;
    };

    static void GetXPOutputTypes(const GrXferProcessor* xp, int* outPrimary, int* outSecondary) {
        GrPorterDuffXPFactory::TestGetXPOutputTypes(xp, outPrimary, outSecondary);
    }
};

static void test_lcd_coverage(skiatest::Reporter* reporter, const GrCaps& caps) {
    GrPipelineAnalysis analysis;
    analysis.fColorPOI.reset(0, kNone_GrColorComponentFlags);
    // Setting the last argument to true will force covPOI to LCD coverage.
    analysis.fCoveragePOI.resetToLCDCoverage(0, kNone_GrColorComponentFlags);

    SkASSERT(!analysis.fColorPOI.isOpaque());
    SkASSERT(!analysis.fColorPOI.isSolidWhite());
    SkASSERT(!analysis.fCoveragePOI.isSolidWhite());
    SkASSERT(analysis.fCoveragePOI.isLCDCoverage());

    for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
        SkBlendMode xfermode = static_cast<SkBlendMode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, analysis);

        switch (xfermode) {
            case SkBlendMode::kClear:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(0 == xpi.fBlendedColor.fKnownColor);
                TEST_ASSERT(kRGBA_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreColor_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kCoverage_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kSkipDraw_OptFlag |
                             kIgnoreColor_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOver:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kSAModulate_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kISAModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kSAModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISC_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kSAModulate_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kISAModulate_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kSAModulate_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kISCModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
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
    GrPipelineAnalysis analysis;
    analysis.fColorPOI.reset(0, kNone_GrColorComponentFlags);
    analysis.fCoveragePOI.reset(0, kNone_GrColorComponentFlags);

    SkASSERT(!analysis.fColorPOI.isOpaque());
    SkASSERT(!analysis.fColorPOI.isSolidWhite());
    SkASSERT(!analysis.fCoveragePOI.isSolidWhite());
    SkASSERT(!analysis.fCoveragePOI.isLCDCoverage());

    for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
        SkBlendMode xfermode = static_cast<SkBlendMode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, analysis);

        switch (xfermode) {
            case SkBlendMode::kClear:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(0 == xpi.fBlendedColor.fKnownColor);
                TEST_ASSERT(kRGBA_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreColor_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kCoverage_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2A_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kSkipDraw_OptFlag |
                             kIgnoreColor_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOver:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2A_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kISAModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2A_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kISAModulate_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kISCModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
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

static void test_color_unknown_no_coverage(skiatest::Reporter* reporter, const GrCaps& caps) {
    GrPipelineAnalysis analysis;
    analysis.fColorPOI.reset(GrColorPackRGBA(229, 0, 154, 240), kRGBA_GrColorComponentFlags);
    analysis.fCoveragePOI.reset(GrColorPackA4(255), kRGBA_GrColorComponentFlags);

    SkASSERT(!analysis.fColorPOI.isOpaque());
    SkASSERT(!analysis.fColorPOI.isSolidWhite());
    SkASSERT(analysis.fCoveragePOI.isSolidWhite());
    SkASSERT(!analysis.fCoveragePOI.isLCDCoverage());

    for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
        SkBlendMode xfermode = static_cast<SkBlendMode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, analysis);

        switch (xfermode) {
            case SkBlendMode::kClear:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(0 == xpi.fBlendedColor.fKnownColor);
                TEST_ASSERT(kRGBA_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kIgnoreColor_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(229 == GrColorUnpackR(xpi.fBlendedColor.fKnownColor));
                TEST_ASSERT(0 == GrColorUnpackG(xpi.fBlendedColor.fKnownColor));
                TEST_ASSERT(154 == GrColorUnpackB(xpi.fBlendedColor.fKnownColor));
                TEST_ASSERT(240 == GrColorUnpackA(xpi.fBlendedColor.fKnownColor));
                TEST_ASSERT(kRGBA_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kSkipDraw_OptFlag |
                             kIgnoreColor_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOver:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kCanTweakAlphaForCoverage_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kCanTweakAlphaForCoverage_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kCanTweakAlphaForCoverage_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kCanTweakAlphaForCoverage_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kCanTweakAlphaForCoverage_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSC_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kCanTweakAlphaForCoverage_OptFlag == xpi.fOptFlags);
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
    GrPipelineAnalysis analysis;
    analysis.fColorPOI.reset(GrColorPackA4(255), kA_GrColorComponentFlag);
    analysis.fCoveragePOI.reset(0, kNone_GrColorComponentFlags);

    SkASSERT(analysis.fColorPOI.isOpaque());
    SkASSERT(!analysis.fColorPOI.isSolidWhite());
    SkASSERT(!analysis.fCoveragePOI.isSolidWhite());
    SkASSERT(!analysis.fCoveragePOI.isLCDCoverage());

    for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
        SkBlendMode xfermode = static_cast<SkBlendMode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, analysis);

        switch (xfermode) {
            case SkBlendMode::kClear:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(0 == xpi.fBlendedColor.fKnownColor);
                TEST_ASSERT(kRGBA_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreColor_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kCoverage_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                // We don't really track per-component blended output anymore.
                TEST_ASSERT(0 == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kSkipDraw_OptFlag |
                             kIgnoreColor_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOver:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                // We don't really track per-component blended output anymore.
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kSkipDraw_OptFlag |
                             kIgnoreColor_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(0 == xpi.fBlendedColor.fKnownColor);
                TEST_ASSERT(kRGBA_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreColor_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kCoverage_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kISCModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
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
    GrPipelineAnalysis analysis;
    analysis.fColorPOI.reset(GrColorPackRGBA(0, 82, 0, 255),
                             kG_GrColorComponentFlag | kA_GrColorComponentFlag);
    analysis.fCoveragePOI.reset(GrColorPackA4(255), kRGBA_GrColorComponentFlags);

    SkASSERT(analysis.fColorPOI.isOpaque());
    SkASSERT(!analysis.fColorPOI.isSolidWhite());
    SkASSERT(analysis.fCoveragePOI.isSolidWhite());
    SkASSERT(!analysis.fCoveragePOI.isLCDCoverage());

    for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
        SkBlendMode xfermode = static_cast<SkBlendMode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, analysis);

        switch (xfermode) {
            case SkBlendMode::kClear:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(0 == xpi.fBlendedColor.fKnownColor);
                TEST_ASSERT(kRGBA_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kIgnoreColor_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                // We don't really track per-component blended output anymore.
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kSkipDraw_OptFlag |
                             kIgnoreColor_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOver:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                // We don't really track per-component blended output anymore.
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kCanTweakAlphaForCoverage_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kSkipDraw_OptFlag |
                             kIgnoreColor_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(0 == xpi.fBlendedColor.fKnownColor);
                TEST_ASSERT(kRGBA_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kIgnoreColor_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kCanTweakAlphaForCoverage_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kCanTweakAlphaForCoverage_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSC_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT(kCanTweakAlphaForCoverage_OptFlag == xpi.fOptFlags);
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
    class TestLCDCoverageOp : public GrMeshDrawOp {
    public:
        DEFINE_OP_CLASS_ID

        TestLCDCoverageOp() : INHERITED(ClassID()) {}

        const char* name() const override { return "Test LCD Text Op"; }

    private:
        void getPipelineAnalysisInput(GrPipelineAnalysisDrawOpInput* input) const override {
            input->pipelineColorInput()->setKnownFourComponents(GrColorPackRGBA(123, 45, 67, 221));
            input->pipelineCoverageInput()->setUnknownFourComponents();
            input->pipelineCoverageInput()->setUsingLCDCoverage();
        }

        void applyPipelineOptimizations(const GrPipelineOptimizations&) override {}
        bool onCombineIfPossible(GrOp*, const GrCaps&) override  { return false; }
        void onPrepareDraws(Target*) const override {}

        typedef GrMeshDrawOp INHERITED;
    } testLCDCoverageOp;

    GrPipelineAnalysis analysis;
    testLCDCoverageOp.initPipelineAnalysis(&analysis);
    GrProcOptInfo colorPOI = analysis.fColorPOI;
    GrProcOptInfo covPOI = analysis.fCoveragePOI;
    // Prevent unused var warnings in release.
    (void)colorPOI;
    (void)covPOI;

    SkASSERT(colorPOI.hasKnownOutputColor());
    SkASSERT(covPOI.isLCDCoverage());

    const GrXPFactory* xpf = GrPorterDuffXPFactory::Get(SkBlendMode::kSrcOver);
    TEST_ASSERT(!xpf->willNeedDstTexture(caps, analysis));

    sk_sp<GrXferProcessor> xp(xpf->createXferProcessor(analysis, false, nullptr, caps));
    if (!xp) {
        ERRORF(reporter, "Failed to create an XP with LCD coverage.");
        return;
    }

    GrXPFactory::InvariantBlendedColor blendedColor;
    xpf->getInvariantBlendedColor(colorPOI, &blendedColor);
    TEST_ASSERT(blendedColor.fWillBlendWithDst);
    TEST_ASSERT(kNone_GrColorComponentFlags == blendedColor.fKnownColorFlags);

    GrColor overrideColor;
    xp->getOptimizations(analysis, false, &overrideColor, caps);

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
        ctx->textureProvider()->wrapBackendTexture(fakeDesc, kBorrow_GrWrapOwnership));

    static const GrColor testColors[] = {
        0,
        GrColorPackRGBA(0, 82, 0, 255),
        GrColorPackA4(255)
    };
    static const GrColorComponentFlags testColorFlags[] = {
        kNone_GrColorComponentFlags,
        kG_GrColorComponentFlag | kA_GrColorComponentFlag,
        kRGBA_GrColorComponentFlags
    };
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(testColors) == SK_ARRAY_COUNT(testColorFlags));

    for (size_t c = 0; c < SK_ARRAY_COUNT(testColors); c++) {
        GrPipelineAnalysis analysis;
        analysis.fColorPOI.reset(testColors[c], testColorFlags[c]);
        for (int f = 0; f <= 1; f++) {
            if (!f) {
                analysis.fCoveragePOI.reset(0, kNone_GrColorComponentFlags);
            } else {
                analysis.fCoveragePOI.reset(GrColorPackA4(255), kRGBA_GrColorComponentFlags);
            }
            for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
                SkBlendMode xfermode = static_cast<SkBlendMode>(m);
                const GrXPFactory* xpf = GrPorterDuffXPFactory::Get(xfermode);
                GrXferProcessor::DstTexture* dstTexture =
                        xpf->willNeedDstTexture(caps, analysis) ? &fakeDstTexture : 0;
                sk_sp<GrXferProcessor> xp(
                        xpf->createXferProcessor(analysis, false, dstTexture, caps));
                if (!xp) {
                    ERRORF(reporter, "Failed to create an XP without dual source blending.");
                    return;
                }
                TEST_ASSERT(!xp->hasSecondaryOutput());
                GrColor ignoredOverrideColor;
                xp->getOptimizations(analysis, false, &ignoredOverrideColor, caps);
                TEST_ASSERT(!xp->hasSecondaryOutput());
            }
        }
    }
    ctx->getGpu()->deleteTestingOnlyBackendTexture(backendTex);
}

#endif
