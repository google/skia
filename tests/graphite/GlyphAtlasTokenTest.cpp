/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkBlender.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkBlenders.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "include/private/SkTo.h"
#include "include/utils/SkCustomTypeface.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

using namespace skgpu::graphite;

namespace {

constexpr int kOuterGlyphCount = 70;
constexpr SkGlyphID kFirstOuterGlyph = 1;
constexpr SkGlyphID kFirstInnerGlyph = kFirstOuterGlyph + kOuterGlyphCount;
constexpr int kInnerGlyphCount = 140;

// Cap the A8 glyph atlas at its 16-plot minimum (4 pages of 4 256x256 plots) and keep 100px
// glyphs on the direct mask path so 64 100x100 glyphs deterministically fill all 16 plots
void set_small_direct_glyph_atlas(ContextOptions* options) {
    options->fGlyphCacheTextureMaximumBytes = 0;
    options->fMinDistanceFieldFontSize = 384.f;
    options->fGlyphsAsPathsFontSize = 384.f;
}

// Outer glyphs cover the left half of a 1x1 em box; inner glyphs cover the right half with
// identical [0, -1, 1, 0] bounds so plot eviction swaps which half of the quad is opaque
sk_sp<SkTypeface> make_split_half_typeface() {
    SkCustomTypefaceBuilder builder;
    const SkPath leftHalf = SkPathBuilder()
                                    .addRect(SkRect::MakeLTRB(0.f, -1.f, 0.4f, 0.f))
                                    .addRect(SkRect::MakeLTRB(0.96f, -0.04f, 1.f, 0.f))
                                    .detach();
    for (int i = 0; i < kOuterGlyphCount; ++i) {
        builder.setGlyph(SkTo<SkGlyphID>(kFirstOuterGlyph + i), 1.f, leftHalf);
    }

    const SkPath rightHalf = SkPathBuilder()
                                     .addRect(SkRect::MakeLTRB(0.6f, -1.f, 1.f, 0.f))
                                     .addRect(SkRect::MakeLTRB(0.f, -1.f, 0.04f, -0.96f))
                                     .detach();
    for (int i = 0; i < kInnerGlyphCount; ++i) {
        builder.setGlyph(SkTo<SkGlyphID>(kFirstInnerGlyph + i), 1.f, rightHalf);
    }
    return builder.detach();
}

// Record a uniform green picture whose replay draws innerGlyphCount distinct 100x100 A8
// glyphs into the shared Recorder glyph atlas during paint key extraction
sk_sp<SkShader> make_glyph_filling_shader(const SkFont& font, int innerGlyphCount) {
    SkASSERT(innerGlyphCount <= kInnerGlyphCount);
    SkPictureRecorder picRecorder;
    SkCanvas* picCanvas = picRecorder.beginRecording(SkRect::MakeWH(128, 128));
    picCanvas->clear(SK_ColorGREEN);

    SkTextBlobBuilder blobBuilder;
    const auto& run = blobBuilder.allocRunPosH(font, innerGlyphCount, 110.f);
    for (int i = 0; i < innerGlyphCount; ++i) {
        run.glyphs[i] = SkTo<SkGlyphID>(kFirstInnerGlyph + i);
        run.pos[i] = 0.f;
    }
    SkPaint innerPaint;
    innerPaint.setColor(SK_ColorGREEN);
    picCanvas->drawTextBlob(blobBuilder.make(), 0.f, 0.f, innerPaint);

    sk_sp<SkPicture> pic = picRecorder.finishRecordingAsPicture();
    return pic->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkFilterMode::kNearest);
}

void assert_outer_glyph_pixels(skiatest::Reporter* reporter,
                               const SkBitmap& bitmap,
                               int originX,
                               int baselineY) {
    const SkColor left = bitmap.getColor(originX + 20, baselineY - 50);
    const SkColor right = bitmap.getColor(originX + 80, baselineY - 50);
    REPORTER_ASSERT(reporter, left == SK_ColorGREEN, "left=0x%08X", left);
    REPORTER_ASSERT(reporter, right == SK_ColorBLACK, "right=0x%08X", right);
}

