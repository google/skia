/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkMath.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTArray.h"
#include "PictureRenderer.h"
#include "picture_utils.h"

static void usage(const char* argv0) {
    SkDebugf("SkPicture rendering tool\n");
    SkDebugf("\n"
"Usage: \n"
"     %s <input>... <outputDir> \n"
"     [--mode pow2tile minWidth height[%] | simple\n"
"         | tile width[%] height[%]]\n"
"     [--pipe]\n"
"     [--multi count]\n"
"     [--device bitmap"
#if SK_SUPPORT_GPU
" | gpu"
#endif
"]"
, argv0);
    SkDebugf("\n\n");
    SkDebugf(
"     input:     A list of directories and files to use as input. Files are\n"
"                expected to have the .skp extension.\n\n");
    SkDebugf(
"     outputDir: directory to write the rendered images.\n\n");
    SkDebugf(
"     --mode pow2tile minWidth height[%] | simple\n"
"          | tile width[%] height[%]: Run in the corresponding mode.\n"
"                                     Default is simple.\n");
    SkDebugf(
"                     pow2tile minWidth height[%], Creates tiles with widths\n"
"                                                  that are all a power of two\n"
"                                                  such that they minimize the\n"
"                                                  amount of wasted tile space.\n"
"                                                  minWidth is the minimum width\n"
"                                                  of these tiles and must be a\n"
"                                                  power of two. A simple render\n"
"                                                  is done with these tiles.\n");
    SkDebugf(
"                     simple, Render using the default rendering method.\n");
    SkDebugf(
"                     tile width[%] height[%], Do a simple render using tiles\n"
"                                              with the given dimensions.\n");
    SkDebugf("\n");
    SkDebugf(
"     --multi count : Set the number of threads for multi threaded drawing. Must be greater\n"
"                     than 1. Only works with tiled rendering.\n"
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
}

static void make_output_filepath(SkString* path, const SkString& dir,
                                 const SkString& name) {
    sk_tools::make_filepath(path, dir, name);
    // Remove ".skp"
    path->remove(path->size() - 4, 4);
}

static bool render_picture(const SkString& inputPath, const SkString& outputDir,
                           sk_tools::PictureRenderer& renderer) {
    SkString inputFilename;
    sk_tools::get_basename(&inputFilename, inputPath);

    SkFILEStream inputStream;
    inputStream.setPath(inputPath.c_str());
    if (!inputStream.isValid()) {
        SkDebugf("Could not open file %s\n", inputPath.c_str());
        return false;
    }

    bool success = false;
    SkPicture picture(&inputStream, &success, &SkImageDecoder::DecodeStream);
    if (!success) {
        SkDebugf("Could not read an SkPicture from %s\n", inputPath.c_str());
        return false;
    }

    SkDebugf("drawing... [%i %i] %s\n", picture.width(), picture.height(),
             inputPath.c_str());

    renderer.init(&picture);
    renderer.setup();

    SkString outputPath;
    make_output_filepath(&outputPath, outputDir, inputFilename);

    success = renderer.render(&outputPath);
    if (!success) {
        SkDebugf("Could not write to file %s\n", outputPath.c_str());
    }

    renderer.resetState();

    renderer.end();
    return success;
}

static int process_input(const SkString& input, const SkString& outputDir,
                          sk_tools::PictureRenderer& renderer) {
    SkOSFile::Iter iter(input.c_str(), "skp");
    SkString inputFilename;
    int failures = 0;
    if (iter.next(&inputFilename)) {
        do {
            SkString inputPath;
            sk_tools::make_filepath(&inputPath, input, inputFilename);
            if (!render_picture(inputPath, outputDir, renderer)) {
                ++failures;
            }
        } while(iter.next(&inputFilename));
    } else if (SkStrEndsWith(input.c_str(), ".skp")) {
        SkString inputPath(input);
        if (!render_picture(inputPath, outputDir, renderer)) {
            ++failures;
        }
    } else {
        SkString warning;
        warning.printf("Warning: skipping %s\n", input.c_str());
        SkDebugf(warning.c_str());
    }
    return failures;
}

static void parse_commandline(int argc, char* const argv[], SkTArray<SkString>* inputs,
                              sk_tools::PictureRenderer*& renderer){
    const char* argv0 = argv[0];
    char* const* stop = argv + argc;

    sk_tools::PictureRenderer::SkDeviceTypes deviceType =
        sk_tools::PictureRenderer::kBitmap_DeviceType;

    bool usePipe = false;
    int numThreads = 1;
    bool useTiles = false;
    const char* widthString = NULL;
    const char* heightString = NULL;
    bool isPowerOf2Mode = false;
    const char* mode = NULL;

    for (++argv; argv < stop; ++argv) {
        if (0 == strcmp(*argv, "--mode")) {
            SkDELETE(renderer);

            ++argv;
            if (argv >= stop) {
                SkDebugf("Missing mode for --mode\n");
                usage(argv0);
                exit(-1);
            }

            if (0 == strcmp(*argv, "simple")) {
                renderer = SkNEW(sk_tools::SimplePictureRenderer);
            } else if ((0 == strcmp(*argv, "tile")) || (0 == strcmp(*argv, "pow2tile"))) {
                useTiles = true;
                mode = *argv;

                if (0 == strcmp(*argv, "pow2tile")) {
                    isPowerOf2Mode = true;
                }

                ++argv;
                if (argv >= stop) {
                    SkDebugf("Missing width for --mode %s\n", mode);
                    usage(argv0);
                    exit(-1);
                }

                widthString = *argv;
                ++argv;
                if (argv >= stop) {
                    SkDebugf("Missing height for --mode tile\n");
                    usage(argv0);
                    exit(-1);
                }
                heightString = *argv;
            } else {
                SkDebugf("%s is not a valid mode for --mode\n", *argv);
                usage(argv0);
                exit(-1);
            }
        } else if (0 == strcmp(*argv, "--pipe")) {
            usePipe = true;
        } else if (0 == strcmp(*argv, "--multi")) {
            ++argv;
            if (argv >= stop) {
                SkDebugf("Missing arg for --multi\n");
                usage(argv0);
                exit(-1);
            }
            numThreads = atoi(*argv);
            if (numThreads < 2) {
                SkDebugf("Number of threads must be at least 2.\n");
                usage(argv0);
                exit(-1);
            }
        } else if (0 == strcmp(*argv, "--device")) {
            ++argv;
            if (argv >= stop) {
                SkDebugf("Missing mode for --device\n");
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

        } else if ((0 == strcmp(*argv, "-h")) || (0 == strcmp(*argv, "--help"))) {
            SkDELETE(renderer);
            usage(argv0);
            exit(-1);
        } else {
            inputs->push_back(SkString(*argv));
        }
    }

    if (numThreads > 1 && !useTiles) {
        SkDebugf("Multithreaded drawing requires tiled rendering.\n");
        usage(argv0);
        exit(-1);
    }

    if (useTiles) {
        SkASSERT(NULL == renderer);
        sk_tools::TiledPictureRenderer* tiledRenderer = SkNEW(sk_tools::TiledPictureRenderer);
        if (isPowerOf2Mode) {
            int minWidth = atoi(widthString);
            if (!SkIsPow2(minWidth) || minWidth < 0) {
                tiledRenderer->unref();
                SkString err;
                err.printf("-mode %s must be given a width"
                           " value that is a power of two\n", mode);
                SkDebugf(err.c_str());
                usage(argv0);
                exit(-1);
            }
            tiledRenderer->setTileMinPowerOf2Width(minWidth);
        } else if (sk_tools::is_percentage(widthString)) {
            tiledRenderer->setTileWidthPercentage(atof(widthString));
            if (!(tiledRenderer->getTileWidthPercentage() > 0)) {
                tiledRenderer->unref();
                SkDebugf("--mode tile must be given a width percentage > 0\n");
                usage(argv0);
                exit(-1);
            }
        } else {
            tiledRenderer->setTileWidth(atoi(widthString));
            if (!(tiledRenderer->getTileWidth() > 0)) {
                tiledRenderer->unref();
                SkDebugf("--mode tile must be given a width > 0\n");
                usage(argv0);
                exit(-1);
            }
        }

        if (sk_tools::is_percentage(heightString)) {
            tiledRenderer->setTileHeightPercentage(atof(heightString));
            if (!(tiledRenderer->getTileHeightPercentage() > 0)) {
                tiledRenderer->unref();
                SkDebugf("--mode tile must be given a height percentage > 0\n");
                usage(argv0);
                exit(-1);
            }
        } else {
            tiledRenderer->setTileHeight(atoi(heightString));
            if (!(tiledRenderer->getTileHeight() > 0)) {
                tiledRenderer->unref();
                SkDebugf("--mode tile must be given a height > 0\n");
                usage(argv0);
                exit(-1);
            }
        }
        if (numThreads > 1) {
#if SK_SUPPORT_GPU
            if (sk_tools::PictureRenderer::kGPU_DeviceType == deviceType) {
                tiledRenderer->unref();
                SkDebugf("GPU not compatible with multithreaded tiling.\n");
                usage(argv0);
                exit(-1);
            }
#endif
            tiledRenderer->setNumberOfThreads(numThreads);
        }
        tiledRenderer->setUsePipe(usePipe);
        renderer = tiledRenderer;
    } else if (usePipe) {
        renderer = SkNEW(sk_tools::PipePictureRenderer);
    }

    if (inputs->count() < 2) {
        SkDELETE(renderer);
        usage(argv0);
        exit(-1);
    }

    if (NULL == renderer) {
        renderer = SkNEW(sk_tools::SimplePictureRenderer);
    }

    renderer->setDeviceType(deviceType);
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkAutoGraphics ag;
    SkTArray<SkString> inputs;
    sk_tools::PictureRenderer* renderer = NULL;

    parse_commandline(argc, argv, &inputs, renderer);
    SkString outputDir = inputs[inputs.count() - 1];
    SkASSERT(renderer);

    int failures = 0;
    for (int i = 0; i < inputs.count() - 1; i ++) {
        failures += process_input(inputs[i], outputDir, *renderer);
    }
    if (failures != 0) {
        SkDebugf("Failed to render %i pictures.\n", failures);
        return 1;
    }
#if SK_SUPPORT_GPU
#if GR_CACHE_STATS
    if (renderer->isUsingGpuDevice()) {
        GrContext* ctx = renderer->getGrContext();

        ctx->printCacheStats();
    }
#endif
#endif

    SkDELETE(renderer);
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
