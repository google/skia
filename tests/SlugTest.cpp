/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
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
#include "include/utils/SkCustomTypeface.h"
#include "src/gpu/MaskFormat.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

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

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Slug_draw_then_serialize,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();
    SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    auto surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    REPORTER_ASSERT(reporter, surface);
    auto canvas = surface->getCanvas();

    auto typeface = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
    SkFont font(typeface);
    font.setSize(16);

    static const char* kText = "Hello World";
    size_t textLen = strlen(kText);
    SkTextBlobBuilder builder;
    const SkTextBlobBuilder::RunBuffer& runBuf = builder.allocRun(font, textLen, 0, 0);
    font.textToGlyphs(kText, textLen, SkTextEncoding::kUTF8, SkSpan(runBuf.glyphs, textLen));
    auto blob = builder.make();

    SkPaint paint;
    paint.setAntiAlias(true);

    sk_sp<sktext::gpu::Slug> slug = sktext::gpu::Slug::ConvertBlob(canvas, *blob, {10, 10}, paint);
    REPORTER_ASSERT(reporter, slug != nullptr);

    // First draw and flush initializes backend data.
    slug->draw(canvas, paint);
    dContext->flushAndSubmit();

    // Verify the slug can be redrawn safely. Note: This does not test whether the Slug
    // works when the atlas is full and evicted glyphs need to be re-fetched from the strike.
    // That will be covered in a separate test.
    slug->draw(canvas, paint);
    dContext->flushAndSubmit();

    // Verify the slug can be serialized post-draw without losing strike descriptor info.
    sk_sp<SkData> data = slug->serialize();
    REPORTER_ASSERT(reporter, data != nullptr && data->size() > 0);

    // Verify the serialized slug can be deserialized and drawn.
    sk_sp<sktext::gpu::Slug> deserializedSlug =
            sktext::gpu::Slug::Deserialize(data->data(), data->size(), nullptr);
    REPORTER_ASSERT(reporter, deserializedSlug != nullptr);
    deserializedSlug->draw(canvas, paint);
    dContext->flushAndSubmit();
}

namespace {
constexpr SkRect kGlyphRect = SkRect::MakeLTRB(0, -1, 1, 0);

class RectGlyphDrawable final : public SkDrawable {
protected:
    SkRect onGetBounds() override { return kGlyphRect; }
    void onDraw(SkCanvas* canvas) override {
        canvas->drawRect(kGlyphRect, SkPaint());
    }
};

sk_sp<SkTypeface> make_path_typeface() {
    SkCustomTypefaceBuilder builder;
    builder.setGlyph(1, 1.0f, SkPath::Rect(kGlyphRect));
    return builder.detach();
}

sk_sp<SkTypeface> make_drawable_typeface() {
    SkCustomTypefaceBuilder builder;
    builder.setGlyph(1, 1.0f, sk_make_sp<RectGlyphDrawable>(), kGlyphRect);
    return builder.detach();
}

sk_sp<SkTextBlob> make_blob(sk_sp<SkTypeface> tf, SkScalar size) {
    SkTextBlobBuilder builder;
    builder.allocRun(SkFont(std::move(tf), size), 1, 0, 0).glyphs[0] = 1;
    return builder.make();
}

SkBitmap render_slug(SkSurface* surface, const sktext::gpu::Slug& slug, const SkPaint& paint) {
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);
    slug.draw(canvas, paint);
    SkBitmap bm;
    bm.allocPixels(surface->imageInfo());
    SkAssertResult(surface->readPixels(bm, 0, 0));
    return bm;
}
}  // namespace

