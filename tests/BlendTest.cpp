/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <functional>
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkSurface.h"
#include "SkTaskGroup.h"
#include "SkUtils.h"
#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrResourceProvider.h"
#include "GrSurfaceContext.h"
#include "GrSurfaceProxy.h"
#include "GrTexture.h"
#endif

struct Results { int diffs, diffs_0x00, diffs_0xff, diffs_by_1; };

static bool acceptable(const Results& r) {
#if 0
    SkDebugf("%d diffs, %d at 0x00, %d at 0xff, %d off by 1, all out of 65536\n",
             r.diffs, r.diffs_0x00, r.diffs_0xff, r.diffs_by_1);
#endif
    return r.diffs_by_1 == r.diffs   // never off by more than 1
        && r.diffs_0x00 == 0         // transparent must stay transparent
        && r.diffs_0xff == 0;        // opaque must stay opaque
}

template <typename Fn>
static Results test(Fn&& multiply) {
    Results r = { 0,0,0,0 };
    for (int x = 0; x < 256; x++) {
    for (int y = 0; y < 256; y++) {
        int p = multiply(x, y),
            ideal = (x*y+127)/255;
        if (p != ideal) {
            r.diffs++;
            if (x == 0x00 || y == 0x00) { r.diffs_0x00++; }
            if (x == 0xff || y == 0xff) { r.diffs_0xff++; }
            if (SkTAbs(ideal - p) == 1) { r.diffs_by_1++; }
        }
    }}
    return r;
}

DEF_TEST(Blend_byte_multiply, r) {
    // These are all temptingly close but fundamentally broken.
    int (*broken[])(int, int) = {
        [](int x, int y) { return (x*y)>>8; },
        [](int x, int y) { return (x*y+128)>>8; },
        [](int x, int y) { y += y>>7; return (x*y)>>8; },
    };
    for (auto multiply : broken) { REPORTER_ASSERT(r, !acceptable(test(multiply))); }

    // These are fine to use, but not perfect.
    int (*fine[])(int, int) = {
        [](int x, int y) { return (x*y+x)>>8; },
        [](int x, int y) { return (x*y+y)>>8; },
        [](int x, int y) { return (x*y+255)>>8; },
        [](int x, int y) { y += y>>7; return (x*y+128)>>8; },
    };
    for (auto multiply : fine) { REPORTER_ASSERT(r, acceptable(test(multiply))); }

    // These are pefect.
    int (*perfect[])(int, int) = {
        [](int x, int y) { return (x*y+127)/255; },  // Duh.
        [](int x, int y) { int p = (x*y+128); return (p+(p>>8))>>8; },
        [](int x, int y) { return ((x*y+128)*257)>>16; },
    };
    for (auto multiply : perfect) { REPORTER_ASSERT(r, test(multiply).diffs == 0); }
}

#if SK_SUPPORT_GPU
namespace {
static sk_sp<SkSurface> create_gpu_surface_backend_texture_as_render_target(
        GrContext* context, int sampleCnt, GrPixelConfig config, GrBackendObject* outTexture) {
    const int kWidth = 10;
    const int kHeight = 10;
    std::unique_ptr<uint32_t[]> pixels(new uint32_t[kWidth * kHeight]);
    sk_memset32(pixels.get(), SK_ColorBLACK, kWidth * kHeight);
    GrBackendTextureDesc desc;
    desc.fConfig = config;
    desc.fWidth = kWidth;
    desc.fHeight = kHeight;
    desc.fFlags = kRenderTarget_GrBackendTextureFlag;
    desc.fTextureHandle = context->getGpu()->createTestingOnlyBackendTexture(pixels.get(), kWidth,
                                                                             kHeight, config, true);
    desc.fSampleCnt = sampleCnt;
    sk_sp<SkSurface> surface =
            SkSurface::MakeFromBackendTextureAsRenderTarget(context, desc, nullptr);
    if (!surface) {
        context->getGpu()->deleteTestingOnlyBackendTexture(desc.fTextureHandle);
        return nullptr;
    }
    *outTexture = desc.fTextureHandle;
    return surface;
}
}

// Tests blending to a surface with no texture available.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ES2BlendWithNoTexture, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    for (int sampleCnt : {0, 4, 8}) {
        GrBackendObject textureObject;
        // BGRA forces a framebuffer blit on ES2.
        sk_sp<SkSurface> surface = create_gpu_surface_backend_texture_as_render_target(
                context, sampleCnt, kBGRA_8888_GrPixelConfig, &textureObject);

        if (!surface && sampleCnt > 0) {
            // Some platforms don't support MSAA.
            continue;
        }
        REPORTER_ASSERT(reporter, !!surface);

        SkCanvas* canvas = surface->getCanvas();
        SkPaint black_paint;
        black_paint.setColor(SK_ColorBLACK);
        canvas->drawRect(SkRect::MakeXYWH(0, 0, 10, 10), black_paint);

        // Blend with hard light, which will force a readback.
        SkPaint white_paint;
        white_paint.setColor(SK_ColorWHITE);
        white_paint.setBlendMode(SkBlendMode::kHardLight);
        canvas->drawRect(SkRect::MakeXYWH(5, 5, 5, 5), white_paint);

        // Read back the pixel at 6,6 and make sure it is not black.
        SkBitmap bitmap;
        REPORTER_ASSERT(reporter, bitmap.tryAllocN32Pixels(10, 10));
        bitmap.lockPixels();
        REPORTER_ASSERT(reporter, surface->readPixels(bitmap.info(), bitmap.getPixels(),
                                                      bitmap.rowBytes(), 0, 0));

        REPORTER_ASSERT(reporter,
                        *((uint32_t*)bitmap.getPixels() + 3 * bitmap.width() + 3) == SK_ColorBLACK);
        REPORTER_ASSERT(reporter,
                        *((uint32_t*)bitmap.getPixels() + 7 * bitmap.width() + 7) != SK_ColorBLACK);

        bitmap.unlockPixels();
        surface.reset();
        context->getGpu()->deleteTestingOnlyBackendTexture(textureObject);
    }
}
#endif
