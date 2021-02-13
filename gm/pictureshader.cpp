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
    PictureShaderGM(SkScalar tileSize, SkScalar sceneSize, bool useLocalMatrixWrapper = false)
        : fTileSize(tileSize)
        , fSceneSize(sceneSize)
        , fUseLocalMatrixWrapper(useLocalMatrixWrapper) {}

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


    SkString onShortName() override {
        return SkStringPrintf("pictureshader%s", fUseLocalMatrixWrapper ? "_localwrapper" : "");
    }

    SkISize onISize() override {
        return SkISize::Make(1400, 1450);
    }

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
        SkASSERT(tileMode < SK_ARRAY_COUNT(kTileConfigs));

        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(SK_ColorLTGRAY);

        canvas->save();
        canvas->concat(matrix);
        canvas->drawRect(SkRect::MakeWH(fSceneSize, fSceneSize), paint);
        canvas->drawRect(SkRect::MakeXYWH(fSceneSize * 1.1f, 0, fSceneSize, fSceneSize), paint);

        auto pictureShader = fPicture->makeShader(kTileConfigs[tileMode].tmx,
                                                  kTileConfigs[tileMode].tmy,
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

    sk_sp<SkPicture> fPicture;
    SkBitmap fBitmap;

    SkScalar    fTileSize;
    SkScalar    fSceneSize;
    bool        fUseLocalMatrixWrapper;

    using INHERITED = GM;
};

DEF_GM(return new PictureShaderGM(50, 100);)
DEF_GM(return new PictureShaderGM(50, 100, true);)

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

    p.setShader(picture->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat));
    canvas->drawPaint(p);
}

#ifdef SK_SUPPORT_LEGACY_SETFILTERQUALITY
/*
    Test picture-shader's filtering (after the tile is created.
    The GM draws a 2x2 grid of tiled images (circle, square, X)

    Column 0 should be hard-edged
    Column 1 should be filtered

    Row 0 deduces this from the paint (legacy behavior)
    Row 1 takes this as an explicit parameter (SkFilterMode)
 */
DEF_SIMPLE_GM(picture_shader_filter, canvas, 230, 230) {
    auto pic = [&] {
        SkRect r = SkRect::MakeWH(100, 100);
        SkPictureRecorder recorder;
        SkCanvas* c = recorder.beginRecording(r);
        SkPaint paint;
        paint.setStroke(true);
        c->drawRect({5, 5, 95, 95}, paint);
        c->drawCircle(50, 50, 30, paint);
        c->drawLine(5, 1, 95,95, paint);
        c->drawLine(5,95, 95, 1, paint);
        return recorder.finishRecordingAsPicture();
    }();

    struct {
        SkPoint         fLoc;
        SkFilterMode    fFilter;
        bool            fInheritFromPaint;

        void setup(SkPaint* paint, sk_sp<SkPicture> pic) const {
            SkTileMode tm = SkTileMode::kRepeat;
            sk_sp<SkShader> sh;
            if (fInheritFromPaint) {
                sh = pic->makeShader(tm, tm, nullptr, nullptr);
                paint->setFilterQuality(fFilter == SkFilterMode::kNearest ? kNone_SkFilterQuality
                                                                          : kLow_SkFilterQuality);
            } else {
                sh = pic->makeShader(tm, tm, fFilter, nullptr, nullptr);
                // the draw should ignore paint's filterquality,
                // but we'll set it to something wacky just to be test that
                paint->setFilterQuality(kHigh_SkFilterQuality);
            }
            paint->setShader(sh);
        }
    } recs[] = {
        { {0, 0}, SkFilterMode::kNearest, true },
        { {1, 0}, SkFilterMode::kLinear,  true },
        { {0, 1}, SkFilterMode::kNearest, false },
        { {1, 1}, SkFilterMode::kLinear,  false },
    };

    canvas->translate(10, 10);
    canvas->scale(1.0f/3, 1.0f/3);
    for (const auto& r : recs) {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->translate(r.fLoc.fX * 330, r.fLoc.fY * 330);
        SkPaint paint;
        r.setup(&paint, pic);
        canvas->drawRect({0, 0, 300, 300}, paint);
    }
}
#endif
