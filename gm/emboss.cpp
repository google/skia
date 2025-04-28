/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFont.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "include/utils/SkTextUtils.h"
#include "src/core/SkBlurMask.h"
#include "src/effects/SkEmbossMaskFilter.h"
#include "tools/fonts/FontToolUtils.h"

static sk_sp<SkImage> make_bm() {
    auto surf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100));

    SkPaint paint;
    paint.setAntiAlias(true);
    surf->getCanvas()->drawCircle(50, 50, 50, paint);
    return surf->makeImageSnapshot();
}

class EmbossGM : public skiagm::GM {
public:
    EmbossGM() {
    }

protected:
    SkString getName() const override { return SkString("emboss"); }

    SkISize getISize() override { return SkISize::Make(600, 120); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        auto img = make_bm();
        canvas->drawImage(img, 10, 10);
        canvas->translate(img->width() + SkIntToScalar(10), 0);

        paint.setMaskFilter(SkEmbossMaskFilter::Make(
            SkBlurMask::ConvertRadiusToSigma(3),
            { { SK_Scalar1, SK_Scalar1, SK_Scalar1 }, 0, 128, 16*2 }));
        canvas->drawImage(img, 10, 10, SkSamplingOptions(), &paint);
        canvas->translate(img->width() + SkIntToScalar(10), 0);

        // this combination of emboss+colorfilter used to crash -- so we exercise it to
        // confirm that we have a fix.
        paint.setColorFilter(SkColorFilters::Blend(0xFFFF0000, SkBlendMode::kSrcATop));
        canvas->drawImage(img, 10, 10, SkSamplingOptions(), &paint);
        canvas->translate(img->width() + SkIntToScalar(10), 0);

        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(10));
        paint.setMaskFilter(SkEmbossMaskFilter::Make(
            SkBlurMask::ConvertRadiusToSigma(4),
            { { SK_Scalar1, SK_Scalar1, SK_Scalar1 }, 0, 128, 16*2 }));
        paint.setColorFilter(nullptr);
        paint.setShader(SkShaders::Color(SK_ColorBLUE));
        paint.setDither(true);
        canvas->drawCircle(SkIntToScalar(50), SkIntToScalar(50),
                           SkIntToScalar(30), paint);
        canvas->translate(SkIntToScalar(100), 0);

        SkFont font = SkFont(ToolUtils::DefaultPortableTypeface(), 50);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawString("Hello", 0, 50, font, paint);

        paint.setShader(nullptr);
        paint.setColor(SK_ColorGREEN);
        canvas->drawString("World", 0, 100, font, paint);
    }

private:
    using INHERITED = skiagm::GM;
};

DEF_GM(return new EmbossGM;)


void draw_emboss_example(SkCanvas* canvas,
                         const SkRect& bounds,
                         const SkEmbossMaskFilter::Light& light,
                         SkScalar blurSigma,
                         const char* label) {
    canvas->save();
    canvas->clipRect(bounds);
    canvas->translate(bounds.left(), bounds.top());

    SkRect contentBounds = SkRect::MakeWH(bounds.width(), bounds.height());

    sk_sp<SkMaskFilter> embossFilter = SkEmbossMaskFilter::Make(blurSigma, light);
    if (!embossFilter) {
        SkDebugf("Failed to create emboss filter for: %s\n", label);
        return;
    }

    SkPaint paint1, paint2;
    paint1.setAntiAlias(true);
    paint1.setColor(SK_ColorRED);
    paint1.setMaskFilter(embossFilter); // Apply the same filter

    paint2.setAntiAlias(true);
    paint2.setColor(SK_ColorBLUE);
    paint2.setMaskFilter(embossFilter); // Apply the same filter

    // Draw shapes overlapping
    SkScalar radius = contentBounds.width() * 0.25f;
    SkScalar offset = radius * 0.3f;

    canvas->drawCircle(contentBounds.centerX() - offset, contentBounds.centerY(), radius, paint1);

    SkRect rect = SkRect::MakeXYWH(contentBounds.centerX() + offset - radius,
                                   contentBounds.centerY() - radius,
                                   radius * 2, radius * 2);
    canvas->drawRect(rect, paint2);

    SkPaint textPaint;
    textPaint.setColor(SK_ColorBLACK);
    SkTextUtils::DrawString(canvas, label, 5, 15, ToolUtils::DefaultPortableFont(), textPaint);

    canvas->restore();
}