void check_single_outer_glyph(skiatest::Reporter* reporter,
                              Context* context,
                              const SkFont& font,
                              int innerGlyphCount,
                              sk_sp<SkBlender> blender = nullptr) {
    auto recorder = context->makeRecorder();
    REPORTER_ASSERT(reporter, recorder);
    const SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
    REPORTER_ASSERT(reporter, surface);

    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorBLACK);
    if (blender) {
        SkPaint bgPaint;
        bgPaint.setColor(SK_ColorBLACK);
        canvas->drawRect(SkRect::MakeWH(256, 256), bgPaint);
    }

    SkTextBlobBuilder blobBuilder;
    const auto& run = blobBuilder.allocRunPosH(font, 1, 0.f);
    run.glyphs[0] = kFirstOuterGlyph;
    run.pos[0] = 0.f;

    SkPaint paint;
    paint.setBlender(std::move(blender));
    paint.setShader(make_glyph_filling_shader(font, innerGlyphCount));
    canvas->drawTextBlob(blobBuilder.make(), 10.f, 110.f, paint);

    SkBitmap bitmap;
    bitmap.allocPixels(info);
    REPORTER_ASSERT(reporter, surface->readPixels(bitmap, 0, 0));
    assert_outer_glyph_pixels(reporter, bitmap, 10, 110);
}

void check_multi_slice_outer_blob(skiatest::Reporter* reporter,
                                  Context* context,
                                  const SkFont& font) {
    auto recorder = context->makeRecorder();
    REPORTER_ASSERT(reporter, recorder);
    const SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
    REPORTER_ASSERT(reporter, surface);

    SkTextBlobBuilder blobBuilder;
    const auto& run = blobBuilder.allocRunPos(font, kOuterGlyphCount);
    for (int i = 0; i < kOuterGlyphCount; ++i) {
        run.glyphs[i] = SkTo<SkGlyphID>(kFirstOuterGlyph + i);
        if (i == 0) {
            run.points()[i] = {10.f, 110.f};
        } else if (i == kOuterGlyphCount - 1) {
            run.points()[i] = {130.f, 110.f};
        } else {
            run.points()[i] = {10.f, 240.f};
        }
    }
    sk_sp<SkTextBlob> outerBlob = blobBuilder.make();

    // Draw twice to cover both initial GlyphVector population and cached SubRun reuse
    for (int pass = 0; pass < 2; ++pass) {
        skiatest::ReporterContext passCtx(reporter, pass == 0 ? "initial" : "cached");
        surface->getCanvas()->clear(SK_ColorBLACK);

        SkPaint paint;
        paint.setShader(make_glyph_filling_shader(font, /*innerGlyphCount=*/80));
        surface->getCanvas()->drawTextBlob(outerBlob, 0.f, 0.f, paint);

        SkBitmap bitmap;
        bitmap.allocPixels(info);
        REPORTER_ASSERT(reporter, surface->readPixels(bitmap, 0, 0));
        assert_outer_glyph_pixels(reporter, bitmap, 10, 110);
        assert_outer_glyph_pixels(reporter, bitmap, 130, 110);
    }
}

// DrawListLayer charges already-drawn clips their actual render steps, so a glyph draw with more
// clips than the conservative 4-steps-per-clip budget allows can still reach the atlas retry
void set_small_glyph_atlas_draw_list_layer(ContextOptions* options) {
    set_small_direct_glyph_atlas(options);
    options->fUseDrawListLayer = true;
}

}  // anonymous namespace

// Verify that atlas-backed text draws preserve their own glyph masks when paint key extraction
// (such as SkPictureShader replay) flushes or fills the shared Recorder glyph atlas
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(DrawAtlasGlyphTokenLifetime,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           testContext,
                                           /*anonymous options*/,
                                           set_small_direct_glyph_atlas,
                                           /*condition=*/true,
                                           CtsEnforcement::kNextRelease) {
    if (context->supportsProtectedContent()) {
        return;
    }

    SkFont font(make_split_half_typeface(), 100.f);
    font.setEdging(SkFont::Edging::kAlias);

    {
        skiatest::ReporterContext subcase(reporter, "evict_mid_toKey");
        check_single_outer_glyph(reporter, context, font, /*innerGlyphCount=*/80);
    }
    {
        skiatest::ReporterContext subcase(reporter, "atlas_full_current_token");
        check_single_outer_glyph(reporter, context, font, /*innerGlyphCount=*/64);
    }
    {
        skiatest::ReporterContext subcase(reporter, "atlas_full_after_dst_read_flush");
        // Use a runtime blender rather than an advanced SkBlendMode so DstUsage::kDstReadRequired
        // is forced even on backends that advertise hardware advanced blending
        check_single_outer_glyph(
                reporter,
                context,
                font,
                /*innerGlyphCount=*/128,
                SkBlenders::Arithmetic(0.f, 1.f, 1.f, 0.f, /*enforcePremul=*/false));
    }
    {
        skiatest::ReporterContext subcase(reporter, "multi_slice_blob");
        check_multi_slice_outer_blob(reporter, context, font);
    }
}

