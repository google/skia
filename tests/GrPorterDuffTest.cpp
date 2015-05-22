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
#include "GrGpu.h"
#include "GrXferProcessor.h"
#include "effects/GrPorterDuffXferProcessor.h"

////////////////////////////////////////////////////////////////////////////////

static void test_alpha_unknown_with_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_alpha_unknown_no_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_alpha_opaque_with_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_alpha_opaque_no_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_color_white_with_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_color_white_no_coverage(skiatest::Reporter* reporter, const GrCaps& caps);

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

    test_alpha_unknown_with_coverage(reporter, caps);
    test_alpha_unknown_no_coverage(reporter, caps);
    test_alpha_opaque_with_coverage(reporter, caps);
    test_alpha_opaque_no_coverage(reporter, caps);
    test_color_white_with_coverage(reporter, caps);
    test_color_white_with_coverage(reporter, caps);
    test_color_white_no_coverage(reporter, caps);
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
    kNone_Opt                         = GrXferProcessor::kNone_Opt,
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
            SkAutoTUnref<GrXferProcessor> xp(xpf->createXferProcessor(colorPOI, covPOI, 0, caps));
            TEST_ASSERT(!xpf->willNeedDstCopy(caps, colorPOI, covPOI));
            xpf->getInvariantOutput(colorPOI, covPOI, &fInvariantOutput);
            fOptFlags = xp->getOptimizations(colorPOI, covPOI, false, 0, caps);
            GetXPOutputTypes(xp, &fPrimaryOutputType, &fSecondaryOutputType);
            xp->getBlendInfo(&fBlendInfo);
        }

        GrXPFactory::InvariantOutput fInvariantOutput;
        int fOptFlags;
        int fPrimaryOutputType;
        int fSecondaryOutputType;
        GrXferProcessor::BlendInfo fBlendInfo;
    };

    static void GetXPOutputTypes(const GrXferProcessor* xp, int* outPrimary, int* outSecondary) {
        GrPorterDuffXPFactory::TestGetXPOutputTypes(xp, outPrimary, outSecondary);
    }
};

