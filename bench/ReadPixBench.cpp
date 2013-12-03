
/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkCanvas.h"


/**
 * This bench mark tests the use case where the user writes the a canvas
 * and then reads small chunks from it repeatedly. This can cause trouble
 * for the GPU as readbacks are very expensive.
 */
class ReadPixBench : public SkBenchmark {
public:
    ReadPixBench() {}

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return "readpix";
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        canvas->clear(SK_ColorBLACK);

        SkISize size = canvas->getDeviceSize();

        int offX = (size.width() - kWindowSize) / kNumStepsX;
        int offY = (size.height() - kWindowSize) / kNumStepsY;

        SkPaint paint;

        paint.setColor(SK_ColorBLUE);

        canvas->drawCircle(SkIntToScalar(size.width()/2),
                           SkIntToScalar(size.height()/2),
                           SkIntToScalar(size.width()/2),
                           paint);

        SkBitmap bitmap;

        bitmap.setConfig(SkBitmap::kARGB_8888_Config, kWindowSize, kWindowSize);

        for (int i = 0; i < loops; i++) {
            for (int x = 0; x < kNumStepsX; ++x) {
                for (int y = 0; y < kNumStepsY; ++y) {
                    canvas->readPixels(&bitmap, x * offX, y * offY);
                }
            }
        }
    }

private:
    static const int kNumStepsX = 30;
    static const int kNumStepsY = 30;
    static const int kWindowSize = 5;

    typedef SkBenchmark INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new ReadPixBench(); )
