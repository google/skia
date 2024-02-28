/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkPerlinNoiseShader.h"

#include <utility>

namespace {

enum class Type {
    kFractalNoise,
    kTurbulence,
};

sk_sp<SkShader> noise_shader(Type type,
                             float baseFrequencyX,
                             float baseFrequencyY,
                             int numOctaves,
                             float seed,
                             bool stitchTiles,
                             SkISize size) {
    return (type == Type::kFractalNoise)
                   ? SkShaders::MakeFractalNoise(baseFrequencyX,
                                                 baseFrequencyY,
                                                 numOctaves,
                                                 seed,
                                                 stitchTiles ? &size : nullptr)
                   : SkShaders::MakeTurbulence(baseFrequencyX,
                                               baseFrequencyY,
                                               numOctaves,
                                               seed,
                                               stitchTiles ? &size : nullptr);
}

class PerlinNoiseGM : public skiagm::GM {
    static constexpr SkISize kSize = {80, 80};

    SkString getName() const override { return SkString("perlinnoise"); }

    SkISize getISize() override { return {220, 620}; }

    void drawRect(SkCanvas* canvas, SkPoint pt, const SkPaint& paint, const SkISize& size) {
        canvas->save();
        canvas->translate(pt.fX, pt.fY);
        SkRect r = SkRect::MakeWH(SkIntToScalar(size.width()), SkIntToScalar(size.height()));
        canvas->drawRect(r, paint);
        canvas->restore();
    }

    void test(SkCanvas* canvas, SkPoint pt, Type type, bool stitch,
              SkVector baseFrequency, int numOctaves, float seed, SkISize tileSize = {40, 40}) {
        sk_sp<SkShader> shader = noise_shader(type,
                                              baseFrequency.fX,
                                              baseFrequency.fY,
                                              numOctaves,
                                              seed,
                                              stitch,
                                              tileSize);
        SkPaint paint;
        paint.setShader(std::move(shader));
        if (stitch) {
            this->drawRect(canvas, pt, paint, tileSize);
            pt.fX += tileSize.width();
            this->drawRect(canvas, pt, paint, tileSize);
            pt.fY += tileSize.height();
            this->drawRect(canvas, pt, paint, tileSize);
            pt.fX -= tileSize.width();
            this->drawRect(canvas, pt, paint, tileSize);
        } else {
            this->drawRect(canvas, pt, paint, kSize);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        this->test(canvas, SkPoint{  0,   0}, Type::kFractalNoise, /*stitch=*/false,
                   SkVector{0.1f, 0.1f}, /*numOctaves=*/0, /*seed=*/0);
        this->test(canvas, SkPoint{100,   0}, Type::kTurbulence, /*stitch=*/false,
                   SkVector{0.1f, 0.1f}, /*numOctaves=*/0, /*seed=*/0);

        this->test(canvas, SkPoint{  0, 100}, Type::kFractalNoise, /*stitch=*/false,
                   SkVector{0.1f, 0.1f}, /*numOctaves=*/2, /*seed=*/0);
        this->test(canvas, SkPoint{100, 100}, Type::kFractalNoise, /*stitch=*/true,
                   SkVector{0.05f, 0.1f}, /*numOctaves=*/1, /*seed=*/0);

        this->test(canvas, SkPoint{  0, 200}, Type::kTurbulence, /*stitch=*/true,
                   SkVector{0.1f, 0.1f}, /*numOctaves=*/1, /*seed=*/0);
        this->test(canvas, SkPoint{100, 200}, Type::kTurbulence, /*stitch=*/false,
                   SkVector{0.2f, 0.4f}, /*numOctaves=*/5, /*seed=*/0);

        this->test(canvas, SkPoint{  0, 300}, Type::kFractalNoise, /*stitch=*/false,
                   SkVector{0.1f, 0.1f}, /*numOctaves=*/3, /*seed=*/1);
        this->test(canvas, SkPoint{100, 300}, Type::kFractalNoise, /*stitch=*/false,
                   SkVector{0.1f, 0.1f}, /*numOctaves=*/3, /*seed=*/4);

        canvas->save();
        canvas->scale(0.75f, 1.0f);

        this->test(canvas, SkPoint{  0, 400}, Type::kFractalNoise, /*stitch=*/false,
                   SkVector{0.1f, 0.1f}, /*numOctaves=*/2, /*seed=*/0);
        this->test(canvas, SkPoint{100, 400}, Type::kFractalNoise, /*stitch=*/true,
                   SkVector{0.1f, 0.05f}, /*numOctaves=*/1, /*seed=*/0);

        canvas->restore();

        // Matches Chromium test case in svg/filters/feTurbulence-tiled.svg
        this->test(canvas, SkPoint{  0, 500}, Type::kTurbulence, /*stitch=*/true,
                   SkVector{0.03f, 0.03f}, /*numOctaves=*/1, /*seed=*/0, /*tileSize=*/{50, 50});

        // Matches Chromium test case in css3/filters/effect-reference.html
        this->test(canvas, SkPoint{120, 500}, Type::kTurbulence, /*stitch=*/false,
                   SkVector{0.05f, 0.05f}, /*numOctaves=*/2, /*seed=*/0);

    }

private:
    using INHERITED = GM;
};

class PerlinNoiseLocalMatrixGM : public skiagm::GM {
    static constexpr SkISize kSize = {80, 80};

