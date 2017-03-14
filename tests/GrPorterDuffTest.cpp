/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if SK_SUPPORT_GPU

#include "GrAppliedClip.h"
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
    kIgnoreColor_OptFlag              = GrXferProcessor::kIgnoreColor_OptFlag,
    kCanTweakAlphaForCoverage_OptFlag = GrXferProcessor::kCanTweakAlphaForCoverage_OptFlag
};

class GrPorterDuffTest {
public:
    struct XPInfo {
        XPInfo(skiatest::Reporter* reporter, SkBlendMode xfermode, const GrCaps& caps,
               const GrProcessorSet::FragmentProcessorAnalysis& analysis) {
            const GrXPFactory* xpf = GrPorterDuffXPFactory::Get(xfermode);
            fReadsDst = GrXPFactory::WillReadDst(xpf, analysis);
            sk_sp<GrXferProcessor> xp(xpf->createXferProcessor(analysis, false, nullptr, caps));
            TEST_ASSERT(!GrXPFactory::WillNeedDstTexture(xpf, caps, analysis));
            fOptFlags = xp->getOptimizations(analysis);
            GetXPOutputTypes(xp.get(), &fPrimaryOutputType, &fSecondaryOutputType);
            xp->getBlendInfo(&fBlendInfo);
            TEST_ASSERT(!xp->willReadDstColor());
            TEST_ASSERT(xp->hasSecondaryOutput() == GrBlendCoeffRefsSrc2(fBlendInfo.fDstBlend));
        }

        bool fReadsDst;
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
    GrPipelineInput lcdInput;
    lcdInput.setToLCDCoverage();
    GrProcessorSet::FragmentProcessorAnalysis analysis(GrPipelineInput(), lcdInput, caps);

    SkASSERT(!analysis.isOutputColorOpaque());
    SkASSERT(!analysis.hasKnownOutputColor());
    SkASSERT(analysis.hasLCDCoverage());

