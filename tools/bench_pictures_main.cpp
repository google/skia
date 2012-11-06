/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "BenchTimer.h"
#include "PictureBenchmark.h"
#include "SkBenchLogger.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkMath.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "picture_utils.h"

const int DEFAULT_REPEATS = 1;

static void usage(const char* argv0) {
    SkDebugf("SkPicture benchmarking tool\n");
    SkDebugf("\n"
"Usage: \n"
"     %s <inputDir>...\n"
"     [--logFile filename][--timers [wcgWC]*][--logPerIter 1|0][--min]\n"
"     [--repeat] \n"
"     [--mode pow2tile minWidth height[] | record | simple\n"
"             | tile width[] height[] | playbackCreation]\n"
"     [--pipe]\n"
"     [--bbh bbhType]\n"
"     [--multi numThreads]\n"
"     [--device bitmap"
#if SK_SUPPORT_GPU
" | gpu"
#endif
"]"
, argv0);
    SkDebugf("\n\n");
    SkDebugf(
"     inputDir:  A list of directories and files to use as input. Files are\n"
"                expected to have the .skp extension.\n\n"
"     --logFile filename : destination for writing log output, in addition to stdout.\n");
    SkDebugf("     --logPerIter 1|0 : "
             "Log each repeat timer instead of mean, default is disabled.\n");
    SkDebugf("     --min : Print the minimum times (instead of average).\n");
    SkDebugf("     --timers [wcgWC]* : "
             "Display wall, cpu, gpu, truncated wall or truncated cpu time for each picture.\n");
    SkDebugf(
"     --mode pow2tile minWidht height[] | record | simple\n"
"            | tile width[] height[] | playbackCreation:\n"
"            Run in the corresponding mode.\n"
"            Default is simple.\n");
    SkDebugf(
"                     pow2tile minWidth height[], Creates tiles with widths\n"
"                                                 that are all a power of two\n"
"                                                 such that they minimize the\n"
"                                                 amount of wasted tile space.\n"
"                                                 minWidth is the minimum width\n"
"                                                 of these tiles and must be a\n"
"                                                 power of two. Simple\n"
"                                                 rendering using these tiles\n"
"                                                 is benchmarked.\n");
    SkDebugf(
"                     record, Benchmark picture to picture recording.\n");
    SkDebugf(
"                     simple, Benchmark a simple rendering.\n");
    SkDebugf(
"                     tile width[] height[], Benchmark simple rendering using\n"
"                                            tiles with the given dimensions.\n");
    SkDebugf(
"                     playbackCreation, Benchmark creation of the SkPicturePlayback.\n");
    SkDebugf("\n");
    SkDebugf(
"     --multi numThreads : Set the number of threads for multi threaded drawing. Must be greater\n"
"                          than 1. Only works with tiled rendering.\n"
"     --pipe: Benchmark SkGPipe rendering. Currently incompatible with \"mode\".\n");
    SkDebugf(
"     --bbh bbhType: Set the bounding box hierarchy type to be used. Accepted\n"
"                    values are: none, rtree. Default value is none.\n"
"                    Not compatible with --pipe.\n");
    SkDebugf(
"     --device bitmap"
#if SK_SUPPORT_GPU
" | gpu"
#endif
": Use the corresponding device. Default is bitmap.\n");
    SkDebugf(
"                     bitmap, Render to a bitmap.\n");
#if SK_SUPPORT_GPU
    SkDebugf(
"                     gpu, Render to the GPU.\n");
#endif
    SkDebugf("\n");
    SkDebugf(
"     --repeat:  "
"Set the number of times to repeat each test."
" Default is %i.\n", DEFAULT_REPEATS);
}

SkBenchLogger gLogger;

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
    SkPicture picture(&inputStream, &success, &SkImageDecoder::DecodeStream);
    if (!success) {
        SkString err;
        err.printf("Could not read an SkPicture from %s\n", inputPath.c_str());
        gLogger.logError(err);
        return false;
    }

    SkString filename;
    sk_tools::get_basename(&filename, inputPath);

    SkString result;
    result.printf("running bench [%i %i] %s ", picture.width(),
                  picture.height(), filename.c_str());
    gLogger.logProgress(result);

    benchmark.run(&picture);
    return true;
}

