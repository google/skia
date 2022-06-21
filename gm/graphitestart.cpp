/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrRecordingContext.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

namespace {

sk_sp<SkShader> create_gradient_shader(SkRect r) {
    SkPoint pts[2] = { {r.fLeft, r.fTop}, {r.fRight, r.fTop} };
    SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    float offsets[] = { 0.0f, 0.75f, 1.0f };

    return SkGradientShader::MakeLinear(pts, colors, offsets, std::size(colors),
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

    sk_sp<SkImage> img = SkImage::MakeFromBitmap(bitmap);
    img = ToolUtils::MakeTextureImage(destCanvas, std::move(img));

    return img->makeShader(tmX, tmY, SkSamplingOptions());
}

sk_sp<SkShader> create_blend_shader(SkCanvas* destCanvas, SkBlendMode bm) {
    constexpr SkColor4f kTransYellow = {1.0f, 1.0f, 0.0f, 0.5f};

    sk_sp<SkShader> dst = SkShaders::Color(kTransYellow, nullptr);
    return SkShaders::Blend(bm,
                            std::move(dst),
                            create_image_shader(destCanvas,
                                                SkTileMode::kRepeat, SkTileMode::kRepeat));
}

void draw_image_shader_tile(SkCanvas* canvas) {
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
        canvas->scale(0.5f, 0.5f);
        canvas->drawPath(path, p);

        canvas->save();
            canvas->concat(SkMatrix::RotateDeg(90, {64, 64}));
            canvas->translate(128, 0);
            canvas->drawPath(path, p);
        canvas->restore();
    canvas->restore();
}

void draw_gradient_tile(SkCanvas* canvas) {
    SkRect r{1, 1, 127, 127};
    SkPaint p;
    p.setShader(create_gradient_shader(r));

    canvas->save();
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
        GetResourceAsBitmap("images/color_wheel.gif", &fBitmap);
    }

protected:
    static const int kWidth = 256;
    static const int kHeight = 384;

    SkString onShortName() override {
        return SkString("graphitestart");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {

        canvas->save();
        canvas->clipRRect(SkRRect::MakeRectXY({16.f, 16.f, 240.f, 366.f}, 32.f, 32.f), true);

        // UL corner
        draw_image_shader_tile(canvas);

        // UR corner
        draw_gradient_tile(canvas);

        // LL corner
        {
            SkPaint p;
            p.setColor(SK_ColorRED);
            canvas->drawRect({2, 129, 127, 255}, p);
        }

        // LR corner
        {
            SkPaint p;
            p.setShader(create_blend_shader(canvas, SkBlendMode::kModulate));
            canvas->drawRect({129, 129, 255, 255}, p);
        }

        canvas->restore();

#ifdef SK_GRAPHITE_ENABLED
        // TODO: failing serialize test on Linux, not sure what's going on
        canvas->writePixels(fBitmap, 0, 256);
#endif

        draw_blend_mode_swatches(canvas, SkRect::MakeXYWH(128, 256, 128, 128));
    }

private:
    SkBitmap fBitmap;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new GraphiteStartGM;)

}  // namespace skiagm
