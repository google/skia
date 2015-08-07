/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkXfermode.h"
#include "Test.h"

#if SK_SUPPORT_GPU

#include "GrContextFactory.h"
#include "GrContextOptions.h"
#include "GrGpu.h"
#include "GrResourceProvider.h"
#include "GrXferProcessor.h"
#include "batches/GrBatch.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "gl/GrGLCaps.h"

////////////////////////////////////////////////////////////////////////////////

static void test_color_unknown_with_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_color_unknown_no_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_color_opaque_with_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_color_opaque_no_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_lcd_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_no_dual_source_blending(skiatest::Reporter* reporter);

DEF_GPUTEST(GrPorterDuff, reporter, factory) {
    GrContext* ctx = factory->get(GrContextFactory::kNull_GLContextType);
    if (!ctx) {
        SkFAIL("Failed to create null context.");
        return;
    }

    const GrCaps& caps = *ctx->getGpu()->caps();
    if (!caps.shaderCaps()->dualSourceBlendingSupport()) {
        SkFAIL("Null context does not support dual source blending.");
        return;
    }

    test_color_unknown_with_coverage(reporter, caps);
    test_color_unknown_no_coverage(reporter, caps);
    test_color_opaque_with_coverage(reporter, caps);
    test_color_opaque_no_coverage(reporter, caps);
    test_lcd_coverage(reporter, caps);
    test_no_dual_source_blending(reporter);
}

////////////////////////////////////////////////////////////////////////////////

#define TEST_ASSERT(...) REPORTER_ASSERT(reporter, __VA_ARGS__)

enum {
    kNone_OutputType,
    kCoverage_OutputType,
    kModulate_OutputType,
    kISAModulate_OutputType,
    kISCModulate_OutputType
};

enum {
    kNone_OptFlags                    = GrXferProcessor::kNone_OptFlags,
    kSkipDraw_OptFlag                 = GrXferProcessor::kSkipDraw_OptFlag,
    kIgnoreColor_OptFlag              = GrXferProcessor::kIgnoreColor_OptFlag,
    kIgnoreCoverage_OptFlag           = GrXferProcessor::kIgnoreCoverage_OptFlag,
    kCanTweakAlphaForCoverage_OptFlag = GrXferProcessor::kCanTweakAlphaForCoverage_OptFlag
};

