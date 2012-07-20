/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBitmap.h"
#include "SamplePipeControllers.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkDevice.h"
#include "SkGPipe.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTDArray.h"
#include "picture_utils.h"

enum {
    kDefaultTileWidth = 256,
    kDefaultTileHeight = 256
};

static void usage(const char* argv0) {
    SkDebugf("SkPicture rendering tool\n");
    SkDebugf("\n"
"Usage: \n"
"     %s <input>... <outputDir> \n"
"     [--pipe | --tile]"
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
"     --tile : Render using tiles.\n");
}

static void simple_render(SkPicture* pict, SkBitmap* bitmap);
typedef void (*RenderFunc) (SkPicture*, SkBitmap*);

static void make_output_filepath(SkString* path, const SkString& dir,
                                 const SkString& name) {
    sk_tools::make_filepath(path, dir, name);
    path->remove(path->size() - 3, 3);
    path->append("png");
}

static void simple_render(SkPicture* pict, SkBitmap* bitmap) {
    sk_tools::setup_bitmap(bitmap, pict->width(), pict->height());
    SkCanvas canvas(*bitmap);
    canvas.drawPicture(*pict);
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

static void pipe_run(SkPicture* picture, SkCanvas* canvas) {
    PipeController pipeController(canvas);
    SkGPipeWriter writer;
    SkCanvas* pipeCanvas = writer.startRecording(&pipeController);
    pipeCanvas->drawPicture(*picture);
    writer.endRecording();
}

static void pipe_render(SkPicture* picture, SkBitmap* bitmap) {
    sk_tools::setup_bitmap(bitmap, picture->width(), picture->height());

    SkCanvas canvas(*bitmap);

    pipe_run(picture, &canvas);
}

struct TileInfo {
    SkBitmap* fBitmap;
    SkCanvas* fCanvas;
};

// Clips the tile to an area that is completely in what the SkPicture says is the
// drawn-to area. This is mostly important for tiles on the right and bottom edges
// as they may go over this area and the picture may have some commands that
// draw outside of this area and so should not actually be written.
static void clip_tile(SkPicture* picture, const TileInfo& tile) {
    SkRect clip = SkRect::MakeWH(SkIntToScalar(picture->width()),
                                 SkIntToScalar(picture->height()));
    tile.fCanvas->clipRect(clip);
}

static void add_tile(SkPicture* picture, SkTDArray<TileInfo>* tiles,
                     int tile_x_start, int tile_y_start) {
    TileInfo* tile = tiles->push();

    tile->fBitmap = new SkBitmap();
    sk_tools::setup_bitmap(tile->fBitmap, kDefaultTileWidth, kDefaultTileHeight);

    tile->fCanvas = new SkCanvas(*(tile->fBitmap));
    tile->fCanvas->translate(SkIntToScalar(-tile_x_start), SkIntToScalar(-tile_y_start));
    clip_tile(picture, *tile);
}

static void setup_tiles(SkPicture* picture, SkTDArray<TileInfo>* tiles) {
    for (int tile_y_start = 0; tile_y_start < picture->height();
         tile_y_start += kDefaultTileWidth) {
        for (int tile_x_start = 0; tile_x_start < picture->width();
             tile_x_start += kDefaultTileHeight) {
            add_tile(picture, tiles, tile_x_start, tile_y_start);
        }
    }
}

static void copy_tiles_to_bitmap(const SkTDArray<TileInfo>& tiles, SkBitmap* bitmap) {
    SkCanvas destination(*bitmap);

    int tile_index = 0;
    for (int tile_y_start = 0; tile_y_start < bitmap->height();
         tile_y_start += kDefaultTileWidth) {
        for (int tile_x_start = 0; tile_x_start < bitmap->width();
             tile_x_start += kDefaultTileHeight) {
            SkBitmap source = tiles[tile_index].fCanvas->getDevice()->accessBitmap(false);
            destination.drawBitmap(source, tile_x_start, tile_y_start);
            ++tile_index;
        }
    }
}

static void tile_render(SkPicture* picture, SkBitmap* bitmap) {
    sk_tools::setup_bitmap(bitmap, picture->width(), picture->height());

    SkTDArray<TileInfo> tiles;
    setup_tiles(picture, &tiles);

    for (int i = 0; i < tiles.count(); ++i) {
        tiles[i].fCanvas->drawPicture(*picture);
    }

    copy_tiles_to_bitmap(tiles, bitmap);

    for (int i = 0; i < tiles.count(); ++i) {
        delete tiles[i].fCanvas;
        delete tiles[i].fBitmap;
    }
}

static void render_picture(const SkString& inputPath, const SkString& outputDir,
                           RenderFunc renderFunc) {
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
    renderFunc(&picture, &bitmap);
    write_output(outputDir, inputFilename, bitmap);
}

static void process_input(const SkString& input, const SkString& outputDir,
                          RenderFunc renderFunc) {
    SkOSFile::Iter iter(input.c_str(), "skp");
    SkString inputFilename;

    if (iter.next(&inputFilename)) {
        do {
            SkString inputPath;
            sk_tools::make_filepath(&inputPath, input, inputFilename);
            render_picture(inputPath, outputDir, renderFunc);
        } while(iter.next(&inputFilename));
    } else {
        SkString inputPath(input);
        render_picture(inputPath, outputDir, renderFunc);
    }
}

static void parse_commandline(int argc, char* const argv[], SkTArray<SkString>* inputs,
                              RenderFunc* renderFunc){
    const char* argv0 = argv[0];
    char* const* stop = argv + argc;

    for (++argv; argv < stop; ++argv) {
        if (0 == strcmp(*argv, "--pipe")) {
            *renderFunc = pipe_render;
        } else if (0 == strcmp(*argv, "--tile")) {
            *renderFunc = tile_render;
        } else if ((0 == strcmp(*argv, "-h")) || (0 == strcmp(*argv, "--help"))) {
            usage(argv0);
            exit(-1);
        } else {
            inputs->push_back(SkString(*argv));
        }
    }

    if (inputs->count() < 2) {
        usage(argv0);
        exit(-1);
    }
}

int main(int argc, char* const argv[]) {
    SkTArray<SkString> inputs;
    RenderFunc renderFunc = simple_render;

    parse_commandline(argc, argv, &inputs, &renderFunc);
    SkString outputDir = inputs[inputs.count() - 1];

    for (int i = 0; i < inputs.count() - 1; i ++) {
        process_input(inputs[i], outputDir, renderFunc);
    }
}