static void test_alpha_unknown_with_coverage(skiatest::Reporter* reporter, const GrCaps& caps) {
    GrProcOptInfo colorPOI, covPOI;
    colorPOI.calcWithInitialValues(NULL, 0, 0, kNone_GrColorComponentFlags, false);
    covPOI.calcWithInitialValues(NULL, 0, 0, kNone_GrColorComponentFlags, true);

    SkASSERT(!colorPOI.isOpaque());
    SkASSERT(!colorPOI.isSolidWhite());
    SkASSERT(!covPOI.isSolidWhite());

    for (int m = 0; m <= SkXfermode::kLastCoeffMode; m++) {
        SkXfermode::Mode xfermode = static_cast<SkXfermode::Mode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, colorPOI, covPOI);

        switch (xfermode) {
            case SkXfermode::kClear_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreColor_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kCoverage_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrc_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kNone_Opt) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2A_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDst_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstOver_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrcIn_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kNone_Opt) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2A_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstIn_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kNone_Opt) == xpi.fOptFlags);
                TEST_ASSERT(kISAModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrcOut_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kNone_Opt) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2A_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstOut_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrcATop_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstATop_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kNone_Opt) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kISAModulate_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kIS2C_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kXor_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kPlus_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kModulate_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kNone_Opt) == xpi.fOptFlags);
                TEST_ASSERT(kISCModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kScreen_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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

static void test_alpha_unknown_no_coverage(skiatest::Reporter* reporter, const GrCaps& caps) {
    GrProcOptInfo colorPOI, covPOI;
    colorPOI.calcWithInitialValues(NULL, 0, GrColorPackRGBA(229, 0, 154, 0),
                                   kR_GrColorComponentFlag | kB_GrColorComponentFlag, false);
    covPOI.calcWithInitialValues(NULL, 0, GrColorPackA4(255), kRGBA_GrColorComponentFlags, true);

    SkASSERT(!colorPOI.isOpaque());
    SkASSERT(!colorPOI.isSolidWhite());
    SkASSERT(covPOI.isSolidWhite());

    for (int m = 0; m <= SkXfermode::kLastCoeffMode; m++) {
        SkXfermode::Mode xfermode = static_cast<SkXfermode::Mode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, colorPOI, covPOI);

        switch (xfermode) {
            case SkXfermode::kClear_Mode:
                TEST_ASSERT(!xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(0 == xpi.fInvariantOutput.fBlendedColor);
                TEST_ASSERT(kRGBA_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(!xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(229 == GrColorUnpackR(xpi.fInvariantOutput.fBlendedColor));
                TEST_ASSERT(154 == GrColorUnpackB(xpi.fInvariantOutput.fBlendedColor));
                TEST_ASSERT((kR_GrColorComponentFlag |
                             kB_GrColorComponentFlag) == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDst_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstIn_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrcOut_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstOut_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kXor_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSC_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kScreen_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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

static void test_alpha_opaque_with_coverage(skiatest::Reporter* reporter, const GrCaps& caps) {
    GrProcOptInfo colorPOI, covPOI;
    colorPOI.calcWithInitialValues(NULL, 0, GrColorPackA4(255), kA_GrColorComponentFlag, false);
    covPOI.calcWithInitialValues(NULL, 0, 0, kNone_GrColorComponentFlags, true);

    SkASSERT(colorPOI.isOpaque());
    SkASSERT(!colorPOI.isSolidWhite());
    SkASSERT(!covPOI.isSolidWhite());

    for (int m = 0; m <= SkXfermode::kLastCoeffMode; m++) {
        SkXfermode::Mode xfermode = static_cast<SkXfermode::Mode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, colorPOI, covPOI);

        switch (xfermode) {
            case SkXfermode::kClear_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreColor_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kCoverage_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrc_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDst_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstOver_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrcIn_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstIn_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstOut_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreColor_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kCoverage_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrcATop_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstATop_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kXor_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kPlus_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kModulate_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kNone_Opt) == xpi.fOptFlags);
                TEST_ASSERT(kISCModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kScreen_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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

static void test_alpha_opaque_no_coverage(skiatest::Reporter* reporter, const GrCaps& caps) {
    GrProcOptInfo colorPOI, covPOI;
    colorPOI.calcWithInitialValues(NULL, 0, GrColorPackRGBA(0, 82, 0, 255),
                                   kG_GrColorComponentFlag | kA_GrColorComponentFlag, false);
    covPOI.calcWithInitialValues(NULL, 0, GrColorPackA4(255), kRGBA_GrColorComponentFlags, true);

    SkASSERT(colorPOI.isOpaque());
    SkASSERT(!colorPOI.isSolidWhite());
    SkASSERT(covPOI.isSolidWhite());

    for (int m = 0; m <= SkXfermode::kLastCoeffMode; m++) {
        SkXfermode::Mode xfermode = static_cast<SkXfermode::Mode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, colorPOI, covPOI);

        switch (xfermode) {
            case SkXfermode::kClear_Mode:
                TEST_ASSERT(!xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(0 == xpi.fInvariantOutput.fBlendedColor);
                TEST_ASSERT(kRGBA_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(!xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(82 == GrColorUnpackG(xpi.fInvariantOutput.fBlendedColor));
                TEST_ASSERT(255 == GrColorUnpackA(xpi.fInvariantOutput.fBlendedColor));
                TEST_ASSERT((kG_GrColorComponentFlag |
                             kA_GrColorComponentFlag) == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDst_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(!xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(82 == GrColorUnpackG(xpi.fInvariantOutput.fBlendedColor));
                TEST_ASSERT(255 == GrColorUnpackA(xpi.fInvariantOutput.fBlendedColor));
                TEST_ASSERT((kG_GrColorComponentFlag |
                             kA_GrColorComponentFlag) == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstOver_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstIn_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstOut_Mode:
                TEST_ASSERT(!xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(0 == xpi.fInvariantOutput.fBlendedColor);
                TEST_ASSERT(kRGBA_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstATop_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kPlus_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSC_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kScreen_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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

static void test_color_white_with_coverage(skiatest::Reporter* reporter, const GrCaps& caps) {
    GrProcOptInfo colorPOI, covPOI;
    colorPOI.calcWithInitialValues(NULL, 0, GrColorPackA4(255), kRGBA_GrColorComponentFlags, false);
    covPOI.calcWithInitialValues(NULL, 0, 0, kNone_GrColorComponentFlags, true);

    SkASSERT(colorPOI.isOpaque());
    SkASSERT(colorPOI.isSolidWhite());
    SkASSERT(!covPOI.isSolidWhite());

    for (int m = 0; m <= SkXfermode::kLastCoeffMode; m++) {
        SkXfermode::Mode xfermode = static_cast<SkXfermode::Mode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, colorPOI, covPOI);

        switch (xfermode) {
            case SkXfermode::kClear_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreColor_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kCoverage_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrc_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDst_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstOver_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrcIn_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstIn_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstOut_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreColor_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kCoverage_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kSrcATop_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstATop_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kXor_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kISA_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kPlus_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kCanTweakAlphaForCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kModulate_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kNone_Opt) == xpi.fOptFlags);
                TEST_ASSERT(kISCModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kReverseSubtract_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDC_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kScreen_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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

static void test_color_white_no_coverage(skiatest::Reporter* reporter, const GrCaps& caps) {
    GrProcOptInfo colorPOI, covPOI;
    colorPOI.calcWithInitialValues(NULL, 0, GrColorPackA4(255), kRGBA_GrColorComponentFlags, false);
    covPOI.calcWithInitialValues(NULL, 0, GrColorPackA4(255), kRGBA_GrColorComponentFlags, true);

    SkASSERT(colorPOI.isOpaque());
    SkASSERT(colorPOI.isSolidWhite());
    SkASSERT(covPOI.isSolidWhite());

    for (int m = 0; m <= SkXfermode::kLastCoeffMode; m++) {
        SkXfermode::Mode xfermode = static_cast<SkXfermode::Mode>(m);
        const GrPorterDuffTest::XPInfo xpi(reporter, xfermode, caps, colorPOI, covPOI);

        switch (xfermode) {
            case SkXfermode::kClear_Mode:
                TEST_ASSERT(!xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(0 == xpi.fInvariantOutput.fBlendedColor);
                TEST_ASSERT(kRGBA_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(!xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(GrColorPackA4(255) == xpi.fInvariantOutput.fBlendedColor);
                TEST_ASSERT(kRGBA_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDst_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(!xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(GrColorPackA4(255) == xpi.fInvariantOutput.fBlendedColor);
                TEST_ASSERT(kRGBA_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kOne_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstOver_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstIn_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstOut_Mode:
                TEST_ASSERT(!xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(0 == xpi.fInvariantOutput.fBlendedColor);
                TEST_ASSERT(kRGBA_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kDstATop_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kIDA_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kPlus_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
                TEST_ASSERT((kIgnoreCoverage_OptFlag) == xpi.fOptFlags);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(kAdd_GrBlendEquation == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(kZero_GrBlendCoeff == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(kSC_GrBlendCoeff == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWriteColor);
                break;
            case SkXfermode::kScreen_Mode:
                TEST_ASSERT(xpi.fInvariantOutput.fWillBlendWithDst);
                TEST_ASSERT(kNone_GrColorComponentFlags == xpi.fInvariantOutput.fBlendedColorFlags);
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

#endif

