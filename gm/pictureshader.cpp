/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "tools/ToolUtils.h"

static struct {
    SkTileMode tmx;
    SkTileMode tmy;
} kTileConfigs[] = {
    { SkTileMode::kRepeat, SkTileMode::kRepeat },
    { SkTileMode::kRepeat, SkTileMode::kClamp  },
    { SkTileMode::kMirror, SkTileMode::kRepeat },
};

class PictureShaderGM : public skiagm::GM {
public:
    PictureShaderGM(SkScalar tileSize, SkScalar sceneSize, bool useLocalMatrixWrapper = false,
                    float alpha = 1)
        : fTileSize(tileSize)
        , fSceneSize(sceneSize)
        , fAlpha(alpha)
        , fUseLocalMatrixWrapper(useLocalMatrixWrapper)
    {}

 protected:
    void onOnceBeforeDraw() override {
       // Build the picture.
        SkPictureRecorder recorder;
        SkCanvas* pictureCanvas = recorder.beginRecording(fTileSize, fTileSize);
        this->drawTile(pictureCanvas);
        fPicture = recorder.finishRecordingAsPicture();

        // Build a reference bitmap.
        fBitmap.allocN32Pixels(SkScalarCeilToInt(fTileSize), SkScalarCeilToInt(fTileSize));
        fBitmap.eraseColor(SK_ColorTRANSPARENT);
        SkCanvas bitmapCanvas(fBitmap);
        this->drawTile(&bitmapCanvas);
    }

    SkString getName() const override {
        return SkStringPrintf("pictureshader%s%s",
                              fUseLocalMatrixWrapper ? "_localwrapper" : "",
                              fAlpha < 1 ? "_alpha" : "");
    }

    SkISize getISize() override { return SkISize::Make(1400, 1450); }

    void onDraw(SkCanvas* canvas) override {
        this->drawSceneColumn(canvas, SkPoint::Make(0, 0), 1, 1, 0);
        this->drawSceneColumn(canvas, SkPoint::Make(0, fSceneSize * 6.4f), 1, 2, 0);
        this->drawSceneColumn(canvas, SkPoint::Make(fSceneSize * 2.4f, 0), 1, 1, 1);
        this->drawSceneColumn(canvas, SkPoint::Make(fSceneSize * 2.4f, fSceneSize * 6.4f), 1, 1, 2);
        this->drawSceneColumn(canvas, SkPoint::Make(fSceneSize * 4.8f, 0), 2, 1, 0);
        this->drawSceneColumn(canvas, SkPoint::Make(fSceneSize * 9.6f, 0), 2, 2, 0);

        // One last custom row to exercise negative scaling
        SkMatrix ctm, localMatrix;
        ctm.setTranslate(fSceneSize * 2.1f, fSceneSize * 13.8f);
        ctm.preScale(-1, -1);
        localMatrix.setScale(2, 2);
        this->drawScene(canvas, ctm, localMatrix, 0);

        ctm.setTranslate(fSceneSize * 2.4f, fSceneSize * 12.8f);
        localMatrix.setScale(-1, -1);
        this->drawScene(canvas, ctm, localMatrix, 0);

        ctm.setTranslate(fSceneSize * 4.8f, fSceneSize * 12.3f);
        ctm.preScale(2, 2);
        this->drawScene(canvas, ctm, localMatrix, 0);

        ctm.setTranslate(fSceneSize * 13.8f, fSceneSize * 14.3f);
        ctm.preScale(-2, -2);
        localMatrix.setTranslate(fTileSize / 4, fTileSize / 4);
        localMatrix.preRotate(45);
        localMatrix.preScale(-2, -2);
        this->drawScene(canvas, ctm, localMatrix, 0);
    }

private:
    void drawSceneColumn(SkCanvas* canvas, const SkPoint& pos, SkScalar scale, SkScalar localScale,
                         unsigned tileMode) {
        SkMatrix ctm, localMatrix;

        ctm.setTranslate(pos.x(), pos.y());
        ctm.preScale(scale, scale);
        localMatrix.setScale(localScale, localScale);
        this->drawScene(canvas, ctm, localMatrix, tileMode);

        ctm.setTranslate(pos.x(), pos.y() + fSceneSize * 1.2f * scale);
        ctm.preScale(scale, scale);
        localMatrix.setTranslate(fTileSize / 4, fTileSize / 4);
        localMatrix.preScale(localScale, localScale);
        this->drawScene(canvas, ctm, localMatrix, tileMode);

        ctm.setTranslate(pos.x(), pos.y() + fSceneSize * 2.4f * scale);
        ctm.preScale(scale, scale);
        localMatrix.setRotate(45);
        localMatrix.preScale(localScale, localScale);
        this->drawScene(canvas, ctm, localMatrix, tileMode);

        ctm.setTranslate(pos.x(), pos.y() + fSceneSize * 3.6f * scale);
        ctm.preScale(scale, scale);
        localMatrix.setSkew(1, 0);
        localMatrix.preScale(localScale, localScale);
        this->drawScene(canvas, ctm, localMatrix, tileMode);

        ctm.setTranslate(pos.x(), pos.y() + fSceneSize * 4.8f * scale);
        ctm.preScale(scale, scale);
        localMatrix.setTranslate(fTileSize / 4, fTileSize / 4);
        localMatrix.preRotate(45);
        localMatrix.preScale(localScale, localScale);
        this->drawScene(canvas, ctm, localMatrix, tileMode);
    }

