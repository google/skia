/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/SkTDArray.h"
#include "include/private/chromium/Slug.h"
#include "src/gpu/MaskFormat.h"
#include "src/text/gpu/SlugImpl.h"
#include "src/text/gpu/VertexFiller.h"
#include "src/utils/SkFloatUtils.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/text/gpu/TextBlobTools.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#endif

#include <cstdint>
#include <cstring>

struct GrContextOptions;

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Slug_empty,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    auto surface(SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info));
    auto canvas = surface->getCanvas();

    static const char* kText = " ";
    auto typeface = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
    SkFont font(typeface);
    size_t txtLen = strlen(kText);
    int glyphCount = font.countText(kText, txtLen, SkTextEncoding::kUTF8);

    SkTDArray<SkGlyphID> glyphs;
    glyphs.append(glyphCount);
    font.textToGlyphs(kText, txtLen, SkTextEncoding::kUTF8, glyphs);

    SkTextBlobBuilder builder;

    font.setSubpixel(true);
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setTypeface(typeface);
    font.setSize(16);

    const SkTextBlobBuilder::RunBuffer& buf = builder.allocRun(font, glyphs.size(), 0, 0);
    memcpy(buf.glyphs, glyphs.begin(), glyphs.size() * sizeof(SkGlyphID));
    auto blob = builder.make();

    SkPaint p;
    p.setAntiAlias(true);
    sk_sp<sktext::gpu::Slug> slug = sktext::gpu::Slug::ConvertBlob(canvas, *blob, {10, 10}, p);
    REPORTER_ASSERT(reporter, slug == nullptr);
}

static void set_sdf_options(GrContextOptions* options) {
    options->fMinDistanceFieldFontSize = 4;
    options->fGlyphsAsPathsFontSize = 256;
    options->fSupportBilerpFromGlyphAtlas = true;
}

DEF_GANESH_TEST_FOR_CONTEXTS(Slug_b520113415,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             set_sdf_options,
                             CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();

    SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    auto surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    REPORTER_ASSERT(reporter, surface);
    auto canvas = surface->getCanvas();

    // This matrix is big enough such that with the font below...
    SkMatrix canvasMatrix = SkMatrix::Scale(1.2345f, 6.7890f);

    canvas->save();
    canvas->setMatrix(canvasMatrix);

    //  ... it will force SDFT (6.789 x 24 = ~162.9).
    auto typeface = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
    SkFont font(typeface);
    font.setSubpixel(true);
    font.setSize(24);

    static const char* kText = "A";
    int glyphCount = font.countText(kText, 1, SkTextEncoding::kUTF8);
    SkTDArray<SkGlyphID> glyphs;
    glyphs.append(glyphCount);
    font.textToGlyphs(kText, 1, SkTextEncoding::kUTF8, glyphs);

    SkTextBlobBuilder builder;
    const SkTextBlobBuilder::RunBuffer& buf = builder.allocRun(font, glyphs.size(), 0, 0);
    memcpy(buf.glyphs, glyphs.begin(), glyphs.size() * sizeof(SkGlyphID));
    auto blob = builder.make();

    SkPaint paint;
    paint.setAntiAlias(true);

    sk_sp<sktext::gpu::Slug> slug = sktext::gpu::Slug::ConvertBlob(canvas, *blob, {0, 0}, paint);
    canvas->restore();

    if (!slug) {
        return;
    }

    sk_sp<SkData> data = slug->serialize();
    if (!data || data->size() == 0) {
        return;
    }

    // Copy data to a writable buffer so we can corrupt it
    size_t size = data->size();
    std::unique_ptr<uint8_t[]> writableData(new uint8_t[size]);
    memcpy(writableData.get(), data->data(), size);


    // The creationMatrix of VertexFiller inside SDFTSubRun is located 216 bytes in.
    // We can overwrite it to have a matrix with perspective.
    if (size < 216 + 9 * sizeof(float)) {
        ERRORF(reporter, "Serialized Slug is too small to contain creationMatrix!");
        return;
    }
    float* f = reinterpret_cast<float*>(writableData.get() + 216);
    f[0] = 1.0f; f[1] = 0.0f; f[2] = 0.0f;
    f[3] = 0.0f; f[4] = 1.0f; f[5] = 0.0f;
    f[6] = 0.0078125f; f[7] = 0.0f; f[8] = 1.0f;

    // Deserialize the forged slug. This should return nullptr. Previously, when we went to draw it,
    // the perspective would cause issues.
    sk_sp<sktext::gpu::Slug> forgedSlug =
            sktext::gpu::Slug::Deserialize(writableData.get(), size, nullptr);
    REPORTER_ASSERT(reporter, forgedSlug == nullptr);
}

DEF_GANESH_TEST_FOR_CONTEXTS(Slug_b520571816,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             set_sdf_options,
                             CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();

    SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    auto surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    REPORTER_ASSERT(reporter, surface);
    auto canvas = surface->getCanvas();

    // Scale enough to force SDFT.
    SkMatrix canvasMatrix = SkMatrix::Scale(10.0f, 10.0f);
    canvas->save();
    canvas->setMatrix(canvasMatrix);

    auto typeface = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
    SkFont font(typeface);
    font.setSize(24);

    static const char* kText = "A";
    SkTextBlobBuilder builder;
    const SkTextBlobBuilder::RunBuffer& runBuf = builder.allocRun(font, 1, 0, 0);
    font.textToGlyphs(kText, 1, SkTextEncoding::kUTF8, SkSpan(runBuf.glyphs, 1));
    auto blob = builder.make();

    SkPaint paint;
    paint.setAntiAlias(true);

    sk_sp<sktext::gpu::Slug> slug = sktext::gpu::Slug::ConvertBlob(canvas, *blob, {0, 0}, paint);
    SkASSERT_RELEASE(slug);
    canvas->restore();

    sk_sp<SkData> data = slug->serialize();
    SkASSERT_RELEASE(data && data->size());

    size_t size = data->size();
    std::unique_ptr<uint8_t[]> writableData(new uint8_t[size]);
    memcpy(writableData.get(), data->data(), size);

    // Corrupt the maskFormat of VertexFiller inside SDFTSubRun at offset 208 from
    // kA8 (0) to kARGB (2).
    if (size < 212) {
        ERRORF(reporter, "Serialized Slug is too small to contain maskFormat!");
        return;
    }
    int32_t* formatPtr = reinterpret_cast<int32_t*>(writableData.get() + 208);
    if (*formatPtr != 0) {
        ERRORF(reporter, "maskFormat is not kA8 at expected offset 208!");
        return;
    }
    *formatPtr = static_cast<int32_t>(skgpu::MaskFormat::kARGB);

    // Since our validation fix will be active, this must fail and return nullptr. Previously, when
    // we went to draw it, the unexpected mask format would cause issues.
    sk_sp<sktext::gpu::Slug> forgedSlug =
            sktext::gpu::Slug::Deserialize(writableData.get(), size, nullptr);
    REPORTER_ASSERT(reporter, forgedSlug == nullptr);
}

