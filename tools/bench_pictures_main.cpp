/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "BenchLogger.h"
#include "Timer.h"
#include "CopyTilesRenderer.h"
#include "CrashHandler.h"
#include "LazyDecodeBitmap.h"
#include "PictureBenchmark.h"
#include "PictureRenderingFlags.h"
#include "PictureResultsWriter.h"
#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkDiscardableMemoryPool.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkMath.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "picture_utils.h"

BenchLogger gLogger;
PictureResultsLoggerWriter gLogWriter(&gLogger);
PictureResultsMultiWriter gWriter;

// Flags used by this file, in alphabetical order.
DEFINE_bool(countRAM, false, "Count the RAM used for bitmap pixels in each skp file");
DECLARE_bool(deferImageDecoding);
DEFINE_string(filter, "",
        "type:flag : Enable canvas filtering to disable a paint flag, "
        "use no blur or low quality blur, or use no hinting or "
        "slight hinting. For all flags except AAClip, specify the "
        "type of primitive to effect, or choose all. for AAClip "
        "alone, the filter affects all clips independent of type. "
        "Specific flags are listed above.");
DEFINE_string(logFile, "", "Destination for writing log output, in addition to stdout.");
DEFINE_bool(logPerIter, false, "Log each repeat timer instead of mean.");
DEFINE_string(jsonLog, "", "Destination for writing JSON data.");
DEFINE_bool(min, false, "Print the minimum times (instead of average).");
DECLARE_string(readPath);
DEFINE_int32(repeat, 1, "Set the number of times to repeat each test.");
DEFINE_bool(timeIndividualTiles, false, "Report times for drawing individual tiles, rather than "
            "times for drawing the whole page. Requires tiled rendering.");
DEFINE_bool(purgeDecodedTex, false, "Purge decoded and GPU-uploaded textures "
            "after each iteration.");
DEFINE_string(timers, "c", "[wcgWC]*: Display wall, cpu, gpu, truncated wall or truncated cpu time"
              " for each picture.");
DEFINE_bool(trackDeferredCaching, false, "Only meaningful with --deferImageDecoding and "
            "SK_LAZY_CACHE_STATS set to true. Report percentage of cache hits when using "
            "deferred image decoding.");

#if GR_GPU_STATS
DEFINE_bool(gpuStats, false, "Only meaningful with gpu configurations. "
            "Report some GPU call statistics.");
#endif

DEFINE_bool(mpd, false, "If true, use MultiPictureDraw to render.");

// Buildbot-specific parameters
DEFINE_string(builderName, "", "Name of the builder this is running on.");
DEFINE_int32(buildNumber, -1, "Build number of the build this test is running on");
DEFINE_int32(timestamp, 0, "Timestamp of the revision of Skia being tested.");
DEFINE_string(gitHash, "", "Commit hash of the revision of Skia being run.");
DEFINE_int32(gitNumber, -1, "Git number of the revision of Skia being run.");


static char const * const gFilterTypes[] = {
    "paint",
    "point",
    "line",
    "bitmap",
    "rect",
    "oval",
    "path",
    "text",
    "all",
};

static const size_t kFilterTypesCount = sizeof(gFilterTypes) / sizeof(gFilterTypes[0]);

static char const * const gFilterFlags[] = {
    "antiAlias",
    "filterBitmap",
    "dither",
    "underlineText",
    "strikeThruText",
    "fakeBoldText",
    "linearText",
    "subpixelText",
    "devKernText",
    "LCDRenderText",
    "embeddedBitmapText",
    "autoHinting",
    "verticalText",
    "genA8FromLCD",
    "blur",
    "hinting",
    "slightHinting",
    "AAClip",
};

static const size_t kFilterFlagsCount = sizeof(gFilterFlags) / sizeof(gFilterFlags[0]);

