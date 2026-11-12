/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/mock/GrMockTypes.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

// These tests use GrMockOptions::fMaxVertexAttributes to force GrOpsRenderPass::bindPipeline() to
// fail for any draw whose geometry processor needs more vertex attributes than the limit. The
// failed draw must fail its render task and be reported through the public API (FlushResult,
// readPixels, GetBackendTextureFromImage).

namespace {

SkImageInfo premul_info() {
    return SkImageInfo::Make(16, 16, kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                             SkColorSpace::MakeSRGB());
}

SkImageInfo unpremul_info() {
    return SkImageInfo::Make(16, 16, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType,
                             SkColorSpace::MakeSRGB());
}

sk_sp<SkSurface> make_surface(GrDirectContext* dContext) {
    return SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes, premul_info());
}

// Draws a non-AA rect. Its geometry processor uses 2 vertex attributes (position + color).
void draw_two_attribute_rect(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    paint.setAntiAlias(false);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
}

// Draws a skewed AA rect. Its geometry processor uses 3 vertex attributes
// (position + coverage + color).
void draw_three_attribute_rect(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(SK_ColorGREEN);
    paint.setAntiAlias(true);
    canvas->save();
    canvas->skew(0.5f, 0.0f);
    canvas->drawRect(SkRect::MakeWH(8, 8), paint);
    canvas->restore();
}

}  // anonymous namespace

// Non-regression baseline: with default mock options, every operation exercised by the failure
// tests below succeeds.
DEF_GANESH_TEST(GaneshDrawFailure_DefaultContextSucceeds,
                reporter,
                options,
                CtsEnforcement::kNever) {
    GrMockOptions mockOptions;
    sk_sp<GrDirectContext> dContext = GrDirectContext::MakeMock(&mockOptions, options);
    REPORTER_ASSERT(reporter, dContext);
    sk_sp<SkSurface> surface = make_surface(dContext.get());
    REPORTER_ASSERT(reporter, surface);
    if (!surface) {
        return;
    }
    SkCanvas* canvas = surface->getCanvas();

    SkBitmap premulDst;
    premulDst.allocPixels(premul_info());
    SkBitmap unpremulDst;
    unpremulDst.allocPixels(unpremul_info());

    SkPaint paint;
    paint.setColor(SK_ColorGREEN);

    canvas->drawRect(SkRect::MakeWH(8, 8), paint);
    REPORTER_ASSERT(reporter, skgpu::ganesh::Flush(surface.get()).fSuccess);

    canvas->drawRect(SkRect::MakeWH(8, 8), paint);
    REPORTER_ASSERT(reporter, surface->readPixels(premulDst, 0, 0));

    canvas->drawRect(SkRect::MakeWH(8, 8), paint);
    REPORTER_ASSERT(reporter, surface->readPixels(unpremulDst, 0, 0));

    canvas->drawRRect(SkRRect::MakeRectXY(SkRect::MakeWH(8, 8), 2, 2), paint);
    REPORTER_ASSERT(reporter, skgpu::ganesh::Flush(surface.get()).fSuccess);

    sk_sp<SkImage> snapshot = surface->makeImageSnapshot();
    REPORTER_ASSERT(reporter, snapshot);
    sk_sp<SkImage> converted = snapshot->makeColorSpace(
            dContext->asRecorder(), SkColorSpace::MakeSRGBLinear(), {});
    REPORTER_ASSERT(reporter, converted);
    GrBackendTexture backendTex;
    REPORTER_ASSERT(reporter,
                    SkImages::GetBackendTextureFromImage(converted,
                                                         &backendTex,
                                                         /*flushPendingGrContextIO=*/true));
}

