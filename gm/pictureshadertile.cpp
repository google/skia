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

constexpr SkScalar kPictureSize = SK_Scalar1;
constexpr SkScalar kFillSize = 100;
constexpr unsigned kRowSize = 6;

constexpr struct {
    SkScalar x, y, w, h;
    SkScalar offsetX, offsetY;
} tiles[] = {
    {      0,      0,    1,    1,      0,    0 },
    {  -0.5f,  -0.5f,    1,    1,      0,    0 },
    {   0.5f,   0.5f,    1,    1,      0,    0 },

    {      0,      0, 1.5f, 1.5f,      0,    0 },
    {  -0.5f,  -0.5f, 1.5f, 1.5f,      0,    0 },
    {   0.5f,   0.5f, 1.5f, 1.5f,      0,    0 },

    {      0,      0, 0.5f, 0.5f,      0,    0 },
    {  0.25f,  0.25f, 0.5f, 0.5f,      0,    0 },
    { -0.25f, -0.25f, 0.5f, 0.5f,      0,    0 },

    {      0,      0,    1,    1,   0.5f, 0.5f },
    {  -0.5f,  -0.5f,    1,    1,   0.5f, 0.5f },
    {   0.5f,   0.5f,    1,    1,   0.5f, 0.5f },

    {      0,      0, 1.5f, 1.5f,   0.5f, 0.5f },
    {  -0.5f,  -0.5f, 1.5f, 1.5f,   0.5f, 0.5f },
    {   0.5f,   0.5f, 1.5f, 1.5f,   0.5f, 0.5f },

    {      0,      0, 1.5f,    1,      0,    0 },
    {  -0.5f,  -0.5f, 1.5f,    1,      0,    0 },
    {   0.5f,   0.5f, 1.5f,    1,      0,    0 },

    {      0,      0, 0.5f,    1,      0,    0 },
    {  0.25f,  0.25f, 0.5f,    1,      0,    0 },
    { -0.25f, -0.25f, 0.5f,    1,      0,    0 },

    {      0,      0,    1, 1.5f,      0,    0 },
    {  -0.5f,  -0.5f,    1, 1.5f,      0,    0 },
    {   0.5f,   0.5f,    1, 1.5f,      0,    0 },

    {      0,      0,    1, 0.5f,      0,    0 },
    {  0.25f,  0.25f,    1, 0.5f,      0,    0 },
    { -0.25f, -0.25f,    1, 0.5f,      0,    0 },
};

static void draw_scene(SkCanvas* canvas, SkScalar pictureSize) {
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

class PictureShaderTileGM : public skiagm::GM {
protected:

    SkString onShortName() override {
        return SkString("pictureshadertile");
    }

    SkISize onISize() override {
        return SkISize::Make(800, 600);
    }

    void onOnceBeforeDraw() override {
        SkPictureRecorder recorder;
        SkCanvas* pictureCanvas = recorder.beginRecording(kPictureSize, kPictureSize);
        draw_scene(pictureCanvas, kPictureSize);
        sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());

        SkPoint offset = SkPoint::Make(100, 100);
        pictureCanvas = recorder.beginRecording(SkRect::MakeXYWH(offset.x(), offset.y(),
                                                                 kPictureSize, kPictureSize));
        pictureCanvas->translate(offset.x(), offset.y());
        draw_scene(pictureCanvas, kPictureSize);
        sk_sp<SkPicture> offsetPicture(recorder.finishRecordingAsPicture());

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

            sk_sp<SkPicture> pictureRef = picture;
            SkRect* tilePtr = &tile;

            if (tile == SkRect::MakeWH(kPictureSize, kPictureSize)) {
                // When the tile == picture bounds, exercise the picture + offset path.
                pictureRef = offsetPicture;
                tilePtr = nullptr;
            }

            fShaders[i] = pictureRef->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                                 &localMatrix, tilePtr);
        }
    }

    void onDraw(SkCanvas* canvas) override {
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
    sk_sp<SkShader> fShaders[SK_ARRAY_COUNT(tiles)];

    typedef GM INHERITED;
};

DEF_GM(return new PictureShaderTileGM;)
