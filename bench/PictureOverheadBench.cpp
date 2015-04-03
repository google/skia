/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// A benchmark designed to isolate the constant overheads of picture recording.
// We record a very tiny (one op) picture; one op is better than none, as it forces
// us to allocate memory to store that op... we can't just cheat by holding onto NULLs.

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkPictureRecorder.h"

struct PictureOverheadBench : public Benchmark {
    const char* onGetName() override { return "picture_overhead"; }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    void onDraw(const int loops, SkCanvas*) override {
        SkPictureRecorder rec;
        for (int i = 0; i < loops; i++) {
            SkCanvas* c = rec.beginRecording(SkRect::MakeWH(2000,3000));
                c->drawRect(SkRect::MakeXYWH(10, 10, 1000, 1000), SkPaint());
            SkAutoTUnref<SkPicture> pic(rec.endRecordingAsPicture());
        }
    }
};
DEF_BENCH(return new PictureOverheadBench;)