    for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
        SkBlendMode xfermode = static_cast<SkBlendMode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, analysis);
        switch (xfermode) {
            case SkBlendMode::kClear:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kIgnoreColor_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kCoverage_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kIgnoreColor_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOver:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kSAModulate_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kISAModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kSAModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISC_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kSAModulate_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kISAModulate_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kSAModulate_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kISCModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(xpi.fReadsDst);
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
    GrProcessorSet::FragmentProcessorAnalysis analysis(GrPipelineInput(), GrPipelineInput(), caps);

    SkASSERT(!analysis.isOutputColorOpaque());
    SkASSERT(!analysis.hasKnownOutputColor());
    SkASSERT(!analysis.hasLCDCoverage());
    SkASSERT(analysis.hasCoverage());

    for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
        SkBlendMode xfermode = static_cast<SkBlendMode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, analysis);
        switch (xfermode) {
            case SkBlendMode::kClear:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kIgnoreColor_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kCoverage_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2A_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kIgnoreColor_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOver:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2A_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kISAModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2A_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kISAModulate_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kISCModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(xpi.fReadsDst);
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
    GrProcessorSet::FragmentProcessorAnalysis analysis(GrColorPackRGBA(229, 0, 154, 240),
                                                       GrColorPackA4(255), caps);

    SkASSERT(!analysis.isOutputColorOpaque());
    SkASSERT(analysis.hasKnownOutputColor());
    SkASSERT(!analysis.hasCoverage());

    for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
        SkBlendMode xfermode = static_cast<SkBlendMode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, analysis);
        switch (xfermode) {
            case SkBlendMode::kClear:
                TEST_ASSERT(!xpi.fReadsDst);
                TEST_ASSERT(kIgnoreColor_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(!xpi.fReadsDst);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kIgnoreColor_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOver:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT(kCanTweakAlphaForCoverage_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT(kCanTweakAlphaForCoverage_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT(kCanTweakAlphaForCoverage_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT(kCanTweakAlphaForCoverage_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT(kCanTweakAlphaForCoverage_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSC_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(xpi.fReadsDst);
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
    GrProcessorSet::FragmentProcessorAnalysis analysis(GrPipelineInput::Opaque::kYes,
                                                       GrPipelineInput(), caps);

    SkASSERT(analysis.isOutputColorOpaque());
    SkASSERT(!analysis.hasKnownOutputColor());
    SkASSERT(analysis.hasCoverage());
    SkASSERT(!analysis.hasLCDCoverage());

    for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
        SkBlendMode xfermode = static_cast<SkBlendMode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, analysis);
        switch (xfermode) {
            case SkBlendMode::kClear:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kIgnoreColor_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kCoverage_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kIgnoreColor_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOver:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kIgnoreColor_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kIgnoreColor_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kCoverage_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kNone_OptFlags) == xpi.fOptFlags);
                TEST_ASSERT(kISCModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(xpi.fReadsDst);
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
    GrProcessorSet::FragmentProcessorAnalysis analysis(GrPipelineInput::Opaque::kYes,
                                                       GrColorPackA4(255), caps);

    SkASSERT(analysis.isOutputColorOpaque());
    SkASSERT(!analysis.hasKnownOutputColor());
    SkASSERT(!analysis.hasCoverage());

    for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
        SkBlendMode xfermode = static_cast<SkBlendMode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, analysis);

        switch (xfermode) {
            case SkBlendMode::kClear:
                TEST_ASSERT(!xpi.fReadsDst);
                TEST_ASSERT(kIgnoreColor_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(!xpi.fReadsDst);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kIgnoreColor_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOver:
                TEST_ASSERT(!xpi.fReadsDst);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT(kCanTweakAlphaForCoverage_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT((kIgnoreColor_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(!xpi.fReadsDst);
                TEST_ASSERT(kIgnoreColor_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT(kCanTweakAlphaForCoverage_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT(kCanTweakAlphaForCoverage_OptFlag == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(xpi.fReadsDst);
                TEST_ASSERT(kNone_OptFlags == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSC_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(xpi.fReadsDst);
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
        void getFragmentProcessorAnalysisInputs(
                FragmentProcessorAnalysisInputs* input) const override {
            input->colorInput()->setToConstant(GrColorPackRGBA(123, 45, 67, 221));
            input->coverageInput()->setToLCDCoverage();
        }

        void applyPipelineOptimizations(const GrPipelineOptimizations&) override {}
        bool onCombineIfPossible(GrOp*, const GrCaps&) override  { return false; }
        void onPrepareDraws(Target*) const override {}

        typedef GrMeshDrawOp INHERITED;
    } testLCDCoverageOp;

    GrProcessorSet::FragmentProcessorAnalysis analysis;
    GrAppliedClip clip;
    testLCDCoverageOp.analyzeProcessors(&analysis, GrProcessorSet(GrPaint()), &clip, caps);

    SkASSERT(analysis.hasKnownOutputColor());
    SkASSERT(analysis.hasLCDCoverage());

    const GrXPFactory* xpf = GrPorterDuffXPFactory::Get(SkBlendMode::kSrcOver);
    TEST_ASSERT(!GrXPFactory::WillNeedDstTexture(xpf, caps, analysis));

    sk_sp<GrXferProcessor> xp(xpf->createXferProcessor(analysis, false, nullptr, caps));
    if (!xp) {
        ERRORF(reporter, "Failed to create an XP with LCD coverage.");
        return;
    }

    TEST_ASSERT(GrXPFactory::WillReadDst(xpf, analysis));

    xp->getOptimizations(analysis);

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

    static const GrPipelineInput colorInputs[] = {GrPipelineInput(),
                                                  GrPipelineInput(GrPipelineInput::Opaque::kYes),
                                                  GrPipelineInput(GrColorPackRGBA(0, 82, 17, 100)),
                                                  GrPipelineInput(GrColorPackRGBA(0, 82, 17, 255))};

    for (const auto& colorInput : colorInputs) {
        GrProcessorSet::FragmentProcessorAnalysis analysis;
        for (bool fractionalCoverage : {true, false}) {
            if (fractionalCoverage) {
                analysis = GrProcessorSet::FragmentProcessorAnalysis(colorInput, GrPipelineInput(),
                                                                     caps);
            } else {
                analysis = GrProcessorSet::FragmentProcessorAnalysis(colorInput, GrColorPackA4(255),
                                                                     caps);
            }
            for (int m = 0; m <= (int)SkBlendMode::kLastCoeffMode; m++) {
                SkBlendMode xfermode = static_cast<SkBlendMode>(m);
                const GrXPFactory* xpf = GrPorterDuffXPFactory::Get(xfermode);
                GrXferProcessor::DstTexture* dstTexture =
                        GrXPFactory::WillNeedDstTexture(xpf, caps, analysis) ? &fakeDstTexture : 0;
                sk_sp<GrXferProcessor> xp(
                        xpf->createXferProcessor(analysis, false, dstTexture, caps));
                if (!xp) {
                    ERRORF(reporter, "Failed to create an XP without dual source blending.");
                    return;
                }
                TEST_ASSERT(!xp->hasSecondaryOutput());
                xp->getOptimizations(analysis);
                TEST_ASSERT(!xp->hasSecondaryOutput());
            }
        }
    }
    ctx->getGpu()->deleteTestingOnlyBackendTexture(backendTex);
}

#endif
