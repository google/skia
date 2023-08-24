/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"
#include "include/core/SkColorType.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/mock/GrMockTypes.h"
#include "include/private/SkColorData.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/Blend.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColor.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrDstProxyView.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrProcessorAnalysis.h"
#include "src/gpu/ganesh/GrProcessorSet.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrUserStencilSettings.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/effects/GrPorterDuffXferProcessor.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/ContextType.h"
#include "tools/gpu/ManagedBackendTexture.h"

#include <initializer_list>
#include <utility>

class GrTextureProxy;

////////////////////////////////////////////////////////////////////////////////

static void test_color_unknown_with_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_color_not_opaque_no_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_color_opaque_with_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_color_opaque_no_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_lcd_coverage(skiatest::Reporter* reporter, const GrCaps& caps);
static void test_lcd_coverage_fallback_case(skiatest::Reporter* reporter, const GrCaps& caps);

DEF_GANESH_TEST(GrPorterDuff, reporter, /*ctxInfo*/, CtsEnforcement::kApiLevel_T) {
    GrMockOptions mockOptions;
    mockOptions.fDualSourceBlendingSupport = true;
    sk_sp<GrDirectContext> context = GrDirectContext::MakeMock(&mockOptions, GrContextOptions());
    const GrCaps& caps = *context->priv().getGpu()->caps();

    if (!caps.shaderCaps()->fDualSourceBlendingSupport) {
        SK_ABORT("Null context does not support dual source blending.");
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
            colorInput, coverageInput, nullptr, &GrUserStencilSettings::kUnused, caps,
            GrClampType::kAuto, &overrideColor);
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
            fUnaffectedByDstValue = analysis.unaffectedByDstValue();
            fIgnoresInputColor = analysis.inputColorIsIgnored();
            sk_sp<const GrXferProcessor> xp(
                    GrXPFactory::MakeXferProcessor(xpf, inputColor, inputCoverage, caps,
                                                   GrClampType::kAuto));
            TEST_ASSERT(!analysis.requiresDstTexture() ||
                        (isLCD &&
                         !caps.shaderCaps()->fDstReadInShaderSupport &&
                         (SkBlendMode::kSrcOver != xfermode ||
                          !inputColor.isOpaque())));
            // Porter Duff modes currently only use fixed-function or shader blending, and Ganesh
            // doesn't yet make use of framebuffer fetches that require a barrier
            // (e.g., QCOM_shader_framebuffer_fetch_noncoherent). So dst textures and xfer barriers
            // should always go hand in hand for Porter Duff modes.
            TEST_ASSERT(analysis.requiresDstTexture() == analysis.requiresNonOverlappingDraws());
            GetXPOutputTypes(xp.get(), &fPrimaryOutputType, &fSecondaryOutputType);
            fBlendInfo = xp->getBlendInfo();
            TEST_ASSERT(!xp->willReadDstColor() ||
                        (isLCD && (SkBlendMode::kSrcOver != xfermode ||
                                   !inputColor.isOpaque())));
            TEST_ASSERT(xp->hasSecondaryOutput() ==
                        skgpu::BlendCoeffRefsSrc2(fBlendInfo.fDstBlend));
        }

        bool fCompatibleWithCoverageAsAlpha;
        bool fUnaffectedByDstValue;
        bool fIgnoresInputColor;
        int fPrimaryOutputType;
        int fSecondaryOutputType;
        skgpu::BlendInfo fBlendInfo;
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
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcOver:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kSAModulate_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kIS2C == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kInvalid_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kInvalid_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
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
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kCoverage_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kReverseSubtract == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kDC == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kIS2A == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcOver:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kISA == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kIDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kIS2A == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kISAModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kReverseSubtract == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kDC == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kCoverage_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kIDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kIS2A == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kISA == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kISA == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kISAModulate_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kIDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kIS2C == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kIDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kISA == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kISCModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kReverseSubtract == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kDC == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kISC == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
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
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcOver:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kISA == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kIDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kSA == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kIDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kISA == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kISA == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kIDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kSA == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kIDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kISA == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kSC == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kISC == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
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
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kCoverage_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kReverseSubtract == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kDC == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kISA == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcOver:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kISA == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kIDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kISA == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kIDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kISA == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(!xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kCoverage_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kReverseSubtract == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kDC == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kISA == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kIDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kIDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kISA == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(kISCModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kReverseSubtract == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kDC == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kISC == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
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
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrc:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDst:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcOver:
                // We don't specialize opaque src-over. See note in GrPorterDuffXferProcessor.cpp
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                if (caps.shouldCollapseSrcOverToSrcWhenAble()) {
                    TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                } else {
                    TEST_ASSERT(skgpu::BlendCoeff::kISA == xpi.fBlendInfo.fDstBlend);
                }
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstOver:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kIDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcIn:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstIn:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(!xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcOut:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kIDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstOut:
                TEST_ASSERT(xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kNone_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kSrcATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kDstATop:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kIDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kXor:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kIDA == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kPlus:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kModulate:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kZero == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kSC == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
                break;
            case SkBlendMode::kScreen:
                TEST_ASSERT(!xpi.fIgnoresInputColor);
                TEST_ASSERT(xpi.fCompatibleWithCoverageAsAlpha);
                TEST_ASSERT(!xpi.fUnaffectedByDstValue);
                TEST_ASSERT(kModulate_OutputType == xpi.fPrimaryOutputType);
                TEST_ASSERT(kNone_OutputType == xpi.fSecondaryOutputType);
                TEST_ASSERT(skgpu::BlendEquation::kAdd == xpi.fBlendInfo.fEquation);
                TEST_ASSERT(skgpu::BlendCoeff::kOne == xpi.fBlendInfo.fSrcBlend);
                TEST_ASSERT(skgpu::BlendCoeff::kISC == xpi.fBlendInfo.fDstBlend);
                TEST_ASSERT(xpi.fBlendInfo.fWritesColor);
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
            GrXPFactory::MakeXferProcessor(xpf, color, coverage, caps, autoClamp));
    if (!xp_opaque) {
        ERRORF(reporter, "Failed to create an XP with LCD coverage.");
        return;
    }

    skgpu::BlendInfo blendInfo = xp_opaque->getBlendInfo();
    TEST_ASSERT(blendInfo.fWritesColor);

    // Test with non-opaque alpha
    color = SkPMColor4f::FromBytes_RGBA(GrColorPackRGBA(123, 45, 67, 221));
    coverage = GrProcessorAnalysisCoverage::kLCD;
    TEST_ASSERT(!(GrXPFactory::GetAnalysisProperties(xpf, color, coverage, caps, autoClamp) &
                GrXPFactory::AnalysisProperties::kRequiresDstTexture));
    sk_sp<const GrXferProcessor> xp(
            GrXPFactory::MakeXferProcessor(xpf, color, coverage, caps, autoClamp));
    if (!xp) {
        ERRORF(reporter, "Failed to create an XP with LCD coverage.");
        return;
    }

    blendInfo = xp->getBlendInfo();
    TEST_ASSERT(blendInfo.fWritesColor);
}

DEF_GANESH_TEST(PorterDuffNoDualSourceBlending, reporter, options, CtsEnforcement::kApiLevel_T) {
    GrContextOptions opts = options;
    opts.fSuppressDualSourceBlending = true;
    sk_gpu_test::GrContextFactory mockFactory(opts);
    auto ctx = mockFactory.get(skgpu::ContextType::kMock);
    if (!ctx) {
        SK_ABORT("Failed to create mock context without ARB_blend_func_extended.");
    }

    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
    const GrCaps& caps = *ctx->priv().caps();
    if (caps.shaderCaps()->fDualSourceBlendingSupport) {
        SK_ABORT("Mock context failed to honor request for no ARB_blend_func_extended.");
    }

    auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(
            ctx, 100, 100, kRGBA_8888_SkColorType, GrMipmapped::kNo, GrRenderable::kNo);
    if (!mbet) {
        ERRORF(reporter, "Could not make texture.");
        return;
    }
    GrDstProxyView fakeDstProxyView;
    {
        sk_sp<GrTextureProxy> proxy = proxyProvider->wrapBackendTexture(
                mbet->texture(), kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, kRead_GrIOType,
                mbet->refCountedCallback());
        skgpu::Swizzle swizzle =
                caps.getReadSwizzle(mbet->texture().getBackendFormat(), GrColorType::kRGBA_8888);
        fakeDstProxyView.setProxyView({std::move(proxy), kTopLeft_GrSurfaceOrigin, swizzle});
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
                        GrXPFactory::MakeXferProcessor(xpf, colorInput, coverageType, caps,
                                                       GrClampType::kAuto));
                if (!xp) {
                    ERRORF(reporter, "Failed to create an XP without dual source blending.");
                    return;
                }
                TEST_ASSERT(!xp->hasSecondaryOutput());
            }
        }
    }
}
