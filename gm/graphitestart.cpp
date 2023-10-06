/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkColorFilterPriv.h"
#include "tools/DecodeUtils.h"
#include "tools/GpuToolUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

namespace {

sk_sp<SkShader> create_gradient_shader(SkRect r,
                                       const std::array<SkColor, 3>& colors,
                                       const std::array<float, 3>& offsets) {
    SkPoint pts[2] = { {r.fLeft, r.fTop}, {r.fRight, r.fTop} };

    return SkGradientShader::MakeLinear(pts, colors.data(), offsets.data(), std::size(colors),
                                        SkTileMode::kClamp);
}

sk_sp<SkShader> create_image_shader(SkCanvas* destCanvas, SkTileMode tmX, SkTileMode tmY) {
    SkBitmap bitmap;

    {
        SkImageInfo ii = SkImageInfo::Make(64, 64, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        bitmap.allocPixels(ii);
        bitmap.eraseColor(SK_ColorWHITE);

        SkCanvas tmpCanvas(bitmap);

        SkColor colors[3][3] = {
                { SK_ColorRED,    SK_ColorDKGRAY, SK_ColorBLUE },
                { SK_ColorLTGRAY, SK_ColorCYAN,   SK_ColorYELLOW },
                { SK_ColorGREEN,  SK_ColorWHITE,  SK_ColorMAGENTA }
        };

        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                SkPaint paint;
                paint.setColor(colors[y][x]);
                tmpCanvas.drawRect(SkRect::MakeXYWH(x*21, y*21, 22, 22), paint);
            }
        }

        bitmap.setAlphaType(kOpaque_SkAlphaType);
        bitmap.setImmutable();
    }

    sk_sp<SkImage> img = SkImages::RasterFromBitmap(bitmap);
    img = ToolUtils::MakeTextureImage(destCanvas, std::move(img));
    if (img) {
        return img->makeShader(tmX, tmY, SkSamplingOptions());
    } else {
        return nullptr;
    }
}

sk_sp<SkShader> create_blend_shader(SkCanvas* destCanvas, SkBlendMode bm) {
    constexpr SkColor4f kTransYellow = {1.0f, 1.0f, 0.0f, 0.5f};

    sk_sp<SkShader> dst = SkShaders::Color(kTransYellow, nullptr);
    return SkShaders::Blend(bm,
                            std::move(dst),
                            create_image_shader(destCanvas,
                                                SkTileMode::kRepeat, SkTileMode::kRepeat));
}

sk_sp<SkColorFilter> create_grayscale_colorfilter() {
    float matrix[20] = {};
    matrix[0] = matrix[5] = matrix[10] = 0.2126f;
    matrix[1] = matrix[6] = matrix[11] = 0.7152f;
    matrix[2] = matrix[7] = matrix[12] = 0.0722f;
    matrix[18] = 1.0f;
    return SkColorFilters::Matrix(matrix);
}

void draw_image_shader_tile(SkCanvas* canvas, SkRect clipRect) {
    SkPaint p;
    p.setShader(create_image_shader(canvas, SkTileMode::kClamp, SkTileMode::kRepeat));

    SkPath path;
    path.moveTo(1,   1);
    path.lineTo(32,  127);
    path.lineTo(96,  127);
    path.lineTo(127, 1);
    path.lineTo(63,  32);
    path.close();

    canvas->save();
        canvas->clipRect(clipRect);
        canvas->scale(0.5f, 0.5f);
        canvas->drawPath(path, p);

        canvas->save();
            canvas->concat(SkMatrix::RotateDeg(90, {64, 64}));
            canvas->translate(128, 0);
            canvas->drawPath(path, p);
        canvas->restore();
    canvas->restore();
}

void draw_gradient_tile(SkCanvas* canvas, SkRect clipRect) {
    SkRect r{1, 1, 127, 127};
    SkPaint p;
    p.setShader(create_gradient_shader(r,
                                       { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE },
                                       { 0.0f, 0.75f, 1.0f }));

    canvas->save();
        canvas->clipRect(clipRect);
        canvas->translate(128, 0);
        canvas->scale(0.5f, 0.5f);
        canvas->drawRect(r, p);

        canvas->save();
            canvas->concat(SkMatrix::RotateDeg(90, {64, 64}));
            canvas->translate(128, 0);
            canvas->drawRect(r, p);
        canvas->restore();
    canvas->restore();
}

