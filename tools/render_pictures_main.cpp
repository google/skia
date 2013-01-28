/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CopyTilesRenderer.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
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
"     %s <input>... \n"
"     [-w <outputDir>]\n"
"     [--mode pow2tile minWidth height | copyTile width height | simple\n"
"         | tile width height]\n"
"     [--pipe]\n"
"     [--bbh bbhType]\n"
"     [--multi count]\n"
"     [--validate [--maxComponentDiff n]]\n"
"     [--writeWholeImage]\n"
"     [--clone n]\n"
"     [--viewport width height][--scale sf]\n"
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
"     --mode pow2tile minWidth height | copyTile width height | simple\n"
"          | tile width height | rerecord: Run in the corresponding mode.\n"
"                                     Default is simple.\n");
    SkDebugf(
"                     pow2tile minWidth height, Creates tiles with widths\n"
"                                                  that are all a power of two\n"
"                                                  such that they minimize the\n"
"                                                  amount of wasted tile space.\n"
"                                                  minWidth is the minimum width\n"
"                                                  of these tiles and must be a\n"
"                                                  power of two. A simple render\n"
"                                                  is done with these tiles.\n");
    SkDebugf(
"                     simple, Render using the default rendering method.\n"
"                     rerecord, Record the picture as a new skp, with the bitmaps PNG encoded.\n"
             );
    SkDebugf(
"                     tile width height, Do a simple render using tiles\n"
"                                              with the given dimensions.\n"
"                     copyTile width height, Draw the picture, then copy it into tiles.\n"
"                                                Does not support percentages.\n"
"                                                If the picture is large enough, breaks it into\n"
"                                                larger tiles (and draws the picture once per\n"
"                                                larger tile) to avoid creating a large canvas.\n"
"                                                Add --tiles x y to specify the number of tiles\n"
"                                                per larger tile in the x and y direction.\n"
             );
    SkDebugf("\n");
    SkDebugf(
"     --multi count : Set the number of threads for multi threaded drawing. Must be greater\n"
"                     than 1. Only works with tiled rendering.\n"
"     --viewport width height : Set the viewport.\n"
"     --scale sf : Scale drawing by sf.\n"
"     --pipe: Benchmark SkGPipe rendering. Currently incompatible with \"mode\".\n");
    SkDebugf(
"     --validate: Verify that the rendered image contains the same pixels as "
"the picture rendered in simple mode.\n"
"     --maxComponentDiff: maximum diff on a component. Default is 256, "
"which means we report but we do not generate an error.\n"
"     --writeWholeImage: In tile mode, write the entire rendered image to a "
"file, instead of an image for each tile.\n");
    SkDebugf(
"     --clone n: Clone the picture n times before rendering.\n");
    SkDebugf(
"     --bbh bbhType [width height]: Set the bounding box hierarchy type to\n"
"                     be used. Accepted values are: none, rtree, grid. Default\n"
"                     value is none. Not compatible with --pipe. With value\n"
"                     'grid', width and height must be specified. 'grid' can\n"
"                     only be used with modes tile, record, and\n"
"                     playbackCreation.");
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

static bool render_picture(const SkString& inputPath, const SkString* outputDir,
                           sk_tools::PictureRenderer& renderer,
                           SkBitmap** out,
                           int clones) {
    SkString inputFilename;
    sk_tools::get_basename(&inputFilename, inputPath);

    SkFILEStream inputStream;
    inputStream.setPath(inputPath.c_str());
    if (!inputStream.isValid()) {
        SkDebugf("Could not open file %s\n", inputPath.c_str());
        return false;
    }

    bool success = false;
    SkPicture* picture = SkNEW_ARGS(SkPicture,
            (&inputStream, &success, &SkImageDecoder::DecodeStream));
    if (!success) {
        SkDebugf("Could not read an SkPicture from %s\n", inputPath.c_str());
        return false;
    }

    for (int i = 0; i < clones; ++i) {
        SkPicture* clone = picture->clone();
        SkDELETE(picture);
        picture = clone;
    }

    SkDebugf("drawing... [%i %i] %s\n", picture->width(), picture->height(),
             inputPath.c_str());

    renderer.init(picture);
    renderer.setup();

    SkString* outputPath = NULL;
    if (NULL != outputDir) {
        outputPath = SkNEW(SkString);
        make_output_filepath(outputPath, *outputDir, inputFilename);
    }

    success = renderer.render(outputPath, out);
    if (outputPath) {
        if (!success) {
            SkDebugf("Could not write to file %s\n", outputPath->c_str());
        }
        SkDELETE(outputPath);
    }

    renderer.end();

    SkDELETE(picture);
    return success;
}