class GrPorterDuffTest {
public:
    struct XPInfo {
        XPInfo(skiatest::Reporter* reporter, SkXfermode::Mode xfermode, const GrCaps& caps,
               const GrProcOptInfo& colorPOI, const GrProcOptInfo& covPOI) {
            SkAutoTUnref<GrXPFactory> xpf(GrPorterDuffXPFactory::Create(xfermode));
            SkAutoTUnref<GrXferProcessor> xp(
                xpf->createXferProcessor(colorPOI, covPOI, false, NULL, caps));
            TEST_ASSERT(!xpf->willNeedDstTexture(caps, colorPOI, covPOI, false));
            xpf->getInvariantBlendedColor(colorPOI, &fBlendedColor);
            fOptFlags = xp->getOptimizations(colorPOI, covPOI, false, NULL, caps);
            GetXPOutputTypes(xp, &fPrimaryOutputType, &fSecondaryOutputType);
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

static void test_color_unknown_with_coverage(skiatest::Reporter* reporter, const GrCaps& caps) {
    GrProcOptInfo colorPOI, covPOI;
    colorPOI.calcWithInitialValues(NULL, 0, 0, kNone_GrColorComponentFlags, false);
    covPOI.calcWithInitialValues(NULL, 0, 0, kNone_GrColorComponentFlags, true);

    SkASSERT(!colorPOI.isOpaque());
    SkASSERT(!colorPOI.isSolidWhite());
    SkASSERT(!covPOI.isSolidWhite());
    SkASSERT(!covPOI.isFourChannelOutput());

    for (int m = 0; m <= SkXfermode::kLastCoeffMode; m++) {
        SkXfermode::Mode xfermode = static_cast<SkXfermode::Mode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, colorPOI, covPOI);

        switch (xfermode) {
            case SkXfermode::kClear_Mode:
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
            case SkXfermode::kSrc_Mode:
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
            case SkXfermode::kDst_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kSkipDraw_OptFlag |
                             kIgnoreColor_OptFlag |
                             kIgnoreCoverage_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrcOver_Mode:
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
            case SkXfermode::kDstOver_Mode:
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
            case SkXfermode::kSrcIn_Mode:
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
            case SkXfermode::kDstIn_Mode:
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
            case SkXfermode::kSrcOut_Mode:
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
            case SkXfermode::kDstOut_Mode:
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
            case SkXfermode::kSrcATop_Mode:
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
            case SkXfermode::kDstATop_Mode:
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
            case SkXfermode::kXor_Mode:
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
            case SkXfermode::kPlus_Mode:
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
            case SkXfermode::kModulate_Mode:
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
            case SkXfermode::kScreen_Mode:
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
    GrProcOptInfo colorPOI, covPOI;
    colorPOI.calcWithInitialValues(NULL, 0, GrColorPackRGBA(229, 0, 154, 0),
                                   kR_GrColorComponentFlag | kB_GrColorComponentFlag, false);
    covPOI.calcWithInitialValues(NULL, 0, GrColorPackA4(255), kRGBA_GrColorComponentFlags, true);

    SkASSERT(!colorPOI.isOpaque());
    SkASSERT(!colorPOI.isSolidWhite());
    SkASSERT(covPOI.isSolidWhite());
    SkASSERT(!covPOI.isFourChannelOutput());

    for (int m = 0; m <= SkXfermode::kLastCoeffMode; m++) {
        SkXfermode::Mode xfermode = static_cast<SkXfermode::Mode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, colorPOI, covPOI);

        switch (xfermode) {
            case SkXfermode::kClear_Mode:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(0 == xpi.fBlendedColor.fKnownColor);
                TEST_ASSERT(kRGBA_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreColor_OptFlag |
                             kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrc_Mode:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(229 == GrColorUnpackR(xpi.fBlendedColor.fKnownColor));
                TEST_ASSERT(154 == GrColorUnpackB(xpi.fBlendedColor.fKnownColor));
                TEST_ASSERT((kR_GrColorComponentFlag |
                             kB_GrColorComponentFlag) == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDst_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kSkipDraw_OptFlag |
                             kIgnoreColor_OptFlag |
                             kIgnoreCoverage_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrcOver_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstOver_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrcIn_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstIn_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrcOut_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstOut_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrcATop_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstATop_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kXor_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kPlus_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kModulate_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSC_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kScreen_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
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
    GrProcOptInfo colorPOI, covPOI;
    colorPOI.calcWithInitialValues(NULL, 0, GrColorPackA4(255), kA_GrColorComponentFlag, false);
    covPOI.calcWithInitialValues(NULL, 0, 0, kNone_GrColorComponentFlags, true);

    SkASSERT(colorPOI.isOpaque());
    SkASSERT(!colorPOI.isSolidWhite());
    SkASSERT(!covPOI.isSolidWhite());
    SkASSERT(!covPOI.isFourChannelOutput());

    for (int m = 0; m <= SkXfermode::kLastCoeffMode; m++) {
        SkXfermode::Mode xfermode = static_cast<SkXfermode::Mode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, colorPOI, covPOI);

        switch (xfermode) {
            case SkXfermode::kClear_Mode:
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
            case SkXfermode::kSrc_Mode:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(255 == GrColorUnpackA(xpi.fBlendedColor.fKnownColor));
                TEST_ASSERT(kA_GrColorComponentFlag == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDst_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kSkipDraw_OptFlag |
                             kIgnoreColor_OptFlag |
                             kIgnoreCoverage_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrcOver_Mode:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(255 == GrColorUnpackA(xpi.fBlendedColor.fKnownColor));
                TEST_ASSERT(kA_GrColorComponentFlag == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstOver_Mode:
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
            case SkXfermode::kSrcIn_Mode:
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
            case SkXfermode::kDstIn_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kSkipDraw_OptFlag |
                             kIgnoreColor_OptFlag |
                             kIgnoreCoverage_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrcOut_Mode:
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
            case SkXfermode::kDstOut_Mode:
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
            case SkXfermode::kSrcATop_Mode:
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
            case SkXfermode::kDstATop_Mode:
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
            case SkXfermode::kXor_Mode:
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
            case SkXfermode::kPlus_Mode:
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
            case SkXfermode::kModulate_Mode:
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
            case SkXfermode::kScreen_Mode:
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
    GrProcOptInfo colorPOI, covPOI;
    colorPOI.calcWithInitialValues(NULL, 0, GrColorPackRGBA(0, 82, 0, 255),
                                   kG_GrColorComponentFlag | kA_GrColorComponentFlag, false);
    covPOI.calcWithInitialValues(NULL, 0, GrColorPackA4(255), kRGBA_GrColorComponentFlags, true);

    SkASSERT(colorPOI.isOpaque());
    SkASSERT(!colorPOI.isSolidWhite());
    SkASSERT(covPOI.isSolidWhite());
    SkASSERT(!covPOI.isFourChannelOutput());

    for (int m = 0; m <= SkXfermode::kLastCoeffMode; m++) {
        SkXfermode::Mode xfermode = static_cast<SkXfermode::Mode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, colorPOI, covPOI);

        switch (xfermode) {
            case SkXfermode::kClear_Mode:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(0 == xpi.fBlendedColor.fKnownColor);
                TEST_ASSERT(kRGBA_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreColor_OptFlag |
                             kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrc_Mode:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(82 == GrColorUnpackG(xpi.fBlendedColor.fKnownColor));
                TEST_ASSERT(255 == GrColorUnpackA(xpi.fBlendedColor.fKnownColor));
                TEST_ASSERT((kG_GrColorComponentFlag |
                             kA_GrColorComponentFlag) == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDst_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kSkipDraw_OptFlag |
                             kIgnoreColor_OptFlag |
                             kIgnoreCoverage_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrcOver_Mode:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(82 == GrColorUnpackG(xpi.fBlendedColor.fKnownColor));
                TEST_ASSERT(255 == GrColorUnpackA(xpi.fBlendedColor.fKnownColor));
                TEST_ASSERT((kG_GrColorComponentFlag |
                             kA_GrColorComponentFlag) == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstOver_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrcIn_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstIn_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kSkipDraw_OptFlag |
                             kIgnoreColor_OptFlag |
                             kIgnoreCoverage_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrcOut_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstOut_Mode:
                TEST_ASSERT(!xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(0 == xpi.fBlendedColor.fKnownColor);
                TEST_ASSERT(kRGBA_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreColor_OptFlag |
                             kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrcATop_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstATop_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kXor_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kPlus_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kModulate_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSC_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kScreen_Mode:
                TEST_ASSERT(xpi.fBlendedColor.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fBlendedColor.fKnownColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag |
                             kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
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

static void test_lcd_coverage(skiatest::Reporter* reporter, const GrCaps& caps) {
    class : public GrBatch {
        void getInvariantOutputColor(GrInitInvariantOutput* out) const override {
            out->setKnownFourComponents(GrColorPackRGBA(123, 45, 67, 221));
        }

        void getInvariantOutputCoverage(GrInitInvariantOutput* out) const override {
            out->setUnknownFourComponents();
            out->setUsingLCDCoverage();
        }

        const char* name() const override { return "Test LCD Text Batch"; }
        void initBatchTracker(const GrPipelineInfo&) override {}
        bool onCombineIfPossible(GrBatch*) override  { return false; }
        void generateGeometry(GrBatchTarget*) override {}

    } testLCDCoverageBatch;

    GrProcOptInfo colorPOI, covPOI;
    colorPOI.calcColorWithBatch(&testLCDCoverageBatch, NULL, 0);
    covPOI.calcCoverageWithBatch(&testLCDCoverageBatch, NULL, 0);

    SkASSERT(kRGBA_GrColorComponentFlags == colorPOI.validFlags());
    SkASSERT(covPOI.isFourChannelOutput());

    SkAutoTUnref<GrXPFactory> xpf(GrPorterDuffXPFactory::Create(SkXfermode::kSrcOver_Mode));
    TEST_ASSERT(!xpf->willNeedDstTexture(caps, colorPOI, covPOI, false));

    SkAutoTUnref<GrXferProcessor> xp(
        xpf->createXferProcessor(colorPOI, covPOI, false, NULL, caps));
    if (!xp) {
        ERRORF(reporter, "Failed to create an XP with LCD coverage.");
        return;
    }

    GrXPFactory::InvariantBlendedColor blendedColor;
    xpf->getInvariantBlendedColor(colorPOI, &blendedColor);
    TEST_ASSERT(blendedColor.fWillBlendWithDst);
    TEST_ASSERT(kNone_GrColorComponentFlags == blendedColor.fKnownColorFlags);

    GrColor overrideColor;
    xp->getOptimizations(colorPOI, covPOI, false, &overrideColor, caps);

    GrXferProcessor::BlendInfo blendInfo;
    xp->getBlendInfo(&blendInfo);
    TEST_ASSERT(blendInfo.fWriteColor);
}

static void test_no_dual_source_blending(skiatest::Reporter* reporter) {
    GrContextOptions opts;
    opts.fSuppressDualSourceBlending = true;
    GrContextFactory factory(opts);
    factory.get(GrContextFactory::kNull_GLContextType);
    GrContext* ctx = factory.get(GrContextFactory::kNull_GLContextType);
    if (!ctx) {
        SkFAIL("Failed to create null context without ARB_blend_func_extended.");
        return;
    }

    const GrCaps& caps = *ctx->caps();
    if (caps.shaderCaps()->dualSourceBlendingSupport()) {
        SkFAIL("Null context failed to honor request for no ARB_blend_func_extended.");
        return;
    }

    GrBackendTextureDesc fakeDesc;
    fakeDesc.fConfig = kRGBA_8888_GrPixelConfig;
    fakeDesc.fWidth = fakeDesc.fHeight = 100;
    fakeDesc.fTextureHandle = 1;
    SkAutoTUnref<GrTexture> fakeTexture(ctx->textureProvider()->wrapBackendTexture(fakeDesc,
        kBorrow_GrWrapOwnership));
    GrXferProcessor::DstTexture fakeDstTexture;
    fakeDstTexture.setTexture(fakeTexture);

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
        GrProcOptInfo colorPOI;
        colorPOI.calcWithInitialValues(NULL, 0, testColors[c], testColorFlags[c], false);
        for (int f = 0; f <= 1; f++) {
            GrProcOptInfo covPOI;
            if (!f) {
                covPOI.calcWithInitialValues(NULL, 0, 0, kNone_GrColorComponentFlags, true);
            } else {
                covPOI.calcWithInitialValues(NULL, 0, GrColorPackA4(255),
                                             kRGBA_GrColorComponentFlags, true);
            }
            for (int m = 0; m <= SkXfermode::kLastCoeffMode; m++) {
                SkXfermode::Mode xfermode = static_cast<SkXfermode::Mode>(m);
                SkAutoTUnref<GrXPFactory> xpf(GrPorterDuffXPFactory::Create(xfermode));
                GrXferProcessor::DstTexture* dstTexture =
                    xpf->willNeedDstTexture(caps, colorPOI, covPOI, false) ? &fakeDstTexture : 0;
                SkAutoTUnref<GrXferProcessor> xp(
                    xpf->createXferProcessor(colorPOI, covPOI, false, dstTexture, caps));
                if (!xp) {
                    ERRORF(reporter, "Failed to create an XP without dual source blending.");
                    return;
                }
                TEST_ASSERT(!xp->hasSecondaryOutput());
                xp->getOptimizations(colorPOI, covPOI, false, 0, caps);
                TEST_ASSERT(!xp->hasSecondaryOutput());
            }
        }
    }
}

#endif

