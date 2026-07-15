/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkSurface.h"
#include "include/utils/SkCustomTypeface.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <functional>
#include <type_traits>

#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#endif

template <typename T>
static void disable_sdft_options(T* options) {
    options->fMinDistanceFieldFontSize = 384.f;
    options->fGlyphsAsPathsFontSize = 384.f;
    options->fSupportBilerpFromGlyphAtlas = true;
    if constexpr (std::is_same_v<T, GrContextOptions>) {
        options->fMaxTextureSizeOverride = 1024;
    } else {
        options->fOptionsPriv->fMaxTextureSizeOverride = 1024;
    }
}

static void test_direct_mask_limit(skiatest::Reporter* reporter,
                                   SkSurface* surface,
                                   const std::function<void()>& submit) {
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorBLACK);

    SkCustomTypefaceBuilder builder;
    SkPath path = SkPath::Rect(SkRect::MakeLTRB(0.f, -1.f, 1.f, 0.f));
    builder.setGlyph(1, 1.0f, path);

    SkFont font(builder.detach(), 253);
    font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

    SkPaint paint;
    paint.setColor(SK_ColorWHITE);

    SkGlyphID glyph = 1;
    SkPoint position = {0, 0};
    canvas->drawGlyphs({&glyph, 1}, {&position, 1}, {100, 300}, font, paint);

    submit();

    SkBitmap bitmap;
    bitmap.allocN32Pixels(512, 512);
    REPORTER_ASSERT(reporter, surface->readPixels(bitmap, 0, 0));

    bool hasWhitePixel = false;
    for (int y = 0; y < 512; ++y) {
        for (int x = 0; x < 512; ++x) {
            if (bitmap.getColor(x, y) != SK_ColorBLACK) {
                hasWhitePixel = true;
                break;
            }
        }
        if (hasWhitePixel) {
            break;
        }
    }
    REPORTER_ASSERT(reporter, hasWhitePixel, "Draw failed (all pixels are black)");
}

#if defined(SK_GANESH)
DEF_GANESH_TEST_FOR_CONTEXTS(DirectMaskLimitTest_Ganesh,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             disable_sdft_options<GrContextOptions>,
                             CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();
    if (dContext->supportsProtectedContent()) {
        return;
    }
    auto surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo,
                                            SkImageInfo::MakeN32Premul(512, 512));
    if (!surface) {
        return;
    }
    test_direct_mask_limit(reporter, surface.get(), [&] { dContext->flushAndSubmit(); });
}
#endif // SK_GANESH

#if defined(SK_GRAPHITE)
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(DirectMaskLimitTest_Graphite,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           testContext,
                                           /* anonymous options */,
                                           disable_sdft_options<skgpu::graphite::ContextOptions>,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    if (context->supportsProtectedContent()) {
        return;
    }
    auto recorder = context->makeRecorder();
    if (!recorder) {
        return;
    }
    auto surface = SkSurfaces::RenderTarget(recorder.get(),
                                            SkImageInfo::MakeN32Premul(512, 512));
    if (!surface) {
        return;
    }
    test_direct_mask_limit(reporter, surface.get(), [&] {
        auto recording = recorder->snap();
        context->insertRecording({recording.get()});
        context->submit(skgpu::graphite::SyncToCpu::kYes);
    });
}
#endif // SK_GRAPHITE