static SkString filtersName(sk_tools::PictureRenderer::DrawFilterFlags* drawFilters) {
    int all = drawFilters[0];
    size_t tIndex;
    for (tIndex = 1; tIndex < SkDrawFilter::kTypeCount; ++tIndex) {
        all &= drawFilters[tIndex];
    }
    SkString result;
    for (size_t fIndex = 0; fIndex < kFilterFlagsCount; ++fIndex) {
        SkString types;
        if (all & (1 << fIndex)) {
            types = gFilterTypes[SkDrawFilter::kTypeCount];
        } else {
            for (tIndex = 0; tIndex < SkDrawFilter::kTypeCount; ++tIndex) {
                if (drawFilters[tIndex] & (1 << fIndex)) {
                    types += gFilterTypes[tIndex];
                }
            }
        }
        if (!types.size()) {
            continue;
        }
        result += "_";
        result += types;
        result += ".";
        result += gFilterFlags[fIndex];
    }
    return result;
}

static SkString filterTypesUsage() {
    SkString result;
    for (size_t index = 0; index < kFilterTypesCount; ++index) {
        result += gFilterTypes[index];
        if (index < kFilterTypesCount - 1) {
            result += " | ";
        }
    }
    return result;
}

static SkString filterFlagsUsage() {
    SkString result;
    size_t len = 0;
    for (size_t index = 0; index < kFilterFlagsCount; ++index) {
        result += gFilterFlags[index];
        if (result.size() - len >= 72) {
            result += "\n\t\t";
            len = result.size();
        }
        if (index < kFilterFlagsCount - 1) {
            result += " | ";
        }
    }
    return result;
}

#if SK_LAZY_CACHE_STATS
static int32_t gTotalCacheHits;
static int32_t gTotalCacheMisses;
#endif

static bool run_single_benchmark(const SkString& inputPath,
                                 sk_tools::PictureBenchmark& benchmark) {
    SkFILEStream inputStream;

    inputStream.setPath(inputPath.c_str());
    if (!inputStream.isValid()) {
        SkString err;
        err.printf("Could not open file %s\n", inputPath.c_str());
        gLogger.logError(err);
        return false;
    }

    SkDiscardableMemoryPool* pool = SkGetGlobalDiscardableMemoryPool();
    // Since the old picture has been deleted, all pixels should be cleared.
    SkASSERT(pool->getRAMUsed() == 0);
    if (FLAGS_countRAM) {
        pool->setRAMBudget(SK_MaxU32);
        // Set the limit to max, so all pixels will be kept
    }

    SkPicture::InstallPixelRefProc proc;
    if (FLAGS_deferImageDecoding) {
        proc = &sk_tools::LazyDecodeBitmap;
    } else {
        proc = &SkImageDecoder::DecodeMemory;
    }
    SkAutoTUnref<SkPicture> picture(SkPicture::CreateFromStream(&inputStream, proc));

    if (NULL == picture.get()) {
        SkString err;
        err.printf("Could not read an SkPicture from %s\n", inputPath.c_str());
        gLogger.logError(err);
        return false;
    }

    SkString filename = SkOSPath::Basename(inputPath.c_str());

    gWriter.bench(filename.c_str(),
                  SkScalarCeilToInt(picture->cullRect().width()),
                  SkScalarCeilToInt(picture->cullRect().height()));

    benchmark.run(picture, FLAGS_mpd);

#if SK_LAZY_CACHE_STATS
    if (FLAGS_trackDeferredCaching) {
        int cacheHits = pool->getCacheHits();
        int cacheMisses = pool->getCacheMisses();
        pool->resetCacheHitsAndMisses();
        SkString hitString;
        hitString.printf("Cache hit rate: %f\n", (double) cacheHits / (cacheHits + cacheMisses));
        gLogger.logProgress(hitString);
        gTotalCacheHits += cacheHits;
        gTotalCacheMisses += cacheMisses;
    }
#endif
    if (FLAGS_countRAM) {
        SkString ramCount("RAM used for bitmaps: ");
        size_t bytes = pool->getRAMUsed();
        if (bytes > 1024) {
            size_t kb = bytes / 1024;
            if (kb > 1024) {
                size_t mb = kb / 1024;
                ramCount.appendf("%zi MB\n", mb);
            } else {
                ramCount.appendf("%zi KB\n", kb);
            }
        } else {
            ramCount.appendf("%zi bytes\n", bytes);
        }
        gLogger.logProgress(ramCount);
    }

    return true;
}

