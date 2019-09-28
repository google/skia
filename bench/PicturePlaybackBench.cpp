/*
 * Copyright 2011 Google Inc.
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
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/utils/SkRandom.h"

// This is designed to emulate about 4 screens of textual content

///////////////////////////////////////////////////////////////////////////////

// Chrome draws into small tiles with impl-side painting.
// This benchmark measures the relative performance of our bounding-box hierarchies,
// both when querying tiles perfectly and when not.
enum BBH  { kNone, kRTree };
enum Mode { kTiled, kRandom };
class TiledPlaybackBench : public Benchmark {
public:
    TiledPlaybackBench(BBH bbh, Mode mode) : fBBH(bbh), fMode(mode), fName("tiled_playback") {
        switch (fBBH) {
            case kNone:     fName.append("_none"    ); break;
            case kRTree:    fName.append("_rtree"   ); break;
        }
        switch (fMode) {
            case kTiled:  fName.append("_tiled" ); break;
            case kRandom: fName.append("_random"); break;
        }
    }

    const char* onGetName() override { return fName.c_str(); }
    SkIPoint onGetSize() override { return SkIPoint::Make(1024,1024); }

    void onDelayedSetup() override {
        std::unique_ptr<SkBBHFactory> factory;
        switch (fBBH) {
            case kNone:                                                 break;
            case kRTree:    factory.reset(new SkRTreeFactory);          break;
        }

        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(1024, 1024, factory.get());
            SkRandom rand;
            for (int i = 0; i < 10000; i++) {
                SkScalar x = rand.nextRangeScalar(0, 1024),
                         y = rand.nextRangeScalar(0, 1024),
                         w = rand.nextRangeScalar(0, 128),
                         h = rand.nextRangeScalar(0, 128);
                SkPaint paint;
                paint.setColor(rand.nextU());
                paint.setAlpha(0xFF);
                canvas->drawRect(SkRect::MakeXYWH(x,y,w,h), paint);
            }
        fPic = recorder.finishRecordingAsPicture();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; i++) {
            // This inner loop guarantees we make the same choices for all bench variants.
            SkRandom rand;
            for (int j = 0; j < 10; j++) {
                SkScalar x = 0, y = 0;
                switch (fMode) {
                    case kTiled:  x = SkScalar(256 * rand.nextULessThan(4));
                                  y = SkScalar(256 * rand.nextULessThan(4));
                                  break;
                    case kRandom: x = rand.nextRangeScalar(0, 768);
                                  y = rand.nextRangeScalar(0, 768);
                                  break;
                }
                SkAutoCanvasRestore ar(canvas, true/*save now*/);
                canvas->clipRect(SkRect::MakeXYWH(x,y,256,256));
                fPic->playback(canvas);
            }
        }
    }

private:
    BBH                 fBBH;
    Mode                fMode;
    SkString            fName;
    sk_sp<SkPicture>    fPic;
};

DEF_BENCH( return new TiledPlaybackBench(kNone,     kRandom); )
DEF_BENCH( return new TiledPlaybackBench(kNone,     kTiled ); )
DEF_BENCH( return new TiledPlaybackBench(kRTree,    kRandom); )
DEF_BENCH( return new TiledPlaybackBench(kRTree,    kTiled ); )
