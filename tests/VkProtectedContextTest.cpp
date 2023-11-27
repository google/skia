/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a Vulkan protected memory specific test.

#include "include/core/SkTypes.h"

#if defined(SK_VULKAN)

#include "include/core/SkAlphaType.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrTypes.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/vk/VkTestHelper.h"

#if defined(SK_GANESH)
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/TextureInfo.h"
#endif

#include <memory>
#include <utility>

struct GrContextOptions;

static const int kSize = 8;

namespace {

void create_nonprotected_context(skiatest::Reporter* reporter, skiatest::TestType testType) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(testType, /* isProtected= */ false);
    REPORTER_ASSERT(reporter, helper);
}

} // anonymous namespace

DEF_GANESH_TEST(VkProtectedContext_CreateNonprotectedContext,
                reporter,
                options,
                CtsEnforcement::kNever) {
    create_nonprotected_context(reporter, skiatest::TestType::kGanesh);
}

#if defined(SK_GRAPHITE)

DEF_GRAPHITE_TEST(VkProtectedContext_CreateNonprotectedContext_Graphite,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    create_nonprotected_context(reporter, skiatest::TestType::kGraphite);
}

#endif

DEF_GANESH_TEST(VkProtectedContext_CreateProtectedContext,
                reporter,
                options,
                CtsEnforcement::kNever) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(skiatest::TestType::kGanesh,
                                                              /* isProtected= */ true);
}

DEF_GRAPHITE_TEST(VkProtectedContext_CreateProtectedContext_Graphite,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(skiatest::TestType::kGraphite,
                                                              /* isProtected= */ true);
}

namespace {

void create_protected_surface(skiatest::Reporter* reporter, skiatest::TestType testType) {

    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(testType, /* isProtected= */ true);
    if (!helper) {
        return;
    }

    REPORTER_ASSERT(reporter, helper->isValid());

    sk_sp<SkSurface> textureable = helper->createSurface({ kSize, kSize },
                                                         /* textureable= */ true,
                                                         /* isProtected= */ true);
    REPORTER_ASSERT(reporter, textureable);

    sk_sp<SkSurface> untextureable = helper->createSurface({ kSize, kSize },
                                                           /* textureable= */ false,
                                                           /* isProtected= */ true);
    REPORTER_ASSERT(reporter, untextureable);
}

} // anonymous namespace

DEF_GANESH_TEST(VkProtectedContext_CreateProtectedSkSurface,
                reporter,
                options,
                CtsEnforcement::kNever) {
    create_protected_surface(reporter, skiatest::TestType::kGanesh);
}

DEF_GRAPHITE_TEST(VkProtectedContext_CreateProtectedSkSurface_Graphite,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    create_protected_surface(reporter, skiatest::TestType::kGraphite);
}

namespace {

void create_backend_texture_ganesh(skiatest::Reporter* reporter,
                                   bool contextIsProtected,
                                   bool beTexIsProtected) {

    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(skiatest::TestType::kGanesh,
                                                              contextIsProtected);
    if (!helper) {
        return;
    }

    REPORTER_ASSERT(reporter, helper->isValid());
    GrDirectContext* dContext = helper->directContext();

    GrBackendTexture backendTex = dContext->createBackendTexture(kSize,
                                                                 kSize,
                                                                 kRGBA_8888_SkColorType,
                                                                 skgpu::Mipmapped::kNo,
                                                                 GrRenderable::kNo,
                                                                 GrProtected(beTexIsProtected));

    REPORTER_ASSERT(reporter, backendTex.isValid() == (contextIsProtected == beTexIsProtected));
    if (backendTex.isValid()) {
        REPORTER_ASSERT(reporter, backendTex.isProtected() == beTexIsProtected);
    }

    dContext->deleteBackendTexture(backendTex);
}

} // anonymous namespace