static void setup_benchmark(sk_tools::PictureBenchmark* benchmark) {
    sk_tools::PictureRenderer::DrawFilterFlags drawFilters[SkDrawFilter::kTypeCount];
    sk_bzero(drawFilters, sizeof(drawFilters));

    if (FLAGS_filter.count() > 0) {
        const char* filters = FLAGS_filter[0];
        const char* colon = strchr(filters, ':');
        if (colon) {
            int32_t type = -1;
            size_t typeLen = colon - filters;
            for (size_t tIndex = 0; tIndex < kFilterTypesCount; ++tIndex) {
                if (typeLen == strlen(gFilterTypes[tIndex])
                        && !strncmp(filters, gFilterTypes[tIndex], typeLen)) {
                    type = SkToS32(tIndex);
                    break;
                }
            }
            if (type < 0) {
                SkString err;
                err.printf("Unknown type for --filter %s\n", filters);
                gLogger.logError(err);
                exit(-1);
            }
            int flag = -1;
            size_t flagLen = strlen(filters) - typeLen - 1;
            for (size_t fIndex = 0; fIndex < kFilterFlagsCount; ++fIndex) {
                if (flagLen == strlen(gFilterFlags[fIndex])
                        && !strncmp(colon + 1, gFilterFlags[fIndex], flagLen)) {
                    flag = 1 << fIndex;
                    break;
                }
            }
            if (flag < 0) {
                SkString err;
                err.printf("Unknown flag for --filter %s\n", filters);
                gLogger.logError(err);
                exit(-1);
            }
            for (int index = 0; index < SkDrawFilter::kTypeCount; ++index) {
                if (type != SkDrawFilter::kTypeCount && index != type) {
                    continue;
                }
                drawFilters[index] = (sk_tools::PictureRenderer::DrawFilterFlags)
                        (drawFilters[index] | flag);
            }
        } else {
            SkString err;
            err.printf("Unknown arg for --filter %s : missing colon\n", filters);
            gLogger.logError(err);
            exit(-1);
        }
    }

    if (FLAGS_timers.count() > 0) {
        size_t index = 0;
        bool timerWall = false;
        bool truncatedTimerWall = false;
        bool timerCpu = false;
        bool truncatedTimerCpu = false;
        bool timerGpu = false;
        while (index < strlen(FLAGS_timers[0])) {
            switch (FLAGS_timers[0][index]) {
                case 'w':
                    timerWall = true;
                    break;
                case 'c':
                    timerCpu = true;
                    break;
                case 'W':
                    truncatedTimerWall = true;
                    break;
                case 'C':
                    truncatedTimerCpu = true;
                    break;
                case 'g':
                    timerGpu = true;
                    break;
                default:
                    SkDebugf("mystery character\n");
                    break;
            }
            index++;
        }
        benchmark->setTimersToShow(timerWall, truncatedTimerWall, timerCpu, truncatedTimerCpu,
                                  timerGpu);
    }

    SkString errorString;
    SkAutoTUnref<sk_tools::PictureRenderer> renderer(parseRenderer(errorString,
                                                                   kBench_PictureTool));

    if (errorString.size() > 0) {
        gLogger.logError(errorString);
    }

    if (NULL == renderer.get()) {
        exit(-1);
    }

    if (FLAGS_timeIndividualTiles) {
        sk_tools::TiledPictureRenderer* tiledRenderer = renderer->getTiledRenderer();
        if (NULL == tiledRenderer) {
            gLogger.logError("--timeIndividualTiles requires tiled rendering.\n");
            exit(-1);
        }
        if (!tiledRenderer->supportsTimingIndividualTiles()) {
            gLogger.logError("This renderer does not support --timeIndividualTiles.\n");
            exit(-1);
        }
        benchmark->setTimeIndividualTiles(true);
    }

    benchmark->setPurgeDecodedTex(FLAGS_purgeDecodedTex);

    if (FLAGS_readPath.count() < 1) {
        gLogger.logError(".skp files or directories are required.\n");
        exit(-1);
    }

    renderer->setDrawFilters(drawFilters, filtersName(drawFilters));
    if (FLAGS_logPerIter) {
        benchmark->setTimerResultType(TimerData::kPerIter_Result);
    } else if (FLAGS_min) {
        benchmark->setTimerResultType(TimerData::kMin_Result);
    } else {
        benchmark->setTimerResultType(TimerData::kAvg_Result);
    }
    benchmark->setRenderer(renderer);
    benchmark->setRepeats(FLAGS_repeat);
    benchmark->setWriter(&gWriter);
}

