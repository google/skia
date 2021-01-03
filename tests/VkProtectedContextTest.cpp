/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a Vulkan protected memory specific test.

#include "include/core/SkTypes.h"

#if SK_SUPPORT_GPU && defined(SK_VULKAN)

#include "include/core/SkCanvas.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/GrVkExtensions.h"
#include "tests/Test.h"
#include "tools/gpu/BackendSurfaceFactory.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/vk/VkTestHelper.h"

static sk_sp<SkSurface> create_protected_sksurface(GrDirectContext* dContext,
                                                   skiatest::Reporter* reporter,
                                                   bool textureable = true) {
    const int kW = 8;
    const int kH = 8;
    SkSurfaceProps surfaceProps = SkSurfaceProps(0, kRGB_H_SkPixelGeometry);
    sk_sp<SkSurface> surface;
    if (textureable) {
        surface = sk_gpu_test::MakeBackendTextureSurface(dContext,
                                                         {kW, kH},
                                                         kTopLeft_GrSurfaceOrigin,
                                                         1,
                                                         kRGBA_8888_SkColorType,
                                                         /* color space */ nullptr,
                                                         GrMipmapped::kNo,
                                                         GrProtected::kYes,
                                                         &surfaceProps);
    } else {
        surface = sk_gpu_test::MakeBackendRenderTargetSurface(dContext,
                                                              {kW, kH},
                                                              kTopLeft_GrSurfaceOrigin,
                                                              1,
                                                              kRGBA_8888_SkColorType,
                                                              /* color space */ nullptr,
                                                              GrProtected::kYes,
                                                              &surfaceProps);
    }
    if (!surface) {
        ERRORF(reporter, "Could not create protected surface.");
        return nullptr;
    }
    if (textureable) {
        GrBackendTexture backendTex =
                surface->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess);
        REPORTER_ASSERT(reporter, backendTex.isValid());
        REPORTER_ASSERT(reporter, backendTex.isProtected());
    } else {
        GrBackendRenderTarget backendRT =
                surface->getBackendRenderTarget(SkSurface::kFlushRead_BackendHandleAccess);
        REPORTER_ASSERT(reporter, backendRT.isValid());
        REPORTER_ASSERT(reporter, backendRT.isProtected());
    }
    return surface;
}

DEF_GPUTEST(VkProtectedContext_CreateNonprotectedContext, reporter, options) {
    auto nonprotectedTestHelper = std::make_unique<VkTestHelper>(false);
    REPORTER_ASSERT(reporter, nonprotectedTestHelper->init());
}

DEF_GPUTEST(VkProtectedContext_CreateProtectedContext, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
}

DEF_GPUTEST(VkProtectedContext_CreateProtectedSkSurface, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }

    auto dContext = protectedTestHelper->directContext();
    REPORTER_ASSERT(reporter, dContext != nullptr);
    create_protected_sksurface(dContext, reporter, /*textureable*/ true);
    create_protected_sksurface(dContext, reporter, /*textureable*/ false);
}

DEF_GPUTEST(VkProtectedContext_CreateNonprotectedTextureInProtectedContext, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext() != nullptr);

    const int kW = 8;
    const int kH = 8;
    GrBackendTexture backendTex =
        protectedTestHelper->directContext()->createBackendTexture(
            kW, kH, kRGBA_8888_SkColorType, GrMipmapped::kNo, GrRenderable::kNo,
            GrProtected::kNo);
    REPORTER_ASSERT(reporter, !backendTex.isValid());
}

DEF_GPUTEST(VkProtectedContext_CreateProtectedTextureInNonprotectedContext, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(false);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext() != nullptr);

    const int kW = 8;
    const int kH = 8;
    GrBackendTexture backendTex =
        protectedTestHelper->directContext()->createBackendTexture(
            kW, kH, kRGBA_8888_SkColorType, GrMipmapped::kNo, GrRenderable::kNo,
            GrProtected::kYes);
    REPORTER_ASSERT(reporter, !backendTex.isValid());
}

DEF_GPUTEST(VkProtectedContext_ReadFromProtectedSurface, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext() != nullptr);

    auto surface = create_protected_sksurface(protectedTestHelper->directContext(), reporter);
    REPORTER_ASSERT(reporter, surface);
    REPORTER_ASSERT(reporter, !surface->readPixels(SkImageInfo(), nullptr, 8, 0, 0));
}

