/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>

namespace skiagm {

constexpr SkColor gPathColor = SK_ColorBLACK;
constexpr SkColor gClipAColor = SK_ColorBLUE;
constexpr SkColor gClipBColor = SK_ColorRED;

class ComplexClipGM : public GM {
public:
    ComplexClipGM(bool aaclip, bool saveLayer, bool invertDraw)
    : fDoAAClip(aaclip)
    , fDoSaveLayer(saveLayer)
    , fInvertDraw(invertDraw) {
        this->setBGColor(0xFFDEDFDE);
    }

protected:
    SkString getName() const override {
        SkString str;
        str.printf("complexclip_%s%s%s",
                   fDoAAClip ? "aa" : "bw",
                   fDoSaveLayer ? "_layer" : "",
                   fInvertDraw ? "_invert" : "");
        return str;
    }

    SkISize getISize() override { return SkISize::Make(388, 780); }

    void onDraw(SkCanvas* canvas) override {
        SkPath path = SkPathBuilder()
                        .moveTo(0,   50)
                        .quadTo(0,   0,   50,  0)
                        .lineTo(175, 0)
                        .quadTo(200, 0,   200, 25)
                        .lineTo(200, 150)
                        .quadTo(200, 200, 150, 200)
                        .lineTo(0,   200)
                        .close()
                        .moveTo(50,  50)
                        .lineTo(150, 50)
                        .lineTo(150, 125)
                        .quadTo(150, 150, 125, 150)
                        .lineTo(50,  150)
                        .close()
                        .detach();
        if (fInvertDraw) {
            path.setFillType(SkPathFillType::kInverseEvenOdd);
        } else {
            path.setFillType(SkPathFillType::kEvenOdd);
        }
        SkPaint pathPaint;
        pathPaint.setAntiAlias(true);
        pathPaint.setColor(gPathColor);

        SkPath clipA = SkPath::Polygon({{10,  20}, {165, 22}, {70,  105}, {165, 177}, {-5,  180}}, true);

        SkPath clipB = SkPath::Polygon({{40,  10}, {190, 15}, {195, 190}, {40,  185}, {155, 100}}, true);

        SkFont font(ToolUtils::DefaultPortableTypeface(), 20);

        constexpr struct {
            SkClipOp fOp;
            const char*      fName;
        } gOps[] = { //extra spaces in names for measureText
            {SkClipOp::kIntersect,         "Isect "},
            {SkClipOp::kDifference,        "Diff " },
        };

        canvas->translate(20, 20);
        canvas->scale(3 * SK_Scalar1 / 4, 3 * SK_Scalar1 / 4);

        if (fDoSaveLayer) {
            // We want the layer to appear symmetric relative to actual
            // device boundaries so we need to "undo" the effect of the
            // scale and translate
            SkRect bounds = SkRect::MakeLTRB(
              4.0f/3.0f * -20,
              4.0f/3.0f * -20,
              4.0f/3.0f * (this->getISize().fWidth - 20),
              4.0f/3.0f * (this->getISize().fHeight - 20));

            bounds.inset(100, 100);
            SkPaint boundPaint;
            boundPaint.setColor(SK_ColorRED);
            boundPaint.setStyle(SkPaint::kStroke_Style);
            canvas->drawRect(bounds, boundPaint);
            canvas->clipRect(bounds);
            canvas->saveLayer(&bounds, nullptr);
        }

        for (int invBits = 0; invBits < 4; ++invBits) {
            canvas->save();
            for (size_t op = 0; op < std::size(gOps); ++op) {
                this->drawHairlines(canvas, path, clipA, clipB);

                bool doInvA = SkToBool(invBits & 1);
                bool doInvB = SkToBool(invBits & 2);
                canvas->save();
                    // set clip
                    clipA.setFillType(doInvA ? SkPathFillType::kInverseEvenOdd :
                                      SkPathFillType::kEvenOdd);
                    clipB.setFillType(doInvB ? SkPathFillType::kInverseEvenOdd :
                                      SkPathFillType::kEvenOdd);
                    canvas->clipPath(clipA, fDoAAClip);
                    canvas->clipPath(clipB, gOps[op].fOp, fDoAAClip);

                    // In the inverse case we need to prevent the draw from covering the whole
                    // canvas.
                    if (fInvertDraw) {
                        SkRect rectClip = clipA.getBounds();
                        rectClip.join(path.getBounds());
                        rectClip.join(path.getBounds());
                        rectClip.outset(5, 5);
                        canvas->clipRect(rectClip);
                    }

                    // draw path clipped
                    canvas->drawPath(path, pathPaint);
                canvas->restore();


                SkPaint paint;
                SkScalar txtX = 45;
                paint.setColor(gClipAColor);
                const char* aTxt = doInvA ? "InvA " : "A ";
                canvas->drawSimpleText(aTxt, strlen(aTxt), SkTextEncoding::kUTF8, txtX, 220, font, paint);
                txtX += font.measureText(aTxt, strlen(aTxt), SkTextEncoding::kUTF8);
                paint.setColor(SK_ColorBLACK);
                canvas->drawSimpleText(gOps[op].fName, strlen(gOps[op].fName), SkTextEncoding::kUTF8, txtX, 220,
                                       font, paint);
                txtX += font.measureText(gOps[op].fName, strlen(gOps[op].fName), SkTextEncoding::kUTF8);
                paint.setColor(gClipBColor);
                const char* bTxt = doInvB ? "InvB " : "B ";
                canvas->drawSimpleText(bTxt, strlen(bTxt), SkTextEncoding::kUTF8, txtX, 220, font, paint);

                canvas->translate(250,0);
            }
            canvas->restore();
            canvas->translate(0, 250);
        }

        if (fDoSaveLayer) {
            canvas->restore();
        }
    }
private:
    void drawHairlines(SkCanvas* canvas, const SkPath& path,
                       const SkPath& clipA, const SkPath& clipB) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        const SkAlpha fade = 0x33;