DEF_GANESH_TEST(VkProtectedContext_CreateBackendTextures,
                reporter,
                options,
                CtsEnforcement::kNever) {
    for (bool contextIsProtected : { true, false }) {
        for (bool beTexIsProtected : { true, false }) {
            create_backend_texture_ganesh(reporter, contextIsProtected, beTexIsProtected);
        }
    }
}

#if defined(SK_GRAPHITE)

namespace {

void create_backend_texture_graphite(skiatest::Reporter* reporter,
                                     bool contextIsProtected,
                                     bool beTexIsProtected) {
    using namespace skgpu::graphite;

    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(skiatest::TestType::kGraphite,
                                                              contextIsProtected);
    if (!helper) {
        return;
    }

    REPORTER_ASSERT(reporter, helper->isValid());

    VulkanTextureInfo vkTextureInfo;
    vkTextureInfo.fFlags = beTexIsProtected ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;
    vkTextureInfo.fFormat = VK_FORMAT_R8G8B8A8_UNORM;
    vkTextureInfo.fImageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    TextureInfo textureInfo(vkTextureInfo);

    BackendTexture backendTex = helper->recorder()->createBackendTexture({ kSize, kSize },
                                                                         textureInfo);
    REPORTER_ASSERT(reporter, backendTex.isValid() == (contextIsProtected == beTexIsProtected));
    if (backendTex.isValid()) {
        REPORTER_ASSERT(reporter,
                        backendTex.info().isProtected() == skgpu::Protected(beTexIsProtected));
    }

    helper->recorder()->deleteBackendTexture(backendTex);
}

} // anonymous namespace

DEF_GRAPHITE_TEST(VkProtectedContext_CreateBackendTextures_Graphite,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    for (bool contextIsProtected : { true, false }) {
        for (bool beTexIsProtected : { true, false }) {
            create_backend_texture_graphite(reporter, contextIsProtected, beTexIsProtected);
        }
    }
}

#endif // SK_GRAPHITE

DEF_GANESH_TEST(VkProtectedContext_ReadFromProtectedSurface,
                reporter,
                options,
                CtsEnforcement::kNever) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(skiatest::TestType::kGanesh,
                                                              /* isProtected= */ true);
    if (!helper) {
        return;
    }

    REPORTER_ASSERT(reporter, helper->isValid());

    sk_sp<SkSurface> surface = helper->createSurface({ kSize, kSize },
                                                     /* textureable= */ true,
                                                     /* isProtected= */ true);
    REPORTER_ASSERT(reporter, surface);
    REPORTER_ASSERT(reporter, !surface->readPixels(SkImageInfo(), nullptr, 8, 0, 0));
}

namespace {

struct AsyncContext {
    bool fCalled = false;
    std::unique_ptr<const SkSurface::AsyncReadResult> fResult;
};

void async_callback(void* c, std::unique_ptr<const SkSurface::AsyncReadResult> result) {
    auto context = static_cast<AsyncContext*>(c);
    context->fResult = std::move(result);
    context->fCalled = true;
}

void async_read_from_protected_surface(skiatest::Reporter* reporter,
                                              skiatest::TestType testType) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(testType, /* isProtected= */ true);
    if (!helper) {
        return;
    }

    REPORTER_ASSERT(reporter, helper->isValid());

    sk_sp<SkSurface> surface = helper->createSurface({ kSize, kSize },
                                                     /* textureable= */ true,
                                                     /* isProtected= */ true);
    REPORTER_ASSERT(reporter, surface);
    AsyncContext cbContext;
    const SkImageInfo imageInfo = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType,
                                                    kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
    surface->asyncRescaleAndReadPixelsYUV420(kIdentity_SkYUVColorSpace, SkColorSpace::MakeSRGB(),
                                             imageInfo.bounds(), imageInfo.dimensions(),
                                             SkSurface::RescaleGamma::kSrc,
                                             SkSurface::RescaleMode::kNearest,
                                             &async_callback, &cbContext);