namespace {

struct AsyncContext {
    bool fCalled = false;
    std::unique_ptr<const SkSurface::AsyncReadResult> fResult;
};

static void async_callback(void* c, std::unique_ptr<const SkSurface::AsyncReadResult> result) {
    auto context = static_cast<AsyncContext*>(c);
    context->fResult = std::move(result);
    context->fCalled = true;
};

}  // anonymous namespace

DEF_GPUTEST(VkProtectedContext_AsyncReadFromProtectedSurface, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }

    auto dContext = protectedTestHelper->directContext();

    REPORTER_ASSERT(reporter, dContext != nullptr);

    auto surface = create_protected_sksurface(dContext, reporter);
    REPORTER_ASSERT(reporter, surface);
    AsyncContext cbContext;
    const auto image_info = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                      SkColorSpace::MakeSRGB());
    surface->asyncRescaleAndReadPixelsYUV420(kIdentity_SkYUVColorSpace, SkColorSpace::MakeSRGB(),
                                             image_info.bounds(), image_info.dimensions(),
                                             SkSurface::RescaleGamma::kSrc,
                                             SkSurface::RescaleMode::kNearest,
                                             &async_callback, &cbContext);
    dContext->submit();
    while (!cbContext.fCalled) {
        dContext->checkAsyncWorkCompletion();
    }
    REPORTER_ASSERT(reporter, !cbContext.fResult);
}

DEF_GPUTEST(VkProtectedContext_DrawRectangle, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext() != nullptr);

    auto surface = create_protected_sksurface(protectedTestHelper->directContext(), reporter);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
}

DEF_GPUTEST(VkProtectedContext_DrawRectangleWithAntiAlias, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext() != nullptr);

    auto surface = create_protected_sksurface(protectedTestHelper->directContext(), reporter);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setAntiAlias(true);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
}

DEF_GPUTEST(VkProtectedContext_DrawRectangleWithBlendMode, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext() != nullptr);

    auto surface = create_protected_sksurface(protectedTestHelper->directContext(), reporter);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setBlendMode(SkBlendMode::kColorDodge);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
}

DEF_GPUTEST(VkProtectedContext_DrawRectangleWithFilter, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext() != nullptr);

    auto surface = create_protected_sksurface(protectedTestHelper->directContext(), reporter);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kFill_Style);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(
          SkBlurStyle::kOuter_SkBlurStyle, 1.1f));
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
}

DEF_GPUTEST(VkProtectedContext_DrawThinPath, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext() != nullptr);

    auto surface = create_protected_sksurface(protectedTestHelper->directContext(), reporter);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(.4f);
    canvas->drawPath(SkPath().moveTo(4, 4).lineTo(6, 6), paint);
}

DEF_GPUTEST(VkProtectedContext_SaveLayer, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext() != nullptr);

    auto surface = create_protected_sksurface(protectedTestHelper->directContext(), reporter);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    canvas->saveLayer(nullptr, nullptr);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
    canvas->restore();
}


DEF_GPUTEST(VkProtectedContext_DrawProtectedImageOnProtectedSurface, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext() != nullptr);

    // Create protected image.
    auto surface1 = create_protected_sksurface(protectedTestHelper->directContext(), reporter);
    REPORTER_ASSERT(reporter, surface1);
    auto image = surface1->makeImageSnapshot();
    REPORTER_ASSERT(reporter, image);

    // Create protected canvas.
    auto surface2 = create_protected_sksurface(protectedTestHelper->directContext(), reporter);
    REPORTER_ASSERT(reporter, surface2);
    SkCanvas* canvas = surface2->getCanvas();
    REPORTER_ASSERT(reporter, canvas);

    canvas->drawImage(image, 0, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Test out DDLs using a protected Vulkan context

void DDLMakeRenderTargetTestImpl(GrDirectContext*, skiatest::Reporter*);

DEF_GPUTEST(VkProtectedContext_DDLMakeRenderTargetTest, reporter, ctxInfo) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext() != nullptr);

    DDLMakeRenderTargetTestImpl(protectedTestHelper->directContext(), reporter);
}

void DDLSurfaceCharacterizationTestImpl(GrDirectContext*, skiatest::Reporter*);

DEF_GPUTEST(VkProtectedContext_DDLSurfaceCharacterizationTest, reporter, ctxInfo) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext() != nullptr);

    DDLSurfaceCharacterizationTestImpl(protectedTestHelper->directContext(), reporter);
}

#endif  // SK_SUPPORT_GPU && defined(SK_VULKAN)
