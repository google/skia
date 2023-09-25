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
#include "include/core/SkPathEffect.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"

#include "tools/Resources.h"
#include "tools/ToolUtils.h"


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

    SkAssertResult(cropRect->intersect(kExampleBounds));

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

        SkAssertResult(contentBounds->intersect(kExampleBounds));
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
                                         kN32_SkColorType, kPremul_SkAlphaType);
    auto surf = SkSurfaces::Raster(srcII);

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
        SkRect buffer = contentBounds->makeOutset(1.f, 1.f);
        surf->getCanvas()->clipRect(buffer, SkClipOp::kDifference);
        surf->getCanvas()->clear(SK_ColorRED);
    }

    return surf->makeImageSnapshot();
}

// Subset 'image' to contentBounds, apply 'contentTile' mode to fill 'cropRect'-sized image.
sk_sp<SkImage> make_cropped_image(sk_sp<SkImage> image,
                                  const SkRect& contentBounds,
                                  SkTileMode contentTile,
                                  const SkRect& cropRect) {
    auto surface = SkSurfaces::Raster(
            image->imageInfo().makeWH(SkScalarCeilToInt(cropRect.width()),
                                      SkScalarCeilToInt(cropRect.height())));
    auto content = image->makeSubset(nullptr,
                                     contentTile == SkTileMode::kDecal ? contentBounds.roundOut()
                                                                       : contentBounds.roundIn());
    if (!content || !surface) {
        return nullptr;
    }
    SkPaint tiledContent;
    tiledContent.setShader(content->makeShader(contentTile, contentTile,
                                               SkFilterMode::kNearest,
                                               SkMatrix::Translate(contentBounds.left(),
                                                                   contentBounds.top())));
    surface->getCanvas()->translate(-cropRect.left(), -cropRect.top());
    surface->getCanvas()->drawPaint(tiledContent);
    return surface->makeImageSnapshot();
}

