/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "BenchTimer.h"
#include "LazyDecodeBitmap.h"
#include "PictureBenchmark.h"
#include "PictureRenderer.h"
#include "SkBenchmark.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTArray.h"
#include "TimerData.h"

static const int kNumNormalRecordings = 10;
static const int kNumRTreeRecordings = 10;
static const int kNumPlaybacks = 1;
static const size_t kNumBaseBenchmarks = 3;
static const size_t kNumTileSizes = 3;
static const size_t kNumBbhPlaybackBenchmarks = 3;
static const size_t kNumBenchmarks = kNumBaseBenchmarks + kNumBbhPlaybackBenchmarks;

enum BenchmarkType {
    kNormal_BenchmarkType = 0,
    kRTree_BenchmarkType,
};

struct Histogram {
    Histogram() {
        // Make fCpuTime negative so that we don't mess with stats:
        fCpuTime = SkIntToScalar(-1);
    }
    SkScalar fCpuTime;
    SkString fPath;
};


////////////////////////////////////////////////////////////////////////////////
// Defined below.
struct BenchmarkControl;

typedef void (*BenchmarkFunction)
    (const BenchmarkControl&, const SkString&, SkPicture*, BenchTimer*);

static void benchmark_playback(
    const BenchmarkControl&, const SkString&, SkPicture*, BenchTimer*);
static void benchmark_recording(
    const BenchmarkControl&, const SkString&, SkPicture*, BenchTimer*);
////////////////////////////////////////////////////////////////////////////////

/**
 * Acts as a POD containing information needed to run a benchmark.
 * Provides static methods to poll benchmark info from an index.
 */
struct BenchmarkControl {
    SkISize fTileSize;
    BenchmarkType fType;
    BenchmarkFunction fFunction;
    SkString fName;

    /**
     * Will construct a BenchmarkControl instance from an index between 0 an kNumBenchmarks.
     */
    static BenchmarkControl Make(size_t i) {
        SkASSERT(kNumBenchmarks > i);
        BenchmarkControl benchControl;
        benchControl.fTileSize = GetTileSize(i);
        benchControl.fType = GetBenchmarkType(i);
        benchControl.fFunction = GetBenchmarkFunc(i);
        benchControl.fName = GetBenchmarkName(i);
        return benchControl;
    }

    enum BaseBenchmarks {
        kNormalRecord = 0,
        kRTreeRecord,
        kNormalPlayback,
    };

    static SkISize fTileSizes[kNumTileSizes];

    static SkISize GetTileSize(size_t i) {
        // Two of the base benchmarks don't need a tile size. But to maintain simplicity
        // down the pipeline we have to let a couple of values unused.
        if (i < kNumBaseBenchmarks) {
            return SkISize::Make(256, 256);
        }
        if (i >= kNumBaseBenchmarks && i < kNumBenchmarks) {
            return fTileSizes[i - kNumBaseBenchmarks];
        }
        SkASSERT(0);
        return SkISize::Make(0, 0);
    }

    static BenchmarkType GetBenchmarkType(size_t i) {
        if (i < kNumBaseBenchmarks) {
            switch (i) {
            case kNormalRecord:
                return kNormal_BenchmarkType;
            case kNormalPlayback:
                return kNormal_BenchmarkType;
            case kRTreeRecord:
                return kRTree_BenchmarkType;
            }
        }
        if (i < kNumBenchmarks) {
            return kRTree_BenchmarkType;
        }
        SkASSERT(0);
        return kRTree_BenchmarkType;
    }

    static BenchmarkFunction GetBenchmarkFunc(size_t i) {
        // Base functions.
        switch (i) {
            case kNormalRecord:
                return benchmark_recording;
            case kNormalPlayback:
                return benchmark_playback;
            case kRTreeRecord:
                return benchmark_recording;
        }
        // RTree playbacks
        if (i < kNumBenchmarks) {
            return benchmark_playback;
        }
        SkASSERT(0);
        return NULL;
    }

    static SkString GetBenchmarkName(size_t i) {
        // Base benchmark names
        switch (i) {
            case kNormalRecord:
                return SkString("normal_recording");
            case kNormalPlayback:
                return SkString("normal_playback");
            case kRTreeRecord:
                return SkString("rtree_recording");
        }
        // RTree benchmark names.
        if (i < kNumBenchmarks) {
            SkASSERT(i >= kNumBaseBenchmarks);
            SkString name;
            name.printf("rtree_playback_%dx%d",
                    fTileSizes[i - kNumBaseBenchmarks].fWidth,
                    fTileSizes[i - kNumBaseBenchmarks].fHeight);
            return name;

        } else {
            SkASSERT(0);
        }
        return SkString("");
    }

};

