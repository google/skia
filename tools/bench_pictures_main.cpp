/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "BenchTimer.h"
#include "PictureBenchmark.h"
#include "SkCanvas.h"
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
"     [--repeat] \n"
"     [--mode pipe | record | simple | tile width[%] height[%] | unflatten]\n"
"     [--device bitmap"
#if SK_SUPPORT_GPU
" | gpu"
#endif
"]"
, argv0);
    SkDebugf("\n\n");
    SkDebugf(
"     inputDir:  A list of directories and files to use as input. Files are\n"
"                expected to have the .skp extension.\n\n");
    SkDebugf(
"     --mode pipe | record | simple | tile width[%] height[%] | unflatten: Run\n"
"                in the corresponding mode. Default is simple.\n");
    SkDebugf(
"                     pipe, Benchmark SkGPipe rendering.\n");
    SkDebugf(
"                     record, Benchmark picture to picture recording.\n");
    SkDebugf(
"                     simple, Benchmark a simple rendering.\n");
    SkDebugf(
"                     tile width[%] height[%], Benchmark simple rendering using\n"
"                                              tiles with the given dimensions.\n");
    SkDebugf(
"                     unflatten, Benchmark picture unflattening.\n");
    SkDebugf("\n");
    SkDebugf(
"     --device bitmap"
#if SK_SUPPORT_GPU
" | gpu"
#endif
": Use the corresponding device. Default is bitmap.\n");
    SkDebugf(
"                     bitmap, Render to a bitmap.\n");
    SkDebugf(
#if SK_SUPPORT_GPU
"                     gpu, Render to the GPU.\n");
#endif
    SkDebugf("\n");
    SkDebugf(
"     --repeat:  "
"Set the number of times to repeat each test."
" Default is %i.\n", DEFAULT_REPEATS);
}

static void run_single_benchmark(const SkString& inputPath,
                                 sk_tools::PictureBenchmark& benchmark) {
    SkFILEStream inputStream;

    inputStream.setPath(inputPath.c_str());
    if (!inputStream.isValid()) {
        SkDebugf("Could not open file %s\n", inputPath.c_str());
        return;
    }

    SkPicture picture(&inputStream);

    SkString filename;
    sk_tools::get_basename(&filename, inputPath);
    SkDebugf("running bench [%i %i] %s ", picture.width(), picture.height(),
           filename.c_str());

    benchmark.run(&picture);
}

static void parse_commandline(int argc, char* const argv[], SkTArray<SkString>* inputs,
                              sk_tools::PictureBenchmark*& benchmark) {
    const char* argv0 = argv[0];
    char* const* stop = argv + argc;

    int repeats = DEFAULT_REPEATS;
    sk_tools::PictureRenderer::SkDeviceTypes deviceType =
        sk_tools::PictureRenderer::kBitmap_DeviceType;

    for (++argv; argv < stop; ++argv) {
        if (0 == strcmp(*argv, "--repeat")) {
            ++argv;
            if (argv < stop) {
                repeats = atoi(*argv);
                if (repeats < 1) {
                    SkDELETE(benchmark);
                    SkDebugf("--repeat must be given a value > 0\n");
                    exit(-1);
                }
            } else {
                SkDELETE(benchmark);
                SkDebugf("Missing arg for --repeat\n");
                usage(argv0);
                exit(-1);
            }
        } else if (0 == strcmp(*argv, "--mode")) {
            SkDELETE(benchmark);

            ++argv;
            if (argv >= stop) {
                SkDebugf("Missing mode for --mode\n");
                usage(argv0);
                exit(-1);
            }

            if (0 == strcmp(*argv, "pipe")) {
                benchmark = SkNEW(sk_tools::PipePictureBenchmark);
            } else if (0 == strcmp(*argv, "record")) {
                benchmark = SkNEW(sk_tools::RecordPictureBenchmark);
            } else if (0 == strcmp(*argv, "simple")) {
                benchmark = SkNEW(sk_tools::SimplePictureBenchmark);
            } else if (0 == strcmp(*argv, "tile")) {
                sk_tools::TiledPictureBenchmark* tileBenchmark =
                    SkNEW(sk_tools::TiledPictureBenchmark);
                ++argv;
                if (argv >= stop) {
                    SkDELETE(tileBenchmark);
                    SkDebugf("Missing width for --mode tile\n");
                    usage(argv0);
                    exit(-1);
                }

                if (sk_tools::is_percentage(*argv)) {
                    tileBenchmark->setTileWidthPercentage(atof(*argv));
                    if (!(tileBenchmark->getTileWidthPercentage() > 0)) {
                        SkDELETE(tileBenchmark);
                        SkDebugf("--mode tile must be given a width percentage > 0\n");
                        exit(-1);
                    }
                } else {
                    tileBenchmark->setTileWidth(atoi(*argv));
                    if (!(tileBenchmark->getTileWidth() > 0)) {
                        SkDELETE(tileBenchmark);
                        SkDebugf("--mode tile must be given a width > 0\n");
                        exit(-1);
                    }
                }

                ++argv;
                if (argv >= stop) {
                    SkDELETE(tileBenchmark);
                    SkDebugf("Missing height for --mode tile\n");
                    usage(argv0);
                    exit(-1);
                }

                if (sk_tools::is_percentage(*argv)) {
                    tileBenchmark->setTileHeightPercentage(atof(*argv));
                    if (!(tileBenchmark->getTileHeightPercentage() > 0)) {
                        SkDELETE(tileBenchmark);
                        SkDebugf("--mode tile must be given a height percentage > 0\n");
                        exit(-1);
                    }
                } else {
                    tileBenchmark->setTileHeight(atoi(*argv));
                    if (!(tileBenchmark->getTileHeight() > 0)) {
                        SkDELETE(tileBenchmark);
                        SkDebugf("--mode tile must be given a height > 0\n");
                        exit(-1);
                    }
                }

                benchmark = tileBenchmark;
            } else if (0 == strcmp(*argv, "unflatten")) {
                benchmark = SkNEW(sk_tools::UnflattenPictureBenchmark);
            } else {
                SkDebugf("%s is not a valid mode for --mode\n", *argv);
                usage(argv0);
                exit(-1);
            }
        }  else if (0 == strcmp(*argv, "--device")) {
            ++argv;
            if (argv >= stop) {
                SkDebugf("Missing mode for --deivce\n");
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
                SkDebugf("%s is not a valid mode for --device\n", *argv);
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
    SkTArray<SkString> inputs;
    sk_tools::PictureBenchmark* benchmark = NULL;

    parse_commandline(argc, argv, &inputs, benchmark);

    for (int i = 0; i < inputs.count(); ++i) {
        process_input(inputs[i], *benchmark);
    }

    SkDELETE(benchmark);
}