    SkString getName() const override { return SkString("perlinnoise_localmatrix"); }

    SkISize getISize() override { return {640, 480}; }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(10, 10);

        SkPaint paint;
        paint.setShader(noise_shader(Type::kFractalNoise, 0.1f, 0.1f, 2, 0, false, kSize));

        const SkScalar w = SkIntToScalar(kSize.width());
        const SkScalar h = SkIntToScalar(kSize.height());

        SkRect r = SkRect::MakeWH(w, h);
        canvas->drawRect(r, paint);

        canvas->save();
        canvas->translate(w * 5/4, 0);
        canvas->drawRect(r, paint);
        canvas->restore();

        canvas->save();
        canvas->translate(0, h + 10);
        canvas->scale(2, 2);
        canvas->drawRect(r, paint);
        canvas->restore();

        canvas->save();
        canvas->translate(w + 100, h + 10);
        canvas->scale(2, 2);
        canvas->drawRect(r, paint);
        canvas->restore();

        // The next row should draw the same as the previous, even though we are using a local
        // matrix instead of the canvas.

        canvas->translate(0, h * 2 + 10);

        SkMatrix lm;
        lm.setScale(2, 2);
        paint.setShader(paint.getShader()->makeWithLocalMatrix(lm));
        r.fRight += r.width();
        r.fBottom += r.height();

        canvas->save();
        canvas->translate(0, h + 10);
        canvas->drawRect(r, paint);
        canvas->restore();

        canvas->save();
        canvas->translate(w + 100, h + 10);
        canvas->drawRect(r, paint);
        canvas->restore();
    }
};

// Demonstrate skbug.com/14166 (Perlin noise shader doesn't rotate correctly)
class PerlinNoiseRotatedGM : public skiagm::GM {
    static constexpr SkISize kCellSize = { 100, 100 };
    static constexpr SkISize kRectSize = { 60, 60 };
    static constexpr int kPad = 10;
    static constexpr int kCellsX = 3;
    static constexpr int kCellsY = 2;

    SkString getName() const override { return SkString("perlinnoise_rotated"); }

    SkISize getISize() override {
        return {2 * kPad + kCellsX * kCellSize.width(), 2 * kPad + kCellsY * kCellSize.height()};
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint outline;
        outline.setColor(SK_ColorBLACK);
        outline.setStrokeWidth(2.0f);
        outline.setStyle(SkPaint::kStroke_Style);
        outline.setAntiAlias(true);

        const SkRect kRectToDraw = SkRect::MakeWH(kRectSize.width(), kRectSize.height());
        const SkRect kMarker = SkRect::MakeWH(5, 5);

        float yOffset = kPad;
        for (auto type : { Type::kFractalNoise, Type::kTurbulence }) {
            float xOffset = kPad;

            SkPaint p;
            p.setShader(noise_shader(type, 0.05f, 0.05f, 1, 0, false, kRectSize));

            for (float rotation : {0.0f, 10.0f, 80.0f}) {
                int saveCount = canvas->save();
                canvas->translate(xOffset, yOffset);

                canvas->drawRect(SkRect::MakeWH(kCellSize.fWidth, kCellSize.fHeight), outline);

                canvas->save();

                canvas->translate(kCellSize.fWidth / 2.0f, kCellSize.fHeight / 2.0f);
                canvas->rotate(rotation);
                canvas->translate(-kRectSize.fWidth/2.0f, -kRectSize.fHeight/2.0f);

                canvas->drawRect(kRectToDraw, p);

                canvas->drawRect(kRectToDraw, outline);
                canvas->drawRect(kMarker, outline);

                canvas->restoreToCount(saveCount);

                xOffset += kCellSize.width();
            }

            yOffset += kCellSize.height();
        }
    }
};

// Demonstrate skbug.com/14411 (Intel GPUs show artifacts when applying perlin noise to layers)
class PerlinNoiseLayeredGM : public skiagm::GM {
    SkString getName() const override { return SkString("perlinnoise_layered"); }

    SkISize getISize() override { return {500, 500}; }

    void onDraw(SkCanvas* canvas) override {
        const sk_sp<SkImageFilter> perlin = SkImageFilters::ColorFilter(
                SkColorFilters::Matrix(SkColorMatrix()),
                SkImageFilters::Shader(SkShaders::MakeFractalNoise(0.3f, 0.3f, 1, 4)));

        const SkPaint paint;
        canvas->saveLayer(nullptr, &paint);
        {
            SkPaint p;
            p.setImageFilter(perlin);
            canvas->drawPaint(p);
        }
        canvas->restore();

        canvas->saveLayer(nullptr, nullptr);
        {
            SkPaint p;
            p.setImageFilter(perlin);
            canvas->drawPaint(p);
        }
        canvas->restore();
    }
};

} // anonymous namespace

DEF_GM(return new PerlinNoiseGM;)
DEF_GM(return new PerlinNoiseLocalMatrixGM;)
DEF_GM(return new PerlinNoiseRotatedGM;)
DEF_GM(return new PerlinNoiseLayeredGM;)