    void drawTile(SkCanvas* canvas) {
        SkPaint paint;
        paint.setColor(SK_ColorGREEN);
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAntiAlias(true);

        canvas->drawCircle(fTileSize / 4, fTileSize / 4, fTileSize / 4, paint);
        canvas->drawRect(SkRect::MakeXYWH(fTileSize / 2, fTileSize / 2,
                                          fTileSize / 2, fTileSize / 2), paint);

        paint.setColor(SK_ColorRED);
        canvas->drawLine(fTileSize / 2, fTileSize * 1 / 3,
                         fTileSize / 2, fTileSize * 2 / 3, paint);
        canvas->drawLine(fTileSize * 1 / 3, fTileSize / 2,
                         fTileSize * 2 / 3, fTileSize / 2, paint);
    }

    void drawScene(SkCanvas* canvas, const SkMatrix& matrix, const SkMatrix& localMatrix,
                   unsigned tileMode) {
        SkASSERT(tileMode < std::size(kTileConfigs));

        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(SK_ColorLTGRAY);

        canvas->save();
        canvas->concat(matrix);
        canvas->drawRect(SkRect::MakeWH(fSceneSize, fSceneSize), paint);
        canvas->drawRect(SkRect::MakeXYWH(fSceneSize * 1.1f, 0, fSceneSize, fSceneSize), paint);

        paint.setAlphaf(fAlpha);

        auto pictureShader = fPicture->makeShader(kTileConfigs[tileMode].tmx,
                                                  kTileConfigs[tileMode].tmy,
                                                  SkFilterMode::kNearest,
                                                  fUseLocalMatrixWrapper ? nullptr : &localMatrix,
                                                  nullptr);
        paint.setShader(fUseLocalMatrixWrapper
                            ? pictureShader->makeWithLocalMatrix(localMatrix)
                            : pictureShader);
        canvas->drawRect(SkRect::MakeWH(fSceneSize, fSceneSize), paint);

        canvas->translate(fSceneSize * 1.1f, 0);

        auto bitmapShader = fBitmap.makeShader(kTileConfigs[tileMode].tmx,
                                               kTileConfigs[tileMode].tmy,
                                               SkSamplingOptions(),
                                               fUseLocalMatrixWrapper ? nullptr : &localMatrix);
        paint.setShader(fUseLocalMatrixWrapper
                            ? bitmapShader->makeWithLocalMatrix(localMatrix)
                            : bitmapShader);
        canvas->drawRect(SkRect::MakeWH(fSceneSize, fSceneSize), paint);

        canvas->restore();
    }

    const SkScalar   fTileSize;
    const SkScalar   fSceneSize;
    const float      fAlpha;
    const bool       fUseLocalMatrixWrapper;

    sk_sp<SkPicture> fPicture;
    SkBitmap         fBitmap;

    using INHERITED = GM;
};