static inline int getByte(uint32_t value, int index) {
    SkASSERT(0 <= index && index < 4);
    return (value >> (index * 8)) & 0xFF;
}

static int MaxByteDiff(uint32_t v1, uint32_t v2) {
    return SkMax32(SkMax32(abs(getByte(v1, 0) - getByte(v2, 0)), abs(getByte(v1, 1) - getByte(v2, 1))),
                   SkMax32(abs(getByte(v1, 2) - getByte(v2, 2)), abs(getByte(v1, 3) - getByte(v2, 3))));
}

static bool render_picture(const SkString& inputPath, const SkString* outputDir,
                           sk_tools::PictureRenderer& renderer,
                           bool validate, int maxComponentDiff,
                           bool writeWholeImage,
                           int clones) {
    int diffs[256] = {0};
    SkBitmap* bitmap = NULL;
    bool success = render_picture(inputPath,
        writeWholeImage ? NULL : outputDir,
        renderer,
        validate || writeWholeImage ? &bitmap : NULL, clones);

    if (!success || ((validate || writeWholeImage) && bitmap == NULL)) {
        SkDebugf("Failed to draw the picture.\n");
        SkDELETE(bitmap);
        return false;
    }

    if (validate) {
        SkBitmap* referenceBitmap = NULL;
        sk_tools::SimplePictureRenderer referenceRenderer;
        success = render_picture(inputPath, NULL, referenceRenderer,
                                 &referenceBitmap, 0);

        if (!success || !referenceBitmap) {
            SkDebugf("Failed to draw the reference picture.\n");
            SkDELETE(bitmap);
            SkDELETE(referenceBitmap);
            return false;
        }

        if (success && (bitmap->width() != referenceBitmap->width())) {
            SkDebugf("Expected image width: %i, actual image width %i.\n",
                     referenceBitmap->width(), bitmap->width());
            SkDELETE(bitmap);
            SkDELETE(referenceBitmap);
            return false;
        }
        if (success && (bitmap->height() != referenceBitmap->height())) {
            SkDebugf("Expected image height: %i, actual image height %i",
                     referenceBitmap->height(), bitmap->height());
            SkDELETE(bitmap);
            SkDELETE(referenceBitmap);
            return false;
        }

        for (int y = 0; success && y < bitmap->height(); y++) {
            for (int x = 0; success && x < bitmap->width(); x++) {
                int diff = MaxByteDiff(*referenceBitmap->getAddr32(x, y),
                                       *bitmap->getAddr32(x, y));
                SkASSERT(diff >= 0 && diff <= 255);
                diffs[diff]++;

                if (diff > maxComponentDiff) {
                    SkDebugf("Expected pixel at (%i %i) exceedds maximum "
                                 "component diff of %i: 0x%x, actual 0x%x\n",
                             x, y, maxComponentDiff,
                             *referenceBitmap->getAddr32(x, y),
                             *bitmap->getAddr32(x, y));
                    SkDELETE(bitmap);
                    SkDELETE(referenceBitmap);
                    return false;
                }
            }
        }
        SkDELETE(referenceBitmap);

        for (int i = 1; i <= 255; ++i) {
            if(diffs[i] > 0) {
                SkDebugf("Number of pixels with max diff of %i is %i\n", i, diffs[i]);
            }
        }
    }

    if (writeWholeImage) {
        sk_tools::force_all_opaque(*bitmap);
        if (NULL != outputDir && writeWholeImage) {
            SkString inputFilename;
            sk_tools::get_basename(&inputFilename, inputPath);
            SkString outputPath;
            make_output_filepath(&outputPath, *outputDir, inputFilename);
            outputPath.append(".png");
            if (!SkImageEncoder::EncodeFile(outputPath.c_str(), *bitmap,
                                            SkImageEncoder::kPNG_Type, 100)) {
                SkDebugf("Failed to draw the picture.\n");
                success = false;
            }
        }
    }
    SkDELETE(bitmap);

    return success;
}