void draw_colorfilter_swatches(SkCanvas* canvas, SkRect clipRect) {
    static constexpr int kNumTilesPerSide = 3;

    SkSize tileSize = { clipRect.width() / kNumTilesPerSide, clipRect.height() / kNumTilesPerSide };

    // Quantize to four colors
    uint8_t table1[256];
    for (int i = 0; i < 256; ++i) {
        table1[i] = (i/64) * 85;
    }

    // table2 is a band-pass filter for 85-170.
    // table3 re-expands that range to 0..255
    uint8_t table2[256], table3[256];
    for (int i = 0; i < 256; ++i) {
        if (i >= 85 && i <= 170) {
            table2[i] = i;
            table3[i] = ((i - 85) / 85.0f) * 255.0f;
        } else {
            table2[i] = 0;
            table3[i] = 0;
        }
    }

    constexpr SkColor SK_ColorGREY = SkColorSetARGB(0xFF, 0x80, 0x80, 0x80);

    sk_sp<SkColorFilter> colorFilters[kNumTilesPerSide*kNumTilesPerSide];
    static const std::array<SkColor, 3> kGradientColors[kNumTilesPerSide*kNumTilesPerSide] = {
            { SK_ColorBLACK, SK_ColorGREY, SK_ColorWHITE },
            { SK_ColorBLACK, SK_ColorGREY, SK_ColorWHITE },
            { SK_ColorBLACK, SK_ColorGREY, SK_ColorWHITE },
            { SK_ColorBLACK, SK_ColorGREY, SK_ColorWHITE },
            { 0x00000000,    0x80000000,   0xFF000000    },  // the Gaussian CF uses alpha only
            { SK_ColorBLACK, SK_ColorGREY, SK_ColorWHITE },
            { SK_ColorBLACK, SK_ColorGREY, SK_ColorWHITE },
            { SK_ColorBLACK, SK_ColorGREY, SK_ColorWHITE },
            { SK_ColorBLACK, SK_ColorGREY, SK_ColorWHITE },
    };

    colorFilters[0] = SkColorFilters::Lighting(SK_ColorLTGRAY, 0xFF440000);
    colorFilters[1] = SkColorFilters::Table(table1);
    colorFilters[2] = SkColorFilters::Compose(SkColorFilters::TableARGB(nullptr, table3,
                                                                        table3, table3),
                                              SkColorFilters::TableARGB(nullptr, table2,
                                                                        table2, table2));
    colorFilters[3] = SkColorFilters::Blend(SK_ColorGREEN, SkBlendMode::kMultiply);
    colorFilters[4] = SkColorFilterPriv::MakeGaussian();

    colorFilters[5] = SkColorFilters::LinearToSRGBGamma();
    colorFilters[6] = SkColorFilters::SRGBToLinearGamma();

    SkPaint p;

    canvas->save();
        canvas->clipRect(clipRect);
        canvas->translate(clipRect.fLeft, clipRect.fTop);

        for (int y = 0; y < kNumTilesPerSide; ++y) {
            for (int x = 0; x < kNumTilesPerSide; ++x) {
                SkRect r = SkRect::MakeXYWH(x * tileSize.width(), y * tileSize.height(),
                                            tileSize.width(), tileSize.height()).makeInset(1.0f,
                                                                                           1.0f);
                int colorFilterIndex = x*kNumTilesPerSide+y;
                p.setShader(create_gradient_shader(r,
                                                   kGradientColors[colorFilterIndex],
                                                   { 0.0f, 0.5f, 1.0f }));
                p.setColorFilter(colorFilters[colorFilterIndex]);
                canvas->drawRect(r, p);
            }
        }

    canvas->restore();
}

void draw_blend_mode_swatches(SkCanvas* canvas, SkRect clipRect) {
    static const int kTileHeight = 16;
    static const int kTileWidth = 16;
    static const SkColor4f kOpaqueWhite { 1.0f, 1.0f, 1.0f, 1.0f };
    static const SkColor4f kTransBluish { 0.0f, 0.5f, 1.0f, 0.5f };
    static const SkColor4f kTransWhite { 1.0f, 1.0f, 1.0f, 0.75f };

    SkPaint dstPaint;
    dstPaint.setColor(kOpaqueWhite);
    dstPaint.setBlendMode(SkBlendMode::kSrc);
    dstPaint.setAntiAlias(false);

    SkPaint srcPaint;
    srcPaint.setColor(kTransBluish);
    srcPaint.setAntiAlias(false);

    SkRect r = SkRect::MakeXYWH(clipRect.fLeft, clipRect.fTop, kTileWidth, kTileHeight);

    // For the first pass we draw: transparent bluish on top of opaque white
    // For the second pass we draw: transparent white on top of transparent bluish
    for (int passes = 0; passes < 2; ++passes) {
        for (int i = 0; i <= (int)SkBlendMode::kLastCoeffMode; ++i) {
            if (r.fLeft+kTileWidth > clipRect.fRight) {
                r.offsetTo(clipRect.fLeft, r.fTop+kTileHeight);
            }

            canvas->drawRect(r.makeInset(1.0f, 1.0f), dstPaint);
            srcPaint.setBlendMode(static_cast<SkBlendMode>(i));
            canvas->drawRect(r.makeInset(2.0f, 2.0f), srcPaint);

            r.offset(kTileWidth, 0.0f);
        }

        r.offsetTo(clipRect.fLeft, r.fTop+kTileHeight);
        srcPaint.setColor(kTransWhite);
        dstPaint.setColor(kTransBluish);
    }
}

} // anonymous namespace