DEF_TEST(VertexFiller_boundsAndDeviceMatrix, reporter) {
    const SkRect bounds = SkRect::MakeXYWH(10, 10, 20, 20);
    const SkPoint positions[] = {{10, 10}};
    sktext::gpu::VertexFiller directFiller(skgpu::MaskFormat::kA8,
                                           SkMatrix::I(),
                                           bounds,
                                           SkSpan(positions, 1),
                                           /*canDrawDirect=*/true);

    // 1. Integer translation with supportBilerpAtlas = false: succeeds.
    {
        auto [b, m] = directFiller.boundsAndDeviceMatrix(SkMatrix::I(), {5.0f, 5.0f},
                                                         /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, !b.isEmpty());
        REPORTER_ASSERT(reporter, m == SkMatrix::Translate(5.0f, 5.0f));
    }

    // 2. Fractional translation with supportBilerpAtlas = false: dropped.
    {
        auto [b, m] = directFiller.boundsAndDeviceMatrix(SkMatrix::I(), {0.5f, 0.0f},
                                                         /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, b.isEmpty());
    }

    // 3. Matrix scale with supportBilerpAtlas = false: dropped.
    {
        auto [b, m] = directFiller.boundsAndDeviceMatrix(SkMatrix::Scale(2.0f, 2.0f), {0.0f, 0.0f},
                                                         /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, b.isEmpty());
    }

    // 4. Fractional translation with supportBilerpAtlas = true: allowed.
    {
        auto [b, m] = directFiller.boundsAndDeviceMatrix(SkMatrix::I(), {0.5f, 0.0f},
                                                         /*supportBilerpAtlas=*/true);
        REPORTER_ASSERT(reporter, !b.isEmpty());
        REPORTER_ASSERT(reporter, m == SkMatrix::Translate(0.5f, 0.0f));
    }

    // 5. Non-direct filler (e.g. transformed/SDFT) with supportBilerpAtlas = false: allowed.
    sktext::gpu::VertexFiller transformedFiller(skgpu::MaskFormat::kA8,
                                                SkMatrix::I(),
                                                bounds,
                                                SkSpan(positions, 1),
                                                /*canDrawDirect=*/false);
    {
        auto [b, m] = transformedFiller.boundsAndDeviceMatrix(SkMatrix::I(), {0.5f, 0.0f},
                                                              /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, !b.isEmpty());
        REPORTER_ASSERT(reporter, m == SkMatrix::Translate(0.5f, 0.0f));
    }

    // 6. Translations whose difference overflows float (to infinity) must be dropped and
    // canUseDirect returns false.
    {
        sktext::gpu::VertexFiller infFiller(skgpu::MaskFormat::kA8,
                                            SkMatrix::Translate(-2e38f, 0.0f),
                                            bounds,
                                            SkSpan(positions, 1),
                                            /*canDrawDirect=*/true);
        auto [b, m] = infFiller.boundsAndDeviceMatrix(SkMatrix::I(), {2e38f, 0.0f},
                                                      /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, b.isEmpty());
        auto [directOk, vec] = infFiller.canUseDirect(SkMatrix::Translate(2e38f, 0.0f));
        REPORTER_ASSERT(reporter, !directOk);
    }

    // 7. Magnitude translation bypass: must be dropped and canUseDirect returns false.
    {
        auto [b, m] = directFiller.boundsAndDeviceMatrix(SkMatrix::I(), {1e25f, 0.0f},
                                                         /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, b.isEmpty());
        auto [directOk, vec] = directFiller.canUseDirect(SkMatrix::Translate(1e25f, 0.0f));
        REPORTER_ASSERT(reporter, !directOk);
    }

    // 8. Offsets are limited to +/-kMaxDirectCoord so that direct-mask vertex positions stay
    // exactly representable and convert safely to integers.
    {
        constexpr SkScalar kMax = sktext::gpu::VertexFiller::kMaxDirectCoord;
        auto [okAtLimit, offsetAtLimit] =
                directFiller.canUseDirect(SkMatrix::Translate(kMax, -kMax));
        REPORTER_ASSERT(reporter, okAtLimit);
        REPORTER_ASSERT(reporter, offsetAtLimit == SkVector::Make(kMax, -kMax));
        auto [okPastLimit, _] = directFiller.canUseDirect(SkMatrix::Translate(kMax + 1, 0.0f));
        REPORTER_ASSERT(reporter, !okPastLimit);
        auto [b, m] = directFiller.boundsAndDeviceMatrix(SkMatrix::I(), {0.0f, kMax + 1},
                                                         /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, b.isEmpty());
    }

    // 9. Large opposing translations: drawMatrix = Translate(-3e6f, 0) and drawOrigin = {3e6f, 0}
    // exceed kMaxDirectCoord individually, but the net offset {0, 0} is in range.
    {
        auto [b, m] = directFiller.boundsAndDeviceMatrix(SkMatrix::Translate(-3e6f, 0.0f),
                                                         {3e6f, 0.0f},
                                                         /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, !b.isEmpty());
        REPORTER_ASSERT(reporter, m == SkMatrix::I());
    }
}