    helper->submitAndWaitForCompletion(&cbContext.fCalled);
    REPORTER_ASSERT(reporter, !cbContext.fResult);
}

}  // anonymous namespace

DEF_GANESH_TEST(VkProtectedContext_AsyncReadFromProtectedSurface,
                reporter,
                options,
                CtsEnforcement::kNever) {
    async_read_from_protected_surface(reporter, skiatest::TestType::kGanesh);
}

DEF_GRAPHITE_TEST(VkProtectedContext_AsyncReadFromProtectedSurface_Graphite,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    async_read_from_protected_surface(reporter, skiatest::TestType::kGraphite);
}

namespace {

void draw_rectangle(skiatest::Reporter* reporter, skiatest::TestType testType) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(testType, /* isProtected= */ true);
    if (!helper) {
        return;
    }

    REPORTER_ASSERT(reporter, helper->isValid());

    sk_sp<SkSurface> surface = helper->createSurface({ kSize, kSize },
                                                     /* textureable= */ true,
                                                     /* isProtected= */ true);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
}

}  // anonymous namespace

DEF_GANESH_TEST(VkProtectedContext_DrawRectangle,
                reporter,
                options,
                CtsEnforcement::kNever) {
    draw_rectangle(reporter, skiatest::TestType::kGanesh);
}

DEF_GRAPHITE_TEST(VkProtectedContext_DrawRectangle_Graphite,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    draw_rectangle(reporter, skiatest::TestType::kGraphite);
}

namespace {

void draw_rectangle_with_aa(skiatest::Reporter* reporter, skiatest::TestType testType) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(testType, /* isProtected= */ true);
    if (!helper) {
        return;
    }

    REPORTER_ASSERT(reporter, helper->isValid());

    sk_sp<SkSurface> surface = helper->createSurface({ kSize, kSize },
                                                     /* textureable= */ true,
                                                     /* isProtected= */ true);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setAntiAlias(true);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
}

}  // anonymous namespace

DEF_GANESH_TEST(VkProtectedContext_DrawRectangleWithAntiAlias,
                reporter,
                options,
                CtsEnforcement::kNever) {
    draw_rectangle_with_aa(reporter, skiatest::TestType::kGanesh);
}

DEF_GRAPHITE_TEST(VkProtectedContext_DrawRectangleWithAntiAlias_Graphite,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    draw_rectangle_with_aa(reporter, skiatest::TestType::kGraphite);
}

namespace {

void draw_rectangle_with_blendmode(skiatest::Reporter* reporter, skiatest::TestType testType) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(testType, /* isProtected= */ true);
    if (!helper) {
        return;
    }

    REPORTER_ASSERT(reporter, helper->isValid());

    sk_sp<SkSurface> surface = helper->createSurface({ kSize, kSize },
                                                     /* textureable= */ true,
                                                     /* isProtected= */ true);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setBlendMode(SkBlendMode::kColorDodge);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
}

}  // anonymous namespace

DEF_GANESH_TEST(VkProtectedContext_DrawRectangleWithBlendMode,
                reporter,
                options,
                CtsEnforcement::kNever) {
    draw_rectangle_with_blendmode(reporter, skiatest::TestType::kGanesh);
}

DEF_GRAPHITE_TEST(VkProtectedContext_DrawRectangleWithBlendMode_Graphite,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    draw_rectangle_with_blendmode(reporter, skiatest::TestType::kGraphite);
}

namespace {

void draw_rectangle_with_filter(skiatest::Reporter* reporter, skiatest::TestType testType) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(testType, /* isProtected= */ true);
    if (!helper) {
        return;
    }

    REPORTER_ASSERT(reporter, helper->isValid());

    sk_sp<SkSurface> surface = helper->createSurface({ kSize, kSize },
                                                     /* textureable= */ true,
                                                     /* isProtected= */ true);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kFill_Style);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kOuter_SkBlurStyle, 30.0f));
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
}

}  // anonymous namespace

