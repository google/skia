/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Timer.h"
#include "Benchmark.h"
#include "LazyDecodeBitmap.h"
#include "PictureBenchmark.h"
#include "PictureRenderer.h"
#include "SkCommandLineFlags.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTArray.h"

typedef sk_tools::PictureRenderer::BBoxHierarchyType BBoxType;
static const int kBBoxTypeCount = sk_tools::PictureRenderer::kLast_BBoxHierarchyType + 1;


DEFINE_string2(skps, r, "", "The list of SKPs to benchmark.");
DEFINE_string(bb_types, "", "The set of bbox types to test. If empty, all are tested. "
                       "Should be one or more of none, quadtree, rtree, tilegrid.");
DEFINE_int32(record, 100, "Number of times to record each SKP.");
DEFINE_int32(playback, 1, "Number of times to playback each SKP.");
DEFINE_int32(tilesize, 256, "The size of a tile.");

struct Measurement {
    SkString fName;
    double fRecordAverage[kBBoxTypeCount];
    double fPlaybackAverage[kBBoxTypeCount];
};

const char* kBBoxHierarchyTypeNames[kBBoxTypeCount] = {
    "none", // kNone_BBoxHierarchyType
    "quadtree", // kQuadTree_BBoxHierarchyType
    "rtree", // kRTree_BBoxHierarchyType
    "tilegrid", // kTileGrid_BBoxHierarchyType
};

static SkPicture* pic_from_path(const char path[]) {
    SkFILEStream stream(path);
    if (!stream.isValid()) {
        SkDebugf("-- Can't open '%s'\n", path);
        return NULL;
    }
    return SkPicture::CreateFromStream(&stream, &sk_tools::LazyDecodeBitmap);
}

/**
 * This function is the sink to which all work ends up going.
 * @param renderer The renderer to use to perform the work.
 *                 To measure rendering, use a TiledPictureRenderer.
 *                 To measure recording, use a RecordPictureRenderer.
 * @param bBoxType The bounding box hierarchy type to use.
 * @param pic The picture to draw to the renderer.
 * @param numRepeats The number of times to repeat the draw.
 * @param timer The timer used to benchmark the work.
 */