std::map<SkBlendMode, std::string> blendModeStrs = {
    {SkBlendMode::kSrc, "Src"},
    {SkBlendMode::kDst, "Dst"},
    {SkBlendMode::kSrcOver, "SrcOver"},
    {SkBlendMode::kDstOver, "DstOver"},
    {SkBlendMode::kSrcIn, "SrcIn"},
    {SkBlendMode::kDstIn, "DstIn"},
    {SkBlendMode::kSrcOut, "SrcOut"},
    {SkBlendMode::kDstOut, "DstOut"},
    {SkBlendMode::kSrcATop, "SrcATop"},
    {SkBlendMode::kDstATop, "DstATop"},
    {SkBlendMode::kXor, "Xor"},
    {SkBlendMode::kPlus, "Plus"},
    {SkBlendMode::kModulate, "Modulate"},
    {SkBlendMode::kScreen, "Screen"},
    {SkBlendMode::kOverlay, "Overlay"},
    {SkBlendMode::kMultiply, "Multiply"},
    {SkBlendMode::kDarken, "Darken"},
    {SkBlendMode::kLighten, "Lighten"},
};


void draw_emboss_blend_example(SkCanvas* canvas,
                               const SkRect& bounds,
                               sk_sp<SkMaskFilter> embossFilter,
                               SkBlendMode blendMode,
                               const char* label) {
    canvas->save();
    canvas->clipRect(bounds);
    canvas->translate(bounds.left(), bounds.top());

    SkRect contentBounds = SkRect::MakeWH(bounds.width(), bounds.height());

    SkPaint paint1, paint2;
    paint1.setAntiAlias(true);
    paint1.setColor(SK_ColorRED);

    paint2.setAntiAlias(true);
    paint2.setColor(SK_ColorBLUE);
    paint2.setMaskFilter(embossFilter);
    paint2.setBlendMode(blendMode);     // Apply the specified blend mode

    // Draw shapes overlapping
    SkScalar radius = contentBounds.width() * 0.25f;
    SkScalar offset = radius * 0.3f;

    // Draw red circle
    canvas->drawCircle(contentBounds.centerX() - offset, contentBounds.centerY(), radius, paint1);

    // Draw blue rect over the red circle
    SkRect rect = SkRect::MakeXYWH(contentBounds.centerX() + offset - radius,
                                   contentBounds.centerY() - radius,
                                   radius * 2, radius * 2);
    canvas->drawRect(rect, paint2);

    SkPaint textPaint;
    textPaint.setColor(SK_ColorRED);
    SkTextUtils::DrawString(canvas, label, 5, 15, ToolUtils::DefaultPortableFont(), textPaint);

    canvas->restore();
}