static int process_input(const char* input,
                         sk_tools::PictureBenchmark& benchmark) {
    SkString inputAsSkString(input);
    SkOSFile::Iter iter(input, "skp");
    SkString inputFilename;
    int failures = 0;
    if (iter.next(&inputFilename)) {
        do {
            SkString inputPath = SkOSPath::Join(input, inputFilename.c_str());
            if (!run_single_benchmark(inputPath, benchmark)) {
                ++failures;
            }
        } while(iter.next(&inputFilename));
    } else if (SkStrEndsWith(input, ".skp")) {
        if (!run_single_benchmark(inputAsSkString, benchmark)) {
            ++failures;
        }
    } else {
        SkString warning;
        warning.printf("Warning: skipping %s\n", input);
        gLogger.logError(warning);
    }
    return failures;
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SetupCrashHandler();
    SkString usage;
    usage.printf("Time drawing .skp files.\n"
                 "\tPossible arguments for --filter: [%s]\n\t\t[%s]",
                 filterTypesUsage().c_str(), filterFlagsUsage().c_str());
    SkCommandLineFlags::SetUsage(usage.c_str());
    SkCommandLineFlags::Parse(argc, argv);

    if (FLAGS_repeat < 1) {
        SkString error;
        error.printf("--repeats must be >= 1. Was %i\n", FLAGS_repeat);
        gLogger.logError(error);
        exit(-1);
    }

    if (FLAGS_logFile.count() == 1) {
        if (!gLogger.SetLogFile(FLAGS_logFile[0])) {
            SkString str;
            str.printf("Could not open %s for writing.\n", FLAGS_logFile[0]);
            gLogger.logError(str);
            // TODO(borenet): We're disabling this for now, due to
            // write-protected Android devices.  The very short-term
            // solution is to ignore the fact that we have no log file.
            //exit(-1);
        }
    }

    SkAutoTDelete<PictureJSONResultsWriter> jsonWriter;
    if (FLAGS_jsonLog.count() == 1) {
        SkASSERT(FLAGS_builderName.count() == 1 && FLAGS_gitHash.count() == 1);
        jsonWriter.reset(SkNEW(PictureJSONResultsWriter(
                        FLAGS_jsonLog[0],
                        FLAGS_builderName[0],
                        FLAGS_buildNumber,
                        FLAGS_timestamp,
                        FLAGS_gitHash[0],
                        FLAGS_gitNumber)));
        gWriter.add(jsonWriter.get());
    }

    gWriter.add(&gLogWriter);


    SkAutoGraphics ag;

    sk_tools::PictureBenchmark benchmark;

    setup_benchmark(&benchmark);

    int failures = 0;
    for (int i = 0; i < FLAGS_readPath.count(); ++i) {
        failures += process_input(FLAGS_readPath[i], benchmark);
    }

    if (failures != 0) {
        SkString err;
        err.printf("Failed to run %i benchmarks.\n", failures);
        gLogger.logError(err);
        return 1;
    }
#if SK_LAZY_CACHE_STATS
    if (FLAGS_trackDeferredCaching) {
        SkDebugf("Total cache hit rate: %f\n",
                 (double) gTotalCacheHits / (gTotalCacheHits + gTotalCacheMisses));
    }
#endif

#if GR_GPU_STATS && SK_SUPPORT_GPU
    if (FLAGS_gpuStats && benchmark.renderer()->isUsingGpuDevice()) {
        benchmark.renderer()->getGrContext()->printGpuStats();
    }
#endif

    gWriter.end();
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
