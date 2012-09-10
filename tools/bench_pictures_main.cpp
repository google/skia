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
#include "SkMath.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "picture_utils.h"

const int DEFAULT_REPEATS = 100;

static void usage(const char* argv0) {
    SkDebugf("SkPicture benchmarking tool\n");
    SkDebugf("\n"
"Usage: \n"
"     %s <inputDir>...\n"
"     [--logFile filename]\n"
"     [--repeat] \n"
"     [--mode pow2tile minWidth height[] (multi) | record | simple\n"
"             | tile width[] height[] (multi) | playbackCreation]\n"
"     [--pipe]\n"
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
    SkDebugf(
"     --mode pow2tile minWidht height[] (multi) | record | simple\n"
"            | tile width[] height[] (multi) | playbackCreation:\n"
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
"                                                 is benchmarked.\n"
"                                                 Append \"multi\" for multithreaded\n"
"                                                 drawing.\n");
    SkDebugf(
"                     record, Benchmark picture to picture recording.\n");
    SkDebugf(
"                     simple, Benchmark a simple rendering.\n");
    SkDebugf(
"                     tile width[] height[], Benchmark simple rendering using\n"
"                                            tiles with the given dimensions.\n"
"                                            Append \"multi\" for multithreaded\n"
"                                            drawing.\n");
    SkDebugf(
"                     playbackCreation, Benchmark creation of the SkPicturePlayback.\n");
    SkDebugf("\n");
    SkDebugf(
"     --pipe: Benchmark SkGPipe rendering. Compatible with tiled, multithreaded rendering.\n");
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

static void run_single_benchmark(const SkString& inputPath,
                                 sk_tools::PictureBenchmark& benchmark) {
    SkFILEStream inputStream;

    inputStream.setPath(inputPath.c_str());
    if (!inputStream.isValid()) {
        SkString err;
        err.printf("Could not open file %s\n", inputPath.c_str());
        gLogger.logError(err);
        return;
    }

    SkPicture picture(&inputStream);

    SkString filename;
    sk_tools::get_basename(&filename, inputPath);

    SkString result;
    result.printf("running bench [%i %i] %s ", picture.width(), picture.height(),
                  filename.c_str());
    gLogger.logProgress(result);

    benchmark.run(&picture);
}

static void parse_commandline(int argc, char* const argv[], SkTArray<SkString>* inputs,
                              sk_tools::PictureBenchmark*& benchmark) {
    const char* argv0 = argv[0];
    char* const* stop = argv + argc;

    int repeats = DEFAULT_REPEATS;
    sk_tools::PictureRenderer::SkDeviceTypes deviceType =
        sk_tools::PictureRenderer::kBitmap_DeviceType;

    // Create a string to show our current settings.
    // TODO: Make it prettier. Currently it just repeats the command line.
    SkString commandLine("bench_pictures:");
    for (int i = 1; i < argc; i++) {
        commandLine.appendf(" %s", *(argv+i));
    }
    commandLine.append("\n");

    bool usePipe = false;
    bool multiThreaded = false;
    bool useTiles = false;
    const char* widthString = NULL;
    const char* heightString = NULL;
    bool isPowerOf2Mode = false;
    const char* mode = NULL;
    for (++argv; argv < stop; ++argv) {
        if (0 == strcmp(*argv, "--repeat")) {
            ++argv;
            if (argv < stop) {
                repeats = atoi(*argv);
                if (repeats < 1) {
                    SkDELETE(benchmark);
                    gLogger.logError("--repeat must be given a value > 0\n");
                    exit(-1);
                }
            } else {
                SkDELETE(benchmark);
                gLogger.logError("Missing arg for --repeat\n");
                usage(argv0);
                exit(-1);
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
                    exit(-1);
                }
            } else {
                gLogger.logError("Missing arg for --logFile\n");
                usage(argv0);
                exit(-1);
            }
        } else if (0 == strcmp(*argv, "--mode")) {
            SkDELETE(benchmark);

            ++argv;
            if (argv >= stop) {
                gLogger.logError("Missing mode for --mode\n");
                usage(argv0);
                exit(-1);
            }

            if (0 == strcmp(*argv, "record")) {
                benchmark = SkNEW(sk_tools::RecordPictureBenchmark);
            } else if (0 == strcmp(*argv, "simple")) {
                benchmark = SkNEW(sk_tools::SimplePictureBenchmark);
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
                    usage(argv0);
                    exit(-1);
                }

                widthString = *argv;
                ++argv;
                if (argv >= stop) {
                    gLogger.logError("Missing height for --mode tile\n");
                    usage(argv0);
                    exit(-1);
                }
                heightString = *argv;

                ++argv;
                if (argv < stop && 0 == strcmp(*argv, "multi")) {
                    multiThreaded = true;
                } else {
                    --argv;
                }
            } else if (0 == strcmp(*argv, "playbackCreation")) {
                benchmark = SkNEW(sk_tools::PlaybackCreationBenchmark);
            } else {
                SkString err;
                err.printf("%s is not a valid mode for --mode\n", *argv);
                gLogger.logError(err);
                usage(argv0);
                exit(-1);
            }
        }  else if (0 == strcmp(*argv, "--device")) {
            ++argv;
            if (argv >= stop) {
                gLogger.logError("Missing mode for --deivce\n");
                usage(argv0);
                exit(-1);
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
                usage(argv0);
                exit(-1);
            }

        } else if (0 == strcmp(*argv, "--help") || 0 == strcmp(*argv, "-h")) {
            SkDELETE(benchmark);
            usage(argv0);
            exit(0);
        } else {
            inputs->push_back(SkString(*argv));
        }
    }

    if (useTiles) {
        sk_tools::TiledPictureBenchmark* tileBenchmark = SkNEW(sk_tools::TiledPictureBenchmark);
        if (isPowerOf2Mode) {
            int minWidth = atoi(widthString);
            if (!SkIsPow2(minWidth) || minWidth < 0) {
                SkDELETE(tileBenchmark);
                SkString err;
                err.printf("--mode %s must be given a width"
                         " value that is a power of two\n", mode);
                gLogger.logError(err);
                exit(-1);
            }
            tileBenchmark->setTileMinPowerOf2Width(minWidth);
        } else if (sk_tools::is_percentage(widthString)) {
            tileBenchmark->setTileWidthPercentage(atof(widthString));
            if (!(tileBenchmark->getTileWidthPercentage() > 0)) {
                SkDELETE(tileBenchmark);
                gLogger.logError("--mode tile must be given a width percentage > 0\n");
                exit(-1);
            }
        } else {
            tileBenchmark->setTileWidth(atoi(widthString));
            if (!(tileBenchmark->getTileWidth() > 0)) {
                SkDELETE(tileBenchmark);
                gLogger.logError("--mode tile must be given a width > 0\n");
                exit(-1);
            }
        }

        if (sk_tools::is_percentage(heightString)) {
            tileBenchmark->setTileHeightPercentage(atof(heightString));
            if (!(tileBenchmark->getTileHeightPercentage() > 0)) {
                SkDELETE(tileBenchmark);
                gLogger.logError("--mode tile must be given a height percentage > 0\n");
                exit(-1);
            }
        } else {
            tileBenchmark->setTileHeight(atoi(heightString));
            if (!(tileBenchmark->getTileHeight() > 0)) {
                SkDELETE(tileBenchmark);
                gLogger.logError("--mode tile must be given a height > 0\n");
                exit(-1);
            }
        }
        tileBenchmark->setThreading(multiThreaded);
        tileBenchmark->setUsePipe(usePipe);
        benchmark = tileBenchmark;
    } else if (usePipe) {
        SkDELETE(benchmark);
        benchmark = SkNEW(sk_tools::PipePictureBenchmark);
    }
    if (inputs->count() < 1) {
        SkDELETE(benchmark);
        usage(argv0);
        exit(-1);
    }

    if (NULL == benchmark) {
        benchmark = SkNEW(sk_tools::SimplePictureBenchmark);
    }

    benchmark->setRepeats(repeats);
    benchmark->setDeviceType(deviceType);
    benchmark->setLogger(&gLogger);
    // Report current settings:
    gLogger.logProgress(commandLine);
}

static void process_input(const SkString& input, sk_tools::PictureBenchmark& benchmark) {
    SkOSFile::Iter iter(input.c_str(), "skp");
    SkString inputFilename;

    if (iter.next(&inputFilename)) {
        do {
            SkString inputPath;
            sk_tools::make_filepath(&inputPath, input, inputFilename);
            run_single_benchmark(inputPath, benchmark);
        } while(iter.next(&inputFilename));
    } else {
          run_single_benchmark(input, benchmark);
    }
}

int main(int argc, char* const argv[]) {
    SkAutoGraphics ag;
    SkTArray<SkString> inputs;
    sk_tools::PictureBenchmark* benchmark = NULL;

    parse_commandline(argc, argv, &inputs, benchmark);

    for (int i = 0; i < inputs.count(); ++i) {
        process_input(inputs[i], *benchmark);
    }

    SkDELETE(benchmark);
}
