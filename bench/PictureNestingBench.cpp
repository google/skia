/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkString.h"
#include "include/utils/SkNullCanvas.h"

class PictureNesting : public Benchmark {
public:
    PictureNesting(const char* name, int maxLevel, int maxPictureLevel)
        : fMaxLevel(maxLevel)
        , fMaxPictureLevel(maxPictureLevel) {
        fName.printf("picture_nesting_%s_%d", name, this->countPics());
        fPaint.setColor(SK_ColorRED);
        fPaint.setAntiAlias(true);
        fPaint.setStyle(SkPaint::kStroke_Style);
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void doDraw(SkCanvas* canvas) {
        SkISize canvasSize = onGetSize();
        canvas->save();
        canvas->scale(SkIntToScalar(canvasSize.width()), SkIntToScalar(canvasSize.height()));

        SkDEBUGCODE(int pics = ) this->sierpinsky(canvas, 0, fPaint);
        SkASSERT(pics == this->countPics());

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
            canvas->drawPicture(recorder.finishRecordingAsPicture());
        }

        return pics;
    }

    int fMaxLevel;
    int fMaxPictureLevel;

private:
    int countPics() const {
        // Solve: pics from sierpinsky
        // f(m) = 1 + 3*f(m - 1)
        // f(0) = 0
        //   via "recursive function to closed form" tricks
        // f(m) = 1/2 (3^m - 1)
        int pics = 1;
        for (int i = 0; i < fMaxPictureLevel; i++) {
            pics *= 3;
        }
        pics--;
        pics /= 2;
        return pics;
    }

    SkString fName;
    SkPaint  fPaint;

    using INHERITED = Benchmark;
};

class PictureNestingRecording : public PictureNesting {
public:
    PictureNestingRecording(int maxLevel, int maxPictureLevel)
        : INHERITED("recording", maxLevel, maxPictureLevel) {
    }

protected:
    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

    void onDraw(int loops, SkCanvas*) override {
        SkISize canvasSize = onGetSize();
        SkPictureRecorder recorder;

        for (int i = 0; i < loops; i++) {
            SkCanvas* c = recorder.beginRecording(SkIntToScalar(canvasSize.width()),
                                                  SkIntToScalar(canvasSize.height()));
            this->doDraw(c);
            (void)recorder.finishRecordingAsPicture();
        }
    }

private:
    using INHERITED = PictureNesting;
};

class PictureNestingPlayback : public PictureNesting {
public:
    PictureNestingPlayback(int maxLevel, int maxPictureLevel)
        : INHERITED("playback", maxLevel, maxPictureLevel) {
    }
protected:
    void onDelayedSetup() override {
        this->INHERITED::onDelayedSetup();

        SkISize canvasSize = onGetSize();
        SkPictureRecorder recorder;
        SkCanvas* c = recorder.beginRecording(SkIntToScalar(canvasSize.width()),
                                              SkIntToScalar(canvasSize.height()));

        this->doDraw(c);
        fPicture = recorder.finishRecordingAsPicture();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; i++) {
            canvas->drawPicture(fPicture);
        }
    }

private:
    sk_sp<SkPicture> fPicture;

    using INHERITED = PictureNesting;
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
