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

#include "../include/record/SkRecording.h"

#include "Stats.h"
#include "Timer.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_string2(skps, r, "skps", "Directory containing SKPs to playback.");
DEFINE_int32(samples, 10, "Gather this many samples of each picture playback.");
DEFINE_bool(skr, false, "Play via SkRecord instead of SkPicture.");
DEFINE_int32(tile, 1000000000, "Simulated tile size.");
DEFINE_string(match, "", "The usual filters on file names of SKPs to bench.");
DEFINE_string(timescale, "ms", "Print times in ms, us, or ns");
DEFINE_int32(verbose, 0, "0: print min sample; "
                         "1: print min, mean, max and noise indication "
                         "2: print all samples");

static double timescale() {
    if (FLAGS_timescale.contains("us")) return 1000;
    if (FLAGS_timescale.contains("ns")) return 1000000;
    return 1;
}

static SkPicture* rerecord_with_tilegrid(SkPicture& src) {
    SkTileGridFactory::TileGridInfo info;
    info.fTileInterval.set(FLAGS_tile, FLAGS_tile);
    info.fMargin.setEmpty();
    info.fOffset.setZero();
    SkTileGridFactory factory(info);

    SkPictureRecorder recorder;
    src.draw(recorder.beginRecording(src.width(), src.height(), &factory));
    return recorder.endRecording();
}

static EXPERIMENTAL::SkPlayback* rerecord_with_skr(SkPicture& src) {
    EXPERIMENTAL::SkRecording recording(src.width(), src.height());
    src.draw(recording.canvas());
    return recording.releasePlayback();
}

static void draw(const EXPERIMENTAL::SkPlayback& skr, const SkPicture& skp, SkCanvas* canvas) {
    if (FLAGS_skr) {
        skr.draw(canvas);
    } else {
        skp.draw(canvas);
    }
}

static void bench(SkPMColor* scratch, SkPicture& src, const char* name) {
    SkAutoTUnref<SkPicture> picture(rerecord_with_tilegrid(src));
    SkAutoTDelete<EXPERIMENTAL::SkPlayback> record(rerecord_with_skr(src));

    SkAutoTDelete<SkCanvas> canvas(SkCanvas::NewRasterDirectN32(src.width(),
                                                                src.height(),
                                                                scratch,
                                                                src.width() * sizeof(SkPMColor)));
    canvas->clipRect(SkRect::MakeWH(SkIntToScalar(FLAGS_tile), SkIntToScalar(FLAGS_tile)));

    // Draw once to warm any caches.  The first sample otherwise can be very noisy.
    draw(*record, *picture, canvas.get());

    WallTimer timer;
    const double scale = timescale();
    SkAutoTMalloc<double> samples(FLAGS_samples);
    for (int i = 0; i < FLAGS_samples; i++) {
        // We assume timer overhead (typically, ~30ns) is insignificant
        // compared to draw runtime (at least ~100us, usually several ms).
        timer.start();
        draw(*record, *picture, canvas.get());
        timer.end();
        samples[i] = timer.fWall * scale;
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

    // We share a single scratch bitmap among benches to reduce the profile noise from allocation.
    static const int kMaxArea = 209825221;  // tabl_mozilla is this big.
    SkAutoTMalloc<SkPMColor> scratch(kMaxArea);

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
        SkAutoTUnref<SkPicture> src(SkPicture::CreateFromStream(stream));
        if (!src) {
            SkDebugf("Could not read %s as an SkPicture.\n", path.c_str());
            failed = true;
            continue;
        }

        if (src->width() * src->height() > kMaxArea) {
            SkDebugf("%s (%dx%d) is larger than hardcoded scratch bitmap (%dpx).\n",
                     path.c_str(), src->width(), src->height(), kMaxArea);
            failed = true;
            continue;
        }

        bench(scratch.get(), *src, filename.c_str());
    }
    return failed ? 1 : 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