// PathOpSubmitter stores each glyph's SkGlyphID and its lazily resolved SkPath in IDOrPath.
// Converting glyph IDs to SkPaths during submitDraws must not corrupt the glyph IDs written by
// subsequent Slug::serialize() calls.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Slug_draw_then_serialize_PathSubRun,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();
    auto surface = SkSurfaces::RenderTarget(
            dContext, skgpu::Budgeted::kNo, SkImageInfo::MakeN32Premul(64, 64));
    REPORTER_ASSERT(reporter, surface);

    // 400pt exceeds fGlyphsAsPathsFontSize (324 on desktop, 384 on Android), forcing PathSubRun
    auto blob = make_blob(make_path_typeface(), /*size=*/400);
    REPORTER_ASSERT(reporter, blob != nullptr);

    SkPaint paint;
    paint.setAntiAlias(true);

    sk_sp<sktext::gpu::Slug> slug =
            sktext::gpu::Slug::ConvertBlob(surface->getCanvas(), *blob, {0, 64}, paint);
    REPORTER_ASSERT(reporter, slug != nullptr);

    // Before the first draw, PathOpSubmitter::fIDsOrPaths holds the original SkGlyphIDs
    sk_sp<SkData> preDrawData = slug->serialize();
    REPORTER_ASSERT(reporter, preDrawData != nullptr && preDrawData->size() > 0);

    // Drawing the Slug triggers PathOpSubmitter::submitDraws, which resolves each glyph's SkPath.
    SkBitmap expectedPixels = render_slug(surface.get(), *slug, paint);
    REPORTER_ASSERT(reporter, expectedPixels.getColor(32, 32) == SK_ColorBLACK);

    // Verify that serializing after a draw still preserves the original SkGlyphIDs and data.
    sk_sp<SkData> postDrawData = slug->serialize();
    REPORTER_ASSERT(reporter, postDrawData != nullptr && postDrawData->size() > 0);
    REPORTER_ASSERT(reporter, preDrawData->equals(postDrawData.get()));

    // Verify that deserializing postDrawData produces a Slug that renders matching pixels.
    sk_sp<sktext::gpu::Slug> deserializedSlug =
            sktext::gpu::Slug::Deserialize(postDrawData->data(), postDrawData->size(), nullptr);
    REPORTER_ASSERT(reporter, deserializedSlug != nullptr);
    SkBitmap actualPixels = render_slug(surface.get(), *deserializedSlug, paint);
    REPORTER_ASSERT(reporter, ToolUtils::equal_pixels(expectedPixels, actualPixels));
}

// DrawableOpSubmitter stores each glyph's SkGlyphID and its resolved SkDrawable* in IDOrDrawable.
// Converting glyph IDs to drawables during submitDraws must not corrupt the glyph IDs written by
// subsequent Slug::serialize() calls.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Slug_draw_then_serialize_DrawableSubRun,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();
    auto surface = SkSurfaces::RenderTarget(
            dContext, skgpu::Budgeted::kNo, SkImageInfo::MakeN32Premul(64, 64));
    REPORTER_ASSERT(reporter, surface);

    // 400pt exceeds fGlyphsAsPathsFontSize, forcing a drawable glyph through DrawableSubRun
    auto blob = make_blob(make_drawable_typeface(), /*size=*/400);
    SkPaint paint;
    paint.setAntiAlias(true);

    sk_sp<sktext::gpu::Slug> slug =
            sktext::gpu::Slug::ConvertBlob(surface->getCanvas(), *blob, {0, 64}, paint);
    REPORTER_ASSERT(reporter, slug != nullptr);

    // Before the first draw, DrawableOpSubmitter::fIDsOrDrawables holds the original SkGlyphID (1)
    sk_sp<SkData> preDrawData = slug->serialize();
    REPORTER_ASSERT(reporter, preDrawData != nullptr && preDrawData->size() > 0);

    // Drawing the Slug triggers DrawableOpSubmitter::submitDraws, which resolves each glyph's
    // SkDrawable* pointer.
    SkBitmap expectedPixels = render_slug(surface.get(), *slug, paint);
    REPORTER_ASSERT(reporter, expectedPixels.getColor(32, 32) == SK_ColorBLACK);

    // Verify that serializing after a draw preserves the original SkGlyphID and data.
    sk_sp<SkData> postDrawData = slug->serialize();
    REPORTER_ASSERT(reporter, postDrawData != nullptr && postDrawData->size() > 0);
    REPORTER_ASSERT(reporter, preDrawData->equals(postDrawData.get()));

    // Verify that deserializing postDrawData produces a Slug that renders matching pixels.
    sk_sp<sktext::gpu::Slug> deserializedSlug =
            sktext::gpu::Slug::Deserialize(postDrawData->data(), postDrawData->size(), nullptr);
    REPORTER_ASSERT(reporter, deserializedSlug != nullptr);
    SkBitmap actualPixels = render_slug(surface.get(), *deserializedSlug, paint);
    REPORTER_ASSERT(reporter, ToolUtils::equal_pixels(expectedPixels, actualPixels));
}


