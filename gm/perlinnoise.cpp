/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkPerlinNoiseShader.h"
#include "SkShader.h"

namespace {

enum class Type {
    kFractalNoise,
    kTurbulence,
};

} // anonymous ns

class PerlinNoiseGM : public skiagm::GM {
public:
    PerlinNoiseGM() {
        this->setBGColor(0xFF000000);
        fSize = SkISize::Make(80, 80);
    }

protected:
    virtual SkString onShortName() {
        return SkString("perlinnoise");
    }

    virtual SkISize onISize() {
        return SkISize::Make(200, 500);
    }

    void drawRect(SkCanvas* canvas, int x, int y, const SkPaint& paint, const SkISize& size) {
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        SkRect r = SkRect::MakeWH(SkIntToScalar(size.width()),
                                  SkIntToScalar(size.height()));
        canvas->drawRect(r, paint);
        canvas->restore();
    }

    void test(SkCanvas* canvas, int x, int y, Type type,
              float baseFrequencyX, float baseFrequencyY, int numOctaves, float seed,
              bool stitchTiles) {
        SkISize tileSize = SkISize::Make(fSize.width() / 2, fSize.height() / 2);
        sk_sp<SkShader> shader = (type == Type::kFractalNoise) ?
            SkPerlinNoiseShader::MakeFractalNoise(baseFrequencyX, baseFrequencyY, numOctaves,
                                                  seed, stitchTiles ? &tileSize : nullptr) :
            SkPerlinNoiseShader::MakeTurbulence(baseFrequencyX, baseFrequencyY, numOctaves,
                                                seed, stitchTiles ? &tileSize : nullptr);
        SkPaint paint;
        paint.setShader(std::move(shader));
        if (stitchTiles) {
            drawRect(canvas, x, y, paint, tileSize);
            x += tileSize.width();
            drawRect(canvas, x, y, paint, tileSize);
            y += tileSize.width();
            drawRect(canvas, x, y, paint, tileSize);
            x -= tileSize.width();
            drawRect(canvas, x, y, paint, tileSize);
        } else {
            drawRect(canvas, x, y, paint, fSize);
        }
    }

    virtual void onDraw(SkCanvas* canvas) {
        canvas->clear(SK_ColorBLACK);
        test(canvas,   0,   0, Type::kFractalNoise,
             0.1f, 0.1f, 0, 0, false);
        test(canvas, 100,   0, Type::kTurbulence,
             0.1f, 0.1f, 0, 0, false);

        test(canvas,   0, 100, Type::kFractalNoise,
             0.1f, 0.1f, 2, 0, false);
        test(canvas, 100, 100, Type::kFractalNoise,
             0.05f, 0.1f, 1, 0, true);

        test(canvas,   0, 200, Type::kTurbulence,
             0.1f, 0.1f, 1, 0, true);
        test(canvas, 100, 200, Type::kTurbulence,
             0.2f, 0.4f, 5, 0, false);

        test(canvas,   0, 300, Type::kFractalNoise,
             0.1f, 0.1f, 3, 1, false);
        test(canvas, 100, 300, Type::kFractalNoise,
             0.1f, 0.1f, 3, 4, false);

        canvas->scale(0.75f, 1.0f);

        test(canvas,   0, 400, Type::kFractalNoise,
             0.1f, 0.1f, 2, 0, false);
        test(canvas, 100, 400, Type::kFractalNoise,
             0.1f, 0.05f, 1, 0, true);
    }

private:
    typedef GM INHERITED;
    SkISize fSize;
};

class PerlinNoiseGM2 : public skiagm::GM {
public:
    PerlinNoiseGM2() {
        fSize = SkISize::Make(80, 80);
    }

protected:
    virtual SkString onShortName() {
        return SkString("perlinnoise_localmatrix");
    }

    virtual SkISize onISize() {
        return SkISize::Make(640, 480);
    }

    void install(SkPaint* paint, Type type,
              float baseFrequencyX, float baseFrequencyY, int numOctaves, float seed,
              bool stitchTiles) {
        sk_sp<SkShader> shader = (type == Type::kFractalNoise) ?
            SkPerlinNoiseShader::MakeFractalNoise(baseFrequencyX, baseFrequencyY, numOctaves,
                                                  seed, stitchTiles ? &fSize : nullptr) :
            SkPerlinNoiseShader::MakeTurbulence(baseFrequencyX, baseFrequencyY, numOctaves,
                                                seed, stitchTiles ? &fSize : nullptr);
        paint->setShader(std::move(shader));
    }

    virtual void onDraw(SkCanvas* canvas) {
        canvas->translate(10, 10);

        SkPaint paint;
        install(&paint, Type::kFractalNoise, 0.1f, 0.1f, 2, 0, false);

        const SkScalar w = SkIntToScalar(fSize.width());
        const SkScalar h = SkIntToScalar(fSize.height());

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

private:
    typedef GM INHERITED;
    SkISize fSize;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new PerlinNoiseGM; )
DEF_GM( return new PerlinNoiseGM2; )
