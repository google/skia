/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkBitmap.h"
#include "SkCanvas.h"


/**
 * This bench mark tests the use case where the user writes the a canvas
 * and then reads small chunks from it repeatedly. This can cause trouble
 * for the GPU as readbacks are very expensive.
 */
class ReadPixBench : public Benchmark {
public:
    ReadPixBench() {}

protected:
    const char* onGetName() override {
        return "readpix";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        SkISize size = canvas->getBaseLayerSize();

        int offX = (size.width() - kWindowSize) / kNumStepsX;
        int offY = (size.height() - kWindowSize) / kNumStepsY;

        SkPaint paint;

        paint.setColor(SK_ColorBLUE);

        canvas->drawCircle(SkIntToScalar(size.width()/2),
                           SkIntToScalar(size.height()/2),
                           SkIntToScalar(size.width()/2),
                           paint);

        SkBitmap bitmap;

        bitmap.allocPixels(SkImageInfo::MakeN32Premul(kWindowSize, kWindowSize));

        for (int i = 0; i < loops; i++) {
            for (int x = 0; x < kNumStepsX; ++x) {
                for (int y = 0; y < kNumStepsY; ++y) {
                    canvas->readPixels(bitmap.info(), bitmap.getPixels(), bitmap.rowBytes(),
                                       x * offX, y * offY);
                }
            }
        }
    }

private:
    static const int kNumStepsX = 30;
    static const int kNumStepsY = 30;
    static const int kWindowSize = 5;

    typedef Benchmark INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new ReadPixBench(); )
