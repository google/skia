/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "BenchTimer.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "picture_utils.h"

const int DEFAULT_REPEATS = 100;
const int DEFAULT_TILE_WIDTH = 256;
const int DEFAULT_TILE_HEIGHT = 256;

struct Options;
static void run_simple_benchmark(SkPicture* picture, const SkBitmap&,
                                 const Options&);

struct Options {
    int fRepeats;
    void (*fBenchmark) (SkPicture*, const SkBitmap& bitmap,
                        const Options& options);
    int fTileWidth;
    int fTileHeight;

    Options() : fRepeats(DEFAULT_REPEATS), fBenchmark(run_simple_benchmark),
    fTileWidth(DEFAULT_TILE_WIDTH), fTileHeight(DEFAULT_TILE_HEIGHT){}
};

static void usage(const char* argv0) {
    SkDebugf("SkPicture benchmarking tool\n");
    SkDebugf("\n"
"Usage: \n"
"     %s <inputDir>\n"
"     [--repeat] [--tile width height]"
, argv0);
    SkDebugf("\n\n");
    SkDebugf(
"     inputDir: directory to read the serialized SkPicture files."
"                Files are expected to have the .skp extension.\n\n");
    SkDebugf(
"     --repeat : "
"Set the number of times to repeat each test."
" Default is %i.\n", DEFAULT_REPEATS);
    SkDebugf(
"     --tile width height: "
"Set to use the tiling size and specify the dimensions of each tile."
" Default is to not use tiling\n");
}

static void run_simple_benchmark(SkPicture* picture,
                                 const SkBitmap& bitmap,
                                 const Options& options) {
    SkCanvas canvas(bitmap);

    // We throw this away to remove first time effects (such as paging in this
    // program)
    canvas.drawPicture(*picture);

    BenchTimer timer = BenchTimer(NULL);
    timer.start();
    for (int i = 0; i < options.fRepeats; ++i) {
        canvas.drawPicture(*picture);
    }
    timer.end();

    printf("simple: cmsecs = %6.2f\n", timer.fWall / options.fRepeats);
}

struct TileInfo {
    SkBitmap* fBitmap;
    SkCanvas* fCanvas;
};

static void setup_single_tile(const SkBitmap& bitmap, const Options& options,
                              SkTArray<TileInfo>* tiles,
                              int tile_x_start, int tile_y_start) {
    TileInfo& tile = tiles->push_back();
    tile.fBitmap = new SkBitmap();
    SkIRect rect = SkIRect::MakeXYWH(tile_x_start, tile_y_start,
                                             options.fTileWidth,
                                             options.fTileHeight);
    bitmap.extractSubset(tile.fBitmap, rect);
    tile.fCanvas = new SkCanvas(*(tile.fBitmap));
    tile.fCanvas->translate(-tile_x_start, -tile_y_start);
}

static void setup_tiles(SkPicture* picture, const SkBitmap& bitmap,
                        const Options& options, SkTArray<TileInfo>* tiles) {
    for (int tile_y_start = 0; tile_y_start < picture->height();
         tile_y_start += options.fTileHeight) {
        for (int tile_x_start = 0; tile_x_start < picture->width();
             tile_x_start += options.fTileWidth) {
            setup_single_tile(bitmap, options, tiles, tile_x_start,
                              tile_y_start);
        }
    }

}

static void run_tile_benchmark(SkPicture* picture, const SkBitmap& bitmap,
                               const Options& options) {
    SkTArray<TileInfo> tiles;
    setup_tiles(picture, bitmap, options, &tiles);

    // We throw this away to remove first time effects (such as paging in this
    // program)
    for (int j = 0; j < tiles.count(); ++j) {
        tiles[j].fCanvas->drawPicture(*picture);
    }

    BenchTimer timer = BenchTimer(NULL);
    timer.start();
    for (int i = 0; i < options.fRepeats; ++i) {
        for (int j = 0; j < tiles.count(); ++j) {
            tiles[j].fCanvas->drawPicture(*picture);
        }
    }
    timer.end();

    for (int i = 0; i < tiles.count(); ++i) {
        delete tiles[i].fCanvas;
        delete tiles[i].fBitmap;
    }

    printf("tile%ix%i: cmsecs = %6.2f\n", options.fTileWidth,
           options.fTileHeight, timer.fWall / options.fRepeats);
}

static void run_benchmark(const char* inputDir,
                          const SkString& inputFilename,
                          const Options& options) {
    SkFILEStream inputStream;

    SkString inputPath;
    sk_tools::make_filepath(&inputPath, inputDir, inputFilename);
    inputStream.setPath(inputPath.c_str());
    if (!inputStream.isValid()) {
        SkDebugf("Could not open file %s\n", inputPath.c_str());
        return;
    }

    SkPicture picture(&inputStream);
    SkBitmap bitmap;
    sk_tools::setup_bitmap(&bitmap, picture.width(), picture.height());

    printf("running bench [%i %i] %s ", picture.width(), picture.height(),
           inputFilename.c_str());
    options.fBenchmark(&picture, bitmap, options);
}

static void parse_commandline(int argc, char* const argv[],
                              const char** inputDir, Options* options) {
    const char* argv0 = argv[0];
    char* const* stop = argv + argc;

    for (++argv; argv < stop; ++argv) {
        if (0 == strcmp(*argv, "--repeat")) {
            ++argv;
            if (argv < stop) {
                options->fRepeats = atoi(*argv);
                if (options->fRepeats < 1) {
                    SkDebugf("--repeat must be given a value > 0\n");
                    exit(-1);
                }
            } else {
                SkDebugf("Missing arg for --repeat\n");
                usage(argv0);
                exit(-1);
            }
        } else if (0 == strcmp(*argv, "--tile")) {
            options->fBenchmark = run_tile_benchmark;
            ++argv;
            if (argv < stop) {
                options->fTileWidth = atoi(*argv);
                if (options->fTileWidth < 1) {
                    SkDebugf("--tile must be given a width with a value > 0\n");
                    exit(-1);
                }
            } else {
                SkDebugf("Missing width for --tile\n");
                usage(argv0);
                exit(-1);
            }
            ++argv;
            if (argv < stop) {
                options->fTileHeight = atoi(*argv);
                if (options->fTileHeight < 1) {
                    SkDebugf("--tile must be given a height with a value > 0"
                             "\n");
                    exit(-1);
                }
            } else {
                SkDebugf("Missing height for --tile\n");
                usage(argv0);\
                exit(-1);
            }
        } else if (0 == strcmp(*argv, "--help") || 0 == strcmp(*argv, "-h")) {
            usage(argv0);
            exit(0);
        } else {
            if (NULL == *inputDir) {
                *inputDir = *argv;
            } else {
                usage(argv0);
                exit(-1);
            }
        }
    }

    if (NULL == *inputDir) {
        usage(argv0);
        exit(-1);
    }

}

int main(int argc, char* const argv[]) {
    const char* inputDir = NULL;
    Options options;

    parse_commandline(argc, argv, &inputDir, &options);

    SkOSFile::Iter iter(inputDir, "skp");
    SkString inputFilename;

    while(iter.next(&inputFilename)) {
        run_benchmark(inputDir, inputFilename, options);
    }
}