// The draw-time direct-mask check must reproduce the single-precision positionMatrix computed at
// creation (drawMatrix.preTranslate(drawOrigin)). Recomputing it any other way (e.g. in double)
// turns creation-time float rounding into a fractional offset and rejects an unchanged matrix.
DEF_TEST(VertexFiller_CanUseDirect_matchesCreationRounding, reporter) {
    const SkRect bounds = SkRect::MakeXYWH(10, 10, 20, 20);
    const SkPoint positions[] = {{10, 10}};
    auto make_filler = [&](const SkMatrix& drawMatrix, SkPoint drawOrigin) {
        SkMatrix creationMatrix = drawMatrix;
        creationMatrix.preTranslate(drawOrigin.x(), drawOrigin.y());
        return sktext::gpu::VertexFiller(skgpu::MaskFormat::kA8,
                                         creationMatrix,
                                         bounds,
                                         SkSpan(positions, 1),
                                         /*canDrawDirect=*/true);
    };

    // None of these translation sums are exactly representable as floats.
    SkMatrix scaled = SkMatrix::Scale(1.25f, 1.25f);
    scaled.postTranslate(0.3f, 0.7f);
    const struct {
        SkMatrix drawMatrix;
        SkPoint drawOrigin;
    } kCases[] = {
        {SkMatrix::Translate(100.7f, 0.0f), {10.3f, 30.0f}},
        {scaled,                            {10.3f, 30.1f}},
        {SkMatrix::Translate(0.1f, 0.2f),   {0.2f, 0.1f}},
    };
    for (const auto& c : kCases) {
        // Drawing with the creation matrix must use the direct path with a zero offset.
        auto [b, m] = make_filler(c.drawMatrix, c.drawOrigin)
                              .boundsAndDeviceMatrix(c.drawMatrix, c.drawOrigin,
                                                     /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, !b.isEmpty());
        REPORTER_ASSERT(reporter, m == SkMatrix::I());
    }

    // Integer device-space moves of a rounded creation translation (100.7f + 10.3f rounds to
    // 111.0f) stay direct; fractional moves don't.
    const SkPoint origin = {10.3f, 30.0f};
    const sktext::gpu::VertexFiller filler =
            make_filler(SkMatrix::Translate(100.7f, 0.0f), origin);
    {
        auto [b, m] = filler.boundsAndDeviceMatrix(SkMatrix::Translate(101.7f, 0.0f), origin,
                                                   /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, !b.isEmpty());
        REPORTER_ASSERT(reporter, m == SkMatrix::Translate(1.0f, 0.0f));
    }
    {
        auto [b, m] = filler.boundsAndDeviceMatrix(SkMatrix::Translate(100.7f, 1.0f), origin,
                                                   /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, !b.isEmpty());
        REPORTER_ASSERT(reporter, m == SkMatrix::Translate(0.0f, 1.0f));
    }
    {
        auto [b, m] = filler.boundsAndDeviceMatrix(SkMatrix::Translate(101.2f, 0.0f), origin,
                                                   /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, b.isEmpty());
    }
}

namespace {
sk_sp<SkTextBlob> make_test_blob() {
    auto typeface = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
    SkFont font(typeface);
    font.setSize(16);

    static const char* kText = "A";
    SkTextBlobBuilder builder;
    const SkTextBlobBuilder::RunBuffer& runBuf = builder.allocRun(font, 1, 0, 0);
    font.textToGlyphs(kText, 1, SkTextEncoding::kUTF8, SkSpan(runBuf.glyphs, 1));
    return builder.make();
}

bool surface_has_non_white_pixels(SkSurface* surface) {
    SkBitmap bm;
    bm.allocPixels(surface->imageInfo());
    if (!surface->readPixels(bm, 0, 0)) {
        return false;
    }
    for (int y = 0; y < bm.height(); ++y) {
        for (int x = 0; x < bm.width(); ++x) {
            if (bm.getColor(x, y) != SK_ColorWHITE) {
                return true;
            }
        }
    }
    return false;
}

template <typename FlushFn>
void run_slug_fractional_replay_test(SkSurface* surface,
                                     const sktext::gpu::Slug* slug,
                                     FlushFn flushFn,
                                     skiatest::Reporter* reporter) {
    SkCanvas* canvas = surface->getCanvas();
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);

    // 1. Integer translation: should draw.
    canvas->clear(SK_ColorWHITE);
    canvas->save();
    canvas->translate(5.0f, 5.0f);
    slug->draw(canvas, paint);
    canvas->restore();
    flushFn();
    REPORTER_ASSERT(reporter, surface_has_non_white_pixels(surface));

    // 2. Fractional translation: must be dropped.
    canvas->clear(SK_ColorWHITE);
    canvas->save();
    canvas->translate(0.5f, 0.0f);
    slug->draw(canvas, paint);
    canvas->restore();
    flushFn();
    REPORTER_ASSERT(reporter, !surface_has_non_white_pixels(surface));

    // 3. Matrix scale: must be dropped.
    canvas->clear(SK_ColorWHITE);
    canvas->save();
    canvas->scale(1.5f, 1.5f);
    slug->draw(canvas, paint);
    canvas->restore();
    flushFn();
    REPORTER_ASSERT(reporter, !surface_has_non_white_pixels(surface));
}
}  // namespace

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Slug_fractional_replay_ganesh,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();
    SkImageInfo info = SkImageInfo::MakeN32Premul(64, 64);
    auto surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    REPORTER_ASSERT(reporter, surface);

    sk_sp<SkTextBlob> blob = make_test_blob();
    sk_sp<sktext::gpu::Slug> slug =
            sktext::gpu::Slug::ConvertBlob(surface->getCanvas(), *blob, {10, 30}, SkPaint());
    REPORTER_ASSERT(reporter, slug != nullptr);

    run_slug_fractional_replay_test(surface.get(), slug.get(), [&]() {
        dContext->flushAndSubmit();
    }, reporter);
}

