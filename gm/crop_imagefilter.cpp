/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"

#include "tools/Resources.h"

// TODO(michaelludwig) - This will be made public within SkImageFilters.h at some point
#include "src/effects/imagefilters/SkCropImageFilter.h"


namespace {

static constexpr SkColor kOutputBoundsColor  = SK_ColorRED;
static constexpr SkColor kCropRectColor      = SK_ColorGREEN;
static constexpr SkColor kContentBoundsColor = SK_ColorBLUE;

static constexpr SkRect kExampleBounds = {0.f, 0.f, 100.f, 100.f};

// "Crop" refers to the rect passed to the crop image filter, "Rect" refers to some other rect
// from context, likely the output bounds or the content bounds.
enum class CropRelation {
    kCropOverlapsRect, // Intersect but doesn't fully contain one way or the other
    kCropContainsRect,
    kRectContainsCrop,
    kCropRectDisjoint,
};

SkRect make_overlap(const SkRect& r, float amountX, float amountY) {
    return r.makeOffset(r.width() * amountX, r.height() * amountY);
}

SkRect make_inset(const SkRect& r, float amountX, float amountY) {
    return r.makeInset(r.width() * amountX, r.height() * amountY);
}

SkRect make_outset(const SkRect& r, float amountX, float amountY) {
    return r.makeOutset(r.width() * amountX, r.height() * amountY);
}

SkRect make_disjoint(const SkRect& r, float amountX, float amountY) {
    float xOffset = (amountX > 0.f ? (r.width() + r.width() * amountX) :
                    (amountX < 0.f ? (-r.width() + r.width() * amountX) : 0.f));
    float yOffset = (amountY > 0.f ? (r.height() + r.height() * amountY) :
                    (amountY < 0.f ? (-r.height() + r.height() * amountY) : 0.f));
    return r.makeOffset(xOffset, yOffset);
}

void get_example_rects(CropRelation outputRelation, CropRelation inputRelation, bool hintContent,
                       SkRect* outputBounds, SkRect* cropRect, SkRect* contentBounds) {
    *outputBounds = kExampleBounds.makeInset(20.f, 20.f);
    switch(outputRelation) {
        case CropRelation::kCropOverlapsRect:
            *cropRect = make_overlap(*outputBounds, -0.15f, 0.15f);
            SkASSERT(cropRect->intersects(*outputBounds) &&
                     !cropRect->contains(*outputBounds) &&
                     !outputBounds->contains(*cropRect));
            break;
        case CropRelation::kCropContainsRect:
            *cropRect = make_outset(*outputBounds, 0.15f, 0.15f);
            SkASSERT(cropRect->contains(*outputBounds));
            break;
        case CropRelation::kRectContainsCrop:
            *cropRect = make_inset(*outputBounds, 0.15f, 0.15f);
            SkASSERT(outputBounds->contains(*cropRect));
            break;
        case CropRelation::kCropRectDisjoint:
            *cropRect = make_disjoint(*outputBounds, 0.15f, 0.0f);
            SkASSERT(!cropRect->intersects(*outputBounds));
            break;
    }

    // Determine content bounds for example based on computed crop rect and input relation
    if (hintContent) {
        switch(inputRelation) {
            case CropRelation::kCropOverlapsRect:
                *contentBounds = make_overlap(*cropRect, 0.075f, -0.75f);
                SkASSERT(contentBounds->intersects(*cropRect) &&
                        !contentBounds->contains(*cropRect) &&
                        !cropRect->contains(*contentBounds));
                break;
            case CropRelation::kCropContainsRect:
                *contentBounds = make_inset(*cropRect, 0.075f, 0.075f);
                SkASSERT(cropRect->contains(*contentBounds));
                break;
            case CropRelation::kRectContainsCrop:
                *contentBounds = make_outset(*cropRect, 0.1f, 0.1f);
                SkASSERT(contentBounds->contains(*cropRect));
                break;
            case CropRelation::kCropRectDisjoint:
                *contentBounds = make_disjoint(*cropRect, 0.0f, 0.075f);
                SkASSERT(!contentBounds->intersects(*cropRect));
                break;
        }
    } else {
        *contentBounds = kExampleBounds;
    }
}

// TODO(michaelludwig) - This is a useful test pattern for tile modes and filtering; should
// consolidate it with the similar version in gpu_blur_utils if the GMs remain separate at the end.
sk_sp<SkImage> make_image(SkCanvas* canvas, const SkRect* contentBounds) {
    const float w = kExampleBounds.width();
    const float h = kExampleBounds.height();

    const auto srcII = SkImageInfo::Make(SkISize::Make(SkScalarCeilToInt(w), SkScalarCeilToInt(h)),
                                         kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surf = SkSurface::MakeRaster(srcII);

    surf->getCanvas()->drawColor(SK_ColorDKGRAY);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    // Draw four horizontal lines at 1/4, 3/8, 5/8, 3/4.
    paint.setStrokeWidth(h/16.f);
    paint.setColor(SK_ColorRED);
    surf->getCanvas()->drawLine({0.f, 1.f*h/4.f}, {w, 1.f*h/4.f}, paint);
    paint.setColor(/* sea foam */ 0xFF71EEB8);
    surf->getCanvas()->drawLine({0.f, 3.f*h/8.f}, {w, 3.f*h/8.f}, paint);
    paint.setColor(SK_ColorYELLOW);
    surf->getCanvas()->drawLine({0.f, 5.f*h/8.f}, {w, 5.f*h/8.f}, paint);
    paint.setColor(SK_ColorCYAN);
    surf->getCanvas()->drawLine({0.f, 3.f*h/4.f}, {w, 3.f*h/4.f}, paint);

    // Draw four vertical lines at 1/4, 3/8, 5/8, 3/4.
    paint.setStrokeWidth(w/16.f);
    paint.setColor(/* orange */ 0xFFFFA500);
    surf->getCanvas()->drawLine({1.f*w/4.f, 0.f}, {1.f*h/4.f, h}, paint);
    paint.setColor(SK_ColorBLUE);
    surf->getCanvas()->drawLine({3.f*w/8.f, 0.f}, {3.f*h/8.f, h}, paint);
    paint.setColor(SK_ColorMAGENTA);
    surf->getCanvas()->drawLine({5.f*w/8.f, 0.f}, {5.f*h/8.f, h}, paint);
    paint.setColor(SK_ColorGREEN);
    surf->getCanvas()->drawLine({3.f*w/4.f, 0.f}, {3.f*h/4.f, h}, paint);

    // Fill everything outside of the content bounds with red since it shouldn't be sampled from.
    if (contentBounds) {
        surf->getCanvas()->clipRect(*contentBounds, SkClipOp::kDifference);
        surf->getCanvas()->clear(SK_ColorRED);
    }

    return surf->makeImageSnapshot();
}

void draw_example(
        SkCanvas* canvas,
        SkTileMode inputMode,        // the tile mode of the input to the crop filter
        SkTileMode outputMode,       // the tile mode that the crop filter outputs
        CropRelation outputRelation, // how crop rect relates to output bounds
        CropRelation inputRelation,  // how crop rect relates to content bounds
        bool hintContent) {          // whether or not contentBounds is hinted to saveLayer()
    SkASSERT(inputMode == SkTileMode::kDecal && outputMode == SkTileMode::kDecal);

    // Determine crop rect for example based on output relation
    SkRect outputBounds, cropRect, contentBounds;
    get_example_rects(outputRelation, inputRelation, hintContent,
                      &outputBounds, &cropRect, &contentBounds);

    auto image = make_image(canvas, hintContent ? &contentBounds : nullptr);

    canvas->save();
    canvas->clipRect(kExampleBounds);
    // Visualize the image tiled on the content bounds, semi-transparent
    {
        SkRect clippedContentBounds;
        if (clippedContentBounds.intersect(contentBounds, kExampleBounds)) {
            auto contentImage = image->makeSubset(clippedContentBounds.roundOut());
            SkPaint tiledPaint;
            tiledPaint.setShader(contentImage->makeShader(
                    inputMode, inputMode, SkSamplingOptions(SkFilterMode::kLinear)));
            tiledPaint.setAlphaf(0.15f);

            canvas->save();
            canvas->translate(clippedContentBounds.fLeft, clippedContentBounds.fTop);
            canvas->drawPaint(tiledPaint);
            canvas->restore();
        }
    }

    // Build filter, clip, save layer, draw, restore - the interesting part is in the tile modes
    // and how the various bounds intersect each other.
    {
        sk_sp<SkImageFilter> filter = SkImageFilters::Blur(4.f, 4.f, nullptr);
        filter = SkMakeCropImageFilter(cropRect, std::move(filter));
        SkPaint layerPaint;
        layerPaint.setImageFilter(std::move(filter));

        canvas->save();
        canvas->clipRect(outputBounds);
        canvas->saveLayer(hintContent ? &contentBounds : nullptr, &layerPaint);

        canvas->drawImageRect(image.get(), contentBounds, contentBounds,
                              SkSamplingOptions(SkFilterMode::kLinear), nullptr,
                              SkCanvas::kFast_SrcRectConstraint);
        canvas->restore();
        canvas->restore();
    }

    // Visualize bounds after the actual rendering.
    {
        SkPaint border;
        border.setStyle(SkPaint::kStroke_Style);

        border.setColor(kOutputBoundsColor);
        canvas->drawRect(outputBounds, border);

        border.setColor(kCropRectColor);
        canvas->drawRect(cropRect, border);

        if (hintContent) {
            border.setColor(kContentBoundsColor);
            canvas->drawRect(contentBounds, border);
        }
    }

    canvas->restore();
}


// Draws 2x2 examples for a given input/output tile mode that show 4 relationships between the
// output bounds and the crop rect (intersect, output contains crop, crop contains output, and
// no intersection).
static constexpr SkRect kPaddedTileBounds = {kExampleBounds.fLeft,
                                             kExampleBounds.fTop,
                                             2.f * (kExampleBounds.fRight + 1.f),
                                             2.f * (kExampleBounds.fBottom + 1.f)};
void draw_example_tile(
        SkCanvas* canvas,
        SkTileMode inputMode,
        SkTileMode outputMode,
        CropRelation inputRelation,
        bool hintContent) {
    auto drawQuadrant = [&](int tx, int ty, CropRelation outputRelation) {
        canvas->save();
        canvas->translate(tx * (kExampleBounds.fRight + 1.f), ty * (kExampleBounds.fBottom + 1.f));
        draw_example(canvas, inputMode, outputMode, outputRelation, inputRelation, hintContent);
        canvas->restore();
    };

    // The 4 examples, here Rect refers to the output bounds
    drawQuadrant(0, 0, CropRelation::kCropOverlapsRect); // top left
    drawQuadrant(1, 0, CropRelation::kRectContainsCrop); // top right
    drawQuadrant(0, 1, CropRelation::kCropRectDisjoint); // bot left
    drawQuadrant(1, 1, CropRelation::kCropContainsRect); // bot right

    // Draw dotted lines in the 1px gap between examples
    SkPaint dottedLine;
    dottedLine.setColor(SK_ColorGRAY);
    dottedLine.setStyle(SkPaint::kStroke_Style);
    dottedLine.setStrokeCap(SkPaint::kSquare_Cap);
    static const float kDots[2] = {0.f, 5.f};
    dottedLine.setPathEffect(SkDashPathEffect::Make(kDots, 2, 0.f));

    canvas->drawLine({kPaddedTileBounds.fLeft + 0.5f, kPaddedTileBounds.centerY() - 0.5f},
                     {kPaddedTileBounds.fRight - 0.5f, kPaddedTileBounds.centerY() - 0.5f},
                     dottedLine);
    canvas->drawLine({kPaddedTileBounds.centerX() - 0.5f, kPaddedTileBounds.fTop + 0.5f},
                     {kPaddedTileBounds.centerX() - 0.5f, kPaddedTileBounds.fBottom - 0.5f},
                     dottedLine);
}

// Draw 5 example tiles in a column for 5 relationships between content bounds and crop rect:
// no content hint, intersect, content contains crop, crop contains content, and no intersection
static constexpr SkRect kPaddedColumnBounds = {kPaddedTileBounds.fLeft,
                                               kPaddedTileBounds.fTop,
                                               kPaddedTileBounds.fRight,
                                               5.f * kPaddedTileBounds.fBottom - 1.f};
void draw_example_column(
        SkCanvas* canvas,
        SkTileMode inputMode,
        SkTileMode outputMode) {
    const std::pair<CropRelation, bool> inputRelations[5] = {
            { CropRelation::kCropOverlapsRect, false },
            { CropRelation::kCropOverlapsRect, true },
            { CropRelation::kCropContainsRect, true },
            { CropRelation::kRectContainsCrop, true },
            { CropRelation::kCropRectDisjoint, true }
        };

    canvas->save();
    for (auto [inputRelation, hintContent] : inputRelations) {
        draw_example_tile(canvas, inputMode, outputMode, inputRelation, hintContent);
        canvas->translate(0.f, kPaddedTileBounds.fBottom);
    }

    canvas->restore();
}

// Draw 5x1 grid of examples covering supported input tile modes and crop rect relations
static constexpr int kNumRows = 5;
static constexpr int kNumCols = 1;
static constexpr float kGridWidth = kNumCols * kPaddedColumnBounds.fRight - 1.f;

void draw_example_grid(
        SkCanvas* canvas,
        SkTileMode outputMode) {
    canvas->save();
    for (auto inputMode : {SkTileMode::kDecal}) {
        draw_example_column(canvas, inputMode, outputMode);
        canvas->translate(kPaddedColumnBounds.fRight, 0.f);
    }
    canvas->restore();

    // Draw dashed lines between rows and columns
    SkPaint dashedLine;
    dashedLine.setColor(SK_ColorGRAY);
    dashedLine.setStyle(SkPaint::kStroke_Style);
    dashedLine.setStrokeCap(SkPaint::kSquare_Cap);
    static const float kDashes[2] = {5.f, 15.f};
    dashedLine.setPathEffect(SkDashPathEffect::Make(kDashes, 2, 0.f));

    for (int y = 1; y < kNumRows; ++y) {
        canvas->drawLine({0.5f, y * kPaddedTileBounds.fBottom - 0.5f},
                         {kGridWidth - 0.5f, y * kPaddedTileBounds.fBottom - 0.5f}, dashedLine);
    }
    for (int x = 1; x < kNumCols; ++x) {
        canvas->drawLine({x * kPaddedTileBounds.fRight - 0.5f, 0.5f},
                         {x * kPaddedTileBounds.fRight - 0.5f, kPaddedColumnBounds.fBottom - 0.5f},
                         dashedLine);
    }
}

}  // namespace

namespace skiagm {

class CropImageFilterGM : public GM {
public:
    CropImageFilterGM(SkTileMode outputMode) : fOutputMode(outputMode) {}

protected:
    SkISize onISize() override {
        return {SkScalarRoundToInt(kGridWidth), SkScalarRoundToInt(kPaddedColumnBounds.fBottom)};
    }
    SkString onShortName() override {
        SkString name("crop_imagefilter_");
        switch (fOutputMode) {
            case SkTileMode::kDecal:  name.append("decal");  break;
            case SkTileMode::kClamp:  name.append("clamp");  break;
            case SkTileMode::kRepeat: name.append("repeat"); break;
            case SkTileMode::kMirror: name.append("mirror"); break;
        }
        return name;
    }

    void onDraw(SkCanvas* canvas) override {
        draw_example_grid(canvas, fOutputMode);
    }

private:
    SkTileMode fOutputMode;
};

DEF_GM( return new CropImageFilterGM(SkTileMode::kDecal); )
// TODO(michaelludwig) - will add GM defs for other output tile modes once supported

}  // namespace skiagm
