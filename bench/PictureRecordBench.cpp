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
#include "SkRandom.h"
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
        int n = (int)(N * this->innerLoopScale());
        n = SkMax32(1, n);

        for (int i = 0; i < n; i++) {

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
    virtual float innerLoopScale() const { return 1; }

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

/*
 *  Populates the SkPaint dictionary with a large number of unique paint
 *  objects that differ only by color
 */
class UniquePaintDictionaryRecordBench : public PictureRecordBench {
public:
    UniquePaintDictionaryRecordBench(void* param)
        : INHERITED(param, "unique_paint_dictionary") { }

    enum {
        M = SkBENCHLOOP(15000),   // number of unique paint objects
    };
protected:
    virtual float innerLoopScale() const SK_OVERRIDE { return 0.1f; }
    virtual void recordCanvas(SkCanvas* canvas) {
        SkRandom rand;
        for (int i = 0; i < M; i++) {
            SkPaint paint;
            paint.setColor(rand.nextU());
            canvas->drawPaint(paint);
        }
    }

private:
    typedef PictureRecordBench INHERITED;
};

/*
 *  Populates the SkPaint dictionary with a number of unique paint
 *  objects that get reused repeatedly
 */
class RecurringPaintDictionaryRecordBench : public PictureRecordBench {
public:
    RecurringPaintDictionaryRecordBench(void* param)
        : INHERITED(param, "recurring_paint_dictionary") { }

    enum {
        ObjCount = 100,           // number of unique paint objects
        M = SkBENCHLOOP(50000),   // number of draw iterations
    };
protected:
    virtual float innerLoopScale() const SK_OVERRIDE { return 0.1f; }
    virtual void recordCanvas(SkCanvas* canvas) {

        for (int i = 0; i < M; i++) {
            SkPaint paint;
            paint.setColor(i % ObjCount);
            canvas->drawPaint(paint);
        }
    }

private:
    typedef PictureRecordBench INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkBenchmark* Fact0(void* p) { return new DictionaryRecordBench(p); }
static SkBenchmark* Fact1(void* p) { return new UniquePaintDictionaryRecordBench(p); }
static SkBenchmark* Fact2(void* p) { return new RecurringPaintDictionaryRecordBench(p); }

static BenchRegistry gReg0(Fact0);
static BenchRegistry gReg1(Fact1);
static BenchRegistry gReg2(Fact2);
