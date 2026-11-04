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

#if defined(SK_GRAPHITE)
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(Slug_fractional_replay_graphite,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    using namespace skgpu::graphite;
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