static void do_benchmark_work(sk_tools::PictureRenderer* renderer,
        BBoxType bBoxType,
        SkPicture* pic,
        const int numRepeats,
        Timer* timer) {
    renderer->setBBoxHierarchyType(bBoxType);
    renderer->setGridSize(FLAGS_tilesize, FLAGS_tilesize);
    renderer->init(pic, NULL, NULL, NULL, false);

    SkDebugf("%s %d times...\n", renderer->getConfigName().c_str(), numRepeats);
    for (int i = 0; i < numRepeats; ++i) {
        renderer->setup();
        // Render once to fill caches
        renderer->render();
        // Render again to measure
        timer->start();
        renderer->render();
        timer->end();
    }
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    SkAutoGraphics ag;
    bool includeBBoxType[kBBoxTypeCount];
    for (int bBoxType = 0; bBoxType < kBBoxTypeCount; ++bBoxType) {
        includeBBoxType[bBoxType] = (FLAGS_bb_types.count() == 0) ||
            FLAGS_bb_types.contains(kBBoxHierarchyTypeNames[bBoxType]);
    }
    // go through all the pictures
    SkTArray<Measurement> measurements;
    for (int index = 0; index < FLAGS_skps.count(); ++index) {
        const char* path = FLAGS_skps[index];
        SkPicture* picture = pic_from_path(path);
        if (NULL == picture) {
            SkDebugf("Couldn't create picture. Ignoring path: %s\n", path);
            continue;
        }
        SkDebugf("Benchmarking path: %s\n", path);
        Measurement& measurement = measurements.push_back();
        measurement.fName = path;
        for (int bBoxType = 0; bBoxType < kBBoxTypeCount; ++bBoxType) {
            if (!includeBBoxType[bBoxType]) { continue; }
            if (FLAGS_playback > 0) {
                sk_tools::TiledPictureRenderer playbackRenderer;
                Timer playbackTimer;
                do_benchmark_work(&playbackRenderer, (BBoxType)bBoxType,
                                  picture, FLAGS_playback, &playbackTimer);
                measurement.fPlaybackAverage[bBoxType] = playbackTimer.fCpu;
            }
            if (FLAGS_record > 0) {
                sk_tools::RecordPictureRenderer recordRenderer;
                Timer recordTimer;
                do_benchmark_work(&recordRenderer, (BBoxType)bBoxType,
                                  picture, FLAGS_record, &recordTimer);
                measurement.fRecordAverage[bBoxType] = recordTimer.fCpu;
            }
        }
    }

    Measurement globalMeasurement;
    for (int bBoxType = 0; bBoxType < kBBoxTypeCount; ++bBoxType) {
        if (!includeBBoxType[bBoxType]) { continue; }
        globalMeasurement.fPlaybackAverage[bBoxType] = 0;
        globalMeasurement.fRecordAverage[bBoxType] = 0;
        for (int index = 0; index < measurements.count(); ++index) {
            const Measurement& measurement = measurements[index];
            globalMeasurement.fPlaybackAverage[bBoxType] +=
                measurement.fPlaybackAverage[bBoxType];
            globalMeasurement.fRecordAverage[bBoxType] +=
                measurement.fRecordAverage[bBoxType];
        }
        globalMeasurement.fPlaybackAverage[bBoxType] /= measurements.count();
        globalMeasurement.fRecordAverage[bBoxType] /= measurements.count();
    }

    // Output gnuplot readable histogram data..
    const char* pbTitle = "bbh_shootout_playback.dat";
    const char* recTitle = "bbh_shootout_record.dat";
    SkFILEWStream playbackOut(pbTitle);
    SkFILEWStream recordOut(recTitle);
    recordOut.writeText("# ");
    playbackOut.writeText("# ");
    SkDebugf("---\n");
    for (int bBoxType = 0; bBoxType < kBBoxTypeCount; ++bBoxType) {
        if (!includeBBoxType[bBoxType]) { continue; }
        SkString out;
        out.printf("%s ", kBBoxHierarchyTypeNames[bBoxType]);
        recordOut.writeText(out.c_str());
        playbackOut.writeText(out.c_str());

        if (FLAGS_record > 0) {
            SkDebugf("Average %s recording time: %.3fms\n",
                kBBoxHierarchyTypeNames[bBoxType],
                globalMeasurement.fRecordAverage[bBoxType]);
        }
        if (FLAGS_playback > 0) {
            SkDebugf("Average %s playback time: %.3fms\n",
                kBBoxHierarchyTypeNames[bBoxType],
                globalMeasurement.fPlaybackAverage[bBoxType]);
        }
    }
    recordOut.writeText("\n");
    playbackOut.writeText("\n");
    // Write to file, and save recording averages.
    for (int index = 0; index < measurements.count(); ++index) {
        const Measurement& measurement = measurements[index];
        SkString pbLine;
        SkString recLine;

        pbLine.printf("%d", index);
        recLine.printf("%d", index);
        for (int bBoxType = 0; bBoxType < kBBoxTypeCount; ++bBoxType) {
            if (!includeBBoxType[bBoxType]) { continue; }
            pbLine.appendf(" %f", measurement.fPlaybackAverage[bBoxType]);
            recLine.appendf(" %f", measurement.fRecordAverage[bBoxType]);
        }
        pbLine.appendf("\n");
        recLine.appendf("\n");
        playbackOut.writeText(pbLine.c_str());
        recordOut.writeText(recLine.c_str());
    }
    SkDebugf("\nWrote data to gnuplot-readable files: %s %s\n", pbTitle, recTitle);
    return 0;
}

#if !defined(SK_BUILD_FOR_IOS) && !defined(SK_BUILD_FOR_NACL)
int main(int argc, char** argv) {
    return tool_main(argc, argv);
}
#endif
