/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkNullCanvas.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkString.h"

class PictureNesting : public Benchmark {
public:
    PictureNesting(const char* name, int maxLevel, int maxPictureLevel)
        : fMaxLevel(maxLevel)
        , fMaxPictureLevel(maxPictureLevel) {

        fPaint.setColor(SK_ColorRED);
        fPaint.setAntiAlias(true);
        fPaint.setStyle(SkPaint::kStroke_Style);
        SkAutoTUnref<SkCanvas> nullCanvas(SkCreateNullCanvas());
        fName.printf("picture_nesting_%s_%d", name, this->sierpinsky(nullCanvas, 0, fPaint));
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    void doDraw(SkCanvas* canvas) {
        SkIPoint canvasSize = onGetSize();
        canvas->save();
        canvas->scale(SkIntToScalar(canvasSize.x()), SkIntToScalar(canvasSize.y()));

        this->sierpinsky(canvas, 0, fPaint);

        canvas->restore();
    }

    int sierpinsky(SkCanvas* canvas, int lvl, const SkPaint& paint) {
        if (++lvl > fMaxLevel) {
            return 0;
        }

        int pics = 0;
        bool recordPicture = lvl <= fMaxPictureLevel;
        SkPictureRecorder recorder;
        SkCanvas* c = canvas;

        if (recordPicture) {
            c = recorder.beginRecording(1, 1);
            pics++;
        }

        c->drawLine(0.5, 0, 0, 1, paint);
        c->drawLine(0.5, 0, 1, 1, paint);
        c->drawLine(0,   1, 1, 1, paint);

        c->save();
            c->scale(0.5, 0.5);

            c->translate(0, 1);
            pics += this->sierpinsky(c, lvl, paint);

            c->translate(1, 0);
            pics += this->sierpinsky(c, lvl, paint);

            c->translate(-0.5, -1);
            pics += this->sierpinsky(c, lvl, paint);
        c->restore();

        if (recordPicture) {
            SkAutoTUnref<SkPicture> picture(recorder.endRecording());
            canvas->drawPicture(picture);
        }

        return pics;
    }

    int fMaxLevel;
    int fMaxPictureLevel;

private:
    SkString fName;
    SkPaint  fPaint;

    typedef Benchmark INHERITED;
};

class PictureNestingRecording : public PictureNesting {
public:
    PictureNestingRecording(int maxLevel, int maxPictureLevel)
        : INHERITED("recording", maxLevel, maxPictureLevel) {
    }

protected:
    virtual bool isSuitableFor(Backend backend) {
        return backend == kNonRendering_Backend;
    }

    virtual void onDraw(const int loops, SkCanvas*) {
        SkIPoint canvasSize = onGetSize();
        SkPictureRecorder recorder;

        for (int i = 0; i < loops; i++) {
            SkCanvas* c = recorder.beginRecording(SkIntToScalar(canvasSize.x()),
                                                  SkIntToScalar(canvasSize.y()));
            this->doDraw(c);
            SkAutoTUnref<SkPicture> picture(recorder.endRecording());
        }
    }

private:
    typedef PictureNesting INHERITED;
};

class PictureNestingPlayback : public PictureNesting {
public:
    PictureNestingPlayback(int maxLevel, int maxPictureLevel)
        : INHERITED("playback", maxLevel, maxPictureLevel) {

        SkIPoint canvasSize = onGetSize();
        SkPictureRecorder recorder;
        SkCanvas* c = recorder.beginRecording(SkIntToScalar(canvasSize.x()),
                                              SkIntToScalar(canvasSize.y()));

        this->doDraw(c);
        fPicture.reset(recorder.endRecording());
    }

protected:
    virtual void onDraw(const int loops, SkCanvas* canvas) {
        for (int i = 0; i < loops; i++) {
            canvas->drawPicture(fPicture);
        }
    }

private:
    SkAutoTUnref<SkPicture> fPicture;

    typedef PictureNesting INHERITED;
};

DEF_BENCH( return new PictureNestingRecording(8, 0); )
DEF_BENCH( return new PictureNestingRecording(8, 1); )
DEF_BENCH( return new PictureNestingRecording(8, 2); )
DEF_BENCH( return new PictureNestingRecording(8, 3); )
DEF_BENCH( return new PictureNestingRecording(8, 4); )
DEF_BENCH( return new PictureNestingRecording(8, 5); )
DEF_BENCH( return new PictureNestingRecording(8, 6); )
DEF_BENCH( return new PictureNestingRecording(8, 7); )
DEF_BENCH( return new PictureNestingRecording(8, 8); )

DEF_BENCH( return new PictureNestingPlayback(8, 0); )
DEF_BENCH( return new PictureNestingPlayback(8, 1); )
DEF_BENCH( return new PictureNestingPlayback(8, 2); )
DEF_BENCH( return new PictureNestingPlayback(8, 3); )
DEF_BENCH( return new PictureNestingPlayback(8, 4); )
DEF_BENCH( return new PictureNestingPlayback(8, 5); )
DEF_BENCH( return new PictureNestingPlayback(8, 6); )
DEF_BENCH( return new PictureNestingPlayback(8, 7); )
DEF_BENCH( return new PictureNestingPlayback(8, 8); )
