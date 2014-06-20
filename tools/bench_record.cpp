/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCommandLineFlags.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkStream.h"
#include "SkString.h"

#include "LazyDecodeBitmap.h"
#include "Stats.h"
#include "Timer.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_string2(skps, r, "skps", "Directory containing SKPs to read and re-record.");
DEFINE_int32(samples, 10, "Number of times to re-record each SKP.");
DEFINE_int32(tileGridSize, 512, "Set the tile grid size. Has no effect if bbh is not set to tilegrid.");
DEFINE_string(bbh, "", "Turn on the bbh and select the type, one of rtree, tilegrid, quadtree");
DEFINE_bool(skr, false, "Record SKR instead of SKP.");
DEFINE_string(match, "", "The usual filters on file names of SKPs to bench.");
DEFINE_string(timescale, "us", "Print times in ms, us, or ns");
DEFINE_double(overheadGoal, 0.0001,
              "Try to make timer overhead at most this fraction of our sample measurements.");
DEFINE_int32(verbose, 0, "0: print min sample; "
                         "1: print min, mean, max and noise indication "
                         "2: print all samples");

static double timescale() {
    if (FLAGS_timescale.contains("us")) return 1000;
    if (FLAGS_timescale.contains("ns")) return 1000000;
    return 1;
}

static SkBBHFactory* parse_FLAGS_bbh() {
    if (FLAGS_bbh.isEmpty()) {
        return NULL;
    }

    if (FLAGS_bbh.contains("rtree")) {
        return SkNEW(SkRTreeFactory);
    }
    if (FLAGS_bbh.contains("tilegrid")) {
        SkTileGridFactory::TileGridInfo info;
        info.fTileInterval.set(FLAGS_tileGridSize, FLAGS_tileGridSize);
        info.fMargin.setEmpty();
        info.fOffset.setZero();
        return SkNEW_ARGS(SkTileGridFactory, (info));
    }
    if (FLAGS_bbh.contains("quadtree")) {
        return SkNEW(SkQuadTreeFactory);
    }
    SkDebugf("Invalid bbh type %s, must be one of rtree, tilegrid, quadtree.\n", FLAGS_bbh[0]);
    return NULL;
}

static void rerecord(const SkPicture& src, SkBBHFactory* bbhFactory) {
    SkPictureRecorder recorder;
    if (FLAGS_skr) {
        src.draw(recorder.EXPERIMENTAL_beginRecording(src.width(), src.height(), bbhFactory));
    } else {
        src.draw(recorder.beginRecording(src.width(), src.height(), bbhFactory));
    }
    SkAutoTUnref<SkPicture> pic(recorder.endRecording());
}

static void bench_record(const SkPicture& src,
                         const double timerOverhead,
                         const char* name,
                         SkBBHFactory* bbhFactory) {
    // Rerecord once to warm up any caches.  Otherwise the first sample can be very noisy.
    rerecord(src, bbhFactory);

    // Rerecord once to see how many times we should loop to make timer overhead insignificant.
    WallTimer timer;
    const double scale = timescale();
    do {
        timer.start();
        rerecord(src, bbhFactory);
        timer.end();
    } while (timer.fWall * scale < timerOverhead);  // Loop just in case something bizarre happens.

    // We want (timer overhead / measurement) to be less than FLAGS_overheadGoal.
    // So in each sample, we'll loop enough times to have made that true for our first measurement.
    const int loops = (int)ceil(timerOverhead / timer.fWall / FLAGS_overheadGoal);

    SkAutoTMalloc<double> samples(FLAGS_samples);
    for (int i = 0; i < FLAGS_samples; i++) {
        timer.start();
        for (int j = 0; j < loops; j++) {
            rerecord(src, bbhFactory);
        }
        timer.end();
        samples[i] = timer.fWall * scale / loops;
    }

    Stats stats(samples.get(), FLAGS_samples);
    if (FLAGS_verbose == 0) {
        printf("%g\t%s\n", stats.min, name);
    } else if (FLAGS_verbose == 1) {
        // Get a rough idea of how noisy the measurements were.
        const double noisePercent = 100 * sqrt(stats.var) / stats.mean;
        printf("%g\t%g\t%g\t±%.0f%%\t%s\n", stats.min, stats.mean, stats.max, noisePercent, name);
    } else if (FLAGS_verbose == 2) {
        printf("%s", name);
        for (int i = 0; i < FLAGS_samples; i++) {
            printf("\t%g", samples[i]);
        }
        printf("\n");
    }
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    SkAutoGraphics autoGraphics;

    if (FLAGS_bbh.count() > 1) {
        SkDebugf("Multiple bbh arguments supplied.\n");
        return 1;
    }

    SkAutoTDelete<SkBBHFactory> bbhFactory(parse_FLAGS_bbh());

    // Each run will use this timer overhead estimate to guess how many times it should run.
    static const int kOverheadLoops = 10000000;
    WallTimer timer;
    double overheadEstimate = 0.0;
    const double scale = timescale();
    for (int i = 0; i < kOverheadLoops; i++) {
        timer.start();
        timer.end();
        overheadEstimate += timer.fWall * scale;
    }
    overheadEstimate /= kOverheadLoops;

    SkOSFile::Iter it(FLAGS_skps[0], ".skp");
    SkString filename;
    bool failed = false;
    while (it.next(&filename)) {
        if (SkCommandLineFlags::ShouldSkip(FLAGS_match, filename.c_str())) {
            continue;
        }

        const SkString path = SkOSPath::SkPathJoin(FLAGS_skps[0], filename.c_str());

        SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(path.c_str()));
        if (!stream) {
            SkDebugf("Could not read %s.\n", path.c_str());
            failed = true;
            continue;
        }
        SkAutoTUnref<SkPicture> src(
            SkPicture::CreateFromStream(stream, sk_tools::LazyDecodeBitmap));
        if (!src) {
            SkDebugf("Could not read %s as an SkPicture.\n", path.c_str());
            failed = true;
            continue;
        }
        bench_record(*src, overheadEstimate, filename.c_str(), bbhFactory.get());
    }
    return failed ? 1 : 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
