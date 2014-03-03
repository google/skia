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
#include "SkQuadTreePicture.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTileGridPicture.h"
#include "SkTime.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

// Just reading all the SKPs takes about 2 seconds for me, which is the same as about 100 loops of
// rerecording all the SKPs.  So we default to --loops=900, which makes ~90% of our time spent in
// recording, and this should take ~20 seconds to run.

DEFINE_string2(skps, r, "skps", "Directory containing SKPs to read and re-record.");
DEFINE_int32(loops, 900, "Number of times to re-record each SKP.");
DEFINE_int32(flags, SkPicture::kUsePathBoundsForClip_RecordingFlag, "RecordingFlags to use.");
DEFINE_bool(endRecording, true, "If false, don't time SkPicture::endRecording()");
DEFINE_int32(nullSize, 1000, "Pretend dimension of null source picture.");
DEFINE_int32(tileGridSize, 512, "Set the tile grid size. Has no effect if bbh is not set to tilegrid.");
DEFINE_string(bbh, "", "Turn on the bbh and select the type, one of rtree, tilegrid, quadtree");

typedef SkPicture* (*PictureFactory)(const int width, const int height, int* recordingFlags);

static SkPicture* vanilla_factory(const int width, const int height, int* recordingFlags) {
    return SkNEW(SkPicture);
}

static SkPicture* rtree_factory(const int width, const int height, int* recordingFlags) {
    *recordingFlags |= SkPicture::kOptimizeForClippedPlayback_RecordingFlag;
    return SkNEW(SkPicture);
}

static SkPicture* tilegrid_factory(const int width, const int height, int* recordingFlags) {
    *recordingFlags |= SkPicture::kOptimizeForClippedPlayback_RecordingFlag;
    SkTileGridPicture::TileGridInfo info;
    info.fTileInterval.set(FLAGS_tileGridSize, FLAGS_tileGridSize);
    info.fMargin.setEmpty();
    info.fOffset.setZero();
    return SkNEW_ARGS(SkTileGridPicture, (width, height, info));
}

static SkPicture* quadtree_factory(const int width, const int height, int* recordingFlags) {
    *recordingFlags |= SkPicture::kOptimizeForClippedPlayback_RecordingFlag;
    return SkNEW_ARGS(SkQuadTreePicture, (SkIRect::MakeWH(width, height)));
}

static PictureFactory parse_FLAGS_bbh() {
    if (FLAGS_bbh.isEmpty()) { return &vanilla_factory; }
    if (FLAGS_bbh.count() != 1) {
        SkDebugf("Multiple bbh arguments supplied.\n");
        return NULL;
    }
    if (FLAGS_bbh.contains("rtree")) { return rtree_factory; }
    if (FLAGS_bbh.contains("tilegrid")) { return tilegrid_factory; }
    if (FLAGS_bbh.contains("quadtree")) { return quadtree_factory; }
    SkDebugf("Invalid bbh type %s, must be one of rtree, tilegrid, quadtree.\n", FLAGS_bbh[0]);
    return NULL;
}

static void bench_record(SkPicture* src, const char* name, PictureFactory pictureFactory) {
    const SkMSec start = SkTime::GetMSecs();
    const int width  = src ? src->width()  : FLAGS_nullSize;
    const int height = src ? src->height() : FLAGS_nullSize;

    for (int i = 0; i < FLAGS_loops; i++) {
        int recordingFlags = FLAGS_flags;
        SkAutoTUnref<SkPicture> dst(pictureFactory(width, height, &recordingFlags));
        SkCanvas* canvas = dst->beginRecording(width, height, recordingFlags);
        if (src) src->draw(canvas);
        if (FLAGS_endRecording) dst->endRecording();
    }

    const SkMSec elapsed = SkTime::GetMSecs() - start;
    const double msPerLoop = elapsed / (double)FLAGS_loops;
    printf("%.2g\t%s\n", msPerLoop, name);
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    SkAutoGraphics autoGraphics;

    PictureFactory pictureFactory = parse_FLAGS_bbh();
    if (pictureFactory == NULL) {
        return 1;
    }
    bench_record(NULL, "NULL", pictureFactory);
    if (FLAGS_skps.isEmpty()) return 0;

    SkOSFile::Iter it(FLAGS_skps[0], ".skp");
    SkString filename;
    bool failed = false;
    while (it.next(&filename)) {
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
        bench_record(src, filename.c_str(), pictureFactory);
    }
    return failed ? 1 : 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