namespace skiagm {

// This is just for bootstrapping Graphite.
class GraphiteStartGM : public GM {
public:
    GraphiteStartGM() {
        this->setBGColor(SK_ColorBLACK);
        ToolUtils::GetResourceAsBitmap("images/color_wheel.gif", &fBitmap);
    }

protected:
    static constexpr int kTileWidth = 128;
    static constexpr int kTileHeight = 128;
    static constexpr int kWidth = 3 * kTileWidth;
    static constexpr int kHeight = 3 * kTileHeight;
    static constexpr int kClipInset = 4;

    SkString getName() const override { return SkString("graphitestart"); }

    SkISize getISize() override { return SkISize::Make(kWidth, kHeight); }

    void onDraw(SkCanvas* canvas) override {

        const SkRect clipRect = SkRect::MakeWH(kWidth, kHeight).makeInset(kClipInset, kClipInset);

        canvas->save();
        canvas->clipRRect(SkRRect::MakeRectXY(clipRect, 32.f, 32.f), true);

        // Upper-left corner
        draw_image_shader_tile(canvas, SkRect::MakeXYWH(0, 0, kTileWidth, kTileHeight));

        // Upper-middle tile
        draw_gradient_tile(canvas, SkRect::MakeXYWH(kTileWidth, 0, kTileWidth, kTileHeight));

        // Upper-right corner
        draw_colorfilter_swatches(canvas, SkRect::MakeXYWH(2*kTileWidth, 0,
                                                           kTileWidth, kTileWidth));

        // Middle-left tile
        {
            SkPaint p;
            p.setColor(SK_ColorRED);

            SkRect r = SkRect::MakeXYWH(0, kTileHeight, kTileWidth, kTileHeight);
            canvas->drawRect(r.makeInset(1.0f, 1.0f), p);
        }

        // Middle-middle tile
        {
            SkPaint p;
            p.setShader(create_blend_shader(canvas, SkBlendMode::kModulate));

            SkRect r = SkRect::MakeXYWH(kTileWidth, kTileHeight, kTileWidth, kTileHeight);
            canvas->drawRect(r.makeInset(1.0f, 1.0f), p);
        }

        // Middle-right tile
        {
            sk_sp<SkImage> image(ToolUtils::GetResourceAsImage("images/mandrill_128.png"));
            sk_sp<SkShader> shader;

            if (image) {
                shader = image->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, {});
                shader = shader->makeWithColorFilter(create_grayscale_colorfilter());
            }

            SkPaint p;
            p.setShader(std::move(shader));

            SkRect r = SkRect::MakeXYWH(2*kTileWidth, kTileHeight, kTileWidth, kTileHeight);
            canvas->drawRect(r.makeInset(1.0f, 1.0f), p);
        }

        canvas->restore();

        // Bottom-left corner
#if defined(SK_GRAPHITE)
        // TODO: failing serialize test on Linux, not sure what's going on
        canvas->writePixels(fBitmap, 0, 2*kTileHeight);
#endif

        // Bottom-middle tile
        draw_blend_mode_swatches(canvas, SkRect::MakeXYWH(kTileWidth, 2*kTileHeight,
                                                          kTileWidth, kTileHeight));

        // Bottom-right corner
        {
            const SkRect kTile = SkRect::MakeXYWH(2*kTileWidth, 2*kTileHeight,
                                                  kTileWidth, kTileHeight);

            SkPaint circlePaint;
            circlePaint.setColor(SK_ColorBLUE);
            circlePaint.setBlendMode(SkBlendMode::kSrc);

            canvas->clipRect(kTile);
            canvas->drawRect(kTile.makeInset(10, 20), circlePaint);

            SkPaint restorePaint;
            restorePaint.setBlendMode(SkBlendMode::kPlus);

            canvas->saveLayer(nullptr, &restorePaint);
                circlePaint.setColor(SK_ColorRED);
                circlePaint.setBlendMode(SkBlendMode::kSrc);

                canvas->drawRect(kTile.makeInset(15, 25), circlePaint);
            canvas->restore();
        }
    }

private:
    SkBitmap fBitmap;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new GraphiteStartGM;)

}  // namespace skiagm
