/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkRectPriv.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/viewer/Slide.h"

static constexpr float kLineHeight = 16.f;
static constexpr float kLineInset = 8.f;

static float print_size(SkCanvas* canvas, const char* prefix,
                        std::optional<SkIRect> rect,
                        float x, float y, const SkFont& font, const SkPaint& paint) {
    canvas->drawString(prefix, x, y, font, paint);
    y += kLineHeight;
    SkString sz;
    if (rect) {
        sz.appendf("%d x %d", rect->width(), rect->height());
    } else {
        sz.appendf("infinite");
    }
    canvas->drawString(sz, x, y, font, paint);
    return y + kLineHeight;
}

static float print_info(SkCanvas* canvas,
                        const skif::LayerSpace<SkIRect>& layerContentBounds,
                        const skif::DeviceSpace<SkIRect>& outputBounds,
                        std::optional<skif::DeviceSpace<SkIRect>> hintedOutputBounds,
                        const skif::LayerSpace<SkIRect>& unhintedLayerBounds) {
    SkFont font(ToolUtils::DefaultTypeface(), 12);
    SkPaint text;
    text.setAntiAlias(true);

    float y = kLineHeight;

    text.setColor(SK_ColorRED);
    y = print_size(canvas, "Content (in layer)", SkIRect(layerContentBounds),
                   kLineInset, y, font, text);
    text.setColor(SK_ColorDKGRAY);
    y = print_size(canvas, "Target (in device)", SkIRect(outputBounds),
                   kLineInset, y, font, text);
    text.setColor(SK_ColorBLUE);
    y = print_size(canvas, "Output (w/ hint)",
                   hintedOutputBounds ? SkIRect(*hintedOutputBounds) : std::optional<SkIRect>{},
                   kLineInset, y, font, text);
    text.setColor(SK_ColorGREEN);
    y = print_size(canvas, "Input (w/ no hint)", SkIRect(unhintedLayerBounds),
                   kLineInset, y, font, text);

    return y;
}

static void print_label(SkCanvas* canvas, float x, float y, float value) {
    SkFont font(ToolUtils::DefaultTypeface(), 12);
    SkPaint text;
    text.setAntiAlias(true);

    SkString label;
    label.printf("%.3f", value);

    canvas->drawString(label, x, y + kLineHeight / 2.f, font, text);
}

static SkPaint line_paint(SkColor color, bool dashed = false) {
    SkPaint paint;
    paint.setColor(color);
    paint.setStrokeWidth(0.f);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    if (dashed) {
        SkScalar dash[2] = {10.f, 10.f};
        paint.setPathEffect(SkDashPathEffect::Make(dash, 2, 0.f));
    }
    return paint;
}

static SkPath create_axis_path(const SkRect& rect, float axisSpace) {
    SkPath localSpace;
    for (float y = rect.fTop + axisSpace; y <= rect.fBottom; y += axisSpace) {
        localSpace.moveTo(rect.fLeft, y);
        localSpace.lineTo(rect.fRight, y);
    }
    for (float x = rect.fLeft + axisSpace; x <= rect.fRight; x += axisSpace) {
        localSpace.moveTo(x, rect.fTop);
        localSpace.lineTo(x, rect.fBottom);
    }
    return localSpace;
}

static const SkColor4f kScaleGradientColors[] =
                { { 0.05f, 0.0f, 6.f,  1.f },   // Severe downscaling, s < 1/8, log(s) < -3
                  { 0.6f,  0.6f, 0.8f, 0.6f },  // Okay downscaling,   s < 1/2, log(s) < -1
                  { 1.f,   1.f,  1.f,  0.2f },  // No scaling,         s = 1,   log(s) = 0
                  { 0.95f, 0.6f, 0.5f, 0.6f },  // Okay upscaling,     s > 2,   log(s) > 1
                  { 0.8f,  0.1f, 0.f,  1.f } }; // Severe upscaling,   s > 8,   log(s) > 3
