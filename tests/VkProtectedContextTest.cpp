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
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/vk/VkTestHelper.h"

static sk_sp<SkSurface> create_protected_sksurface(GrContext* context,
                                                   skiatest::Reporter* reporter) {
    const int kW = 8;
    const int kH = 8;
    GrBackendTexture backendTex = context->createBackendTexture(
        kW, kH, kRGBA_8888_SkColorType, GrMipMapped::kNo, GrRenderable::kYes, GrProtected::kYes);
    REPORTER_ASSERT(reporter, backendTex.isValid());
    REPORTER_ASSERT(reporter, backendTex.isProtected());

    SkSurfaceProps surfaceProps = SkSurfaceProps(0, SkSurfaceProps::kLegacyFontHost_InitType);
    sk_sp<SkSurface> surface = SkSurface::MakeFromBackendTexture(
        context, backendTex, kTopLeft_GrSurfaceOrigin, 1,
        kRGBA_8888_SkColorType, nullptr, &surfaceProps);
    REPORTER_ASSERT(reporter, surface);
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
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    const int kW = 8;
    const int kH = 8;
    GrBackendTexture backendTex =
        protectedTestHelper->grContext()->createBackendTexture(
            kW, kH, kRGBA_8888_SkColorType, GrMipMapped::kNo, GrRenderable::kNo,
            GrProtected::kYes);
    REPORTER_ASSERT(reporter, backendTex.isValid());
    REPORTER_ASSERT(reporter, backendTex.isProtected());

    SkSurfaceProps surfaceProps = SkSurfaceProps(0, SkSurfaceProps::kLegacyFontHost_InitType);
    sk_sp<SkSurface> surface = SkSurface::MakeFromBackendTextureAsRenderTarget(
        protectedTestHelper->grContext(), backendTex, kTopLeft_GrSurfaceOrigin, 1,
        kRGBA_8888_SkColorType, nullptr, &surfaceProps);
    REPORTER_ASSERT(reporter, surface);

    protectedTestHelper->grContext()->deleteBackendTexture(backendTex);
}

DEF_GPUTEST(VkProtectedContext_CreateNonprotectedTextureInProtectedContext, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    const int kW = 8;
    const int kH = 8;
    GrBackendTexture backendTex =
        protectedTestHelper->grContext()->createBackendTexture(
            kW, kH, kRGBA_8888_SkColorType, GrMipMapped::kNo, GrRenderable::kNo,
            GrProtected::kNo);
    REPORTER_ASSERT(reporter, !backendTex.isValid());
}

DEF_GPUTEST(VkProtectedContext_CreateProtectedTextureInNonprotectedContext, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(false);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    const int kW = 8;
    const int kH = 8;
    GrBackendTexture backendTex =
        protectedTestHelper->grContext()->createBackendTexture(
            kW, kH, kRGBA_8888_SkColorType, GrMipMapped::kNo, GrRenderable::kNo,
            GrProtected::kYes);
    REPORTER_ASSERT(reporter, !backendTex.isValid());
}

DEF_GPUTEST(VkProtectedContext_ReadFromProtectedSurface, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    auto surface = create_protected_sksurface(protectedTestHelper->grContext(), reporter);
    REPORTER_ASSERT(reporter, surface);
    REPORTER_ASSERT(reporter, !surface->readPixels(SkImageInfo(), nullptr, 8, 0, 0));

    protectedTestHelper->grContext()->deleteBackendTexture(
        surface->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess));
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
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    auto surface = create_protected_sksurface(protectedTestHelper->grContext(), reporter);
    REPORTER_ASSERT(reporter, surface);
    AsyncContext cbContext;
    const auto image_info = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                      SkColorSpace::MakeSRGB());
    surface->asyncRescaleAndReadPixelsYUV420(kIdentity_SkYUVColorSpace, SkColorSpace::MakeSRGB(),
                                             image_info.bounds(), image_info.dimensions(),
                                             SkSurface::RescaleGamma::kSrc, kNone_SkFilterQuality,
                                             &async_callback, &cbContext);
    surface->getContext()->submit();
    while (!cbContext.fCalled) {
        surface->getCanvas()->getGrContext()->checkAsyncWorkCompletion();
    }
    REPORTER_ASSERT(reporter, !cbContext.fResult);

    protectedTestHelper->grContext()->deleteBackendTexture(
        surface->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess));
}

DEF_GPUTEST(VkProtectedContext_DrawRectangle, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    auto surface = create_protected_sksurface(protectedTestHelper->grContext(), reporter);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);

    GrFlushInfo flushInfo;
    flushInfo.fFlags = kSyncCpu_GrFlushFlag;
    surface->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    surface->getContext()->submit(true);
    protectedTestHelper->grContext()->deleteBackendTexture(
        surface->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess));
}

