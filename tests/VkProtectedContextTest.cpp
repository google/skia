/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a Vulkan protected memory specific test.

#include "include/core/SkTypes.h"

#if defined(SK_GANESH) && defined(SK_VULKAN)

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
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/ProtectedUtils.h"
#include "tools/gpu/vk/VkTestHelper.h"

#include <memory>
#include <utility>

struct GrContextOptions;

static const int kSize = 8;

DEF_GANESH_TEST(VkProtectedContext_CreateNonprotectedContext,
                reporter,
                options,
                CtsEnforcement::kNever) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(skiatest::TestType::kGanesh,
                                                              /* isProtected= */ false);
    REPORTER_ASSERT(reporter, helper);
}

DEF_GANESH_TEST(VkProtectedContext_CreateProtectedContext,
                reporter,
                options,
                CtsEnforcement::kNever) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(skiatest::TestType::kGanesh,
                                                              /* isProtected= */ true);
}

DEF_GANESH_TEST(VkProtectedContext_CreateProtectedSkSurface,
                reporter,
                options,
                CtsEnforcement::kNever) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(skiatest::TestType::kGanesh,
                                                              /* isProtected= */ true);
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

DEF_GANESH_TEST(VkProtectedContext_CreateNonprotectedTextureInProtectedContext,
                reporter,
                options,
                CtsEnforcement::kNever) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(skiatest::TestType::kGanesh,
                                                              /* isProtected= */ true);
    if (!helper) {
        return;
    }

    REPORTER_ASSERT(reporter, helper->isValid());

    GrBackendTexture backendTex =
            helper->directContext()->createBackendTexture(kSize,
                                                          kSize,
                                                          kRGBA_8888_SkColorType,
                                                          skgpu::Mipmapped::kNo,
                                                          GrRenderable::kNo,
                                                          GrProtected::kNo);
    REPORTER_ASSERT(reporter, !backendTex.isValid());
}

DEF_GANESH_TEST(VkProtectedContext_CreateProtectedTextureInNonprotectedContext,
                reporter,
                options,
                CtsEnforcement::kNever) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(skiatest::TestType::kGanesh,
                                                              /* isProtected= */ false);
    if (!helper) {
        return;
    }

    REPORTER_ASSERT(reporter, helper->isValid());

    GrBackendTexture backendTex =
            helper->directContext()->createBackendTexture(kSize,
                                                          kSize,
                                                          kRGBA_8888_SkColorType,
                                                          skgpu::Mipmapped::kNo,
                                                          GrRenderable::kNo,
                                                          GrProtected::kYes);
    REPORTER_ASSERT(reporter, !backendTex.isValid());
}

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

static void async_callback(void* c, std::unique_ptr<const SkSurface::AsyncReadResult> result) {
    auto context = static_cast<AsyncContext*>(c);
    context->fResult = std::move(result);
    context->fCalled = true;
}

}  // anonymous namespace

DEF_GANESH_TEST(VkProtectedContext_AsyncReadFromProtectedSurface,
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

DEF_GANESH_TEST(VkProtectedContext_DrawRectangle, reporter, options, CtsEnforcement::kNever) {
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
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
}

DEF_GANESH_TEST(VkProtectedContext_DrawRectangleWithAntiAlias,
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
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setAntiAlias(true);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
}

DEF_GANESH_TEST(VkProtectedContext_DrawRectangleWithBlendMode,
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
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setBlendMode(SkBlendMode::kColorDodge);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
}

DEF_GANESH_TEST(VkProtectedContext_DrawRectangleWithFilter,
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
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kFill_Style);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(
          SkBlurStyle::kOuter_SkBlurStyle, 1.1f));
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
}

DEF_GANESH_TEST(VkProtectedContext_DrawThinPath, reporter, options, CtsEnforcement::kNever) {
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
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(.4f);
    canvas->drawPath(SkPath().moveTo(4, 4).lineTo(6, 6), paint);
}

DEF_GANESH_TEST(VkProtectedContext_SaveLayer, reporter, options, CtsEnforcement::kNever) {
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
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    canvas->saveLayer(nullptr, nullptr);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
    canvas->restore();
}

DEF_GANESH_TEST(VkProtectedContext_DrawProtectedImageOnProtectedSurface,
                reporter,
                options,
                CtsEnforcement::kNever) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(skiatest::TestType::kGanesh,
                                                              /* isProtected= */ true);
    if (!helper) {
        return;
    }

    REPORTER_ASSERT(reporter, helper->isValid());

    // Create protected image.
    sk_sp<SkSurface> surface1 = helper->createSurface({ kSize, kSize },
                                                      /* textureable= */ true,
                                                      /* isProtected= */ true);
    REPORTER_ASSERT(reporter, surface1);
    sk_sp<SkImage> image = surface1->makeImageSnapshot();
    REPORTER_ASSERT(reporter, image);

    // Create protected canvas.
    sk_sp<SkSurface> surface2 = helper->createSurface({ kSize, kSize },
                                                      /* textureable= */ true,
                                                      /* isProtected= */ true);
    REPORTER_ASSERT(reporter, surface2);
    SkCanvas* canvas = surface2->getCanvas();
    REPORTER_ASSERT(reporter, canvas);

    canvas->drawImage(image, 0, 0);
}

#endif  // defined(SK_GANESH) && defined(SK_VULKAN)