        // draw path in hairline
        paint.setColor(gPathColor); paint.setAlpha(fade);
        canvas->drawPath(path, paint);

        // draw clips in hair line
        paint.setColor(gClipAColor); paint.setAlpha(fade);
        canvas->drawPath(clipA, paint);
        paint.setColor(gClipBColor); paint.setAlpha(fade);
        canvas->drawPath(clipB, paint);
    }

    bool fDoAAClip;
    bool fDoSaveLayer;
    bool fInvertDraw;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ComplexClipGM(false, false, false);)
DEF_GM(return new ComplexClipGM(false, false, true);)
DEF_GM(return new ComplexClipGM(false, true, false);)
DEF_GM(return new ComplexClipGM(false, true, true);)
DEF_GM(return new ComplexClipGM(true, false, false);)
DEF_GM(return new ComplexClipGM(true, false, true);)
DEF_GM(return new ComplexClipGM(true, true, false);)
DEF_GM(return new ComplexClipGM(true, true, true);)
}  // namespace skiagm

DEF_SIMPLE_GM(clip_shader, canvas, 840, 650) {
    auto img = ToolUtils::GetResourceAsImage("images/yellow_rose.png");
    auto sh = img->makeShader(SkSamplingOptions());

    SkRect r = SkRect::MakeIWH(img->width(), img->height());
    SkPaint p;

    canvas->translate(10, 10);
    canvas->drawImage(img, 0, 0);

    canvas->save();
    canvas->translate(img->width() + 10, 0);
    canvas->clipShader(sh, SkClipOp::kIntersect);
    p.setColor(SK_ColorRED);
    canvas->drawRect(r, p);
    canvas->restore();

    canvas->save();
    canvas->translate(0, img->height() + 10);
    canvas->clipShader(sh, SkClipOp::kDifference);
    p.setColor(SK_ColorGREEN);
    canvas->drawRect(r, p);
    canvas->restore();

    canvas->save();
    canvas->translate(img->width() + 10, img->height() + 10);
    canvas->clipShader(sh, SkClipOp::kIntersect);
    canvas->save();
    SkMatrix lm = SkMatrix::Scale(1.0f/5, 1.0f/5);
    canvas->clipShader(img->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                       SkSamplingOptions(), lm));
    canvas->drawImage(img, 0, 0);

    canvas->restore();
    canvas->restore();
}

DEF_SIMPLE_GM(clip_shader_layer, canvas, 430, 320) {
    auto img = ToolUtils::GetResourceAsImage("images/yellow_rose.png");
    auto sh = img->makeShader(SkSamplingOptions());

    SkRect r = SkRect::MakeIWH(img->width(), img->height());

    canvas->translate(10, 10);
    // now add the cool clip
    canvas->clipRect(r);
    canvas->clipShader(sh);
    // now draw a layer with the same image, and watch it get restored w/ the clip
    canvas->saveLayer(&r, nullptr);
    canvas->drawColor(0xFFFF0000);
    canvas->restore();
}

