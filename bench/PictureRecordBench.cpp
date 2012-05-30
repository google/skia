/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkString.h"

class PictureRecordBench : public SkBenchmark {
public:
    PictureRecordBench(void* param, const char name[]) : INHERITED(param) {
        fName.printf("picture_record_%s", name);
        fPictureWidth = SkIntToScalar(PICTURE_WIDTH);
        fPictureHeight = SkIntToScalar(PICTURE_HEIGHT);
    }

    enum {
        N = SkBENCHLOOP(25), // number of times to create the picture
        PICTURE_WIDTH = 1000,
        PICTURE_HEIGHT = 4000,
    };
protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) {

        for (int i = 0; i < N; i++) {

            SkPicture picture;

            SkCanvas* pCanvas = picture.beginRecording(PICTURE_WIDTH, PICTURE_HEIGHT);
            recordCanvas(pCanvas);

            // we don't need to draw the picture as the endRecording step will
            // do the work of transferring the recorded content into a playback
            // object.
            picture.endRecording();
        }
    }

    virtual void recordCanvas(SkCanvas* canvas) = 0;

    SkString fName;
    SkScalar fPictureWidth;
    SkScalar fPictureHeight;
    SkScalar fTextSize;
private:
    typedef SkBenchmark INHERITED;
};

/*
 *  An SkPicture has internal dictionaries to store bitmaps, matrices, paints,
 *  and regions.  This bench populates those dictionaries to test the speed of
 *  reading and writing to those particular dictionary data structures.
 */
class DictionaryRecordBench : public PictureRecordBench {
public:
    DictionaryRecordBench(void* param)
        : INHERITED(param, "dictionaries") { }

    enum {
        M = SkBENCHLOOP(100),   // number of elements in each dictionary
    };
protected:
    virtual void recordCanvas(SkCanvas* canvas) {

        const SkPoint translateDelta = getTranslateDelta();

        for (int i = 0; i < M; i++) {

            SkColor color = SK_ColorYELLOW + (i % 255);
            SkIRect rect = SkIRect::MakeWH(i,i);

            canvas->save();

            // set the clip to the given region
            SkRegion region;
            region.setRect(rect);
            canvas->clipRegion(region);

            // fill the clip with a color
            SkPaint paint;
            paint.setColor(color);
            canvas->drawPaint(paint);

            // set a matrix on the canvas
            SkMatrix matrix;
            matrix.setRotate(SkIntToScalar(i % 360));
            canvas->setMatrix(matrix);

            // create a simple bitmap
            SkBitmap bitmap;
            bitmap.setConfig(SkBitmap::kRGB_565_Config, 10, 10);
            bitmap.allocPixels();

            // draw a single color into the bitmap
            SkCanvas bitmapCanvas(bitmap);
            bitmapCanvas.drawColor(SkColorSetA(color, i % 255));

            // draw the bitmap onto the canvas
            canvas->drawBitmapMatrix(bitmap, matrix);

            canvas->restore();
            canvas->translate(translateDelta.fX, translateDelta.fY);
        }
    }

    SkPoint getTranslateDelta() {
        SkIPoint canvasSize = onGetSize();
        return SkPoint::Make(SkIntToScalar((PICTURE_WIDTH - canvasSize.fX)/M),
                             SkIntToScalar((PICTURE_HEIGHT- canvasSize.fY)/M));
    }
private:
    typedef PictureRecordBench INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkBenchmark* Fact0(void* p) { return new DictionaryRecordBench(p); }

static BenchRegistry gReg0(Fact0);
