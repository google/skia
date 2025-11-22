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
#include "include/core/SkBitmap.h"
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
#include "include/gpu/ganesh/GrTypes.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/vk/VkTestHelper.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#endif

#include <memory>
#include <utility>

struct GrContextOptions;

static const int kSize = 8;

#if defined(SK_GANESH)
#define GANESH_TEST(NAME) DEF_GANESH_TEST(VkProtectedContext_##NAME,  \
                                          reporter,                   \
                                          options,                    \
                                          CtsEnforcement::kNever) {   \
    run_test_##NAME(reporter, skiatest::TestType::kGanesh);           \
}

#else
#define GANESH_TEST(NAME)
#endif

#if defined(SK_GRAPHITE)
#define GRAPHITE_TEST(NAME) DEF_GRAPHITE_TEST(VkProtectedContext_##NAME##_Graphite,  \
                                              reporter,                              \
                                              CtsEnforcement::kApiLevel_202504) {    \
    run_test_##NAME(reporter, skiatest::TestType::kGraphite);                        \
}
#else
#define GRAPHITE_TEST(NAME)
#endif

#define DEF_GANESH_AND_GRAPHITE_TEST(NAME, REPORTER, TEST_TYPE)                        \
    void run_test_##NAME(skiatest::Reporter*, skiatest::TestType);                     \
    GANESH_TEST(NAME)                                                                  \
    GRAPHITE_TEST(NAME)                                                                \
    void run_test_##NAME(skiatest::Reporter* REPORTER, skiatest::TestType TEST_TYPE)   \

namespace {

std::unique_ptr<VkTestHelper> create_context(skiatest::TestType testType, bool isProtected = true) {
    return VkTestHelper::Make(testType, isProtected);
}

} // anonymous namespace

DEF_GANESH_AND_GRAPHITE_TEST(CreateNonprotectedContext, reporter, testType) {
    REPORTER_ASSERT(reporter, create_context(testType, false));
}

DEF_GANESH_AND_GRAPHITE_TEST(CreateProtectedContext, reporter, testType) {
    create_context(testType);
}


DEF_GANESH_AND_GRAPHITE_TEST(CreateProtectedSkSurface, reporter, testType) {
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

#if defined(SK_GANESH)
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
#endif

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
    TextureInfo textureInfo = TextureInfos::MakeVulkan(vkTextureInfo);

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
                  CtsEnforcement::kApiLevel_202504) {
    for (bool contextIsProtected : { true, false }) {
        for (bool beTexIsProtected : { true, false }) {
            create_backend_texture_graphite(reporter, contextIsProtected, beTexIsProtected);
        }
    }
}

#endif // SK_GRAPHITE

#if defined(SK_GANESH)
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

// Graphite does not perform Copy-on-Write which is why there is no DEF_GRAPHITE_TEST correlate
DEF_GANESH_TEST(VkProtectedContext_CopyOnWrite,
                reporter,
                options,
                CtsEnforcement::kNever) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(skiatest::TestType::kGanesh,
                                                              /* isProtected= */ true);
    if (!helper) {
        return;
    }

    REPORTER_ASSERT(reporter, helper->isValid());

    GrDirectContext* dContext = helper->directContext();

    SkImageInfo ii = SkImageInfo::Make({ kSize, kSize },
                                       kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);

    // We can't use VkTestHelper::createSurface here bc that will wrap a backend
    // texture which blocks the copy-on-write-behavior
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(dContext,
                                                        skgpu::Budgeted::kNo,
                                                        ii,
                                                        /* sampleCount= */ 1,
                                                        kBottomLeft_GrSurfaceOrigin,
                                                        /* surfaceProps= */ nullptr,
                                                        /* shouldCreateWithMips= */ false,
                                                        /* isProtected= */ true);


    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);

    sk_sp<SkImage> imageBefore = surface->makeImageSnapshot();
    REPORTER_ASSERT(reporter, imageBefore);
    REPORTER_ASSERT(reporter, imageBefore->isProtected());

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);

    sk_sp<SkImage> imageAfter = surface->makeImageSnapshot();
    REPORTER_ASSERT(reporter, imageAfter);
    REPORTER_ASSERT(reporter, imageAfter->isProtected());

    REPORTER_ASSERT(reporter, imageBefore != imageAfter);

    SkBitmap readback;
    readback.allocPixels(imageAfter->imageInfo());
    REPORTER_ASSERT(reporter, !imageAfter->readPixels(dContext, readback.pixmap(), 0, 0));
}
#endif

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

}  // anonymous namespace

DEF_GANESH_AND_GRAPHITE_TEST(AsyncReadFromProtectedSurface, reporter, testType) {
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
    const SkImageInfo imageInfo = SkImageInfo::Make(6, 6, kRGBA_8888_SkColorType,
                                                    kPremul_SkAlphaType, SkColorSpace::MakeSRGB());

    if (testType == skiatest::TestType::kGanesh) {
        surface->asyncRescaleAndReadPixelsYUV420(kIdentity_SkYUVColorSpace,
                                                 SkColorSpace::MakeSRGB(),
                                                 imageInfo.bounds(),
                                                 imageInfo.dimensions(),
                                                 SkSurface::RescaleGamma::kSrc,
                                                 SkSurface::RescaleMode::kNearest,
                                                 &async_callback,
                                                 &cbContext);
    }
#if defined(SK_GRAPHITE)
    else {
        // Graphite deprecates SkSurface::asyncRescaleAndReadPixelsYUVA420 in favor of
        // Context::asyncRescaleAndReadPixelsYUV420.
        skgpu::graphite::Context* context = helper->context();

        context->asyncRescaleAndReadPixelsYUV420(surface.get(),
                                                 kIdentity_SkYUVColorSpace,
                                                 SkColorSpace::MakeSRGB(),
                                                 imageInfo.bounds(),
                                                 imageInfo.dimensions(),
                                                 SkSurface::RescaleGamma::kSrc,
                                                 SkSurface::RescaleMode::kNearest,
                                                 &async_callback,
                                                 &cbContext);
    }
#endif

    helper->submitAndWaitForCompletion(&cbContext.fCalled);
    REPORTER_ASSERT(reporter, !cbContext.fResult);
}

DEF_GANESH_AND_GRAPHITE_TEST(DrawRectangle, reporter, testType) {
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

DEF_GANESH_AND_GRAPHITE_TEST(DrawRectangleWithAntiAlias, reporter, testType) {
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


DEF_GANESH_AND_GRAPHITE_TEST(DrawRectangleWithBlendMode, reporter, testType) {
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

DEF_GANESH_AND_GRAPHITE_TEST(DrawRectangleWithFilter, reporter, testType) {
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

DEF_GANESH_AND_GRAPHITE_TEST(DrawThinPath, reporter, testType) {
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
    canvas->drawPath(SkPath::Line({4, 4}, {6, 6}), paint);
}

DEF_GANESH_AND_GRAPHITE_TEST(SaveLayer, reporter, testType) {
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


DEF_GANESH_AND_GRAPHITE_TEST(DrawProtectedImageOnProtectedSurface, reporter, testType) {
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

#endif  // SK_VULKAN
