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
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "src/core/SkClipOpPriv.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

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
    SkString onShortName() override {
        SkString str;
        str.printf("complexclip_%s%s%s",
                   fDoAAClip ? "aa" : "bw",
                   fDoSaveLayer ? "_layer" : "",
                   fInvertDraw ? "_invert" : "");
        return str;
    }

    SkISize onISize() override { return SkISize::Make(970, 780); }

    void onDraw(SkCanvas* canvas) override {
        SkPath path;
        path.moveTo(0,   50)
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
            .close();
        if (fInvertDraw) {
            path.setFillType(SkPathFillType::kInverseEvenOdd);
        } else {
            path.setFillType(SkPathFillType::kEvenOdd);
        }
        SkPaint pathPaint;
        pathPaint.setAntiAlias(true);
        pathPaint.setColor(gPathColor);

        SkPath clipA;
        clipA.addPoly({{10,  20}, {165, 22}, {70,  105}, {165, 177}, {-5,  180}}, false).close();

        SkPath clipB;
        clipB.addPoly({{40,  10}, {190, 15}, {195, 190}, {40,  185}, {155, 100}}, false).close();

        SkFont font(ToolUtils::create_portable_typeface(), 20);

        constexpr struct {
            SkClipOp fOp;
            const char*      fName;
        } gOps[] = { //extra spaces in names for measureText
            {kIntersect_SkClipOp,         "Isect "},
            {kDifference_SkClipOp,        "Diff " },
            {kUnion_SkClipOp,             "Union "},
            {kXOR_SkClipOp,               "Xor "  },
            {kReverseDifference_SkClipOp, "RDiff "}
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
            for (size_t op = 0; op < SK_ARRAY_COUNT(gOps); ++op) {
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

    typedef GM INHERITED;
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
}

DEF_SIMPLE_GM(clip_shader, canvas, 840, 650) {
    auto img = GetResourceAsImage("images/yellow_rose.png");
    auto sh = img->makeShader();

    SkRect r = SkRect::MakeIWH(img->width(), img->height());
    SkPaint p;

    canvas->translate(10, 10);
    canvas->drawImage(img, 0, 0, nullptr);

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
    canvas->clipShader(img->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, &lm));
    canvas->drawImage(img, 0, 0, nullptr);

    canvas->restore();
    canvas->restore();
}

DEF_SIMPLE_GM(clip_shader_layer, canvas, 430, 320) {
    auto img = GetResourceAsImage("images/yellow_rose.png");
    auto sh = img->makeShader();

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

DEF_SIMPLE_GM(clip_shader_persp, canvas, 1200, 900) {
    SkFont font(ToolUtils::create_portable_typeface(), 12);
    SkPaint textPaint;
    textPaint.setAntiAlias(true);

    auto drawBanner = [&](bool drawPersp, bool clipImgPersp, bool clipGradPersp, bool clipLM) {
        SkString banner;
        banner.printf("DrawImage Persp: %d", drawPersp);
        canvas->drawString(banner.c_str(), 20.f, -50.f, font, textPaint);
        banner.printf("ClipImage Persp: %d, w/ LM: %d", clipImgPersp, clipLM);
        canvas->drawString(banner.c_str(), 20.f, -40.f, font, textPaint);
        banner.printf("ClipGrad Persp:  %d, w/ LM: %d", clipGradPersp, clipLM);
        canvas->drawString(banner.c_str(), 20.f, -30.f, font, textPaint);
    };

    auto img = GetResourceAsImage("images/yellow_rose.png");
    SkMatrix scale = SkMatrix::Scale(1.f / 10.f, 1.f / 10.f);
    auto imgShader = img->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, &scale);

    const SkColor gradColors[] = {SK_ColorBLACK, SK_ColorTRANSPARENT};
    auto gradShader = SkGradientShader::MakeRadial({0.5f * img->width(), 0.5f * img->height()},
                                                    0.1f * img->width(), gradColors, nullptr, 2,
                                                    SkTileMode::kRepeat);

    SkPoint src[4];
    SkRect::Make(img->dimensions()).toQuad(src);
    SkPoint dst[4] = {{0, 10.f},
                      {img->width() + 28.f, -100.f},
                      {img->width() - 28.f, img->height() + 100.f},
                      {0.f, img->height() - 10.f}};
    SkMatrix persp;
    SkAssertResult(persp.setPolyToPoly(src, dst, 4));
    SkIRect grid = persp.mapRect(SkRect::Make(img->dimensions())).roundOut();

    // Manual adjustments to grid size given contents of image to make them pack denser
    grid.fLeft -= 20;

    canvas->translate(10.f, 10.f);
    auto drawPattern = [&]() {
        canvas->clipRect(SkRect::MakeIWH(img->width(), img->height()));
        canvas->clear(SK_ColorBLACK);
        canvas->drawImage(img, 0, 0);
    };

    // The canvas has no perspective
    canvas->save();
    canvas->translate(-grid.fLeft, -grid.fTop);
    drawBanner(/*draw*/ false, /*clipImg*/ false, /*clipGrad*/ false, /*localMat*/ false);

    canvas->clipShader(imgShader);
    canvas->clipShader(gradShader);
    drawPattern();

    canvas->restore();

    // The canvas has perspective before clipShader is called
    canvas->save();
    canvas->translate(-grid.fLeft + grid.width(), -grid.fTop);
    drawBanner(/*draw*/ true, /*clipImg*/ true, /*clipGrad*/ true, /*localMat*/ false);

    canvas->concat(persp);
    canvas->clipShader(imgShader);
    canvas->clipShader(gradShader);
    drawPattern();

    canvas->restore();

    // The canvas has perspective between clipShader calls, clip image w/ perspective
    canvas->save();
    canvas->translate(-grid.fLeft + 2.f * grid.width(), -grid.fTop);
    drawBanner(/*draw*/ true, /*clipImg*/ true, /*clipGrad*/ false, /*localMat*/ false);

    canvas->clipShader(gradShader);
    canvas->concat(persp);
    canvas->clipShader(imgShader);
    drawPattern();

    canvas->restore();

    // The canvas has perspective between clipShader calls, gradient w/ perspective
    canvas->save();
    canvas->translate(-grid.fLeft, -grid.fTop + grid.height());
    drawBanner(/*draw*/ true, /*clipImg*/ false, /*clipGrad*/ true, /*localMat*/ false);

    canvas->clipShader(imgShader);
    canvas->concat(persp);
    canvas->clipShader(gradShader);
    drawPattern();

    canvas->restore();

    // The gradient shader has a perspective local matrix
    canvas->save();
    canvas->translate(-grid.fLeft + grid.width(), -grid.fTop + grid.height());
    drawBanner(/*draw*/ true, /*clipImg*/ false, /*clipGrad*/ true, /*localMat*/ true);

    canvas->clipShader(imgShader);
    canvas->clipShader(gradShader->makeWithLocalMatrix(persp));
    canvas->concat(persp);
    drawPattern();

    canvas->restore();

    // The image shader has a perspective local matrix
    canvas->save();
    canvas->translate(-grid.fLeft + 2.f * grid.width(), -grid.fTop + grid.height());
    drawBanner(/*draw*/ true, /*clipImg*/ true, /*clipGrad*/ false, /*localMat*/ true);

    // makeWithLocalMatrix is a pre-concat, if passed persp to the existing imgShader, we'd end up
    // evaluating S * P, whereas this samples the P * S that we want.
    canvas->clipShader(img->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, &persp)
                          ->makeWithLocalMatrix(scale));
    canvas->clipShader(gradShader);
    canvas->concat(persp);
    drawPattern();

    canvas->restore();

    // Draw groups around the test images that should match (i.e. the local matrix variant should
    // come out the same as its CTM variant).
    SkPaint groupPaint;
    groupPaint.setColor(SK_ColorGRAY);
    groupPaint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(SkRect::MakeXYWH(0.f, grid.height(), 2.f * grid.width(), grid.height())
                            .makeInset(2.f, 0.f), groupPaint);
    canvas->drawRect(SkRect::MakeXYWH(2.f * grid.width(), 0.f, grid.width(), 2.f * grid.height())
                            .makeInset(2.f, 0.f), groupPaint);
}