DEF_SIMPLE_GM(clip_shader_nested, canvas, 256, 256) {
    float w = 64.f;
    float h = 64.f;

    const SkColor gradColors[] = {SK_ColorBLACK, SkColorSetARGB(128, 128, 128, 128)};
    auto s = SkGradientShader::MakeRadial({0.5f * w, 0.5f * h}, 0.1f * w, gradColors, nullptr,
                                            2, SkTileMode::kRepeat, 0, nullptr);

    SkPaint p;

    // A large black rect affected by two gradient clips
    canvas->save();
    canvas->clipShader(s);
    canvas->scale(2.f, 2.f);
    canvas->clipShader(s);
    canvas->drawRect(SkRect::MakeWH(w, h), p);
    canvas->restore();

    canvas->translate(0.f, 2.f * h);

    // A small red rect, with no clipping
    canvas->save();
    p.setColor(SK_ColorRED);
    canvas->drawRect(SkRect::MakeWH(w, h), p);
    canvas->restore();
}

namespace {

// Where is canvas->concat(persp) called relative to the clipShader calls.
enum ConcatPerspective {
    kConcatBeforeClips,
    kConcatAfterClips,
    kConcatBetweenClips
};
// Order in which clipShader(image) and clipShader(gradient) are specified; only meaningful
// when CanvasPerspective is kConcatBetweenClips.
enum ClipOrder {
    kClipImageFirst,
    kClipGradientFirst,

    kDoesntMatter = kClipImageFirst
};
// Which shaders have perspective applied as a local matrix.
enum LocalMatrix {
    kNoLocalMat,
    kImageWithLocalMat,
    kGradientWithLocalMat,
    kBothWithLocalMat
};
struct Config {
    ConcatPerspective fConcat;
    ClipOrder         fOrder;
    LocalMatrix       fLM;
};

static void draw_banner(SkCanvas* canvas, Config config) {
    SkString banner;
    banner.append("Persp: ");

    if (config.fConcat == kConcatBeforeClips || config.fLM == kBothWithLocalMat) {
        banner.append("Both Clips");
    } else {
        SkASSERT((config.fConcat == kConcatBetweenClips && config.fLM == kNoLocalMat) ||
                 (config.fConcat == kConcatAfterClips && (config.fLM == kImageWithLocalMat ||
                                                          config.fLM == kGradientWithLocalMat)));
        if ((config.fConcat == kConcatBetweenClips && config.fOrder == kClipImageFirst) ||
            config.fLM == kGradientWithLocalMat) {
            banner.append("Gradient");
        } else {
            SkASSERT(config.fOrder == kClipGradientFirst || config.fLM == kImageWithLocalMat);
            banner.append("Image");
        }
    }
    if (config.fLM != kNoLocalMat) {
        banner.append(" (w/ LM, should equal top row)");
    }

    static const SkFont kFont(ToolUtils::DefaultPortableTypeface(), 12);
    canvas->drawString(banner.c_str(), 20.f, -30.f, kFont, SkPaint());
};

}  // namespace