DEF_GANESH_TEST(VkProtectedContext_DrawRectangleWithFilter,
                reporter,
                options,
                CtsEnforcement::kNever) {
    draw_rectangle_with_filter(reporter, skiatest::TestType::kGanesh);
}

DEF_GRAPHITE_TEST(VkProtectedContext_DrawRectangleWithFilter_Graphite,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    draw_rectangle_with_filter(reporter, skiatest::TestType::kGraphite);
}

namespace {

void draw_thin_path(skiatest::Reporter* reporter, skiatest::TestType testType) {
    constexpr bool kIsProtected = true;

    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(testType, kIsProtected);
    if (!helper) {
        return;
    }

    REPORTER_ASSERT(reporter, helper->isValid());

    sk_sp<SkSurface> surface = helper->createSurface({ kSize, kSize },
                                                     /* textureable= */ true,
                                                     kIsProtected);
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

}  // anonymous namespace

DEF_GANESH_TEST(VkProtectedContext_DrawThinPath,
                reporter,
                options,
                CtsEnforcement::kNever) {
    draw_thin_path(reporter, skiatest::TestType::kGanesh);
}

DEF_GRAPHITE_TEST(VkProtectedContext_DrawThinPath_Graphite,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    draw_thin_path(reporter, skiatest::TestType::kGraphite);
}

namespace {

void save_layer(skiatest::Reporter* reporter, skiatest::TestType testType) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(testType, /* isProtected= */ true);
    if (!helper) {
        return;
    }

    REPORTER_ASSERT(reporter, helper->isValid());

    sk_sp<SkSurface> surface = helper->createSurface({ kSize, kSize },
                                                     /* textureable= */ true,
                                                     /* isProtected= */ true);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    canvas->saveLayer(nullptr, nullptr);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
    canvas->restore();
}

}  // anonymous namespace

DEF_GANESH_TEST(VkProtectedContext_SaveLayer,
                reporter,
                options,
                CtsEnforcement::kNever) {
    save_layer(reporter, skiatest::TestType::kGanesh);
}

DEF_GRAPHITE_TEST(VkProtectedContext_SaveLayer_Graphite,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    save_layer(reporter, skiatest::TestType::kGraphite);
}

namespace {

void draw_protected_image_on_protected_surface(skiatest::Reporter* reporter,
                                               skiatest::TestType testType) {
    constexpr bool kIsProtected = true;

    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(testType, kIsProtected);
    if (!helper) {
        return;
    }

    REPORTER_ASSERT(reporter, helper->isValid());

    // Create protected image.
    sk_sp<SkSurface> surface1 = helper->createSurface({ kSize, kSize },
                                                      /* textureable= */ true,
                                                      kIsProtected);
    REPORTER_ASSERT(reporter, surface1);
    sk_sp<SkImage> image = surface1->makeImageSnapshot();
    REPORTER_ASSERT(reporter, image);
    REPORTER_ASSERT(reporter, image->isProtected() == kIsProtected);

    // Create protected canvas.
    sk_sp<SkSurface> surface2 = helper->createSurface({ kSize, kSize },
                                                      /* textureable= */ true,
                                                      kIsProtected);
    REPORTER_ASSERT(reporter, surface2);
    SkCanvas* canvas = surface2->getCanvas();
    REPORTER_ASSERT(reporter, canvas);

    canvas->drawImage(image, 0, 0);
}

}  // anonymous namespace

DEF_GANESH_TEST(VkProtectedContext_DrawProtectedImageOnProtectedSurface,
                reporter,
                options,
                CtsEnforcement::kNever) {
    draw_protected_image_on_protected_surface(reporter, skiatest::TestType::kGanesh);
}

DEF_GRAPHITE_TEST(VkProtectedContext_DrawProtectedImageOnProtectedSurface_Graphite,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    draw_protected_image_on_protected_surface(reporter, skiatest::TestType::kGraphite);
}

#endif  // SK_VULKAN
