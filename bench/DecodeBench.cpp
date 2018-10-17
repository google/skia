/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "Resources.h"
#include "SkBitmap.h"
#include "SkCodec.h"

class DecodeBench : public Benchmark {
public:
    DecodeBench(const char* format, const char* filename, bool allFrames = false)
        : fFilename(filename),
          fName(SkStringPrintf("Decode_%s_%s_%s",
                               format,
                               filename,
                               allFrames ? "allFrames" : "firstFrame")),
          fAllFrames(allFrames) {}

    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    const char* onGetName() override { return fName.c_str(); }

    void onDelayedSetup() override {
        SkString qualifiedFilename(SkStringPrintf("images/%s", fFilename));
        fData = GetResourceAsData(qualifiedFilename.c_str());
        auto codec = SkCodec::MakeFromData(fData);
        fBitmap.allocPixels(codec->getInfo());
    }

    void onDraw(int loops, SkCanvas*) override {
        while (loops-- > 0) {
            auto             codec = SkCodec::MakeFromData(fData);
            SkCodec::Options opts;
            int              n = fAllFrames ? codec->getFrameCount() : 1;
            for (int i = 0; i < n; i++) {
                opts.fFrameIndex = i;
                // TODO: is SkASSERT / SkAssertResult the way to go? Benchmarks are
                // usually run with full optimizations, which means that all assertions
                // are no-ops (other than side effects), right??
                SkAssertResult(codec->getPixels(fBitmap.pixmap(), &opts));
            }
        }
    }

private:
    const char* fFilename;
    SkString    fName;
    bool        fAllFrames;

    sk_sp<SkData> fData;
    SkBitmap      fBitmap;
};

DEF_BENCH(return new DecodeBench("BMP", "rle.bmp"));

DEF_BENCH(return new DecodeBench("GIF", "color_wheel.gif", false));
DEF_BENCH(return new DecodeBench("GIF", "color_wheel.gif", true));
DEF_BENCH(return new DecodeBench("GIF", "flightAnim.gif", false));
DEF_BENCH(return new DecodeBench("GIF", "flightAnim.gif", true));
DEF_BENCH(return new DecodeBench("GIF", "test640x479.gif", false));
DEF_BENCH(return new DecodeBench("GIF", "test640x479.gif", true));

DEF_BENCH(return new DecodeBench("JPEG", "color_wheel.jpg"));
DEF_BENCH(return new DecodeBench("JPEG", "mandrill_512_q075.jpg"));

DEF_BENCH(return new DecodeBench("ICO", "color_wheel.ico"));

DEF_BENCH(return new DecodeBench("PNG", "color_wheel.png"));
DEF_BENCH(return new DecodeBench("PNG", "mandrill_512.png"));
DEF_BENCH(return new DecodeBench("PNG", "yellow_rose.png"));

DEF_BENCH(return new DecodeBench("WBMP", "mandrill.wbmp"));

DEF_BENCH(return new DecodeBench("WEBP", "color_wheel.webp"));
DEF_BENCH(return new DecodeBench("WEBP", "yellow_rose.webp"));