static const SkScalar kLogScaleFactors[] = { -3.f, -1.f, 0.f, 1.f, 3.f };
static const SkScalar kGradientStops[] = { 0.f, 0.33333f, 0.5f, 0.66667f, 1.f };
static const int kStopCount = (int) std::size(kScaleGradientColors);

static void draw_scale_key(SkCanvas* canvas, float y) {
    SkRect key = SkRect::MakeXYWH(15.f, y + 30.f, 15.f, 100.f);
    SkPoint pts[] = {{key.centerX(), key.fTop}, {key.centerX(), key.fBottom}};
    sk_sp<SkShader> gradient = SkGradientShader::MakeLinear(
            pts, kScaleGradientColors, nullptr, kGradientStops, kStopCount, SkTileMode::kClamp,
            SkGradientShader::kInterpolateColorsInPremul_Flag, nullptr);
    SkPaint keyPaint;
    keyPaint.setShader(gradient);
    canvas->drawRect(key, keyPaint);
    for (int i = 0; i < kStopCount; ++i) {
        print_label(canvas, key.fRight + 5.f, key.fTop + kGradientStops[i] * key.height(),
                    SkScalarPow(2.f, kLogScaleFactors[i]));
    }
}

static void draw_scale_factors(SkCanvas* canvas, const skif::Mapping& mapping, const SkRect& rect) {
    SkPoint testPoints[5];
    testPoints[0] = {rect.centerX(), rect.centerY()};
    rect.toQuad(testPoints + 1);
    for (int i = 0; i < 5; ++i) {
        float scale = SkMatrixPriv::DifferentialAreaScale(
                mapping.layerToDevice(),
                SkPoint(mapping.paramToLayer(skif::ParameterSpace<SkPoint>(testPoints[i]))));
        SkColor4f color = {0.f, 0.f, 0.f, 1.f};

        if (SkScalarIsFinite(scale)) {
            float logScale = SkScalarLog2(scale);
            for (int j = 0; j <= kStopCount; ++j) {
                if (j == kStopCount) {
                    color = kScaleGradientColors[j - 1];
                    break;
                } else if (kLogScaleFactors[j] >= logScale) {
                    if (j == 0) {
                        color = kScaleGradientColors[0];
                    } else {
                        SkScalar t = (logScale - kLogScaleFactors[j - 1]) /
                                    (kLogScaleFactors[j] - kLogScaleFactors[j - 1]);

                        SkColor4f a = kScaleGradientColors[j - 1] * (1.f - t);
                        SkColor4f b = kScaleGradientColors[j] * t;
                        color = {a.fR + b.fR, a.fG + b.fG, a.fB + b.fB, a.fA + b.fA};
                    }
                    break;
                }
            }
        }

        SkPaint p;
        p.setAntiAlias(true);
        p.setColor4f(color, nullptr);
        canvas->drawRect(SkRect::MakeLTRB(testPoints[i].fX - 4.f, testPoints[i].fY - 4.f,
                                          testPoints[i].fX + 4.f, testPoints[i].fY + 4.f), p);
    }
}

class FilterBoundsSample : public Slide {
public:
    FilterBoundsSample() { fName = "FilterBounds"; }

    void load(SkScalar w, SkScalar h) override {
        fBlur = SkImageFilters::Blur(8.f, 8.f, nullptr);
        fImage = ToolUtils::create_checkerboard_image(
                300, 300, SK_ColorMAGENTA, SK_ColorLTGRAY, 50);
    }

