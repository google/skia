/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkDevice.h"
#include "SkImageEncoder.h"
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
"     [--pipe | --tile width[%] height[%]]"
, argv0);
    SkDebugf("\n\n");
    SkDebugf(
"     input:     A list of directories and files to use as input.\n"
"                    Files are expected to have the .skp extension.\n");
    SkDebugf(
"     outputDir: directory to write the rendered images.\n");
    SkDebugf(
"     --pipe : Render using a SkGPipe\n");
    SkDebugf(
"     --tile width[%] height[%]: Render using tiles with the given dimensions.\n");
}

static void make_output_filepath(SkString* path, const SkString& dir,
                                 const SkString& name) {
    sk_tools::make_filepath(path, dir, name);
    path->remove(path->size() - 3, 3);
    path->append("png");
}

/* since PNG insists on unpremultiplying our alpha, we take no precision chances
    and force all pixels to be 100% opaque, otherwise on compare we may not get
    a perfect match.
 */
static void force_all_opaque(const SkBitmap& bitmap) {
    SkAutoLockPixels lock(bitmap);
    for (int y = 0; y < bitmap.height(); y++) {
        for (int x = 0; x < bitmap.width(); x++) {
            *bitmap.getAddr32(x, y) |= (SK_A32_MASK << SK_A32_SHIFT);
        }
    }
}

static bool write_bitmap(const SkString& path, const SkBitmap& bitmap) {
    SkBitmap copy;
    bitmap.copyTo(&copy, SkBitmap::kARGB_8888_Config);
    force_all_opaque(copy);
    return SkImageEncoder::EncodeFile(path.c_str(), copy,
                                      SkImageEncoder::kPNG_Type, 100);
}

static void write_output(const SkString& outputDir, const SkString& inputFilename,
                         const SkBitmap& bitmap) {
    SkString outputPath;
    make_output_filepath(&outputPath, outputDir, inputFilename);
    bool isWritten = write_bitmap(outputPath, bitmap);
    if (!isWritten) {
        SkDebugf("Could not write to file %s\n", outputPath.c_str());
    }
}

static void render_picture(const SkString& inputPath, const SkString& outputDir,
                           sk_tools::PictureRenderer& renderer) {
    SkString inputFilename;
    sk_tools::get_basename(&inputFilename, inputPath);

    SkFILEStream inputStream;
    inputStream.setPath(inputPath.c_str());
    if (!inputStream.isValid()) {
        SkDebugf("Could not open file %s\n", inputPath.c_str());
        return;
    }

    SkPicture picture(&inputStream);
    SkBitmap bitmap;
    sk_tools::setup_bitmap(&bitmap, picture.width(), picture.height());
    SkCanvas canvas(bitmap);

    renderer.init(picture);
    renderer.render(&picture, &canvas);
    write_output(outputDir, inputFilename, bitmap);
}

static void process_input(const SkString& input, const SkString& outputDir,
                          sk_tools::PictureRenderer& renderer) {
    SkOSFile::Iter iter(input.c_str(), "skp");
    SkString inputFilename;

    if (iter.next(&inputFilename)) {
        do {
            SkString inputPath;
            sk_tools::make_filepath(&inputPath, input, inputFilename);
            render_picture(inputPath, outputDir, renderer);
        } while(iter.next(&inputFilename));
    } else {
        SkString inputPath(input);
        render_picture(inputPath, outputDir, renderer);
    }
}

static void parse_commandline(int argc, char* const argv[], SkTArray<SkString>* inputs,
                              sk_tools::PictureRenderer*& renderer){
    const char* argv0 = argv[0];
    char* const* stop = argv + argc;

    for (++argv; argv < stop; ++argv) {
        if (0 == strcmp(*argv, "--pipe")) {
            renderer = SkNEW(sk_tools::PipePictureRenderer);
        } else if (0 == strcmp(*argv, "--tile")) {
            sk_tools::TiledPictureRenderer* tileRenderer = SkNEW(sk_tools::TiledPictureRenderer);
            ++argv;
            if (argv < stop) {
                if (sk_tools::is_percentage(*argv)) {
                    tileRenderer->setTileWidthPercentage(atof(*argv));
                    if (!(tileRenderer->getTileWidthPercentage() > 0)) {
                        SkDELETE(tileRenderer);
                        SkDebugf("--tile must be given a width percentage > 0\n");
                        exit(-1);
                    }
                } else {
                    tileRenderer->setTileWidth(atoi(*argv));
                    if (!(tileRenderer->getTileWidth() > 0)) {
                        SkDELETE(tileRenderer);
                        SkDebugf("--tile must be given a width > 0\n");
                        exit(-1);
                    }
                }
            } else {
                SkDELETE(tileRenderer);
                SkDebugf("Missing width for --tile\n");
                usage(argv0);
                exit(-1);
            }
            ++argv;
            if (argv < stop) {
                if (sk_tools::is_percentage(*argv)) {
                    tileRenderer->setTileHeightPercentage(atof(*argv));
                    if (!(tileRenderer->getTileHeightPercentage() > 0)) {
                        SkDELETE(tileRenderer);
                        SkDebugf(
                            "--tile must be given a height percentage > 0\n");
                        exit(-1);
                    }
                } else {
                    tileRenderer->setTileHeight(atoi(*argv));
                    if (!(tileRenderer->getTileHeight() > 0)) {
                        SkDELETE(tileRenderer);
                        SkDebugf("--tile must be given a height > 0\n");
                        exit(-1);
                    }
                }
            } else {
                SkDELETE(tileRenderer);
                SkDebugf("Missing height for --tile\n");
                usage(argv0);
                exit(-1);
            }
            renderer = tileRenderer;
        } else if ((0 == strcmp(*argv, "-h")) || (0 == strcmp(*argv, "--help"))) {
            SkDELETE(renderer);
            usage(argv0);
            exit(-1);
        } else {
            inputs->push_back(SkString(*argv));
        }
    }

    if (inputs->count() < 2) {
        SkDELETE(renderer);
        usage(argv0);
        exit(-1);
    }

    if (NULL == renderer) {
        renderer = SkNEW(sk_tools::SimplePictureRenderer);
    }
}

int main(int argc, char* const argv[]) {
    SkTArray<SkString> inputs;
    sk_tools::PictureRenderer* renderer = NULL;

    parse_commandline(argc, argv, &inputs, renderer);
    SkString outputDir = inputs[inputs.count() - 1];
    SkASSERT(renderer);

    for (int i = 0; i < inputs.count() - 1; i ++) {
        process_input(inputs[i], outputDir, *renderer);
    }

    SkDELETE(renderer);
}