SkISize BenchmarkControl::fTileSizes[kNumTileSizes] = {
    SkISize::Make(256, 256),
    SkISize::Make(512, 512),
    SkISize::Make(1024, 1024),
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
 * Returns true when a tiled renderer with no bounding box hierarchy produces the given bitmap.
 */
static bool compare_picture(const SkString& path, const SkBitmap& inBitmap, SkPicture* pic) {
    SkBitmap* bitmap;
    sk_tools::TiledPictureRenderer renderer;
    renderer.setBBoxHierarchyType(sk_tools::PictureRenderer::kNone_BBoxHierarchyType);
    renderer.init(pic);
    renderer.setup();
    renderer.render(&path, &bitmap);
    SkAutoTDelete<SkBitmap> bmDeleter(bitmap);
    renderer.end();

    if (bitmap->getSize() != inBitmap.getSize()) {
        return false;
    }
    return !memcmp(bitmap->getPixels(), inBitmap.getPixels(), bitmap->getSize());
}

/**
 * This function is the sink to which all work ends up going.
 * Renders the picture into the renderer. It may or may not use an RTree.
 * The renderer is chosen upstream. If we want to measure recording, we will
 * use a RecordPictureRenderer. If we want to measure rendering, we will use a
 * TiledPictureRenderer.
 */
static void do_benchmark_work(sk_tools::PictureRenderer* renderer,
        int benchmarkType, const SkString& path, SkPicture* pic,
        const int numRepeats, const char *msg, BenchTimer* timer) {
    SkString msgPrefix;

    switch (benchmarkType){
        case kNormal_BenchmarkType:
            msgPrefix.set("Normal");
            renderer->setBBoxHierarchyType(sk_tools::PictureRenderer::kNone_BBoxHierarchyType);
            break;
        case kRTree_BenchmarkType:
            msgPrefix.set("RTree");
            renderer->setBBoxHierarchyType(sk_tools::PictureRenderer::kRTree_BBoxHierarchyType);
            break;
        default:
            SkASSERT(0);
            break;
    }

    renderer->init(pic);

    /**
     * If the renderer is not tiled, assume we are measuring recording.
     */
    bool isPlayback = (NULL != renderer->getTiledRenderer());
    // Will be non-null during RTree picture playback. For correctness test.
    SkBitmap* bitmap = NULL;

    SkDebugf("%s %s %s %d times...\n", msgPrefix.c_str(), msg, path.c_str(), numRepeats);
    for (int i = 0; i < numRepeats; ++i) {
        // Set up the bitmap.
        SkBitmap** out = NULL;
        if (i == 0 && kRTree_BenchmarkType == benchmarkType && isPlayback) {
            out = &bitmap;
        }

        renderer->setup();
        // Render once to fill caches. Fill bitmap during the first iteration.
        renderer->render(NULL, out);
        // Render again to measure
        timer->start();
        bool result = renderer->render(NULL);
        timer->end();

        // We only care about a false result on playback. RecordPictureRenderer::render will always
        // return false because we are passing a NULL file name on purpose; which is fine.
        if (isPlayback && !result) {
            SkDebugf("Error rendering during playback.\n");
        }
    }
    if (bitmap) {
        SkAutoTDelete<SkBitmap> bmDeleter(bitmap);
        if (!compare_picture(path, *bitmap, pic)) {
            SkDebugf("Error: RTree produced different bitmap\n");
        }
    }
}

/**
 * Call do_benchmark_work with a tiled renderer using the default tile dimensions.
 */
static void benchmark_playback(
        const BenchmarkControl& benchControl,
        const SkString& path, SkPicture* pic, BenchTimer* timer) {
    sk_tools::TiledPictureRenderer renderer;

    SkString message("tiled_playback");
    message.appendf("_%dx%d", benchControl.fTileSize.fWidth, benchControl.fTileSize.fHeight);
    do_benchmark_work(&renderer, benchControl.fType,
            path, pic, kNumPlaybacks, message.c_str(), timer);
}

/**
 * Call do_benchmark_work with a RecordPictureRenderer.
 */
static void benchmark_recording(
        const BenchmarkControl& benchControl,
        const SkString& path, SkPicture* pic, BenchTimer* timer) {
    sk_tools::RecordPictureRenderer renderer;
    int numRecordings = 0;
    switch(benchControl.fType) {
        case kRTree_BenchmarkType:
            numRecordings = kNumRTreeRecordings;
            break;
        case kNormal_BenchmarkType:
            numRecordings = kNumNormalRecordings;
            break;
    }
    do_benchmark_work(&renderer, benchControl.fType,
            path, pic, numRecordings, "recording", timer);
}

/**
 * Takes argc,argv along with one of the benchmark functions defined above.
 * Will loop along all skp files and perform measurments.
 *
 * Returns a SkScalar representing CPU time taken during benchmark.
 * As a side effect, it spits the timer result to stdout.
 * Will return -1.0 on error.
 */
static bool benchmark_loop(
        int argc,
        char **argv,
        const BenchmarkControl& benchControl,
        SkTArray<Histogram>& histogram) {
    static const SkString timeFormat("%f");
    TimerData timerData(argc - 1);
    for (int index = 1; index < argc; ++index) {
        BenchTimer timer;
        SkString path(argv[index]);
        SkAutoTUnref<SkPicture> pic(pic_from_path(path.c_str()));
        if (NULL == pic) {
            SkDebugf("Couldn't create picture. Ignoring path: %s\n", path.c_str());
            continue;
        }
        benchControl.fFunction(benchControl, path, pic, &timer);

        histogram[index - 1].fPath = path;
        histogram[index - 1].fCpuTime = SkDoubleToScalar(timer.fCpu);
    }

    const SkString timerResult = timerData.getResult(
            /*doubleFormat = */ timeFormat.c_str(),
            /*result = */ TimerData::kAvg_Result,
            /*configName = */ benchControl.fName.c_str(),
            /*timerFlags = */ TimerData::kCpu_Flag);

    const char findStr[] = "= ";
    int pos = timerResult.find(findStr);
    if (-1 == pos) {
        SkDebugf("Unexpected output from TimerData::getResult(...). Unable to parse.\n");
        return false;
    }

    SkScalar cpuTime = SkDoubleToScalar(atof(timerResult.c_str() + pos + sizeof(findStr) - 1));
    if (cpuTime == 0) {  // atof returns 0.0 on error.
        SkDebugf("Unable to read value from timer result.\n");
        return false;
    }
    return true;
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkAutoGraphics ag;
    SkString usage;
    usage.printf("Usage: filename [filename]*\n");

    if (argc < 2) {
        SkDebugf("%s\n", usage.c_str());
        return -1;
    }

    SkTArray<Histogram> histograms[kNumBenchmarks];

    for (size_t i = 0; i < kNumBenchmarks; ++i) {
        histograms[i].reset(argc - 1);
        bool success = benchmark_loop(
                argc, argv,
                BenchmarkControl::Make(i),
                histograms[i]);
        if (!success) {
            SkDebugf("benchmark_loop failed at index %d\n", i);
        }
    }

    // Output gnuplot readable histogram data..
    const char* pbTitle = "bbh_shootout_playback.dat";
    const char* recTitle = "bbh_shootout_record.dat";
    SkFILEWStream playbackOut(pbTitle);
    SkFILEWStream recordOut(recTitle);
    recordOut.writeText("# ");
    playbackOut.writeText("# ");
    for (size_t i = 0; i < kNumBenchmarks; ++i) {
        SkString out;
        out.printf("%s ", BenchmarkControl::GetBenchmarkName(i).c_str());
        if (BenchmarkControl::GetBenchmarkFunc(i) == &benchmark_recording) {
            recordOut.writeText(out.c_str());
        }
        if (BenchmarkControl::GetBenchmarkFunc(i) == &benchmark_playback) {
            playbackOut.writeText(out.c_str());
        }
    }
    recordOut.writeText("\n");
    playbackOut.writeText("\n");
    // Write to file, and save recording averages.
    SkScalar avgRecord = SkIntToScalar(0);
    SkScalar avgPlayback = SkIntToScalar(0);
    SkScalar avgRecordRTree = SkIntToScalar(0);
    SkScalar avgPlaybackRTree = SkIntToScalar(0);
    for (int i = 0; i < argc - 1; ++i) {
        SkString pbLine;
        SkString recLine;
        // ==== Write record info
        recLine.printf("%d ", i);
        SkScalar cpuTime = histograms[BenchmarkControl::kNormalRecord][i].fCpuTime;
        recLine.appendf("%f ", cpuTime);
        avgRecord += cpuTime;
        cpuTime = histograms[BenchmarkControl::kRTreeRecord][i].fCpuTime;
        recLine.appendf("%f", cpuTime);
        avgRecordRTree += cpuTime;
        avgPlaybackRTree += cpuTime;

        // ==== Write playback info
        pbLine.printf("%d ", i);
        pbLine.appendf("%f ", histograms[2][i].fCpuTime);  // Start with normal playback time.
        avgPlayback += histograms[kNumBbhPlaybackBenchmarks - 1][i].fCpuTime;
        avgPlaybackRTree += histograms[kNumBbhPlaybackBenchmarks][i].fCpuTime;
        // Append all playback benchmark times.
        for (size_t j = kNumBbhPlaybackBenchmarks; j < kNumBenchmarks; ++j) {
            pbLine.appendf("%f ", histograms[j][i].fCpuTime);
        }
        pbLine.remove(pbLine.size() - 1, 1);  // Remove trailing space from line.
        pbLine.appendf("\n");
        recLine.appendf("\n");
        playbackOut.writeText(pbLine.c_str());
        recordOut.writeText(recLine.c_str());
    }
    avgRecord /= argc - 1;
    avgRecordRTree /= argc - 1;
    avgPlayback /= argc - 1;
    avgPlaybackRTree /= argc - 1;
    SkDebugf("Average base recording time: %.3fms\n", avgRecord);
    SkDebugf("Average recording time with rtree: %.3fms\n", avgRecordRTree);
    SkDebugf("Average base playback time: %.3fms\n", avgPlayback);
    SkDebugf("Average playback time with rtree: %.3fms\n", avgPlaybackRTree);

    SkDebugf("\nWrote data to gnuplot-readable files: %s %s\n", pbTitle, recTitle);

    return 0;
}

#if !defined(SK_BUILD_FOR_IOS) && !defined(SK_BUILD_FOR_NACL)
int main(int argc, char** argv) {
    return tool_main(argc, argv);
}
#endif
