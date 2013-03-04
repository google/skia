/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "BenchTimer.h"
#include "CopyTilesRenderer.h"
#include "PictureBenchmark.h"
#include "PictureRenderingFlags.h"
#include "SkBenchLogger.h"
#include "SkBitmapFactory.h"
#include "SkCanvas.h"
#include "SkFlags.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkMath.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "picture_utils.h"


SkBenchLogger gLogger;

// Flags used by this file, in alphabetical order.
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
DEFINE_bool(min, false, "Print the minimum times (instead of average).");
DECLARE_int32(multi);
DECLARE_string(r);
DEFINE_int32(repeat, 1, "Set the number of times to repeat each test.");
DEFINE_bool(timeIndividualTiles, false, "Report times for drawing individual tiles, rather than "
            "times for drawing the whole page. Requires tiled rendering.");
DEFINE_string(timers, "", "[wcgWC]*: Display wall, cpu, gpu, truncated wall or truncated cpu time"
              " for each picture.");

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

#include "SkData.h"
#include "SkLruImageCache.h"

static SkLruImageCache gLruImageCache(1024*1024);

static bool lazy_decode_bitmap(const void* buffer, size_t size, SkBitmap* bitmap) {
    void* copiedBuffer = sk_malloc_throw(size);
    memcpy(copiedBuffer, buffer, size);
    SkAutoDataUnref data(SkData::NewFromMalloc(copiedBuffer, size));
    SkBitmapFactory factory(&SkImageDecoder::DecodeMemoryToTarget);
    factory.setImageCache(&gLruImageCache);
    return factory.installPixelRef(data, bitmap);
}

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

    bool success = false;
    SkPicture* picture;
    if (FLAGS_deferImageDecoding) {
        picture = SkNEW_ARGS(SkPicture, (&inputStream, &success, &lazy_decode_bitmap));
    } else {
        picture = SkNEW_ARGS(SkPicture, (&inputStream, &success, &SkImageDecoder::DecodeMemory));
    }
    SkAutoTDelete<SkPicture> ad(picture);

    if (!success) {
        SkString err;
        err.printf("Could not read an SkPicture from %s\n", inputPath.c_str());
        gLogger.logError(err);
        return false;
    }

    SkString filename;
    sk_tools::get_basename(&filename, inputPath);

    SkString result;
    result.printf("running bench [%i %i] %s ", picture->width(), picture->height(),
                  filename.c_str());
    gLogger.logProgress(result);

    benchmark.run(picture);
    return true;
}

static void setup_benchmark(sk_tools::PictureBenchmark* benchmark) {
    sk_tools::PictureRenderer::DrawFilterFlags drawFilters[SkDrawFilter::kTypeCount];
    sk_bzero(drawFilters, sizeof(drawFilters));

    if (FLAGS_filter.count() > 0) {
        const char* filters = FLAGS_filter[0];
        const char* colon = strchr(filters, ':');
        if (colon) {
            int type = -1;
            size_t typeLen = colon - filters;
            for (size_t tIndex = 0; tIndex < kFilterTypesCount; ++tIndex) {
                if (typeLen == strlen(gFilterTypes[tIndex])
                        && !strncmp(filters, gFilterTypes[tIndex], typeLen)) {
                    type = tIndex;
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
        if (FLAGS_multi > 1) {
            gLogger.logError("Cannot time individual tiles with more than one thread.\n");
            exit(-1);
        }
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

    if (FLAGS_r.count() < 1) {
        gLogger.logError(".skp files or directories are required.\n");
        exit(-1);
    }

    renderer->setDrawFilters(drawFilters, filtersName(drawFilters));
    benchmark->setPrintMin(FLAGS_min);
    benchmark->setLogPerIter(FLAGS_logPerIter);
    benchmark->setRenderer(renderer);
    benchmark->setRepeats(FLAGS_repeat);
    benchmark->setLogger(&gLogger);
}

static int process_input(const char* input,
                         sk_tools::PictureBenchmark& benchmark) {
    SkString inputAsSkString(input);
    SkOSFile::Iter iter(input, "skp");
    SkString inputFilename;
    int failures = 0;
    if (iter.next(&inputFilename)) {
        do {
            SkString inputPath;
            sk_tools::make_filepath(&inputPath, inputAsSkString, inputFilename);
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
    SkString usage;
    usage.printf("Time drawing .skp files.\n"
                 "\tPossible arguments for --filter: [%s]\n\t\t[%s]",
                 filterTypesUsage().c_str(), filterFlagsUsage().c_str());
    SkFlags::SetUsage(usage.c_str());
    SkFlags::ParseCommandLine(argc, argv);

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


#if SK_ENABLE_INST_COUNT
    gPrintInstCount = true;
#endif
    SkAutoGraphics ag;

    sk_tools::PictureBenchmark benchmark;

    setup_benchmark(&benchmark);

    int failures = 0;
    for (int i = 0; i < FLAGS_r.count(); ++i) {
        failures += process_input(FLAGS_r[i], benchmark);
    }

    if (failures != 0) {
        SkString err;
        err.printf("Failed to run %i benchmarks.\n", failures);
        gLogger.logError(err);
        return 1;
    }
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
