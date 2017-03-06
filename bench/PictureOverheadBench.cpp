/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// A benchmark designed to isolate the constant overheads of picture recording.
// We record an empty picture and a picture with one draw op to force memory allocation.

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkLiteDL.h"
#include "SkLiteRecorder.h"
#include "SkPictureRecorder.h"

template <int kDraws, bool kLite>
struct PictureOverheadBench : public Benchmark {
    PictureOverheadBench() {
        fName.appendf("picture_overhead_%d%s", kDraws, kLite ? "_lite" : "");
    }
    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    void onDraw(int loops, SkCanvas*) override {
        SkLiteRecorder lite;
        SkPictureRecorder rec;

        SkIRect iBounds = {0,0, 2000,3000};
        SkRect bounds = SkRect::Make(iBounds);

        for (int i = 0; i < loops; i++) {
            SkLiteDL liteDL;
            SkCanvas* canvas;
            if (kLite) {
                lite.reset(&liteDL, iBounds);
                canvas = &lite;
            } else {
                rec.beginRecording(bounds);
                canvas = rec.getRecordingCanvas();
            }

            for (int i = 0; i < kDraws; i++) {
                canvas->drawRect({10,10, 1000, 1000}, SkPaint{});
            }

            if (!kLite) {
                (void)rec.finishRecordingAsPicture();
            }
        }
    }

    SkString fName;
};

DEF_BENCH(return (new PictureOverheadBench<0, false>);)
DEF_BENCH(return (new PictureOverheadBench<1, false>);)
DEF_BENCH(return (new PictureOverheadBench<2, false>);)
DEF_BENCH(return (new PictureOverheadBench<10,false>);)
DEF_BENCH(return (new PictureOverheadBench<0,  true>);)
DEF_BENCH(return (new PictureOverheadBench<1,  true>);)
DEF_BENCH(return (new PictureOverheadBench<2,  true>);)
DEF_BENCH(return (new PictureOverheadBench<10, true>);)

///////////////////////////////////////////////////////////////////////////////////////////////////

class ClipOverheadRecordingBench : public Benchmark {
    SkString fName;
    const bool fDoLite;

public:
    ClipOverheadRecordingBench(bool doLite) : fDoLite(doLite) {
        fName.printf("clip_overhead_recording_%s", doLite ? "lite" : "std");
    }

protected:
    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    void onDraw(int loops, SkCanvas*) override {
        SkLiteRecorder lite;
        SkPictureRecorder rec;

        SkIRect iBounds = {0,0, 2000,3000};
        SkRect bounds = SkRect::Make(iBounds);

        for (int i = 0; i < loops; i++) {
            SkLiteDL liteDL;
            SkCanvas* canvas;
            if (fDoLite) {
                lite.reset(&liteDL, iBounds);
                canvas = &lite;
            } else {
                rec.beginRecording(bounds);
                canvas = rec.getRecordingCanvas();
            }

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

            if (!fDoLite) {
                (void)rec.finishRecordingAsPicture();
            }
        }
    }
};
DEF_BENCH( return new ClipOverheadRecordingBench(true); )
DEF_BENCH( return new ClipOverheadRecordingBench(false); )