DEF_SIMPLE_GM(embossmaskfilter, canvas, 640, 960) {
    const int totalWidth = 640;
    const int topHeight = 480;
    const int bottomHeight = 480;

    // Test different values for light:
    {
        const int gridCols = 2;
        const int gridRows = 2;
        const float cellWidth = (float)totalWidth / gridCols;
        const float cellHeight = (float)topHeight / gridRows;

        std::vector<SkEmbossMaskFilter::Light> embossRecs;
        std::vector<std::string> labels;
        SkScalar blurSigma = SkDoubleToScalar(2.5);

        SkEmbossMaskFilter::Light rec;

        // Example 1: Top-left light, medium ambient, low specular
        rec = {{-1.0f, -1.0f, 1.0f}, 0, 80, 16};
        embossRecs.push_back(rec);
        labels.push_back("Light: TL, Amb: 80, Spec: 16");

        // Example 2: Bottom-right light, high ambient, low specular
        rec = {{1.0f, 1.0f, 0.8f}, 0, 180, 16};
        embossRecs.push_back(rec);
        labels.push_back("Light: BR, Amb: 180, Spec: 16");

        // Example 3: Direct top light, low ambient, high specular
        rec = {{0.0f, -1.0f, 1.0f}, 0, 30, 128};
        embossRecs.push_back(rec);
        labels.push_back("Light: Top, Amb: 30, Spec: 128");

        // Example 4: Left light, medium ambient, medium specular
        rec = {{-1.0f, 0.0f, 0.5f}, 0, 80, 64};
        embossRecs.push_back(rec);
        labels.push_back("Light: Left, Amb: 80, Spec: 64");

        size_t exampleIndex = 0;
        for (int r = 0; r < gridRows; ++r) {
            for (int c = 0; c < gridCols; ++c) {
                if (exampleIndex < embossRecs.size()) {
                    // Calculate bounds within the top half
                    SkRect cellBounds = SkRect::MakeXYWH(c * cellWidth, r * cellHeight, cellWidth, cellHeight);
                    draw_emboss_example(canvas,
                                        cellBounds.makeInset(10, 10), // Add padding
                                        embossRecs[exampleIndex],
                                        blurSigma,
                                        labels[exampleIndex].c_str());
                    exampleIndex++;
                }
            }
        }
    }

    // Test different blend modes with the mask filter
    {
        const int gridCols = 6;
        const int gridRows = 3;
        const float cellWidth = (float)totalWidth / gridCols;
        const float cellHeight = (float)bottomHeight / gridRows;

        SkEmbossMaskFilter::Light embossRec = {{-0.707f, -0.707f, 0.707f}, 0, 60, 48};
        SkScalar blurSigma = SkDoubleToScalar(2.0);

        sk_sp<SkMaskFilter> embossMF = SkEmbossMaskFilter::Make(blurSigma, embossRec);
        if (!embossMF) {
            SkDebugf("Failed to create shared emboss filter for blend examples.\n");
            return;
        }


        // Blend Modes to Test
        std::vector<SkBlendMode> blendModesToTest = {
            SkBlendMode::kSrc, // Incorrect for Graphite
            SkBlendMode::kDst,
            SkBlendMode::kSrcOver,
            SkBlendMode::kDstOver,
            SkBlendMode::kSrcIn, // Incorrect for Graphite
            SkBlendMode::kDstIn, // Incorrect for Graphite
            SkBlendMode::kSrcOut, // Incorrect for Graphite
            SkBlendMode::kDstOut,
            SkBlendMode::kSrcATop,
            SkBlendMode::kDstATop, // Incorrect for Graphite
            SkBlendMode::kXor,
            SkBlendMode::kPlus, // Incorrect for Graphite
            SkBlendMode::kModulate,
            SkBlendMode::kScreen,
            SkBlendMode::kOverlay,
            SkBlendMode::kMultiply,
            SkBlendMode::kDarken,
            SkBlendMode::kLighten,
        };

        size_t exampleIndex = 0;
        for (int r = 0; r < gridRows; ++r) {
            for (int c = 0; c < gridCols; ++c) {
                if (exampleIndex < blendModesToTest.size()) {
                    SkBlendMode currentMode = blendModesToTest[exampleIndex];
                    std::string modeName = "Unknown";
                    auto it = blendModeStrs.find(currentMode);
                    if (it != blendModeStrs.end()) {
                        modeName = it->second;
                    }

                    SkString label;
                    label.appendf("%s", modeName.c_str());

                    // Calculate bounds within the bottom half (offset by topHeight)
                    SkRect cellBounds = SkRect::MakeXYWH(c * cellWidth,
                                                         topHeight + r * cellHeight, // Offset Y
                                                         cellWidth, cellHeight);
                    draw_emboss_blend_example(canvas,
                                              cellBounds.makeInset(10, 10), // Add padding
                                              embossMF, // Use the same filter instance
                                              currentMode,
                                              label.c_str());
                    exampleIndex++;
                }
            }
        }
    }
}

DEF_SIMPLE_GM(smallemboss, canvas, 50, 50) {
    sk_sp<SkMaskFilter> embossFilter = SkEmbossMaskFilter::Make(3, {{1, 1, 1}, 0, 0, 16});
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);
    paint.setMaskFilter(embossFilter);

    auto surface = canvas->makeSurface(SkImageInfo::MakeN32Premul(50, 50));
    if (!surface) {
      surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(50, 50));
    }
    SkCanvas* canv = surface->getCanvas();

    SkRect rect = SkRect::MakeXYWH(1, 1, 3, 3);
    canv->drawRect(rect, paint);

    canvas->scale(30, 30);
    auto img = surface->makeImageSnapshot();
    canvas->drawImage(img, 0, 0);
}
