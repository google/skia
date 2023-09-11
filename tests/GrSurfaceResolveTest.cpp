/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/SkColorData.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/GrTextureResolveRenderTask.h"
#include "src/gpu/ganesh/SurfaceContext.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/ops/OpsTask.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/gpu/FenceSync.h"
#include "tools/gpu/ManagedBackendTexture.h"

#include <functional>
#include <initializer_list>
#include <memory>
#include <utility>

struct GrContextOptions;

using namespace sk_gpu_test;

bool check_pixels(skiatest::Reporter* reporter,
                  GrDirectContext* dContext,
                  const GrBackendTexture& tex,
                  const SkImageInfo& info,
                  SkColor expectedColor) {
    // We have to do the readback of the backend texture wrapped in a different Skia surface than
    // the one used in the main body of the test or else the readPixels call will trigger resolves
    // itself.
    sk_sp<SkSurface> surface = SkSurfaces::WrapBackendTexture(dContext,
                                                              tex,
                                                              kTopLeft_GrSurfaceOrigin,
                                                              /*sampleCnt=*/4,
                                                              kRGBA_8888_SkColorType,
                                                              nullptr,
                                                              nullptr);
    SkBitmap actual;
    actual.allocPixels(info);
    if (!surface->readPixels(actual, 0, 0)) {
        return false;
    }

    SkBitmap expected;
    expected.allocPixels(info);
    SkCanvas tmp(expected);
    tmp.clear(expectedColor);
    expected.setImmutable();

    const float tols[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    auto error = std::function<ComparePixmapsErrorReporter>(
        [reporter](int x, int y, const float diffs[4]) {
            SkASSERT(x >= 0 && y >= 0);
            ERRORF(reporter, "mismatch at %d, %d (%f, %f, %f %f)",
                   x, y, diffs[0], diffs[1], diffs[2], diffs[3]);
        });

    return ComparePixels(expected.pixmap(), actual.pixmap(), tols, error);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceResolveTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();

    SkImageInfo info = SkImageInfo::Make(8, 8, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    auto managedTex = ManagedBackendTexture::MakeFromInfo(
            dContext, info, skgpu::Mipmapped::kNo, GrRenderable::kYes);
    if (!managedTex) {
        return;
    }
    auto tex = managedTex->texture();
    // Wrap the backend surface but tell it rendering with MSAA so that the wrapped texture is the
    // resolve.
    sk_sp<SkSurface> surface = SkSurfaces::WrapBackendTexture(dContext,
                                                              tex,
                                                              kTopLeft_GrSurfaceOrigin,
                                                              /*sampleCnt=*/4,
                                                              kRGBA_8888_SkColorType,
                                                              nullptr,
                                                              nullptr);

    if (!surface) {
        return;
    }

    const GrCaps* caps = dContext->priv().caps();
    // In metal and vulkan if we prefer discardable msaa attachments we will also auto resolve. The
    // GrBackendTexture and SkSurface are set up in a way that is compatible with discardable msaa
    // for both backends.
    bool autoResolves = caps->msaaResolvesAutomatically() ||
                        caps->preferDiscardableMSAAAttachment();

    // First do a simple test where we clear the surface than flush with SkSurface::flush. This
    // should trigger the resolve and the texture should have the correct data.
    surface->getCanvas()->clear(SK_ColorRED);
    dContext->flush(surface.get());
    dContext->submit();
    REPORTER_ASSERT(reporter, check_pixels(reporter, dContext, tex, info, SK_ColorRED));

    // Next try doing a GrDirectContext::flush without the surface which will not trigger a resolve
    // on gpus without automatic msaa resolves.
    surface->getCanvas()->clear(SK_ColorBLUE);
    dContext->flush();
    dContext->submit();
    if (autoResolves) {
        REPORTER_ASSERT(reporter, check_pixels(reporter, dContext, tex, info, SK_ColorBLUE));
    } else {
        REPORTER_ASSERT(reporter, check_pixels(reporter, dContext, tex, info, SK_ColorRED));
    }

    // Now doing a surface flush (even without any queued up normal work) should still resolve the
    // surface.
    dContext->flush(surface.get());
    dContext->submit();
    REPORTER_ASSERT(reporter, check_pixels(reporter, dContext, tex, info, SK_ColorBLUE));

    // Test using SkSurface::resolve with a GrDirectContext::flush
    surface->getCanvas()->clear(SK_ColorRED);
    SkSurfaces::ResolveMSAA(surface);
    dContext->flush();
    dContext->submit();
    REPORTER_ASSERT(reporter, check_pixels(reporter, dContext, tex, info, SK_ColorRED));

    // Calling resolve again should cause no issues as it is a no-op (there is an assert in the
    // resolve op that the surface's msaa is dirty, we shouldn't hit that assert).
    SkSurfaces::ResolveMSAA(surface);
    dContext->flush();
    dContext->submit();
    REPORTER_ASSERT(reporter, check_pixels(reporter, dContext, tex, info, SK_ColorRED));

    // Try resolving in the middle of draw calls. Non automatic resolve gpus should only see the
    // results of the first draw.
    surface->getCanvas()->clear(SK_ColorGREEN);
    SkSurfaces::ResolveMSAA(surface);
    surface->getCanvas()->clear(SK_ColorBLUE);
    dContext->flush();
    dContext->submit();
    if (autoResolves) {
        REPORTER_ASSERT(reporter, check_pixels(reporter, dContext, tex, info, SK_ColorBLUE));
    } else {
        REPORTER_ASSERT(reporter, check_pixels(reporter, dContext, tex, info, SK_ColorGREEN));
    }

    // Test that a resolve between draws to a different surface doesn't cause the OpsTasks for that
    // surface to be split. Fails if we hit validation asserts in GrDrawingManager.
    // First clear out dirty msaa from previous test
    dContext->flush(surface.get());

    auto otherSurface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes, info);
    REPORTER_ASSERT(reporter, otherSurface);
    otherSurface->getCanvas()->clear(SK_ColorRED);
    SkSurfaces::ResolveMSAA(surface);
    otherSurface->getCanvas()->clear(SK_ColorBLUE);
    dContext->flush();
    dContext->submit();

    // Make sure resolving a non-msaa surface doesn't trigger a resolve call. We'll hit an assert
    // that the msaa is not dirty if it does.
    REPORTER_ASSERT(reporter, otherSurface);
    otherSurface->getCanvas()->clear(SK_ColorRED);
    SkSurfaces::ResolveMSAA(otherSurface);
    dContext->flush();
    dContext->submit();
}

// This test comes from crbug.com/1355807 and crbug.com/1365578. The underlying issue was:
//  * We would do a non-mipmapped draw of a proxy. This proxy would add a dependency from the ops
//    task to the proxy's last render task, which was a copy task targetting the proxy.
//  * We would do a mipmapped draw of the same proxy to the same ops task.
//    GrRenderTask::addDependency would detect the pre-existing dependency and early out before
//    adding the proxy to a resolve task.
// We also test the case where the first draw should add a MSAA resolve and the second draw should
// add a mipmap resolve.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(NonmippedDrawBeforeMippedDraw,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    using ResolveFlags = GrSurfaceProxy::ResolveFlags;
    auto dc = ctxInfo.directContext();

    if (!dc->priv().caps()->mipmapSupport()) {
        return;
    }

    for (int sampleCount : {1, 4}) {
        GrRenderable renderable = sampleCount > 1 ? GrRenderable::kYes : GrRenderable::kNo;

        auto bef = dc->priv().caps()->getDefaultBackendFormat(GrColorType::kRGBA_8888, renderable);
        if (sampleCount > 1) {
            if (dc->priv().caps()->msaaResolvesAutomatically()) {
                // MSAA won't add a resolve task.
                continue;
            }
            sampleCount = dc->priv().caps()->getRenderTargetSampleCount(sampleCount, bef);
            if (!sampleCount) {
                continue;
            }
        }

        // Create a mipmapped proxy
        auto mmProxy = dc->priv().proxyProvider()->createProxy(bef,
                                                               {64, 64},
                                                               renderable,
                                                               sampleCount,
                                                               skgpu::Mipmapped::kYes,
                                                               SkBackingFit::kExact,
                                                               skgpu::Budgeted::kYes,
                                                               GrProtected::kNo,
                                                               "test MM Proxy");
        GrSurfaceProxyView mmProxyView{mmProxy,
                                       kBottomLeft_GrSurfaceOrigin,
                                       skgpu::Swizzle::RGBA()};

        if (sampleCount > 1) {
            // Make sure MSAA surface needs a resolve by drawing to it. This also adds a last
            // render task to the proxy.
            auto drawContext = skgpu::ganesh::SurfaceDrawContext::Make(dc,
                                                                       GrColorType::kRGBA_8888,
                                                                       mmProxy,
                                                                       nullptr,
                                                                       kBottomLeft_GrSurfaceOrigin,
                                                                       SkSurfaceProps{});
            drawContext->fillWithFP(GrFragmentProcessor::MakeColor(SK_PMColor4fWHITE));
        } else {
            // Use a copy, as in the original bug, to dirty the mipmap status and also install
            // a last render task on the proxy.
            auto src = dc->priv().proxyProvider()->createProxy(bef,
                                                               {64, 64},
                                                               GrRenderable::kNo,
                                                               1,
                                                               skgpu::Mipmapped::kNo,
                                                               SkBackingFit::kExact,
                                                               skgpu::Budgeted::kYes,
                                                               GrProtected::kNo,
                                                               "testSrc");
            skgpu::ganesh::SurfaceContext mmSC(
                    dc, mmProxyView, {GrColorType::kRGBA_8888, kPremul_SkAlphaType, nullptr});
            mmSC.testCopy(src);
        }

        auto drawDst = skgpu::ganesh::SurfaceDrawContext::Make(dc,
                                                               GrColorType::kRGBA_8888,
                                                               nullptr,
                                                               SkBackingFit::kExact,
                                                               {8, 8},
                                                               SkSurfaceProps{},
                                                               "testDrawDst");

        // Do a non-mipmapped draw from the mipmapped texture. This should add a dependency on the
        // copy task recorded above. If the src texture is also multisampled this should record a
        // msaa-only resolve.
        {
            auto te = GrTextureEffect::Make(
                    mmProxyView,
                    kPremul_SkAlphaType,
                    SkMatrix::I(),
                    GrSamplerState{SkFilterMode::kLinear, SkMipmapMode::kNone},
                    *dc->priv().caps());

            GrPaint paint;
            paint.setColorFragmentProcessor(std::move(te));

            drawDst->drawRect(nullptr,
                              std::move(paint),
                              GrAA::kNo,
                              SkMatrix::Scale(1/8.f, 1/8.f),
                              SkRect::Make(mmProxy->dimensions()));
            if (sampleCount > 1) {
                const GrTextureResolveRenderTask* resolveTask =
                        drawDst->getOpsTask()->resolveTask();
                if (!resolveTask) {
                    ERRORF(reporter, "No resolve task after drawing MSAA proxy");
                    return;
                }
                if (resolveTask->flagsForProxy(mmProxy) != ResolveFlags::kMSAA) {
                    ERRORF(reporter, "Expected resolve flags to be kMSAA");
                    return;
                }
            }
        }

        // Now do a mipmapped draw from the same texture. Ensure that even though we have a
        // dependency on the copy task we still ensure that a resolve is recorded.
        {
            auto te = GrTextureEffect::Make(
                    mmProxyView,
                    kPremul_SkAlphaType,
                    SkMatrix::I(),
                    GrSamplerState{SkFilterMode::kLinear, SkMipmapMode::kLinear},
                    *dc->priv().caps());

            GrPaint paint;
            paint.setColorFragmentProcessor(std::move(te));

            drawDst->drawRect(nullptr,
                              std::move(paint),
                              GrAA::kNo,
                              SkMatrix::Scale(1/8.f, 1/8.f),
                              SkRect::Make(mmProxy->dimensions()));
        }
        const GrTextureResolveRenderTask* resolveTask = drawDst->getOpsTask()->resolveTask();
        if (!resolveTask) {
            ERRORF(reporter, "No resolve task after drawing mip mapped proxy");
            return;
        }

        ResolveFlags expectedFlags = GrSurfaceProxy::ResolveFlags::kMipMaps;
        const char* expectedStr = "kMipMaps";
        if (sampleCount > 1) {
            expectedFlags |= GrSurfaceProxy::ResolveFlags::kMSAA;
            expectedStr = "kMipMaps|kMSAA";
        }
        if (resolveTask->flagsForProxy(mmProxy) != expectedFlags) {
            ERRORF(reporter, "Expected resolve flags to be %s", expectedStr);
            return;
        }
    }
}