DEF_GM(return new PictureShaderGM(50, 100);)
DEF_GM(return new PictureShaderGM(50, 100, true);)
DEF_GM(return new PictureShaderGM(50, 100, false, 0.25f);)

DEF_SIMPLE_GM(tiled_picture_shader, canvas, 400, 400) {
    // https://code.google.com/p/skia/issues/detail?id=3398
    SkRect tile = SkRect::MakeWH(100, 100);

    SkPictureRecorder recorder;
    SkCanvas* c = recorder.beginRecording(tile);

    SkRect r = tile;
    r.inset(4, 4);
    SkPaint p;
    p.setColor(ToolUtils::color_to_565(0xFF303F9F));  // dark blue
    c->drawRect(r, p);
    p.setColor(ToolUtils::color_to_565(0xFFC5CAE9));  // light blue
    p.setStrokeWidth(10);
    c->drawLine(20, 20, 80, 80, p);

    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());

    p.setColor(ToolUtils::color_to_565(0xFF8BC34A));  // green
    canvas->drawPaint(p);

    canvas->clipRect(SkRect::MakeXYWH(0, 0, 400, 350));
    p.setColor(0xFFB6B6B6);  // gray
    canvas->drawPaint(p);

    p.setShader(picture->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                    SkFilterMode::kNearest));
    canvas->drawPaint(p);
}

DEF_SIMPLE_GM(pictureshader_persp, canvas, 215, 110) {
    enum class DrawStrategy {
        kDirect,
        kPictureShader,
    };

    auto drawPicture = [](SkCanvas* canvas, sk_sp<SkPicture> picture, DrawStrategy strategy) {
        // Only want local upper 50x50 of 'picture' before we apply decal (or clip)
        SkRect bounds = {0.f, 0.f, 50.f, 50.f};
        switch(strategy) {
            case DrawStrategy::kDirect: {
                canvas->clipRect(bounds, true);
                canvas->drawPicture(picture);
                break; }
            case DrawStrategy::kPictureShader: {
                SkPaint paint;
                paint.setShader(picture->makeShader(SkTileMode::kDecal, SkTileMode::kDecal,
                                                    SkFilterMode::kLinear, nullptr, &bounds));
                canvas->drawRect({0.f, 0.f, 50.f, 50.f}, paint);
                break; }
        }
    };

    auto picture = []() {
        sk_sp<SkTypeface> typeface = SkTypeface::MakeDefault();
        if (!typeface) {
            typeface = SkTypeface::MakeFromName("monospace", SkFontStyle());
        }
        SkFont font;
        font.setTypeface(typeface);
        font.setHinting(SkFontHinting::kNormal);
        font.setSize(8.f);

        SkPaint paint;
        paint.setColor(SK_ColorGREEN);
        SkPictureRecorder recorder;
        SkCanvas* record_canvas = recorder.beginRecording({0, 0, 100, 100});
        record_canvas->drawTextBlob(SkTextBlob::MakeFromString("Hamburgefons", font),
                                    0, 16.f, paint);
        return recorder.finishRecordingAsPicture();
    }();

    SkM44 m;
    m.preScale(2.f, 2.f);
    SkM44 persp = SkM44::Perspective(0.01f, 10.f, SK_ScalarPI / 3.f);
    persp.preTranslate(0.f, 5.f, -0.1f);
    persp.preConcat(SkM44::Rotate({0.f, 1.f, 0.f}, 0.008f));
    m.postConcat(persp);

    canvas->clear(SK_ColorBLACK);
    canvas->translate(5.f, 5.f);
    for (auto strategy : { DrawStrategy::kDirect,
                           DrawStrategy::kPictureShader }) {
        canvas->save();

        SkPaint outline;
        outline.setColor(SK_ColorWHITE);
        outline.setStyle(SkPaint::kStroke_Style);
        outline.setStrokeWidth(1.f);
        canvas->drawRect({-1, -1, 101, 101}, outline);

        canvas->clipRect({0, 0, 100, 100});
        canvas->concat(m);

        drawPicture(canvas, picture, strategy);
        canvas->restore();

        canvas->translate(105.f, 0.f);
    }
}