// Verify that the atlas-full flush-and-retry keeps a glyph draw whose many clips are already
// drawn: they fit the current DrawList at their actual render steps, so the retry must not
// apply the conservative 4-steps-per-clip limit that would drop the text
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(DrawAtlasGlyphRetryKeepsDrawnClips,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           testContext,
                                           /*anonymous options*/,
                                           set_small_glyph_atlas_draw_list_layer,
                                           /*condition=*/true,
                                           CtsEnforcement::kNextRelease) {
    if (context->supportsProtectedContent()) {
        return;
    }

    SkFont font(make_split_half_typeface(), 100.f);
    font.setEdging(SkFont::Edging::kAlias);

    auto recorder = context->makeRecorder();
    REPORTER_ASSERT(reporter, recorder);
    const SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorBLACK);

    // 1100 clips at 4 steps each exceed DrawList::kMaxRenderSteps (4096). Each hypotenuse
    // crosses the glyph quad so ClipStack keeps it, and the clips are drawn in batches since a
    // single draw with all of them fails the conservative check before any is drawn
    constexpr int kClipCount = 1100;
    SkPaint black;
    black.setColor(SK_ColorBLACK);
    const SkRect clipBatchRect = SkRect::MakeLTRB(5, 5, 250, 250);
    for (int i = 0; i < kClipCount; ++i) {
        const SkPath triangle = SkPathBuilder()
                                        .moveTo(-200.f, -200.f)
                                        .lineTo(400.f + 0.01f * i, -200.f)
                                        .lineTo(-200.f, 400.f - 0.01f * i)
                                        .close()
                                        .detach();
        canvas->clipPath(triangle, /*doAntiAlias=*/false);
        if (i % 500 == 499) {
            canvas->drawRect(clipBatchRect, black);
        }
    }
    canvas->drawRect(clipBatchRect, black);

    SkTextBlobBuilder blobBuilder;
    const auto& run = blobBuilder.allocRunPosH(font, 1, 0.f);
    run.glyphs[0] = kFirstOuterGlyph;
    run.pos[0] = 0.f;
    SkPaint paint;
    paint.setShader(make_glyph_filling_shader(font, /*innerGlyphCount=*/64));
    canvas->drawTextBlob(blobBuilder.make(), 10.f, 110.f, paint);

    SkBitmap bitmap;
    bitmap.allocPixels(info);
    REPORTER_ASSERT(reporter, surface->readPixels(bitmap, 0, 0));
    assert_outer_glyph_pixels(reporter, bitmap, 10, 110);
}

// Verify that a mask-filter draw clipped out after drawCoverageMask() staged its scratch-mask read
// drops that read: a staged read left on the DrawContext would be committed by an unrelated later
// draw (asserted empty in ~DrawContext; crbug/586074490)
DEF_GRAPHITE_TEST_FOR_CONTEXTS(DrawCoverageMaskClippedOutDropsScratchRead,
                               /*filter=*/nullptr,
                               reporter,
                               context,
                               testContext,
                               CtsEnforcement::kNextRelease) {
    auto recorder = context->makeRecorder();
    SkImageInfo info = SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorGREEN);

    // A difference clip keeps SkCanvas's conservative bounds at the full device, so the blurred
    // path reaches the mask filter (a path, not a rect, to avoid the analytic blur), and only
    // Graphite's ClipStack rejects the coverage mask draw
    canvas->clipRect(SkRect::MakeLTRB(0, 0, 128, 128), SkClipOp::kDifference);
    SkPaint blurPaint;
    blurPaint.setColor(SK_ColorRED);
    blurPaint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 4.f));
    const SkPath triangle =
            SkPathBuilder().moveTo(30, 30).lineTo(90, 30).lineTo(30, 90).close().detach();
    canvas->drawPath(triangle, blurPaint);

    if (context->supportsProtectedContent()) {
        // Protected surfaces disallow CPU readback; snap to flush tracked devices instead
        std::unique_ptr<Recording> recording = recorder->snap();
        REPORTER_ASSERT(reporter, recording != nullptr);
        return;
    }

    SkBitmap bitmap;
    bitmap.allocPixels(info);
    REPORTER_ASSERT(reporter, canvas->readPixels(bitmap, 0, 0));
    const SkColor inside = bitmap.getColor(45, 45);
    REPORTER_ASSERT(reporter, inside == SK_ColorGREEN, "got 0x%08X", inside);
}
