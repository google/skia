/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRRect.h"

class ClipOverheadRecordingBench : public Benchmark {
public:
    ClipOverheadRecordingBench() {}

private:
    const char* onGetName() override { return "clip_overhead_recording"; }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    void onDraw(int loops, SkCanvas*) override {
        SkPictureRecorder rec;

        for (int loop = 0; loop < loops; loop++) {
            SkCanvas* canvas = rec.beginRecording({0,0, 2000,3000});

            SkPaint paint;
            SkRRect rrect;
            rrect.setOval({0, 0, 1000, 1000});
            for (int i = 0; i < 1000; i++) {
                canvas->save();
                    canvas->translate(10, 10);
                    canvas->clipRect({10,10, 1000, 1000});
                    canvas->drawRRect(rrect, paint);
                canvas->restore();
            }

            (void)rec.finishRecordingAsPicture();
        }
    }
};
DEF_BENCH( return new ClipOverheadRecordingBench; )
