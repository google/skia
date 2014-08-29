/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkShader.h"

static const SkScalar kPictureSize = SK_Scalar1;
static const SkScalar kFillSize = 100;
static const unsigned kRowSize = 6;

static const struct {
    SkScalar x, y, w, h;
    SkScalar offsetX, offsetY;
} tiles[] = {
    {      0,      0,    1,    1,      0,    0 },
    {   0.5f,   0.5f,    1,    1,      0,    0 },
    {  -0.5f,  -0.5f,    1,    1,      0,    0 },

    {      0,      0, 1.5f, 1.5f,      0,    0 },
    {   0.5f,   0.5f, 1.5f, 1.5f,      0,    0 },
    {  -0.5f,  -0.5f, 1.5f, 1.5f,      0,    0 },

    {      0,      0, 0.5f, 0.5f,      0,    0 },
    { -0.25f, -0.25f, 0.5f, 0.5f,      0,    0 },
    {  0.25f,  0.25f, 0.5f, 0.5f,      0,    0 },

    {      0,      0,    1,    1,   0.5f, 0.5f },
    {   0.5f,   0.5f,    1,    1,   0.5f, 0.5f },
    {  -0.5f,  -0.5f,    1,    1,   0.5f, 0.5f },

    {      0,      0, 1.5f, 1.5f,   0.5f, 0.5f },
    {   0.5f,   0.5f, 1.5f, 1.5f,   0.5f, 0.5f },
    {  -0.5f,  -0.5f, 1.5f, 1.5f,   0.5f, 0.5f },

    {      0,      0, 1.5f,    1,      0,    0 },
    {   0.5f,   0.5f, 1.5f,    1,      0,    0 },
    {  -0.5f,  -0.5f, 1.5f,    1,      0,    0 },

    {      0,      0, 0.5f,    1,      0,    0 },
    { -0.25f, -0.25f, 0.5f,    1,      0,    0 },
    {  0.25f,  0.25f, 0.5f,    1,      0,    0 },

    {      0,      0,    1, 1.5f,      0,    0 },
    {   0.5f,   0.5f,    1, 1.5f,      0,    0 },
    {  -0.5f,  -0.5f,    1, 1.5f,      0,    0 },

    {      0,      0,    1, 0.5f,      0,    0 },
    { -0.25f, -0.25f,    1, 0.5f,      0,    0 },
    {  0.25f,  0.25f,    1, 0.5f,      0,    0 },
};

class PictureShaderTileGM : public skiagm::GM {
public:
    PictureShaderTileGM() {
        SkPictureRecorder recorder;
        SkCanvas* pictureCanvas = recorder.beginRecording(kPictureSize, kPictureSize, NULL, 0);
        drawScene(pictureCanvas, kPictureSize);
        SkAutoTUnref<SkPicture> picture(recorder.endRecording());

        for (unsigned i = 0; i < SK_ARRAY_COUNT(tiles); ++i) {
            SkRect tile = SkRect::MakeXYWH(tiles[i].x * kPictureSize,
                                           tiles[i].y * kPictureSize,
                                           tiles[i].w * kPictureSize,
                                           tiles[i].h * kPictureSize);
            SkMatrix localMatrix;
            localMatrix.setTranslate(tiles[i].offsetX * kPictureSize,
                                     tiles[i].offsetY * kPictureSize);
            localMatrix.postScale(kFillSize / (2 * kPictureSize),
                                  kFillSize / (2 * kPictureSize));
            fShaders[i].reset(SkShader::CreatePictureShader(picture,
                              SkShader::kRepeat_TileMode,
                              SkShader::kRepeat_TileMode,
                              &localMatrix,
                              &tile));
        }
    }

protected:
    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        return kSkipTiled_Flag;
    }

    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("pictureshadertile");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(800, 600);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        canvas->clear(SK_ColorBLACK);

        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);

        for (unsigned i = 0; i < SK_ARRAY_COUNT(fShaders); ++i) {
            paint.setShader(fShaders[i]);

            canvas->save();
            canvas->translate((i % kRowSize) * kFillSize * 1.1f,
                              (i / kRowSize) * kFillSize * 1.1f);
            canvas->drawRect(SkRect::MakeWH(kFillSize, kFillSize), paint);
            canvas->restore();
        }
    }

private:
    void drawScene(SkCanvas* canvas, SkScalar pictureSize) {
        canvas->clear(SK_ColorWHITE);

        SkPaint paint;
        paint.setColor(SK_ColorGREEN);
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAntiAlias(true);

        canvas->drawCircle(pictureSize / 4, pictureSize / 4, pictureSize / 4, paint);
        canvas->drawRect(SkRect::MakeXYWH(pictureSize / 2, pictureSize / 2,
                                          pictureSize / 2, pictureSize / 2), paint);

        paint.setColor(SK_ColorRED);
        canvas->drawLine(pictureSize / 2, pictureSize * 1 / 3,
                         pictureSize / 2, pictureSize * 2 / 3, paint);
        canvas->drawLine(pictureSize * 1 / 3, pictureSize / 2,
                         pictureSize * 2 / 3, pictureSize / 2, paint);

        paint.setColor(SK_ColorBLACK);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(SkRect::MakeWH(pictureSize, pictureSize), paint);
    }

    SkAutoTUnref<SkShader> fShaders[SK_ARRAY_COUNT(tiles)];

    typedef GM INHERITED;
};

DEF_GM( return SkNEW(PictureShaderTileGM); )
