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
#include "src/gpu/GrSurfaceDrawContext.h"
#include "tests/Test.h"
#include "tests/sksl/dslfp/GrDSLTestDoStatement.h"
#include "tests/sksl/dslfp/GrDSLTestForStatement.h"
#include "tests/sksl/dslfp/GrDSLTestIfStatement.h"
#include "tests/sksl/dslfp/GrDSLTestSwitchStatement.h"
#include "tests/sksl/dslfp/GrDSLTestSwizzle.h"
#include "tests/sksl/dslfp/GrDSLTestTernary.h"
#include "tests/sksl/dslfp/GrDSLTestWhileStatement.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

static const SkRect kRect = SkRect::MakeWH(1, 1);

template <typename FPClass>
static void test_dsl_fp(skiatest::Reporter* r, GrDirectContext* ctx) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(kRect.width(), kRect.height());
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info));

    std::unique_ptr<GrSurfaceDrawContext> rtCtx =
            GrSurfaceDrawContext::Make(ctx,
                                       GrColorType::kRGBA_8888,
                                       /*colorSpace=*/nullptr,
                                       SkBackingFit::kApprox,
                                       {1, 1});

    GrPaint paint;
    paint.setColorFragmentProcessor(FPClass::Make());
    rtCtx->drawRect(/*clip=*/nullptr, std::move(paint), GrAA::kNo, SkMatrix::I(), kRect);

    SkBitmap bitmap;
    REPORTER_ASSERT(r, bitmap.tryAllocPixels(surface->imageInfo()));
    REPORTER_ASSERT(r, surface->readPixels(bitmap.info(), bitmap.getPixels(), bitmap.rowBytes(),
                                           /*srcX=*/0, /*srcY=*/0));

    SkColor color = bitmap.getColor(0, 0);
    REPORTER_ASSERT(r, color == SkColorSetARGB(0xFF, 0x00, 0xFF, 0x00),
                    "Expected: solid green. Actual: A=%02X R=%02X G=%02X B=%02X.",
                    SkColorGetA(color), SkColorGetR(color), SkColorGetG(color), SkColorGetB(color));
}

#define DSL_FP_TEST(FPClass)                                         \
    DEF_GPUTEST_FOR_RENDERING_CONTEXTS(FPClass, r, ctxInfo) {        \
        return test_dsl_fp<Gr##FPClass>(r, ctxInfo.directContext()); \
    }

DSL_FP_TEST(DSLTestDoStatement)
DSL_FP_TEST(DSLTestForStatement)
DSL_FP_TEST(DSLTestIfStatement)
DSL_FP_TEST(DSLTestSwitchStatement)
DSL_FP_TEST(DSLTestSwizzle)
DSL_FP_TEST(DSLTestTernary)
DSL_FP_TEST(DSLTestWhileStatement)
