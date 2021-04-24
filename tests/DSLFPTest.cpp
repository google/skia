/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "tests/Test.h"
#include "tests/sksl/dslfp/GrDSLFPTest_DoStatement.h"
#include "tests/sksl/dslfp/GrDSLFPTest_ForStatement.h"
#include "tests/sksl/dslfp/GrDSLFPTest_IfStatement.h"
#include "tests/sksl/dslfp/GrDSLFPTest_SwitchStatement.h"
#include "tests/sksl/dslfp/GrDSLFPTest_Swizzle.h"
#include "tests/sksl/dslfp/GrDSLFPTest_Ternary.h"
#include "tests/sksl/dslfp/GrDSLFPTest_WhileStatement.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

template <typename FPClass, typename... Uniforms>
static void test_dsl_fp(skiatest::Reporter* r,
                        GrDirectContext* ctx,
                        bool worksInES2,
                        Uniforms&&... uniforms) {
    if (!worksInES2) {
        // We don't have an ES2 caps bit, so we check for integer support and derivatives support.
        // Our ES2 bots should return false for these.
        if (!ctx->priv().caps()->shaderCaps()->shaderDerivativeSupport() ||
            !ctx->priv().caps()->shaderCaps()->integerSupport()) {
            return;
        }
    }
    std::unique_ptr<GrSurfaceDrawContext> rtCtx =
            GrSurfaceDrawContext::Make(ctx,
                                       GrColorType::kRGBA_8888,
                                       /*colorSpace=*/nullptr,
                                       SkBackingFit::kApprox,
                                       /*dimensions=*/{1, 1},
                                       SkSurfaceProps{});

    rtCtx->fillRectWithFP(SkIRect::MakeWH(1, 1),
                          FPClass::Make(std::forward<Uniforms>(uniforms)...));

    SkImageInfo dstInfo = SkImageInfo::Make(/*width=*/1, /*height=*/1, kRGBA_8888_SkColorType,
                                            kPremul_SkAlphaType, /*cs=*/nullptr);
    GrPixmap dstPM = GrPixmap::Allocate(dstInfo);
    REPORTER_ASSERT(r, rtCtx->readPixels(ctx, dstPM, /*srcPt=*/{0, 0}));

    const GrColor* color = static_cast<const GrColor*>(dstPM.addr());
    REPORTER_ASSERT(r, *color == GrColorPackRGBA(0x00, 0xFF, 0x00, 0xFF),
                    "Expected: solid green. Actual: A=%02X R=%02X G=%02X B=%02X.",
                    GrColorUnpackA(*color), GrColorUnpackR(*color),
                    GrColorUnpackG(*color), GrColorUnpackB(*color));
}

#define DSL_FP_TEST_ES2(FPClass, ...)                                                    \
    DEF_GPUTEST_FOR_RENDERING_CONTEXTS(FPClass, r, ctxInfo) {                            \
        return test_dsl_fp<Gr##FPClass>(r, ctxInfo.directContext(), /*worksInES2=*/true, \
                                        ##__VA_ARGS__);                                  \
    }
#define DSL_FP_TEST(FPClass, ...)                                                         \
    DEF_GPUTEST_FOR_RENDERING_CONTEXTS(FPClass, r, ctxInfo) {                             \
        return test_dsl_fp<Gr##FPClass>(r, ctxInfo.directContext(), /*worksInES2=*/false, \
                                        ##__VA_ARGS__);                                   \
    }

DSL_FP_TEST_ES2(DSLFPTest_IfStatement, /*one:*/ 1.0f)
DSL_FP_TEST_ES2(DSLFPTest_Swizzle)
DSL_FP_TEST_ES2(DSLFPTest_Ternary, /*primaryColors:*/true,
                                   /*colorGreen:*/   SkPMColor4f{0.0, 1.0, 0.0, 1.0},
                                   /*colorRed:*/     SkPMColor4f{1.0, 0.0, 0.0, 1.0},
                                   /*colorOrange:*/  SkPMColor4f{1.0, 0.5, 0.0, 1.0},
                                   /*colorPurple:*/  SkPMColor4f{0.5, 0.0, 0.5, 1.0})
DSL_FP_TEST(DSLFPTest_DoStatement, /*shouldLoop:*/ false)
DSL_FP_TEST(DSLFPTest_ForStatement, /*colorWhite:*/ SkPMColor4f{1, 1, 1, 1})
DSL_FP_TEST(DSLFPTest_SwitchStatement)
DSL_FP_TEST(DSLFPTest_WhileStatement)