#define PRINT_USAGE_AND_EXIT \
    do {                     \
        usage(argv0);        \
        exit(-1);            \
    } while (0)

static void parse_commandline(int argc, char* const argv[], SkTArray<SkString>* inputs,
                              sk_tools::PictureBenchmark* benchmark) {
    const char* argv0 = argv[0];
    char* const* stop = argv + argc;

    int repeats = DEFAULT_REPEATS;
    sk_tools::PictureRenderer::SkDeviceTypes deviceType =
        sk_tools::PictureRenderer::kBitmap_DeviceType;

    SkAutoTUnref<sk_tools::PictureRenderer> renderer(NULL);

    // Create a string to show our current settings.
    // TODO: Make it prettier. Currently it just repeats the command line.
    SkString commandLine("bench_pictures:");
    for (int i = 1; i < argc; i++) {
        commandLine.appendf(" %s", *(argv+i));
    }
    commandLine.append("\n");

    bool usePipe = false;
    int numThreads = 1;
    bool useTiles = false;
    const char* widthString = NULL;
    const char* heightString = NULL;
    bool isPowerOf2Mode = false;
    const char* mode = NULL;
    sk_tools::PictureRenderer::BBoxHierarchyType bbhType =
        sk_tools::PictureRenderer::kNone_BBoxHierarchyType;
    for (++argv; argv < stop; ++argv) {
        if (0 == strcmp(*argv, "--repeat")) {
            ++argv;
            if (argv < stop) {
                repeats = atoi(*argv);
                if (repeats < 1) {
                    gLogger.logError("--repeat must be given a value > 0\n");
                    PRINT_USAGE_AND_EXIT;
                }
            } else {
                gLogger.logError("Missing arg for --repeat\n");
                PRINT_USAGE_AND_EXIT;
            }
        } else if (0 == strcmp(*argv, "--pipe")) {
            usePipe = true;
        } else if (0 == strcmp(*argv, "--logFile")) {
            argv++;
            if (argv < stop) {
                if (!gLogger.SetLogFile(*argv)) {
                    SkString str;
                    str.printf("Could not open %s for writing.", *argv);
                    gLogger.logError(str);
                    usage(argv0);
                    // TODO(borenet): We're disabling this for now, due to
                    // write-protected Android devices.  The very short-term
                    // solution is to ignore the fact that we have no log file.
                    //exit(-1);
                }
            } else {
                gLogger.logError("Missing arg for --logFile\n");
                PRINT_USAGE_AND_EXIT;
            }
        } else if (0 == strcmp(*argv, "--multi")) {
            ++argv;
            if (argv >= stop) {
                gLogger.logError("Missing arg for --multi\n");
                PRINT_USAGE_AND_EXIT;
            }
            numThreads = atoi(*argv);
            if (numThreads < 2) {
                gLogger.logError("Number of threads must be at least 2.\n");
                PRINT_USAGE_AND_EXIT;
            }
        } else if (0 == strcmp(*argv, "--bbh")) {
            ++argv;
            if (argv >= stop) {
                gLogger.logError("Missing value for --bbh\n");
                PRINT_USAGE_AND_EXIT;
            }
            if (0 == strcmp(*argv, "none")) {
                bbhType = sk_tools::PictureRenderer::kNone_BBoxHierarchyType;
            } else if (0 == strcmp(*argv, "rtree")) {
                bbhType = sk_tools::PictureRenderer::kRTree_BBoxHierarchyType;
            } else {
                SkString err;
                err.printf("%s is not a valid value for --bbhType\n", *argv);
                gLogger.logError(err);
                PRINT_USAGE_AND_EXIT;
            }

        } else if (0 == strcmp(*argv, "--mode")) {
            if (renderer.get() != NULL) {
                SkDebugf("Cannot combine modes.\n");
                PRINT_USAGE_AND_EXIT;
            }

            ++argv;
            if (argv >= stop) {
                gLogger.logError("Missing mode for --mode\n");
                PRINT_USAGE_AND_EXIT;
            }

            if (0 == strcmp(*argv, "record")) {
                renderer.reset(SkNEW(sk_tools::RecordPictureRenderer));
            } else if (0 == strcmp(*argv, "simple")) {
                renderer.reset(SkNEW(sk_tools::SimplePictureRenderer));
            } else if ((0 == strcmp(*argv, "tile")) || (0 == strcmp(*argv, "pow2tile"))) {
                useTiles = true;
                mode = *argv;

                if (0 == strcmp(*argv, "pow2tile")) {
                    isPowerOf2Mode = true;
                }

                ++argv;
                if (argv >= stop) {
                    SkString err;
                    err.printf("Missing width for --mode %s\n", mode);
                    gLogger.logError(err);
                    PRINT_USAGE_AND_EXIT;
                }

                widthString = *argv;
                ++argv;
                if (argv >= stop) {
                    gLogger.logError("Missing height for --mode tile\n");
                    PRINT_USAGE_AND_EXIT;
                }
                heightString = *argv;
            } else if (0 == strcmp(*argv, "playbackCreation")) {
                renderer.reset(SkNEW(sk_tools::PlaybackCreationRenderer));
            } else {
                SkString err;
                err.printf("%s is not a valid mode for --mode\n", *argv);
                gLogger.logError(err);
                PRINT_USAGE_AND_EXIT;
            }
        }  else if (0 == strcmp(*argv, "--device")) {
            ++argv;
            if (argv >= stop) {
                gLogger.logError("Missing mode for --device\n");
                PRINT_USAGE_AND_EXIT;
            }

            if (0 == strcmp(*argv, "bitmap")) {
                deviceType = sk_tools::PictureRenderer::kBitmap_DeviceType;
            }
#if SK_SUPPORT_GPU
            else if (0 == strcmp(*argv, "gpu")) {
                deviceType = sk_tools::PictureRenderer::kGPU_DeviceType;
            }
#endif
            else {
                SkString err;
                err.printf("%s is not a valid mode for --device\n", *argv);
                gLogger.logError(err);
                PRINT_USAGE_AND_EXIT;
            }
        } else if (0 == strcmp(*argv, "--timers")) {
            ++argv;
            if (argv < stop) {
                bool timerWall = false;
                bool truncatedTimerWall = false;
                bool timerCpu = false;
                bool truncatedTimerCpu = false;
                bool timerGpu = false;
                for (char* t = *argv; *t; ++t) {
                    switch (*t) {
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
                        default: {
                            break;
                        }
                    }
                }
                benchmark->setTimersToShow(timerWall, truncatedTimerWall, timerCpu,
                                           truncatedTimerCpu, timerGpu);
            } else {
                gLogger.logError("Missing arg for --timers\n");
                PRINT_USAGE_AND_EXIT;
            }
        } else if (0 == strcmp(*argv, "--min")) {
            benchmark->setPrintMin(true);
        } else if (0 == strcmp(*argv, "--logPerIter")) {
            ++argv;
            if (argv < stop) {
                bool log = atoi(*argv) != 0;
                benchmark->setLogPerIter(log);
            } else {
                gLogger.logError("Missing arg for --logPerIter\n");
                PRINT_USAGE_AND_EXIT;
            }
        } else if (0 == strcmp(*argv, "--help") || 0 == strcmp(*argv, "-h")) {
            PRINT_USAGE_AND_EXIT;
        } else {
            inputs->push_back(SkString(*argv));
        }
    }

    if (numThreads > 1 && !useTiles) {
        gLogger.logError("Multithreaded drawing requires tiled rendering.\n");
        PRINT_USAGE_AND_EXIT;
    }

    if (usePipe && sk_tools::PictureRenderer::kNone_BBoxHierarchyType != bbhType) {
        gLogger.logError("--pipe and --bbh cannot be used together\n");
        PRINT_USAGE_AND_EXIT;
    }

    if (useTiles) {
        SkASSERT(NULL == renderer);
        sk_tools::TiledPictureRenderer* tiledRenderer;
        if (numThreads > 1) {
            tiledRenderer = SkNEW_ARGS(sk_tools::MultiCorePictureRenderer, (numThreads));
        } else {
            tiledRenderer = SkNEW(sk_tools::TiledPictureRenderer);
        }
        if (isPowerOf2Mode) {
            int minWidth = atoi(widthString);
            if (!SkIsPow2(minWidth) || minWidth < 0) {
                tiledRenderer->unref();
                SkString err;
                err.printf("-mode %s must be given a width"
                         " value that is a power of two\n", mode);
                gLogger.logError(err);
                PRINT_USAGE_AND_EXIT;
            }
            tiledRenderer->setTileMinPowerOf2Width(minWidth);
        } else if (sk_tools::is_percentage(widthString)) {
            tiledRenderer->setTileWidthPercentage(atof(widthString));
            if (!(tiledRenderer->getTileWidthPercentage() > 0)) {
                tiledRenderer->unref();
                gLogger.logError("--mode tile must be given a width percentage > 0\n");
                PRINT_USAGE_AND_EXIT;
            }
        } else {
            tiledRenderer->setTileWidth(atoi(widthString));
            if (!(tiledRenderer->getTileWidth() > 0)) {
                tiledRenderer->unref();
                gLogger.logError("--mode tile must be given a width > 0\n");
                PRINT_USAGE_AND_EXIT;
            }
        }

        if (sk_tools::is_percentage(heightString)) {
            tiledRenderer->setTileHeightPercentage(atof(heightString));
            if (!(tiledRenderer->getTileHeightPercentage() > 0)) {
                tiledRenderer->unref();
                gLogger.logError("--mode tile must be given a height percentage > 0\n");
                PRINT_USAGE_AND_EXIT;
            }
        } else {
            tiledRenderer->setTileHeight(atoi(heightString));
            if (!(tiledRenderer->getTileHeight() > 0)) {
                tiledRenderer->unref();
                gLogger.logError("--mode tile must be given a height > 0\n");
                PRINT_USAGE_AND_EXIT;
            }
        }
        if (numThreads > 1) {
#if SK_SUPPORT_GPU
            if (sk_tools::PictureRenderer::kGPU_DeviceType == deviceType) {
                tiledRenderer->unref();
                gLogger.logError("GPU not compatible with multithreaded tiling.\n");
                PRINT_USAGE_AND_EXIT;
            }
#endif
        }
        renderer.reset(tiledRenderer);
        if (usePipe) {
            SkDebugf("Pipe rendering is currently not compatible with tiling.\n"
                     "Turning off pipe.\n");
        }
    } else if (usePipe) {
        if (renderer.get() != NULL) {
            SkDebugf("Pipe is incompatible with other modes.\n");
            PRINT_USAGE_AND_EXIT;
        }
        renderer.reset(SkNEW(sk_tools::PipePictureRenderer));
    }
    if (inputs->count() < 1) {
        PRINT_USAGE_AND_EXIT;
    }

    if (NULL == renderer) {
        renderer.reset(SkNEW(sk_tools::SimplePictureRenderer));
    }

    renderer->setBBoxHierarchyType(bbhType);
    benchmark->setRenderer(renderer);
    benchmark->setRepeats(repeats);
    benchmark->setDeviceType(deviceType);
    benchmark->setLogger(&gLogger);
    // Report current settings:
    gLogger.logProgress(commandLine);
}

static int process_input(const SkString& input,
                         sk_tools::PictureBenchmark& benchmark) {
    SkOSFile::Iter iter(input.c_str(), "skp");
    SkString inputFilename;
    int failures = 0;
    if (iter.next(&inputFilename)) {
        do {
            SkString inputPath;
            sk_tools::make_filepath(&inputPath, input, inputFilename);
            if (!run_single_benchmark(inputPath, benchmark)) {
                ++failures;
            }
        } while(iter.next(&inputFilename));
    } else if (SkStrEndsWith(input.c_str(), ".skp")) {
        if (!run_single_benchmark(input, benchmark)) {
            ++failures;
        }
    } else {
        SkString warning;
        warning.printf("Warning: skipping %s\n", input.c_str());
        gLogger.logError(warning);
    }
    return failures;
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
#ifdef SK_ENABLE_INST_COUNT
    gPrintInstCount = true;
#endif
    SkAutoGraphics ag;

    SkTArray<SkString> inputs;
    sk_tools::PictureBenchmark benchmark;

    parse_commandline(argc, argv, &inputs, &benchmark);

    int failures = 0;
    for (int i = 0; i < inputs.count(); ++i) {
        failures += process_input(inputs[i], benchmark);
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