    void draw(SkCanvas* canvas) override {
        // The local content, e.g. what would be submitted to drawRect or the bounds to saveLayer
        const SkRect localContentRect = SkRect::MakeLTRB(100.f, 20.f, 180.f, 140.f);
        SkMatrix ctm = canvas->getLocalToDeviceAs3x3();

        // Base rendering of a filter
        SkPaint blurPaint;
        blurPaint.setImageFilter(fBlur);
        canvas->saveLayer(&localContentRect, &blurPaint);
        canvas->drawImageRect(fImage.get(), localContentRect, localContentRect,
                              SkSamplingOptions(SkFilterMode::kLinear),
                              nullptr, SkCanvas::kFast_SrcRectConstraint);
        canvas->restore();

        // Now visualize the underlying bounds calculations used to determine the layer for the blur
        SkIRect target = ctm.mapRect(localContentRect).roundOut();
        if (!target.intersect(SkIRect::MakeWH(canvas->imageInfo().width(),
                                              canvas->imageInfo().height()))) {
            return;
        }
        skif::DeviceSpace<SkIRect> targetOutput(target);
        skif::ParameterSpace<SkRect> contentBounds(localContentRect);
        skif::ParameterSpace<SkPoint> contentCenter({localContentRect.centerX(),
                                                     localContentRect.centerY()});
        skif::Mapping mapping;
        SkAssertResult(mapping.decomposeCTM(ctm, fBlur.get(), contentCenter));

        // Add axis lines, to show perspective distortion
        canvas->save();
        canvas->setMatrix(mapping.layerToDevice());
        canvas->drawPath(create_axis_path(SkRect(mapping.paramToLayer(contentBounds)), 20.f),
                         line_paint(SK_ColorGRAY));
        canvas->restore();

        // Visualize scale factors at the four corners and center of the local rect
        draw_scale_factors(canvas, mapping, localContentRect);

        // The device content rect, e.g. the clip bounds if 'localContentRect' were used as a clip
        // before the draw or saveLayer, representing what the filter must cover if it affects
        // transparent black or doesn't have a local content hint.
        canvas->setMatrix(SkMatrix::I());
        canvas->drawRect(ctm.mapRect(localContentRect), line_paint(SK_ColorDKGRAY));

        // Layer bounds for the filter, in the layer space compatible with the filter's matrix
        // type requirements.
        skif::LayerSpace<SkIRect> targetOutputInLayer = mapping.deviceToLayer(targetOutput);
        skif::LayerSpace<SkIRect> hintedLayerBounds = as_IFB(fBlur)->getInputBounds(
                mapping, targetOutput, contentBounds);
        skif::LayerSpace<SkIRect> unhintedLayerBounds = as_IFB(fBlur)->getInputBounds(
                mapping, targetOutput, {});

        canvas->setMatrix(mapping.layerToDevice());
        canvas->drawRect(SkRect::Make(SkIRect(targetOutputInLayer)),
                         line_paint(SK_ColorDKGRAY, true));
        canvas->drawRect(SkRect::Make(SkIRect(hintedLayerBounds)), line_paint(SK_ColorRED));
        canvas->drawRect(SkRect::Make(SkIRect(unhintedLayerBounds)), line_paint(SK_ColorGREEN));

        // For visualization purposes, we want to show the layer-space output, this is what we get
        // when contentBounds is provided as a hint in local/parameter space.
        skif::Mapping layerOnly{mapping.layerMatrix()};
        std::optional<skif::DeviceSpace<SkIRect>> hintedOutputBounds =
                as_IFB(fBlur)->getOutputBounds(layerOnly, contentBounds);
        if (hintedOutputBounds) {
            canvas->drawRect(SkRect::Make(SkIRect(*hintedOutputBounds)), line_paint(SK_ColorBLUE));
        }

        canvas->resetMatrix();
        float y = print_info(canvas,
                             mapping.paramToLayer(contentBounds).roundOut(),
                             targetOutput,
                             hintedOutputBounds,
                             unhintedLayerBounds);

        // Draw color key for layer visualization
        draw_scale_key(canvas, y);
    }

private:
    sk_sp<SkImageFilter> fBlur;
    sk_sp<SkImage>       fImage;
};

DEF_SLIDE(return new FilterBoundsSample();)