DEF_GANESH_TEST_FOR_CONTEXTS(Slug_fractional_replay_bilerp_ganesh,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             [](GrContextOptions* options) {
                                 options->fSupportBilerpFromGlyphAtlas = true;
                             },
                             CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();
    SkImageInfo info = SkImageInfo::MakeN32Premul(64, 64);
    auto surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    REPORTER_ASSERT(reporter, surface);

    sk_sp<SkTextBlob> blob = make_test_blob();
    sk_sp<sktext::gpu::Slug> slug =
            sktext::gpu::Slug::ConvertBlob(surface->getCanvas(), *blob, {10, 30}, SkPaint());
    REPORTER_ASSERT(reporter, slug != nullptr);

    SkCanvas* canvas = surface->getCanvas();
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);

    // 1. Identity draw: renders text.
    canvas->clear(SK_ColorWHITE);
    slug->draw(canvas, paint);
    dContext->flushAndSubmit();
    REPORTER_ASSERT(reporter, surface_has_non_white_pixels(surface.get()));

    // 2. Fractional translation: when fSupportBilerpFromGlyphAtlas is true, text must draw.
    canvas->clear(SK_ColorWHITE);
    canvas->save();
    canvas->translate(0.5f, 0.0f);
    slug->draw(canvas, paint);
    canvas->restore();
    dContext->flushAndSubmit();
    REPORTER_ASSERT(reporter, surface_has_non_white_pixels(surface.get()));

    // 3. Matrix scale: when fSupportBilerpFromGlyphAtlas is true, text must draw.
    canvas->clear(SK_ColorWHITE);
    canvas->save();
    canvas->scale(1.5f, 1.5f);
    slug->draw(canvas, paint);
    canvas->restore();
    dContext->flushAndSubmit();
    REPORTER_ASSERT(reporter, surface_has_non_white_pixels(surface.get()));
}

#if defined(SK_GRAPHITE)
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(Slug_fractional_replay_graphite,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    using namespace skgpu::graphite;
    if (context->supportsProtectedContent()) {
        return;
    }
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    SkImageInfo info = SkImageInfo::MakeN32Premul(64, 64);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
    REPORTER_ASSERT(reporter, surface);

    sk_sp<SkTextBlob> blob = make_test_blob();
    sk_sp<sktext::gpu::Slug> slug =
            sktext::gpu::Slug::ConvertBlob(surface->getCanvas(), *blob, {10, 30}, SkPaint());
    REPORTER_ASSERT(reporter, slug != nullptr);

    run_slug_fractional_replay_test(surface.get(), slug.get(), [&]() {
        std::unique_ptr<Recording> recording = recorder->snap();
        context->insertRecording({recording.get()});
        context->submit(SyncToCpu::kYes);
    }, reporter);
}
#endif

// Slugs must stay drawable at their own creation matrix, and at integer translations of it, when
// the creation-time translation sum is not exactly representable as a float.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Slug_inexact_translation_replay_ganesh,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();
    auto surface = SkSurfaces::RenderTarget(
            dContext, skgpu::Budgeted::kNo, SkImageInfo::MakeN32Premul(256, 64));
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    sk_sp<SkTextBlob> blob = make_test_blob();

    // 100.7f + 10.3f rounds, so the creation matrix holds a rounded translation.
    canvas->save();
    canvas->translate(100.7f, 0.0f);
    sk_sp<sktext::gpu::Slug> slug =
            sktext::gpu::Slug::ConvertBlob(canvas, *blob, {10.3f, 30.0f}, SkPaint());
    canvas->restore();
    REPORTER_ASSERT(reporter, slug != nullptr);
    if (!slug) {
        return;
    }
    auto slugImpl = static_cast<const sktext::gpu::SlugImpl*>(slug.get());
    REPORTER_ASSERT(reporter, slugImpl->subRuns()->hasDirectMaskSubRuns());

    // The creation matrix and integer device-space translations of it can be drawn without
    // atlas bilerp support; a fractional translation can't.
    REPORTER_ASSERT(reporter, slugImpl->canDraw(SkMatrix::Translate(100.7f, 0.0f), false));
    REPORTER_ASSERT(reporter, slugImpl->canDraw(SkMatrix::Translate(101.7f, 0.0f), false));
    REPORTER_ASSERT(reporter, slugImpl->canDraw(SkMatrix::Translate(100.7f, 1.0f), false));
    REPORTER_ASSERT(reporter, !slugImpl->canDraw(SkMatrix::Translate(101.2f, 0.0f), false));

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->clear(SK_ColorWHITE);
    canvas->save();
    canvas->translate(100.7f, 0.0f);
    slug->draw(canvas, paint);
    canvas->restore();
    dContext->flushAndSubmit();
    REPORTER_ASSERT(reporter, surface_has_non_white_pixels(surface.get()));
}

#if defined(SK_GRAPHITE)
// Graphite drops direct-mask SubRuns whose transform isn't an integer translation when atlas
// bilerp is unsupported (the default), so both ordinary text and Slugs must compute their
// draw-time offset exactly as the creation matrix was computed.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(Slug_inexact_translation_replay_graphite,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    using namespace skgpu::graphite;
    if (context->supportsProtectedContent()) {
        return;
    }
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    sk_sp<SkSurface> surface =
            SkSurfaces::RenderTarget(recorder.get(), SkImageInfo::MakeN32Premul(256, 64));
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    sk_sp<SkTextBlob> blob = make_test_blob();
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    auto flush = [&]() {
        std::unique_ptr<Recording> recording = recorder->snap();
        context->insertRecording({recording.get()});
        context->submit(SyncToCpu::kYes);
    };

    // Ordinary text.
    canvas->clear(SK_ColorWHITE);
    canvas->save();
    canvas->translate(100.7f, 0.0f);
    canvas->drawTextBlob(blob, 10.3f, 30.0f, paint);
    canvas->restore();
    flush();
    REPORTER_ASSERT(reporter, surface_has_non_white_pixels(surface.get()));

    // A Slug drawn with the matrix it was created with.
    canvas->clear(SK_ColorWHITE);
    canvas->save();
    canvas->translate(100.7f, 0.0f);
    sk_sp<sktext::gpu::Slug> slug =
            sktext::gpu::Slug::ConvertBlob(canvas, *blob, {10.3f, 30.0f}, SkPaint());
    REPORTER_ASSERT(reporter, slug != nullptr);
    if (slug) {
        slug->draw(canvas, paint);
    }
    canvas->restore();
    flush();
    REPORTER_ASSERT(reporter, surface_has_non_white_pixels(surface.get()));
}
#endif