DEF_GPUTEST(VkProtectedContext_DrawRectangleWithAntiAlias, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    auto surface = create_protected_sksurface(protectedTestHelper->grContext(), reporter);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setAntiAlias(true);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);

    GrFlushInfo flushInfo;
    flushInfo.fFlags = kSyncCpu_GrFlushFlag;
    surface->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    surface->getContext()->submit(true);
    protectedTestHelper->grContext()->deleteBackendTexture(
        surface->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess));
}

DEF_GPUTEST(VkProtectedContext_DrawRectangleWithBlendMode, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    auto surface = create_protected_sksurface(protectedTestHelper->grContext(), reporter);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setBlendMode(SkBlendMode::kColorDodge);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);

    GrFlushInfo flushInfo;
    flushInfo.fFlags = kSyncCpu_GrFlushFlag;
    surface->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    surface->getContext()->submit(true);
    protectedTestHelper->grContext()->deleteBackendTexture(
        surface->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess));
}

DEF_GPUTEST(VkProtectedContext_DrawRectangleWithFilter, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    auto surface = create_protected_sksurface(protectedTestHelper->grContext(), reporter);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kFill_Style);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(
          SkBlurStyle::kOuter_SkBlurStyle, 1.1f));
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);

    GrFlushInfo flushInfo;
    flushInfo.fFlags = kSyncCpu_GrFlushFlag;
    surface->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    surface->getContext()->submit(true);
    protectedTestHelper->grContext()->deleteBackendTexture(
        surface->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess));
}

DEF_GPUTEST(VkProtectedContext_DrawThinPath, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    auto surface = create_protected_sksurface(protectedTestHelper->grContext(), reporter);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(.4f);
    canvas->drawPath(SkPath().moveTo(4, 4).lineTo(6, 6), paint);

    GrFlushInfo flushInfo;
    flushInfo.fFlags = kSyncCpu_GrFlushFlag;
    surface->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    surface->getContext()->submit(true);
    protectedTestHelper->grContext()->deleteBackendTexture(
        surface->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess));
}

DEF_GPUTEST(VkProtectedContext_SaveLayer, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    auto surface = create_protected_sksurface(protectedTestHelper->grContext(), reporter);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    canvas->saveLayer(nullptr, nullptr);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
    canvas->restore();

    GrFlushInfo flushInfo;
    flushInfo.fFlags = kSyncCpu_GrFlushFlag;
    surface->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    surface->getContext()->submit(true);
    protectedTestHelper->grContext()->deleteBackendTexture(
        surface->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess));
}


DEF_GPUTEST(VkProtectedContext_DrawProtectedImageOnProtectedSurface, reporter, options) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    // Create protected image.
    auto surface1 = create_protected_sksurface(protectedTestHelper->grContext(), reporter);
    REPORTER_ASSERT(reporter, surface1);
    auto image = surface1->makeImageSnapshot();
    REPORTER_ASSERT(reporter, image);

    // Create protected canvas.
    auto surface2 = create_protected_sksurface(protectedTestHelper->grContext(), reporter);
    REPORTER_ASSERT(reporter, surface2);
    SkCanvas* canvas = surface2->getCanvas();
    REPORTER_ASSERT(reporter, canvas);

    canvas->drawImage(image, 0, 0);

    GrFlushInfo flushInfo;
    flushInfo.fFlags = kSyncCpu_GrFlushFlag;
    surface1->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    surface1->getContext()->submit(true);
    protectedTestHelper->grContext()->deleteBackendTexture(
        surface1->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess));
    surface2->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    surface2->getContext()->submit(true);
    protectedTestHelper->grContext()->deleteBackendTexture(
        surface2->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess));
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Test out DDLs using a protected Vulkan context

void DDLMakeRenderTargetTestImpl(GrContext* context, skiatest::Reporter* reporter);

DEF_GPUTEST(VkProtectedContext_DDLMakeRenderTargetTest, reporter, ctxInfo) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    DDLMakeRenderTargetTestImpl(protectedTestHelper->grContext(), reporter);
}

void DDLSurfaceCharacterizationTestImpl(GrContext* context, skiatest::Reporter* reporter);

DEF_GPUTEST(VkProtectedContext_DDLSurfaceCharacterizationTest, reporter, ctxInfo) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    DDLSurfaceCharacterizationTestImpl(protectedTestHelper->grContext(), reporter);
}

#endif  // SK_SUPPORT_GPU && defined(SK_VULKAN)