void draw_example_tile(
        SkCanvas* canvas,
        SkTileMode inputMode,         // the tile mode applied to content bounds
        CropRelation inputRelation,   // how crop rect relates to content bounds
        bool hintContent,             // whether or not contentBounds is hinted to saveLayer()
        SkTileMode outputMode,        // the tile mode applied to the crop rect output
        CropRelation outputRelation) {// how crop rect relates to output bounds (clip pre-saveLayer)

    // Determine crop rect for example based on output relation
    SkRect outputBounds, cropRect, contentBounds;
    get_example_rects(outputRelation, inputRelation, hintContent,
                      &outputBounds, &cropRect, &contentBounds);
    SkASSERT(kExampleBounds.contains(outputBounds) &&
             kExampleBounds.contains(cropRect) &&
             kExampleBounds.contains(contentBounds));

    auto image = make_image(canvas, hintContent ? &contentBounds : nullptr);

    canvas->save();
    // Visualize the image tiled on the content bounds (blue border) and then tiled on the crop
    // rect (green) border, semi-transparent
    {
        auto cropImage = ToolUtils::MakeTextureImage(
                canvas, make_cropped_image(image, contentBounds, inputMode, cropRect));
        if (cropImage) {
            SkPaint tiledPaint;
            tiledPaint.setShader(cropImage->makeShader(outputMode, outputMode,
                                                       SkFilterMode::kNearest,
                                                       SkMatrix::Translate(cropRect.left(),
                                                                           cropRect.top())));
            tiledPaint.setAlphaf(0.25f);

            canvas->save();
            canvas->clipRect(kExampleBounds);
            canvas->drawPaint(tiledPaint);
            canvas->restore();
        }
    }

    // Build filter, clip, save layer, draw, restore - the interesting part is in the tile modes
    // and how the various bounds intersect each other.
    {
        sk_sp<SkImageFilter> filter = SkImageFilters::Crop(contentBounds, inputMode, nullptr);
        filter = SkImageFilters::Blur(4.f, 4.f, std::move(filter));
        filter = SkImageFilters::Crop(cropRect, outputMode, std::move(filter));
        SkPaint layerPaint;
        layerPaint.setImageFilter(std::move(filter));

        canvas->save();
        canvas->clipRect(outputBounds);
        canvas->saveLayer(hintContent ? &contentBounds : nullptr, &layerPaint);

        auto tmp = ToolUtils::MakeTextureImage(canvas, image);
        canvas->drawImageRect(tmp, contentBounds, contentBounds,
                              SkSamplingOptions(SkFilterMode::kNearest), nullptr,
                              SkCanvas::kStrict_SrcRectConstraint);
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

// Draw 5 example tiles in a column for 5 relationships between content bounds and crop rect:
// no content hint, intersect, content contains crop, crop contains content, and no intersection
void draw_example_column(
        SkCanvas* canvas,
        SkTileMode inputMode,
        SkTileMode outputMode,
        CropRelation outputRelation) {
    const std::pair<CropRelation, bool> inputRelations[5] = {
            { CropRelation::kCropOverlapsRect, false },
            { CropRelation::kCropOverlapsRect, true },
            { CropRelation::kCropContainsRect, true },
            { CropRelation::kRectContainsCrop, true },
            { CropRelation::kCropRectDisjoint, true }
        };

    canvas->save();
    for (auto [inputRelation, hintContent] : inputRelations) {
        draw_example_tile(canvas, inputMode, inputRelation, hintContent,
                          outputMode, outputRelation);
        canvas->translate(0.f, kExampleBounds.fBottom + 1.f);
    }

    canvas->restore();
}

// Draw 5x4 grid of examples covering supported input tile modes and crop rect relations
static constexpr int kNumRows = 5;
static constexpr int kNumCols = 4;
static constexpr float kGridWidth = kNumCols * (kExampleBounds.fRight+1.f) - 1.f;
static constexpr float kGridHeight = kNumRows * (kExampleBounds.fBottom+1.f) - 1.f;

void draw_example_grid(
        SkCanvas* canvas,
        SkTileMode inputMode,
        SkTileMode outputMode) {
    canvas->save();
    for (auto outputRelation : { CropRelation::kCropOverlapsRect,
                                 CropRelation::kCropContainsRect,
                                 CropRelation::kRectContainsCrop,
                                 CropRelation::kCropRectDisjoint }) {
        draw_example_column(canvas, inputMode, outputMode, outputRelation);
        canvas->translate(kExampleBounds.fRight + 1.f, 0.f);
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
        canvas->drawLine({0.5f, y * (kExampleBounds.fBottom+1.f) - 0.5f},
                         {kGridWidth - 0.5f, y * (kExampleBounds.fBottom+1.f) - 0.5f},
                         dashedLine);
    }
    for (int x = 1; x < kNumCols; ++x) {
        canvas->drawLine({x * (kExampleBounds.fRight+1.f) - 0.5f, 0.5f},
                         {x * (kExampleBounds.fRight+1.f) - 0.5f, kGridHeight - 0.5f},
                         dashedLine);
    }
}

}  // namespace

namespace skiagm {

class CropImageFilterGM : public GM {
public:
    CropImageFilterGM(SkTileMode inputMode, SkTileMode outputMode)
            : fInputMode(inputMode)
            , fOutputMode(outputMode) {}

protected:
    SkISize getISize() override {
        return {SkScalarRoundToInt(4.f * (kExampleBounds.fRight + 1.f) - 1.f),
                SkScalarRoundToInt(5.f * (kExampleBounds.fBottom + 1.f) - 1.f)};
    }
    SkString getName() const override {
        SkString name("crop_imagefilter_");
        switch(fInputMode) {
            case SkTileMode::kDecal:  name.append("decal");  break;
            case SkTileMode::kClamp:  name.append("clamp");  break;
            case SkTileMode::kRepeat: name.append("repeat"); break;
            case SkTileMode::kMirror: name.append("mirror"); break;
        }
        name.append("-in_");

        switch (fOutputMode) {
            case SkTileMode::kDecal:  name.append("decal");  break;
            case SkTileMode::kClamp:  name.append("clamp");  break;
            case SkTileMode::kRepeat: name.append("repeat"); break;
            case SkTileMode::kMirror: name.append("mirror"); break;
        }
        name.append("-out");
        return name;
    }

    void onDraw(SkCanvas* canvas) override {
        draw_example_grid(canvas, fInputMode, fOutputMode);
    }

private:
    SkTileMode fInputMode;
    SkTileMode fOutputMode;
};

DEF_GM( return new CropImageFilterGM(SkTileMode::kDecal, SkTileMode::kDecal); )
DEF_GM( return new CropImageFilterGM(SkTileMode::kDecal, SkTileMode::kClamp); )
DEF_GM( return new CropImageFilterGM(SkTileMode::kDecal, SkTileMode::kRepeat); )
DEF_GM( return new CropImageFilterGM(SkTileMode::kDecal, SkTileMode::kMirror); )

DEF_GM( return new CropImageFilterGM(SkTileMode::kClamp, SkTileMode::kDecal); )
DEF_GM( return new CropImageFilterGM(SkTileMode::kClamp, SkTileMode::kClamp); )
DEF_GM( return new CropImageFilterGM(SkTileMode::kClamp, SkTileMode::kRepeat); )
DEF_GM( return new CropImageFilterGM(SkTileMode::kClamp, SkTileMode::kMirror); )

DEF_GM( return new CropImageFilterGM(SkTileMode::kRepeat, SkTileMode::kDecal); )
DEF_GM( return new CropImageFilterGM(SkTileMode::kRepeat, SkTileMode::kClamp); )
DEF_GM( return new CropImageFilterGM(SkTileMode::kRepeat, SkTileMode::kRepeat); )
DEF_GM( return new CropImageFilterGM(SkTileMode::kRepeat, SkTileMode::kMirror); )

DEF_GM( return new CropImageFilterGM(SkTileMode::kMirror, SkTileMode::kDecal); )
DEF_GM( return new CropImageFilterGM(SkTileMode::kMirror, SkTileMode::kClamp); )
DEF_GM( return new CropImageFilterGM(SkTileMode::kMirror, SkTileMode::kRepeat); )
DEF_GM( return new CropImageFilterGM(SkTileMode::kMirror, SkTileMode::kMirror); )

}  // namespace skiagm
