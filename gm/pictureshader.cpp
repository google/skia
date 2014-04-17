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
#include "SkShader.h"

namespace skiagm {

static struct {
    SkShader::TileMode tmx;
    SkShader::TileMode tmy;
} kTileConfigs[] = {
    { SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode },
    { SkShader::kRepeat_TileMode, SkShader::kClamp_TileMode  },
    { SkShader::kMirror_TileMode, SkShader::kRepeat_TileMode },
};

class PictureShaderGM : public GM {
public:

    PictureShaderGM(SkScalar tileSize, SkScalar sceneSize)
        : fTileSize(tileSize)
        , fSceneSize(sceneSize) {

        // Build the picture.
        SkPictureRecorder recorder;
        SkCanvas* pictureCanvas = recorder.beginRecording(SkScalarRoundToInt(tileSize),
                                                          SkScalarRoundToInt(tileSize),
                                                          NULL, 0);
        this->drawTile(pictureCanvas);
        SkAutoTUnref<SkPicture> p(recorder.endRecording());

        // Build a reference bitmap.
        SkBitmap bm;
        bm.allocN32Pixels(SkScalarRoundToInt(tileSize), SkScalarRoundToInt(tileSize));
        bm.eraseColor(SK_ColorTRANSPARENT);
        SkCanvas bitmapCanvas(bm);
        this->drawTile(&bitmapCanvas);

        for (unsigned i = 0; i < SK_ARRAY_COUNT(kTileConfigs); ++i) {
            fPictureShaders[i].reset(SkShader::CreatePictureShader(p,
                                                                   kTileConfigs[i].tmx,
                                                                   kTileConfigs[i].tmy));

            fBitmapShaders[i].reset(SkShader::CreateBitmapShader(bm,
                                                                 kTileConfigs[i].tmx,
                                                                 kTileConfigs[i].tmy));
        }
    }

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("pictureshader");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(1400, 1250);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        this->drawSceneColumn(canvas, SkPoint::Make(0, 0), 1, 1, 0);
        this->drawSceneColumn(canvas, SkPoint::Make(0, fSceneSize * 6.4f), 1, 2, 0);
        this->drawSceneColumn(canvas, SkPoint::Make(fSceneSize * 2.4f, 0), 1, 1, 1);
        this->drawSceneColumn(canvas, SkPoint::Make(fSceneSize * 2.4f, fSceneSize * 6.4f), 1, 1, 2);
        this->drawSceneColumn(canvas, SkPoint::Make(fSceneSize * 4.8f, 0), 2, 1, 0);
        this->drawSceneColumn(canvas, SkPoint::Make(fSceneSize * 9.6f, 0), 2, 2, 0);
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

        fPictureShaders[tileMode]->setLocalMatrix(localMatrix);
        paint.setShader(fPictureShaders[tileMode].get());
        canvas->drawRect(SkRect::MakeWH(fSceneSize, fSceneSize), paint);

        canvas->translate(fSceneSize * 1.1f, 0);

        fBitmapShaders[tileMode]->setLocalMatrix(localMatrix);
        paint.setShader(fBitmapShaders[tileMode].get());
        canvas->drawRect(SkRect::MakeWH(fSceneSize, fSceneSize), paint);

        canvas->restore();
    }

    SkScalar    fTileSize;
    SkScalar    fSceneSize;

    SkAutoTUnref<SkShader> fPictureShaders[SK_ARRAY_COUNT(kTileConfigs)];
    SkAutoTUnref<SkShader> fBitmapShaders[SK_ARRAY_COUNT(kTileConfigs)];

    typedef GM INHERITED;
};

DEF_GM( return SkNEW_ARGS(PictureShaderGM, (50, 100)); )
}