namespace {
sk_sp<SkTextBlob> make_positioned_test_blob(SkScalar size, SkPoint pos) {
    auto typeface = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
    SkFont font(typeface, size);
    SkTextBlobBuilder builder;
    const SkTextBlobBuilder::RunBuffer& runBuf = builder.allocRunPos(font, 1);
    font.textToGlyphs("A", 1, SkTextEncoding::kUTF8, SkSpan(runBuf.glyphs, 1));
    runBuf.points()[0] = pos;
    return builder.make();
}
}  // namespace

// Every Slug that creation produces must deserialize: creation enforces the same invariants that
// deserialization checks. Exercise matrices and font sizes that stress those invariants.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Slug_creation_round_trips,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();
    const SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    auto surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    // Device independent fonts make small text eligible for SDFT.
    const SkSurfaceProps difProps(SkSurfaceProps::kUseDeviceIndependentFonts_Flag,
                                  kUnknown_SkPixelGeometry);
    auto difSurface =
            SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info, 0, &difProps);
    REPORTER_ASSERT(reporter, surface && difSurface);
    if (!surface || !difSurface) {
        return;
    }

    SkMatrix rotate;
    rotate.setRotate(45);
    SkMatrix perspective;
    perspective.setPerspY(0.001f);

    const struct {
        const char* name;
        SkMatrix ctm;
        SkScalar fontSize;
        SkPoint glyphPos;
        SkPoint origin;
        bool deviceIndependentFonts;
        bool expectSlug;
    } kCases[] = {
        {"identity", SkMatrix::I(), 16, {0, 0}, {10, 30}, false, true},
        {"rotate", rotate, 16, {0, 0}, {10, 30}, false, true},
        {"perspective", perspective, 16, {0, 0}, {10, 30}, false, true},
        {"inexact translation", SkMatrix::Translate(100.7f, 0), 16, {0, 0}, {10.3f, 30},
         false, true},
        {"compensated large translation", SkMatrix::Translate(5e6f, 0), 16, {-5e6f, 30}, {0, 0},
         false, true},
        {"SDFT", SkMatrix::I(), 40, {0, 0}, {10, 60}, true, true},
        // Singular matrices can't be inverted, so these glyphs can't be direct masks.
        {"nearly singular skew", SkMatrix::MakeAll(1e-12f, 1, 0, 0, 1, 0, 0, 0, 1), 16, {0, 0},
         {10, 30}, false, false},
        {"singular", SkMatrix::MakeAll(1, 1, 0, 1, 1, 0, 0, 0, 1), 16, {0, 0}, {10, 30},
         false, false},
        // The positionMatrix translation overflows to infinity even though the inputs are finite.
        {"overflowing translation", SkMatrix::Scale(1e31f, 1), 16, {0, 0}, {1e8f, 30},
         false, false},
        // Extreme font sizes with compensating scales.
        {"tiny font, huge scale", SkMatrix::Scale(2.5e38f, 2.5e38f), 1e-37f, {0, 0}, {0, 0},
         true, false},
        {"huge font, tiny scale", SkMatrix::Scale(4e-29f, 4e-29f), 1e30f, {0, 0}, {0, 0},
         true, false},
    };
    for (const auto& c : kCases) {
        SkCanvas* canvas =
                c.deviceIndependentFonts ? difSurface->getCanvas() : surface->getCanvas();
        canvas->save();
        canvas->setMatrix(c.ctm);
        sk_sp<sktext::gpu::Slug> slug = sktext::gpu::Slug::ConvertBlob(
                canvas, *make_positioned_test_blob(c.fontSize, c.glyphPos), c.origin, SkPaint());
        canvas->restore();
        REPORTER_ASSERT(reporter, slug || !c.expectSlug, "%s", c.name);
        if (!slug) {
            continue;
        }
        sk_sp<SkData> data = slug->serialize();
        REPORTER_ASSERT(reporter, data, "%s", c.name);
        if (data) {
            REPORTER_ASSERT(reporter,
                            sktext::gpu::Slug::Deserialize(data->data(), data->size(), nullptr),
                            "%s", c.name);
        }
    }
}

static void set_unbounded_glyphs_as_paths_options(GrContextOptions* options) {
    options->fGlyphsAsPathsFontSize = SK_ScalarInfinity;
}

