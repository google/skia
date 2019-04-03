/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkFont.h"
#include "SkGradientShader.h"

// NOTE: The positions define hardstops for the red and green borders. For the repeating degenerate
// gradients, that means the red and green are never visible, so the average color used should only
// be based off of the white, blue, black blend.
static const SkColor COLORS[] = { SK_ColorRED, SK_ColorWHITE, SK_ColorBLUE,
                                  SK_ColorBLACK, SK_ColorGREEN };
static const SkScalar POS[] = { 0.0, 0.0, 0.5, 1.0, 1.0 };
static const int COLOR_CT = SK_ARRAY_COUNT(COLORS);

static const SkTileMode TILE_MODES[] = { SkTileMode::kDecal,
                                         SkTileMode::kRepeat,
                                         SkTileMode::kMirror,
                                         SkTileMode::kClamp };
static const char* TILE_NAMES[] = { "decal", "repeat", "mirror", "clamp" };
static const int TILE_MODE_CT = SK_ARRAY_COUNT(TILE_MODES);

static constexpr int TILE_SIZE = 100;
static constexpr int TILE_GAP = 10;

static const SkPoint CENTER = SkPoint::Make(TILE_SIZE / 2, TILE_SIZE / 2);

typedef sk_sp<SkShader> (*GradientFactory)(SkTileMode tm);

static void draw_tile_header(SkCanvas* canvas) {
    canvas->save();

    for (int i = 0; i < TILE_MODE_CT; ++i) {
        canvas->drawString(TILE_NAMES[i], 0, 0, SkFont(), SkPaint());
        canvas->translate(TILE_SIZE + TILE_GAP, 0);
    }

    canvas->restore();

    // Now adjust to start at rows below the header
    canvas->translate(0, 2 * TILE_GAP);
}

static void draw_row(SkCanvas* canvas, const char* desc, GradientFactory factory) {
    canvas->save();

    SkPaint text;
    text.setAntiAlias(true);

    canvas->translate(0, TILE_GAP);
    canvas->drawString(desc, 0, 0, SkFont(), text);
    canvas->translate(0, TILE_GAP);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kStrokeAndFill_Style);
    paint.setStrokeWidth(2.0f);

    for (int i = 0; i < TILE_MODE_CT; ++i) {
        paint.setShader(factory(TILE_MODES[i]));
        canvas->drawRect(SkRect::MakeWH(TILE_SIZE, TILE_SIZE), paint);
        canvas->translate(TILE_SIZE + TILE_GAP, 0);
    }

    canvas->restore();

    // Now adjust to start the next row below this one (1 gap for text and 2 gap for margin)
    canvas->translate(0, 3 * TILE_GAP + TILE_SIZE);
}

static sk_sp<SkShader> make_linear(SkTileMode mode) {
    // Same position
    SkPoint pts[2] = {CENTER, CENTER};
    return SkGradientShader::MakeLinear(pts, COLORS, POS, COLOR_CT, mode);
}

static sk_sp<SkShader> make_radial(SkTileMode mode) {
    // Radius = 0
    return SkGradientShader::MakeRadial(CENTER, 0.0, COLORS, POS, COLOR_CT, mode);
}

static sk_sp<SkShader> make_sweep(SkTileMode mode) {
    // Start and end angles at 45
    static constexpr SkScalar SWEEP_ANG = 45.0;
    return SkGradientShader::MakeSweep(CENTER.fX, CENTER.fY, COLORS, POS, COLOR_CT, mode,
                                       SWEEP_ANG, SWEEP_ANG, 0, nullptr);
}

static sk_sp<SkShader> make_sweep_zero_ang(SkTileMode mode) {
    // Start and end angles at 0
    return SkGradientShader::MakeSweep(CENTER.fX, CENTER.fY, COLORS, POS, COLOR_CT, mode,
                                       0.0, 0.0, 0, nullptr);
}

static sk_sp<SkShader> make_2pt_conic(SkTileMode mode) {
    // Start and end radius = TILE_SIZE, same position
    return SkGradientShader::MakeTwoPointConical(CENTER, TILE_SIZE / 2, CENTER, TILE_SIZE / 2,
                                                 COLORS, POS, COLOR_CT, mode);
}

static sk_sp<SkShader> make_2pt_conic_zero_rad(SkTileMode mode) {
    // Start and end radius = 0, same position
    return SkGradientShader::MakeTwoPointConical(CENTER, 0.0, CENTER, 0.0, COLORS, POS,
                                                 COLOR_CT, mode);
}

class DegenerateGradientGM : public skiagm::GM {
public:
    DegenerateGradientGM() {

    }

protected:
    SkString onShortName() override {
        return SkString("degenerate_gradients");
    }

    SkISize onISize() override {
        return SkISize::Make(800, 800);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(3 * TILE_GAP, 3 * TILE_GAP);
        draw_tile_header(canvas);

        draw_row(canvas, "linear: empty, blue, blue, green", make_linear);
        draw_row(canvas, "radial:  empty, blue, blue, green", make_radial);
        draw_row(canvas, "sweep-0: empty, blue, blue, green", make_sweep_zero_ang);
        draw_row(canvas, "sweep-45: empty, blue, blue, red 45 degree sector then green",
                 make_sweep);
        draw_row(canvas, "2pt-conic-0: empty, blue, blue, green", make_2pt_conic_zero_rad);
        draw_row(canvas, "2pt-conic-1: empty, blue, blue, full red circle on green",
                 make_2pt_conic);
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM(return new DegenerateGradientGM;)
