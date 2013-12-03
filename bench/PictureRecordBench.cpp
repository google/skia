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
    PictureRecordBench(const char name[])  {
        fName.printf("picture_record_%s", name);
    }

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

    enum {
        PICTURE_WIDTH = 1000,
        PICTURE_HEIGHT = 4000,
    };
protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }
private:
    SkString fName;
    typedef SkBenchmark INHERITED;
};


static const int kMaxLoopsPerCanvas = 10000;

/*
 *  An SkPicture has internal dictionaries to store bitmaps, matrices, paints,
 *  and regions.  This bench populates those dictionaries to test the speed of
 *  reading and writing to those particular dictionary data structures.
 */
class DictionaryRecordBench : public PictureRecordBench {
public:
    DictionaryRecordBench() : INHERITED("dictionaries") {}

protected:
    virtual void onDraw(const int loops, SkCanvas*) SK_OVERRIDE {
        SkAutoTDelete<SkPicture> picture;
        SkCanvas* canvas = NULL;

        const SkPoint translateDelta = getTranslateDelta(loops);

        for (int i = 0; i < loops; i++) {
            if (0 == i % kMaxLoopsPerCanvas) {
                picture.reset(SkNEW(SkPicture));
                canvas = picture->beginRecording(PICTURE_WIDTH, PICTURE_HEIGHT);
            }

            SkColor color = SK_ColorYELLOW + (i % 255);
            SkIRect rect = SkIRect::MakeWH(i % PICTURE_WIDTH, i % PICTURE_HEIGHT);

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

    SkPoint getTranslateDelta(int M) {
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
    UniquePaintDictionaryRecordBench() : INHERITED("unique_paint_dictionary") { }

protected:
    virtual void onDraw(const int loops, SkCanvas*) SK_OVERRIDE {
        SkRandom rand;
        SkPaint paint;
        SkAutoTDelete<SkPicture> picture;
        SkCanvas* canvas = NULL;
        for (int i = 0; i < loops; i++) {
            if (0 == i % kMaxLoopsPerCanvas) {
                picture.reset(SkNEW(SkPicture));
                canvas = picture->beginRecording(PICTURE_WIDTH, PICTURE_HEIGHT);
            }
            paint.setColor(rand.nextU());
            canvas->drawPaint(paint);
        }
    }

private:
    typedef PictureRecordBench INHERITED;
};

/*
 *  Populates the SkPaint dictionary with a number of unique paint
 *  objects that get reused repeatedly.
 *
 *  Re-creating the paint objects in the inner loop slows the benchmark down 10%.
 *  Using setColor(i % objCount) instead of a random color creates a very high rate
 *  of hash conflicts, slowing us down 12%.
 */
class RecurringPaintDictionaryRecordBench : public PictureRecordBench {
public:
    RecurringPaintDictionaryRecordBench() : INHERITED("recurring_paint_dictionary") {
        SkRandom rand;
        for (int i = 0; i < ObjCount; i++) {
            fPaint[i].setColor(rand.nextU());
        }
    }

    enum {
        ObjCount = 100,  // number of unique paint objects
    };
protected:
    virtual void onDraw(const int loops, SkCanvas*) SK_OVERRIDE {
        SkPicture picture;
        SkCanvas* canvas = picture.beginRecording(PICTURE_WIDTH, PICTURE_HEIGHT);
        for (int i = 0; i < loops; i++) {
            canvas->drawPaint(fPaint[i % ObjCount]);
        }
    }

private:
    SkPaint fPaint [ObjCount];
    typedef PictureRecordBench INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new DictionaryRecordBench(); )
DEF_BENCH( return new UniquePaintDictionaryRecordBench(); )
DEF_BENCH( return new RecurringPaintDictionaryRecordBench(); )
