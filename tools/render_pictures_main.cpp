/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTArray.h"
#include "picture_utils.h"

typedef void (*RenderFunc) (SkPicture*, SkBitmap*);

static void usage(const char* argv0) {
    SkDebugf("SkPicture rendering tool\n");
    SkDebugf("\n"
"Usage: \n"
"     %s <input>... <outputDir> \n\n"
, argv0);
    SkDebugf(
"     input:     A list of directories and files to use as input.\n"
"                    Files are expected to have the .skp extension.\n");
    SkDebugf(
"     outputDir: directory to write the rendered images.\n");
}

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
        inputs->push_back(SkString(*argv));
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
