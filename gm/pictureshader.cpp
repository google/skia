/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkBitmap.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkShader.h"

static struct {
    SkShader::TileMode tmx;
    SkShader::TileMode tmy;
} kTileConfigs[] = {
    { SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode },
    { SkShader::kRepeat_TileMode, SkShader::kClamp_TileMode  },
    { SkShader::kMirror_TileMode, SkShader::kRepeat_TileMode },
};

class PictureShaderGM : public skiagm::GM {
public:
    PictureShaderGM(SkScalar tileSize, SkScalar sceneSize)
        : fTileSize(tileSize)
        , fSceneSize(sceneSize) {

        // Build the picture.
        SkPictureRecorder recorder;
        SkCanvas* pictureCanvas = recorder.beginRecording(tileSize, tileSize, NULL, 0);
        this->drawTile(pictureCanvas);
        fPicture.reset(recorder.endRecording());

        // Build a reference bitmap.
        fBitmap.allocN32Pixels(SkScalarCeilToInt(tileSize), SkScalarCeilToInt(tileSize));
        fBitmap.eraseColor(SK_ColorTRANSPARENT);
        SkCanvas bitmapCanvas(fBitmap);
        this->drawTile(&bitmapCanvas);
    }

protected:

    SkString onShortName() SK_OVERRIDE {
        return SkString("pictureshader");
    }

    SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(1400, 1450);
    }

    void onDraw(SkCanvas* canvas) SK_OVERRIDE {
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

        SkAutoTUnref<SkShader> pictureShader(SkShader::CreatePictureShader(
                    fPicture,
                    kTileConfigs[tileMode].tmx,
                    kTileConfigs[tileMode].tmy,
                    &localMatrix,
                    NULL));
        paint.setShader(pictureShader.get());
        canvas->drawRect(SkRect::MakeWH(fSceneSize, fSceneSize), paint);

        canvas->translate(fSceneSize * 1.1f, 0);

        SkAutoTUnref<SkShader> bitmapShader(SkShader::CreateBitmapShader(
                    fBitmap,
                    kTileConfigs[tileMode].tmx,
                    kTileConfigs[tileMode].tmy,
                    &localMatrix));
        paint.setShader(bitmapShader.get());
        canvas->drawRect(SkRect::MakeWH(fSceneSize, fSceneSize), paint);

        canvas->restore();
    }

    SkScalar    fTileSize;
    SkScalar    fSceneSize;

    SkAutoTUnref<SkPicture> fPicture;
    SkBitmap fBitmap;

    typedef GM INHERITED;
};

DEF_GM( return SkNEW_ARGS(PictureShaderGM, (50, 100)); )