// With no glyphs-as-paths limit, the largest SDFT size bucket has an infinite maximum matrix scale,
// which deserialization rejects. Creation must not make SDFTSubRuns with such a range; the glyphs
// use another SubRun type instead, so the Slug still round-trips.
DEF_GANESH_TEST_FOR_CONTEXTS(Slug_unbounded_sdft_range_round_trips,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             set_unbounded_glyphs_as_paths_options,
                             CtsEnforcement::kNever) {
    auto surface = SkSurfaces::RenderTarget(ctxInfo.directContext(), skgpu::Budgeted::kNo,
                                            SkImageInfo::MakeN32Premul(256, 256));
    REPORTER_ASSERT(reporter, surface);
    if (!surface) {
        return;
    }
    // 200 is above the minimum SDFT size, and in the largest SDFT size bucket on all platforms.
    sk_sp<sktext::gpu::Slug> slug = sktext::gpu::Slug::ConvertBlob(
            surface->getCanvas(), *make_positioned_test_blob(200, {0, 0}), {10, 200}, SkPaint());
    REPORTER_ASSERT(reporter, slug);
    if (!slug) {
        return;
    }
    sk_sp<SkData> data = slug->serialize();
    REPORTER_ASSERT(reporter, data);
    if (data) {
        REPORTER_ASSERT(reporter,
                        sktext::gpu::Slug::Deserialize(data->data(), data->size(), nullptr));
    }
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Slug_deserialize_rejects_fractional_direct_positions,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();
    SkImageInfo info = SkImageInfo::MakeN32Premul(64, 64);
    auto surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    REPORTER_ASSERT(reporter, surface);

    sk_sp<SkTextBlob> blob = make_test_blob();
    sk_sp<sktext::gpu::Slug> slug =
            sktext::gpu::Slug::ConvertBlob(surface->getCanvas(), *blob, {10, 30}, SkPaint());
    REPORTER_ASSERT(reporter, slug != nullptr);
    if (!slug) {
        return;
    }
    // The byte offsets below assume the Slug holds a single direct-mask SubRun, serialized last.
    REPORTER_ASSERT(reporter, static_cast<const sktext::gpu::SlugImpl*>(slug.get())
                                      ->subRuns()->hasDirectMaskSubRuns());

    sk_sp<SkData> data = slug->serialize();
    REPORTER_ASSERT(reporter, data != nullptr && data->size() >= 52);
    if (!data || data->size() < 52) {
        return;
    }

    // 1. Verify that the original untampered Slug deserializes successfully.
    sk_sp<sktext::gpu::Slug> validSlug =
            sktext::gpu::Slug::Deserialize(data->data(), data->size(), nullptr);
    REPORTER_ASSERT(reporter, validSlug != nullptr);

    // In a single-glyph direct mask SubRun, the last 12 bytes of the serialized buffer
    // are the point array: uint32_t count (1), float x, float y.
    const size_t countOffset = data->size() - 12;
    const size_t xOffset = data->size() - 8;
    const size_t yOffset = data->size() - 4;

    uint32_t count = 0;
    float origX = 0.0f;
    float origY = 0.0f;
    memcpy(&count, (const uint8_t*)data->data() + countOffset, sizeof(count));
    memcpy(&origX, (const uint8_t*)data->data() + xOffset, sizeof(origX));
    memcpy(&origY, (const uint8_t*)data->data() + yOffset, sizeof(origY));

    REPORTER_ASSERT(reporter, count == 1);
    REPORTER_ASSERT(reporter, SkScalarIsInt(origX) && SkScalarIsInt(origY));

    auto try_deserialize_mutated = [](const SkData* src, size_t offset, auto mutatedValue) -> bool {
        SkTDArray<uint8_t> mutatedData;
        mutatedData.append(src->size(), (const uint8_t*)src->data());
        memcpy(mutatedData.data() + offset, &mutatedValue, sizeof(mutatedValue));
        sk_sp<sktext::gpu::Slug> result =
                sktext::gpu::Slug::Deserialize(mutatedData.data(), mutatedData.size(), nullptr);
        return result != nullptr;
    };

    // 2. Fractional X coordinate must be rejected.
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), xOffset, origX + 0.5f));

    // 3. Fractional Y coordinate must be rejected.
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), yOffset, origY + 0.5f));

    // 4. Non-finite coordinates must be rejected.
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), xOffset, SK_ScalarNaN));
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), yOffset, SK_ScalarInfinity));

    // 5. Extremely large float (out of magnitude bounds) must be rejected.
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), xOffset, 1e25f));
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), yOffset, -1e25f));

    // 6. Flipping canDrawDirect to 0 in a DirectMaskSubRun must be rejected.
    const size_t canDrawDirectOffset = data->size() - 52;
    uint32_t origCanDrawDirect = 0;
    memcpy(&origCanDrawDirect, (const uint8_t*)data->data() + canDrawDirectOffset,
           sizeof(origCanDrawDirect));
    REPORTER_ASSERT(reporter, origCanDrawDirect == 1);
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), canDrawDirectOffset, 0u));

    // Sanity check the layout: the serialized creation matrix must equal the Slug's initial
    // position matrix, Translate(10, 30), stored as 9 floats.
    const size_t creationMatrixOffset = data->size() - 48;
    {
        SkScalar serializedMatrix[9];
        memcpy(serializedMatrix, (const uint8_t*)data->data() + creationMatrixOffset,
               sizeof(serializedMatrix));
        SkScalar expectedMatrix[9];
        SkMatrix::Translate(10, 30).get9(expectedMatrix);
        for (int i = 0; i < 9; ++i) {
            REPORTER_ASSERT(reporter, serializedMatrix[i] == expectedMatrix[i]);
        }
    }

    // 7. Non-finite creationMatrix must be rejected.
    REPORTER_ASSERT(reporter,
                    !try_deserialize_mutated(data.get(), creationMatrixOffset, SK_ScalarNaN));

    // 8. creationMatrix mismatch relative to initialPositionMatrix must be rejected: a different
    // 2x2, and an integer translation that passes every other check.
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), creationMatrixOffset, 2.0f));
    const size_t creationTransXOffset =
            creationMatrixOffset + SkMatrix::kMTransX * sizeof(SkScalar);
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), creationTransXOffset, 11.0f));

    // 9. Slug creation with large coordinates must not assert in VertexFiller::Make
    // and falls back to path rendering.
    {
        surface->getCanvas()->save();
        surface->getCanvas()->translate(1e10f, 0);
        sk_sp<sktext::gpu::Slug> largeSlug =
                sktext::gpu::Slug::ConvertBlob(surface->getCanvas(), *blob, {0, 0}, SkPaint());
        surface->getCanvas()->restore();
        REPORTER_ASSERT(reporter, largeSlug != nullptr);
    }

    // 10. Deserialization must reject non-finite origins.
    const size_t originXOffset = 16;
    const size_t originYOffset = 20;
    {
        // Sanity check the layout: the origin follows the 16-byte source bounds.
        float origOriginX = 0.0f;
        float origOriginY = 0.0f;
        memcpy(&origOriginX, (const uint8_t*)data->data() + originXOffset, sizeof(origOriginX));
        memcpy(&origOriginY, (const uint8_t*)data->data() + originYOffset, sizeof(origOriginY));
        REPORTER_ASSERT(reporter, origOriginX == 10.0f && origOriginY == 30.0f);
    }
    REPORTER_ASSERT(reporter,
                    !try_deserialize_mutated(data.get(), originXOffset, SK_ScalarInfinity));
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), originXOffset, SK_ScalarNaN));
    REPORTER_ASSERT(reporter,
                    !try_deserialize_mutated(data.get(), originYOffset, SK_ScalarInfinity));
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), originYOffset, SK_ScalarNaN));

    // Deserializing with oversized origin succeeds because origin is finite, but canDraw
    // without bilerp atlas drops the draw due to translation bounds.
    {
        SkTDArray<uint8_t> mutatedData;
        mutatedData.append(data->size(), (const uint8_t*)data->data());
        float largeOrigin = 8388608.0f;
        memcpy(mutatedData.data() + originXOffset, &largeOrigin, sizeof(largeOrigin));
        sk_sp<sktext::gpu::Slug> result =
                sktext::gpu::Slug::Deserialize(mutatedData.data(), mutatedData.size(), nullptr);
        REPORTER_ASSERT(reporter, result != nullptr);
        auto slugImpl = static_cast<const sktext::gpu::SlugImpl*>(result.get());
        REPORTER_ASSERT(reporter, !slugImpl->canDraw(SkMatrix::I(), /*supportBilerpAtlas=*/false));
    }

    // 11. Non-direct SubRuns (here SDFT) store creation-space positions. These don't need to be
    // integers, but must be finite and within kMaxGlyphPositionCoord (1e14). Perspective text uses
    // SDFT when the context supports it; otherwise this case is skipped.
    {
        SkMatrix persMatrix;
        persMatrix.setPerspY(0.001f);
        surface->getCanvas()->save();
        surface->getCanvas()->concat(persMatrix);
        sk_sp<sktext::gpu::Slug> sdftSlug =
                sktext::gpu::Slug::ConvertBlob(surface->getCanvas(), *blob, {10, 30}, SkPaint());
        surface->getCanvas()->restore();
        REPORTER_ASSERT(reporter, sdftSlug != nullptr);

        const sktext::gpu::AtlasSubRun* sdftSubRun = nullptr;
        if (sdftSlug) {
            auto sdftSlugImpl = static_cast<const sktext::gpu::SlugImpl*>(sdftSlug.get());
            sdftSubRun = sktext::gpu::TextBlobTools::FirstSubRun(sdftSlugImpl->subRuns().get());
        }
        sk_sp<SkData> sdftData = sdftSlug ? sdftSlug->serialize() : nullptr;
        if (sdftSubRun && sdftSubRun->glyphParams().isSDF && sdftData && sdftData->size() >= 48) {
            // The SDFT SubRun holds the single glyph, so its vertex filler is serialized last: the
            // 9-float creation matrix, then the point array.
            const size_t sdftCountOffset = sdftData->size() - 12;
            const size_t sdftXOffset = sdftData->size() - 8;
            uint32_t sdftCount = 0;
            float sdftX = 0.0f;
            memcpy(&sdftCount, (const uint8_t*)sdftData->data() + sdftCountOffset,
                   sizeof(sdftCount));
            memcpy(&sdftX, (const uint8_t*)sdftData->data() + sdftXOffset, sizeof(sdftX));
            REPORTER_ASSERT(reporter, sdftCount == 1);

            REPORTER_ASSERT(reporter,
                            try_deserialize_mutated(sdftData.get(), sdftXOffset, sdftX + 0.5f));
            REPORTER_ASSERT(reporter,
                            !try_deserialize_mutated(sdftData.get(), sdftXOffset, 1e15f));
            REPORTER_ASSERT(reporter,
                            !try_deserialize_mutated(sdftData.get(), sdftXOffset, -3.4028234e38f));
            REPORTER_ASSERT(reporter,
                            !try_deserialize_mutated(sdftData.get(), sdftXOffset, SK_ScalarNaN));

            // The creation matrix is a pure scale. An infinite scale passes invert() (its inverse
            // is 0), so only the finiteness check on creation matrices rejects it.
            const size_t sdftMatrixOffset = sdftData->size() - 48;
            SkScalar serializedMatrix[9];
            memcpy(serializedMatrix, (const uint8_t*)sdftData->data() + sdftMatrixOffset,
                   sizeof(serializedMatrix));
            const SkScalar sdftScale = serializedMatrix[SkMatrix::kMScaleX];
            SkScalar expectedMatrix[9];
            SkMatrix::Scale(sdftScale, sdftScale).get9(expectedMatrix);
            REPORTER_ASSERT(reporter, sdftScale > 0);
            for (int i = 0; i < 9; ++i) {
                REPORTER_ASSERT(reporter, serializedMatrix[i] == expectedMatrix[i]);
            }
            const size_t sdftScaleXOffset =
                    sdftMatrixOffset + SkMatrix::kMScaleX * sizeof(SkScalar);
            REPORTER_ASSERT(reporter,
                            !try_deserialize_mutated(sdftData.get(), sdftScaleXOffset,
                                                     SK_ScalarInfinity));
        }
    }

    // 12. ConvertBlob with coordinates near FLT_MAX must not produce slugs with extreme/infinite
    // bounds that fail deserialization.
    {
        surface->getCanvas()->save();
        surface->getCanvas()->rotate(45.0f);
        sk_sp<sktext::gpu::Slug> nearMaxSlug =
                sktext::gpu::Slug::ConvertBlob(
                        surface->getCanvas(), *blob, {3.402823e38f, 0}, SkPaint());
        surface->getCanvas()->restore();
        // The extreme coordinates exceed kMaxGlyphPositionCoord and are cleanly dropped.
        REPORTER_ASSERT(reporter, nearMaxSlug == nullptr);
    }

    // 13. Perspective initialPositionMatrix must deserialize successfully for non-direct runs.
    {
        SkMatrix persMatrix;
        persMatrix.setPerspY(0.001f);
        surface->getCanvas()->save();
        surface->getCanvas()->concat(persMatrix);
        sk_sp<sktext::gpu::Slug> persSlug =
                sktext::gpu::Slug::ConvertBlob(surface->getCanvas(), *blob, {10, 30}, SkPaint());
        surface->getCanvas()->restore();
        REPORTER_ASSERT(reporter, persSlug != nullptr);
        sk_sp<SkData> persData = persSlug->serialize();
        REPORTER_ASSERT(reporter, persData != nullptr && persData->size() > 0);
        sk_sp<sktext::gpu::Slug> deserializedPersSlug =
                sktext::gpu::Slug::Deserialize(persData->data(), persData->size(), nullptr);
        REPORTER_ASSERT(reporter, deserializedPersSlug != nullptr);
        auto persSlugImpl = static_cast<const sktext::gpu::SlugImpl*>(deserializedPersSlug.get());
        REPORTER_ASSERT(reporter, persSlugImpl->canDraw(persMatrix, /*supportBilerpAtlas=*/false));
    }

    // 14. Large canvas translation (> 2^21) with a glyph that maps on-screen ({0, 30})
    // produces a valid direct-mask Slug that serializes, deserializes, and draws.
    {
        auto typeface = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
        SkFont testFont(typeface, 16);
        SkTextBlobBuilder posBuilder;
        const SkTextBlobBuilder::RunBuffer& runBuf = posBuilder.allocRunPos(testFont, 1);
        testFont.textToGlyphs("A", 1, SkTextEncoding::kUTF8, SkSpan(runBuf.glyphs, 1));
        runBuf.points()[0] = {-5e6f, 30};
        sk_sp<SkTextBlob> posBlob = posBuilder.make();
        REPORTER_ASSERT(reporter, posBlob != nullptr);

        surface->getCanvas()->save();
        surface->getCanvas()->translate(5e6f, 0);
        sk_sp<sktext::gpu::Slug> largeTransSlug =
                sktext::gpu::Slug::ConvertBlob(
                        surface->getCanvas(), *posBlob, {0, 0}, SkPaint());
        surface->getCanvas()->restore();
        REPORTER_ASSERT(reporter, largeTransSlug != nullptr);

        sk_sp<SkData> largeTransData = largeTransSlug->serialize();
        REPORTER_ASSERT(reporter, largeTransData != nullptr && largeTransData->size() > 0);
        sk_sp<sktext::gpu::Slug> deserialized = sktext::gpu::Slug::Deserialize(
                largeTransData->data(), largeTransData->size(), nullptr);
        REPORTER_ASSERT(reporter, deserialized != nullptr);
        auto slugImpl = static_cast<const sktext::gpu::SlugImpl*>(deserialized.get());
        REPORTER_ASSERT(reporter, slugImpl->subRuns()->hasDirectMaskSubRuns());
        REPORTER_ASSERT(reporter, slugImpl->canDraw(SkMatrix::Translate(5e6f, 0),
                                                    /*supportBilerpAtlas=*/false));

        // When device coordinates exceed 2^21 without compensating glyph offsets,
        // direct mask drawing falls back to path rendering.
        surface->getCanvas()->save();
        surface->getCanvas()->translate(5e6f, 0);
        sk_sp<sktext::gpu::Slug> fallbackSlug =
                sktext::gpu::Slug::ConvertBlob(
                        surface->getCanvas(), *blob, {0, 30}, SkPaint());
        surface->getCanvas()->restore();
        REPORTER_ASSERT(reporter, fallbackSlug != nullptr);
        auto fbSlugImpl = static_cast<const sktext::gpu::SlugImpl*>(fallbackSlug.get());
        REPORTER_ASSERT(reporter, !fbSlugImpl->subRuns()->hasDirectMaskSubRuns());
        REPORTER_ASSERT(reporter, fbSlugImpl->canDraw(SkMatrix::Translate(5e6f, 0),
                                                      /*supportBilerpAtlas=*/false));
        // Without direct-mask SubRuns the Slug never samples unpadded atlas entries, so it can be
        // drawn with integer and fractional translations without bilerp atlas support.
        REPORTER_ASSERT(reporter, fbSlugImpl->canDraw(SkMatrix::Translate(5e6f + 10.0f, 5.0f),
                                                      /*supportBilerpAtlas=*/false));
        REPORTER_ASSERT(reporter, fbSlugImpl->canDraw(SkMatrix::Translate(5e6f + 10.5f, 5.0f),
                                                      /*supportBilerpAtlas=*/false));
    }

    // 15. Opposing large translation Slug can draw when replayed with creation matrix.
    {
        surface->getCanvas()->save();
        surface->getCanvas()->translate(-3e6f, 0);
        sk_sp<sktext::gpu::Slug> opposingSlug =
                sktext::gpu::Slug::ConvertBlob(
                        surface->getCanvas(), *blob, {3e6f, 30}, SkPaint());
        surface->getCanvas()->restore();
        REPORTER_ASSERT(reporter, opposingSlug != nullptr);
        auto oppSlugImpl = static_cast<const sktext::gpu::SlugImpl*>(opposingSlug.get());
        REPORTER_ASSERT(reporter, oppSlugImpl->canDraw(SkMatrix::Translate(-3e6f, 0),
                                                       /*supportBilerpAtlas=*/false));

        // Verify end-to-end execution through GPU op creation and vertex generation.
        SkPaint testPaint;
        testPaint.setColor(SK_ColorBLACK);
        surface->getCanvas()->clear(SK_ColorWHITE);
        surface->getCanvas()->save();
        surface->getCanvas()->translate(-3e6f, 0);
        opposingSlug->draw(surface->getCanvas(), testPaint);
        surface->getCanvas()->restore();
        dContext->flushAndSubmit();
        REPORTER_ASSERT(reporter, surface_has_non_white_pixels(surface.get()));
    }
}