DEF_SIMPLE_GM(clip_shader_persp, canvas, 1370, 1030) {
    // Each draw has a clipShader(image-shader), a clipShader(gradient-shader), a concat(persp-mat),
    // and each shader may or may not be wrapped with a perspective local matrix.

    // Pairs of configs that should match in appearance where first config doesn't use a local
    // matrix (top row of GM) and the second does (bottom row of GM).
    Config matches[][2] = {
            // Everything has perspective
            {{kConcatBeforeClips,  kDoesntMatter,      kNoLocalMat},
             {kConcatAfterClips,   kDoesntMatter,      kBothWithLocalMat}},
            // Image shader has perspective
            {{kConcatBetweenClips, kClipGradientFirst, kNoLocalMat},
             {kConcatAfterClips,   kDoesntMatter,      kImageWithLocalMat}},
            // Gradient shader has perspective
            {{kConcatBetweenClips, kClipImageFirst,    kNoLocalMat},
             {kConcatAfterClips,   kDoesntMatter,      kGradientWithLocalMat}}
    };

    // The image that is drawn
    auto img = ToolUtils::GetResourceAsImage("images/yellow_rose.png");
    // Scale factor always applied to the image shader so that it tiles
    SkMatrix scale = SkMatrix::Scale(1.f / 4.f, 1.f / 4.f);
    // The perspective matrix applied wherever needed
    SkPoint src[4];
    SkRect::Make(img->dimensions()).toQuad(src);
    SkPoint dst[4] = {{0, 80.f},
                      {img->width() + 28.f, -100.f},
                      {img->width() - 28.f, img->height() + 100.f},
                      {0.f, img->height() - 80.f}};
    SkMatrix persp;
    SkAssertResult(persp.setPolyToPoly(src, dst, 4));

    SkMatrix perspScale = SkMatrix::Concat(persp, scale);

    auto drawConfig = [&](Config config) {
        canvas->save();

        draw_banner(canvas, config);

        // Make clipShaders (possibly with local matrices)
        bool gradLM = config.fLM == kGradientWithLocalMat || config.fLM == kBothWithLocalMat;
        const SkColor gradColors[] = {SK_ColorBLACK, SkColorSetARGB(128, 128, 128, 128)};
        auto gradShader = SkGradientShader::MakeRadial({0.5f * img->width(), 0.5f * img->height()},
                                                        0.1f * img->width(), gradColors, nullptr, 2,
                                                        SkTileMode::kRepeat, 0,
                                                        gradLM ? &persp : nullptr);
        bool imageLM = config.fLM == kImageWithLocalMat || config.fLM == kBothWithLocalMat;
        auto imgShader = img->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                         SkSamplingOptions(), imageLM ? perspScale : scale);

        // Perspective before any clipShader
        if (config.fConcat == kConcatBeforeClips) {
            canvas->concat(persp);
        }

        // First clipshader
        canvas->clipShader(config.fOrder == kClipImageFirst ? imgShader : gradShader);

        // Perspective between clipShader
        if (config.fConcat == kConcatBetweenClips) {
            canvas->concat(persp);
        }

        // Second clipShader
        canvas->clipShader(config.fOrder == kClipImageFirst ? gradShader : imgShader);

        // Perspective after clipShader
        if (config.fConcat == kConcatAfterClips) {
            canvas->concat(persp);
        }

        // Actual draw and clip boundary are the same for all configs
        canvas->clipIRect(img->bounds());
        canvas->clear(SK_ColorBLACK);
        canvas->drawImage(img, 0, 0);

        canvas->restore();
    };

    SkIRect grid = persp.mapRect(SkRect::Make(img->dimensions())).roundOut();
    grid.fLeft -= 20; // manual adjust to look nicer

    canvas->translate(10.f, 10.f);

    for (size_t i = 0; i < std::size(matches); ++i) {
        canvas->save();
        canvas->translate(-grid.fLeft, -grid.fTop);
        drawConfig(matches[i][0]);
        canvas->translate(0.f, grid.height());
        drawConfig(matches[i][1]);
        canvas->restore();

        canvas->translate(grid.width(), 0.f);
    }
}

DEF_SIMPLE_GM(clip_shader_difference, canvas, 512, 512) {
    auto image = ToolUtils::GetResourceAsImage("images/yellow_rose.png");
    canvas->clear(SK_ColorGRAY);

    SkRect rect = SkRect::MakeWH(256, 256);
    SkMatrix local = SkMatrix::RectToRect(SkRect::MakeWH(image->width(), image->height()),
                                          SkRect::MakeWH(64, 64));
    auto shader = image->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                    SkSamplingOptions(), &local);

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);

    // TL: A rectangle
    {
        canvas->save();
        canvas->translate(0, 0);
        canvas->clipShader(shader, SkClipOp::kDifference);
        canvas->drawRect(rect, paint);
        canvas->restore();
    }
    // TR: A round rectangle
    {
        canvas->save();
        canvas->translate(256, 0);
        canvas->clipShader(shader, SkClipOp::kDifference);
        canvas->drawRRect(SkRRect::MakeRectXY(rect, 64.f, 64.f), paint);
        canvas->restore();
    }
    // BL: A path
    {
        canvas->save();
        canvas->translate(0, 256);
        canvas->clipShader(shader, SkClipOp::kDifference);

        SkPath path;
        path.moveTo(0.f, 128.f);
        path.lineTo(128.f, 256.f);
        path.lineTo(256.f, 128.f);
        path.lineTo(128.f, 0.f);

        SkScalar d = 64.f * SK_ScalarSqrt2;
        path.moveTo(128.f - d, 128.f - d);
        path.lineTo(128.f - d, 128.f + d);
        path.lineTo(128.f + d, 128.f + d);
        path.lineTo(128.f + d, 128.f - d);
        canvas->drawPath(path, paint);
        canvas->restore();
    }
    // BR: Text
    {
        canvas->save();
        canvas->translate(256, 256);
        canvas->clipShader(shader, SkClipOp::kDifference);
        SkFont font = SkFont(ToolUtils::DefaultPortableTypeface(), 64.f);
        for (int y = 0; y < 4; ++y) {
            canvas->drawString("Hello", 32.f, y * 64.f, font, paint);
        }
        canvas->restore();
    }
}