static int process_input(const SkString& input, const SkString* outputDir,
                         sk_tools::PictureRenderer& renderer,
                         bool validate, int maxComponentDiff,
                         bool writeWholeImage, int clones) {
    SkOSFile::Iter iter(input.c_str(), "skp");
    SkString inputFilename;
    int failures = 0;
    SkDebugf("process_input, %s\n", input.c_str());
    if (iter.next(&inputFilename)) {
        do {
            SkString inputPath;
            sk_tools::make_filepath(&inputPath, input, inputFilename);
            if (!render_picture(inputPath, outputDir, renderer,
                                validate, maxComponentDiff,
                                writeWholeImage, clones)) {
                ++failures;
            }
        } while(iter.next(&inputFilename));
    } else if (SkStrEndsWith(input.c_str(), ".skp")) {
        SkString inputPath(input);
        if (!render_picture(inputPath, outputDir, renderer,
                            validate, maxComponentDiff,
                            writeWholeImage, clones)) {
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
                              sk_tools::PictureRenderer*& renderer, SkString*& outputDir,
                              bool* validate, int* maxComponentDiff,
                              bool* writeWholeImage,
                              int* clones){
    const char* argv0 = argv[0];
    char* const* stop = argv + argc;

    sk_tools::PictureRenderer::SkDeviceTypes deviceType =
        sk_tools::PictureRenderer::kBitmap_DeviceType;

    bool usePipe = false;
    int numThreads = 1;
    bool useTiles = false;
    const char* widthString = NULL;
    const char* heightString = NULL;
    int gridWidth = 0;
    int gridHeight = 0;
    bool isPowerOf2Mode = false;
    bool isCopyMode = false;
    const char* xTilesString = NULL;
    const char* yTilesString = NULL;
    const char* mode = NULL;
    bool gridSupported = false;
    sk_tools::PictureRenderer::BBoxHierarchyType bbhType =
        sk_tools::PictureRenderer::kNone_BBoxHierarchyType;
    *validate = false;
    *maxComponentDiff = 256;
    *writeWholeImage = false;
    *clones = 0;
    SkISize viewport;
    viewport.setEmpty();
    SkScalar scaleFactor = SK_Scalar1;

    for (++argv; argv < stop; ++argv) {
        if (0 == strcmp(*argv, "--mode")) {
            if (renderer != NULL) {
                renderer->unref();
                SkDebugf("Cannot combine modes.\n");
                usage(argv0);
                exit(-1);
            }

            ++argv;
            if (argv >= stop) {
                SkDebugf("Missing mode for --mode\n");
                usage(argv0);
                exit(-1);
            }

            if (0 == strcmp(*argv, "simple")) {
                renderer = SkNEW(sk_tools::SimplePictureRenderer);
            } else if ((0 == strcmp(*argv, "tile")) || (0 == strcmp(*argv, "pow2tile"))
                       || 0 == strcmp(*argv, "copyTile")) {
                useTiles = true;
                mode = *argv;

                if (0 == strcmp(*argv, "pow2tile")) {
                    isPowerOf2Mode = true;
                } else if (0 == strcmp(*argv, "copyTile")) {
                    isCopyMode = true;
                } else {
                    gridSupported = true;
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
                    SkDebugf("Missing height for --mode %s\n", mode);
                    usage(argv0);
                    exit(-1);
                }
                heightString = *argv;
            } else if (0 == strcmp(*argv, "rerecord")) {
                renderer = SkNEW(sk_tools::RecordPictureRenderer);
            } else {
                SkDebugf("%s is not a valid mode for --mode\n", *argv);
                usage(argv0);
                exit(-1);
            }
        } else if (0 == strcmp(*argv, "--bbh")) {
            ++argv;
            if (argv >= stop) {
                SkDebugf("Missing value for --bbh\n");
                usage(argv0);
                exit(-1);
            }
            if (0 == strcmp(*argv, "none")) {
                bbhType = sk_tools::PictureRenderer::kNone_BBoxHierarchyType;
            } else if (0 == strcmp(*argv, "rtree")) {
                bbhType = sk_tools::PictureRenderer::kRTree_BBoxHierarchyType;
            } else if (0 == strcmp(*argv, "grid")) {
                bbhType = sk_tools::PictureRenderer::kTileGrid_BBoxHierarchyType;
                ++argv;
                if (argv >= stop) {
                    SkDebugf("Missing width for --bbh grid\n");
                    usage(argv0);
                    exit(-1);
                }
                gridWidth = atoi(*argv);
                ++argv;
                if (argv >= stop) {
                    SkDebugf("Missing height for --bbh grid\n");
                    usage(argv0);
                    exit(-1);
                }
                gridHeight = atoi(*argv);
            } else {
                SkDebugf("%s is not a valid value for --bbhType\n", *argv);
                usage(argv0);
                exit(-1);;
            }
        } else if (0 == strcmp(*argv, "--viewport")) {
            ++argv;
            if (argv >= stop) {
                SkDebugf("Missing width for --viewport\n");
                usage(argv0);
                exit(-1);
            }
            viewport.fWidth = atoi(*argv);
            ++argv;
            if (argv >= stop) {
                SkDebugf("Missing height for --viewport\n");
                usage(argv0);
                exit(-1);
            }
            viewport.fHeight = atoi(*argv);
        } else if (0 == strcmp(*argv, "--scale")) {
            ++argv;
            if (argv >= stop) {
                SkDebugf("Missing scaleFactor for --scale\n");
                usage(argv0);
                exit(-1);
            }
            scaleFactor = SkDoubleToScalar(atof(*argv));
        } else if (0 == strcmp(*argv, "--tiles")) {
            ++argv;
            if (argv >= stop) {
                SkDebugf("Missing x for --tiles\n");
                usage(argv0);
                exit(-1);
            }
            xTilesString = *argv;
            ++argv;
            if (argv >= stop) {
                SkDebugf("Missing y for --tiles\n");
                usage(argv0);
                exit(-1);
            }
            yTilesString = *argv;
        } else if (0 == strcmp(*argv, "--pipe")) {
            usePipe = true;
        } else if (0 == strcmp(*argv, "--multi")) {
            ++argv;
            if (argv >= stop) {
                SkSafeUnref(renderer);
                SkDebugf("Missing arg for --multi\n");
                usage(argv0);
                exit(-1);
            }
            numThreads = atoi(*argv);
            if (numThreads < 2) {
                SkSafeUnref(renderer);
                SkDebugf("Number of threads must be at least 2.\n");
                usage(argv0);
                exit(-1);
            }
        } else if (0 == strcmp(*argv, "--clone")) {
            ++argv;
            if (argv >= stop) {
                SkSafeUnref(renderer);
                SkDebugf("Missing arg for --clone\n");
                usage(argv0);
                exit(-1);
            }
            *clones = atoi(*argv);
            if (*clones < 0) {
                SkSafeUnref(renderer);
                SkDebugf("Number of clones must be at least 0.\n");
                usage(argv0);
                exit(-1);
            }
        } else if (0 == strcmp(*argv, "--device")) {
            ++argv;
            if (argv >= stop) {
                SkSafeUnref(renderer);
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
                SkSafeUnref(renderer);
                SkDebugf("%s is not a valid mode for --device\n", *argv);
                usage(argv0);
                exit(-1);
            }

        } else if ((0 == strcmp(*argv, "-h")) || (0 == strcmp(*argv, "--help"))) {
            SkSafeUnref(renderer);
            usage(argv0);
            exit(-1);
        } else if (0 == strcmp(*argv, "-w")) {
            ++argv;
            if (argv >= stop) {
                SkDebugf("Missing output directory for -w\n");
                usage(argv0);
                exit(-1);
            }
            outputDir = SkNEW_ARGS(SkString, (*argv));
        } else if (0 == strcmp(*argv, "--validate")) {
            *validate = true;
        } else if (0 == strcmp(*argv, "--maxComponentDiff")) {
            if (!*validate) {
                SkDebugf("--maxComponentDiff must be used only with --validate\n");
                usage(argv0);
                exit(-1);
            }
            ++argv;
            if (argv >= stop) {
                SkDebugf("Missing arg for --maxComponentDiff\n");
                usage(argv0);
                exit(-1);
            }
            *maxComponentDiff = atoi(*argv);
            if (*maxComponentDiff < 0 || *maxComponentDiff > 256) {
                SkSafeUnref(renderer);
                SkDebugf("maxComponentDiff: 0 - 256.\n");
                usage(argv0);
                exit(-1);
            }
        } else if (0 == strcmp(*argv, "--writeWholeImage")) {
            *writeWholeImage = true;
        } else {
            inputs->push_back(SkString(*argv));
        }
    }

    if (numThreads > 1 && !useTiles) {
        SkSafeUnref(renderer);
        SkDebugf("Multithreaded drawing requires tiled rendering.\n");
        usage(argv0);
        exit(-1);
    }

    if (usePipe && sk_tools::PictureRenderer::kNone_BBoxHierarchyType != bbhType) {
        SkDebugf("--pipe and --bbh cannot be used together\n");
        usage(argv0);
        exit(-1);
    }

    if (sk_tools::PictureRenderer::kTileGrid_BBoxHierarchyType == bbhType &&
        !gridSupported) {
        SkDebugf("'--bbh grid' is not compatible with specified --mode.\n");
        usage(argv0);
        exit(-1);
    }

    if (useTiles) {
        SkASSERT(NULL == renderer);
        sk_tools::TiledPictureRenderer* tiledRenderer;
        if (isCopyMode) {
            int x, y;
            if (xTilesString != NULL) {
                SkASSERT(yTilesString != NULL);
                x = atoi(xTilesString);
                y = atoi(yTilesString);
                if (x <= 0 || y <= 0) {
                    SkDebugf("--tiles must be given values > 0\n");
                    usage(argv0);
                    exit(-1);
                }
            } else {
                x = y = 4;
            }
            tiledRenderer = SkNEW_ARGS(sk_tools::CopyTilesRenderer, (x, y));
        } else if (numThreads > 1) {
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
                SkDebugf(err.c_str());
                usage(argv0);
                exit(-1);
            }
            tiledRenderer->setTileMinPowerOf2Width(minWidth);
        } else if (sk_tools::is_percentage(widthString)) {
            if (isCopyMode) {
                tiledRenderer->unref();
                SkString err;
                err.printf("--mode %s does not support percentages.\n", mode);
                SkDebugf(err.c_str());
                usage(argv0);
                exit(-1);
            }
            tiledRenderer->setTileWidthPercentage(atof(widthString));
            if (!(tiledRenderer->getTileWidthPercentage() > 0)) {
                tiledRenderer->unref();
                SkDebugf("--mode %s must be given a width percentage > 0\n", mode);
                usage(argv0);
                exit(-1);
            }
        } else {
            tiledRenderer->setTileWidth(atoi(widthString));
            if (!(tiledRenderer->getTileWidth() > 0)) {
                tiledRenderer->unref();
                SkDebugf("--mode %s must be given a width > 0\n", mode);
                usage(argv0);
                exit(-1);
            }
        }

        if (sk_tools::is_percentage(heightString)) {
            if (isCopyMode) {
                tiledRenderer->unref();
                SkString err;
                err.printf("--mode %s does not support percentages.\n", mode);
                SkDebugf(err.c_str());
                usage(argv0);
                exit(-1);
            }
            tiledRenderer->setTileHeightPercentage(atof(heightString));
            if (!(tiledRenderer->getTileHeightPercentage() > 0)) {
                tiledRenderer->unref();
                SkDebugf("--mode %s must be given a height percentage > 0\n", mode);
                usage(argv0);
                exit(-1);
            }
        } else {
            tiledRenderer->setTileHeight(atoi(heightString));
            if (!(tiledRenderer->getTileHeight() > 0)) {
                tiledRenderer->unref();
                SkDebugf("--mode %s must be given a height > 0\n", mode);
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
        }
        renderer = tiledRenderer;
        if (usePipe) {
            SkDebugf("Pipe rendering is currently not compatible with tiling.\n"
                     "Turning off pipe.\n");
        }
    } else if (usePipe) {
        if (renderer != NULL) {
            renderer->unref();
            SkDebugf("Pipe is incompatible with other modes.\n");
            usage(argv0);
            exit(-1);
        }
        renderer = SkNEW(sk_tools::PipePictureRenderer);
    }

    if (inputs->empty()) {
        SkSafeUnref(renderer);
        if (NULL != outputDir) {
            SkDELETE(outputDir);
        }
        usage(argv0);
        exit(-1);
    }

    if (NULL == renderer) {
        renderer = SkNEW(sk_tools::SimplePictureRenderer);
    }

    renderer->setBBoxHierarchyType(bbhType);
    renderer->setGridSize(gridWidth, gridHeight);
    renderer->setViewport(viewport);
    renderer->setScaleFactor(scaleFactor);
    renderer->setDeviceType(deviceType);
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkAutoGraphics ag;
    SkTArray<SkString> inputs;
    sk_tools::PictureRenderer* renderer = NULL;
    SkString* outputDir = NULL;
    bool validate = false;
    int maxComponentDiff = 256;
    bool writeWholeImage = false;
    int clones = 0;
    parse_commandline(argc, argv, &inputs, renderer, outputDir,
                      &validate, &maxComponentDiff, &writeWholeImage, &clones);
    SkASSERT(renderer);

    int failures = 0;
    for (int i = 0; i < inputs.count(); i ++) {
        failures += process_input(inputs[i], outputDir, *renderer,
                                  validate, maxComponentDiff,
                                  writeWholeImage, clones);
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
    if (NULL != outputDir) {
        SkDELETE(outputDir);
    }
    SkDELETE(renderer);
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