// With fMaxVertexAttributes = 0 every draw fails to bind its pipeline. Each such failure must be
// reported to the caller and, since a failed flush is fatal, must abandon the context. Because
// abandonment is permanent, each scenario below uses its own context; otherwise every scenario
// after the first would fail trivially due to the abandoned context.
DEF_GANESH_TEST(GaneshDrawFailure_PipelineBindFailurePropagates,
                reporter,
                options,
                CtsEnforcement::kNever) {
    auto makeContext = [&]() {
        GrMockOptions mockOptions;
        mockOptions.fMaxVertexAttributes = 0;
        return GrDirectContext::MakeMock(&mockOptions, options);
    };

    SkPaint paint;
    paint.setColor(SK_ColorGREEN);

    // A full-surface clear becomes a load op; it does not bind a draw pipeline and succeeds. This
    // establishes that the failures below are due to the draws and not the context configuration.
    {
        sk_sp<GrDirectContext> dContext = makeContext();
        REPORTER_ASSERT(reporter, dContext);
        sk_sp<SkSurface> surface = make_surface(dContext.get());
        REPORTER_ASSERT(reporter, surface);
        if (!surface) {
            return;
        }

        surface->getCanvas()->clear(SK_ColorBLUE);
        REPORTER_ASSERT(reporter, skgpu::ganesh::Flush(surface.get()).fSuccess,
                        "a clear-only flush should succeed");
        REPORTER_ASSERT(reporter, !dContext->abandoned(),
                        "a successful flush should not abandon the context");
    }

    // An explicit flush of a failing draw reports the failure.
    {
        sk_sp<GrDirectContext> dContext = makeContext();
        REPORTER_ASSERT(reporter, dContext);
        sk_sp<SkSurface> surface = make_surface(dContext.get());
        REPORTER_ASSERT(reporter, surface);
        if (!surface) {
            return;
        }

        surface->getCanvas()->drawRect(SkRect::MakeWH(8, 8), paint);
        REPORTER_ASSERT(reporter, !skgpu::ganesh::Flush(surface.get()).fSuccess,
                        "flushing a failing rect draw should fail");
        REPORTER_ASSERT(reporter, dContext->abandoned());
    }

    // The internal flush performed by a premul readPixels reports the failure.
    {
        sk_sp<GrDirectContext> dContext = makeContext();
        REPORTER_ASSERT(reporter, dContext);
        sk_sp<SkSurface> surface = make_surface(dContext.get());
        REPORTER_ASSERT(reporter, surface);
        if (!surface) {
            return;
        }
        SkBitmap premulDst;
        premulDst.allocPixels(premul_info());

        surface->getCanvas()->drawRect(SkRect::MakeWH(8, 8), paint);
        REPORTER_ASSERT(reporter, !surface->readPixels(premulDst, 0, 0),
                        "premul readPixels of a failing draw should fail");
        REPORTER_ASSERT(reporter, dContext->abandoned());
    }

    // The first unpremul readPixels on a context takes the PM->UPM fast path, which first runs
    // (and flushes) validPMUPMConversionExists()'s conversion test. The pending draw's failure must
    // still be reported to the caller. Note: in this configuration the conversion test's and the
    // readback's own draws also fail, so this verifies that the failure is reported, not which
    // flush observed it.
    {
        sk_sp<GrDirectContext> dContext = makeContext();
        REPORTER_ASSERT(reporter, dContext);
        sk_sp<SkSurface> surface = make_surface(dContext.get());
        REPORTER_ASSERT(reporter, surface);
        if (!surface) {
            return;
        }
        SkBitmap unpremulDst;
        unpremulDst.allocPixels(unpremul_info());

        surface->getCanvas()->drawRect(SkRect::MakeWH(8, 8), paint);
        REPORTER_ASSERT(reporter, !surface->readPixels(unpremulDst, 0, 0),
                        "unpremul readPixels of a failing draw should fail");
        REPORTER_ASSERT(reporter, dContext->abandoned());
    }

    // A failing rrect draw (a different op type) reports the failure.
    {
        sk_sp<GrDirectContext> dContext = makeContext();
        REPORTER_ASSERT(reporter, dContext);
        sk_sp<SkSurface> surface = make_surface(dContext.get());
        REPORTER_ASSERT(reporter, surface);
        if (!surface) {
            return;
        }

        surface->getCanvas()->drawRRect(SkRRect::MakeRectXY(SkRect::MakeWH(8, 8), 2, 2), paint);
        REPORTER_ASSERT(reporter, !skgpu::ganesh::Flush(surface.get()).fSuccess,
                        "flushing a failing rrect draw should fail");
        REPORTER_ASSERT(reporter, dContext->abandoned());
    }

    // Multi-task DAG: the color-space conversion draws into a scratch surface. Flushing it while
    // extracting the backend texture must report the failure.
    {
        sk_sp<GrDirectContext> dContext = makeContext();
        REPORTER_ASSERT(reporter, dContext);
        sk_sp<SkSurface> surface = make_surface(dContext.get());
        REPORTER_ASSERT(reporter, surface);
        if (!surface) {
            return;
        }

        // Give the snapshot defined contents without issuing a (failing) draw.
        surface->getCanvas()->clear(SK_ColorBLUE);
        REPORTER_ASSERT(reporter, skgpu::ganesh::Flush(surface.get()).fSuccess);
        REPORTER_ASSERT(reporter, !dContext->abandoned());

        sk_sp<SkImage> snapshot = surface->makeImageSnapshot();
        REPORTER_ASSERT(reporter, snapshot);
        sk_sp<SkImage> converted = snapshot->makeColorSpace(
                dContext->asRecorder(), SkColorSpace::MakeSRGBLinear(), {});
        REPORTER_ASSERT(reporter, converted);
        if (!converted) {
            return;
        }
        GrBackendTexture backendTex;
        REPORTER_ASSERT(reporter,
                        !SkImages::GetBackendTextureFromImage(converted,
                                                              &backendTex,
                                                              /*flushPendingGrContextIO=*/true),
                        "flushing a failing color-space conversion should fail");
        REPORTER_ASSERT(reporter, dContext->abandoned());
    }
}

