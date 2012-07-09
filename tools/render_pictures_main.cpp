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
#include "picture_utils.h"


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

static void make_output_filepath(SkString* path, const char* dir,
                                 const SkString& name) {
    sk_tools::make_filepath(path, dir, name);
    path->remove(path->size() - 3, 3);
    path->append("png");
}

static void generate_image_from_picture(SkPicture& pict, SkBitmap* bitmap) {
    sk_tools::setup_bitmap(bitmap, pict.width(), pict.height());
    SkCanvas canvas(*bitmap);
    canvas.drawPicture(pict);
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

static void write_output(const char* outputDir, const SkString& inputFilename,
                         const SkBitmap& bitmap) {
    SkString outputPath;
    make_output_filepath(&outputPath, outputDir, inputFilename);
    bool isWritten = write_bitmap(outputPath, bitmap);
    if (!isWritten) {
        SkDebugf("Could not write to file %s\n", outputPath.c_str());
    }
}

static void render_picture(const SkString& inputPath, const char* outputDir) {
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
    generate_image_from_picture(picture, &bitmap);
    write_output(outputDir, inputFilename, bitmap);
}

static void process_input(const char* input, const char* outputDir) {
    SkOSFile::Iter iter(input, "skp");
    SkString inputFilename;

    if (iter.next(&inputFilename)) {
        do {
            SkString inputPath;
            sk_tools::make_filepath(&inputPath, input, inputFilename);
            render_picture(inputPath, outputDir);
        } while(iter.next(&inputFilename));
    } else {
        SkString inputPath(input);
        render_picture(inputPath, outputDir);
    }
}

int main(int argc, char* const argv[]) {
    const char* outputDir;
    if (argc < 3) {
        usage(argv[0]);
        return -1;
    }

    outputDir = argv[argc - 1];

    for (int i = 1; i < argc - 1; i ++) {
        process_input(argv[i], outputDir);
    }
}