// With fMaxVertexAttributes = 2, a 3-attribute draw fails while a 2-attribute draw succeeds.
// A failed flush is fatal: the context is abandoned and all subsequent flushes, even of valid work,
// fail. Since abandonment is permanent, each scenario below uses its own context.
DEF_GANESH_TEST(GaneshDrawFailure_FailedFlushAbandonsContext,
                reporter,
                options,
                CtsEnforcement::kNever) {
    auto makeContext = [&]() {
        GrMockOptions mockOptions;
        mockOptions.fMaxVertexAttributes = 2;
        return GrDirectContext::MakeMock(&mockOptions, options);
    };

    // Precondition: verify the 2-attribute draw is on the passing side of the attribute limit, so
    // that a change in vertex layout can't silently turn this into a different test.
    {
        sk_sp<GrDirectContext> dContext = makeContext();
        REPORTER_ASSERT(reporter, dContext);
        sk_sp<SkSurface> surface = make_surface(dContext.get());
        REPORTER_ASSERT(reporter, surface);
        if (!surface) {
            return;
        }
        SkCanvas* canvas = surface->getCanvas();

        draw_two_attribute_rect(canvas);
        REPORTER_ASSERT(reporter, skgpu::ganesh::Flush(surface.get()).fSuccess,
                        "2-attribute draw should succeed on its own");
        draw_two_attribute_rect(canvas);
        REPORTER_ASSERT(reporter, skgpu::ganesh::Flush(surface.get()).fSuccess,
                        "repeated 2-attribute draws should succeed");
        REPORTER_ASSERT(reporter, !dContext->abandoned(),
                        "successful flushes should not abandon the context");
    }

    // A failing draw on its own fails the flush and abandons the context. Subsequent valid work
    // cannot be flushed.
    {
        sk_sp<GrDirectContext> dContext = makeContext();
        REPORTER_ASSERT(reporter, dContext);
        sk_sp<SkSurface> surface = make_surface(dContext.get());
        REPORTER_ASSERT(reporter, surface);
        if (!surface) {
            return;
        }
        SkCanvas* canvas = surface->getCanvas();

        // Establish that the context is healthy before the failure.
        draw_two_attribute_rect(canvas);
        REPORTER_ASSERT(reporter, skgpu::ganesh::Flush(surface.get()).fSuccess,
                        "2-attribute draw should succeed before the failure");
        REPORTER_ASSERT(reporter, !dContext->abandoned());

        draw_three_attribute_rect(canvas);
        REPORTER_ASSERT(reporter, !skgpu::ganesh::Flush(surface.get()).fSuccess,
                        "3-attribute draw should fail on its own");
        REPORTER_ASSERT(reporter, dContext->abandoned(),
                        "a failed flush should abandon the context");

        draw_two_attribute_rect(canvas);
        REPORTER_ASSERT(reporter, !skgpu::ganesh::Flush(surface.get()).fSuccess,
                        "flush after a failed flush should fail");
        REPORTER_ASSERT(reporter, dContext->abandoned());
    }

    // A failing draw followed by a valid draw in the same flush fails the flush and abandons the
    // context.
    {
        sk_sp<GrDirectContext> dContext = makeContext();
        REPORTER_ASSERT(reporter, dContext);
        sk_sp<SkSurface> surface = make_surface(dContext.get());
        REPORTER_ASSERT(reporter, surface);
        if (!surface) {
            return;
        }
        SkCanvas* canvas = surface->getCanvas();

        draw_three_attribute_rect(canvas);
        draw_two_attribute_rect(canvas);
        REPORTER_ASSERT(reporter, !skgpu::ganesh::Flush(surface.get()).fSuccess,
                        "flush containing a failing draw should fail");
        REPORTER_ASSERT(reporter, dContext->abandoned(),
                        "a failed mixed flush should abandon the context");

        draw_two_attribute_rect(canvas);
        REPORTER_ASSERT(reporter, !skgpu::ganesh::Flush(surface.get()).fSuccess,
                        "flush after a failed mixed flush should fail");
    }
}
