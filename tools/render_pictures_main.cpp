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


static void usage(const char* argv0) {
    SkDebugf("SkPicture rendering tool\n");
    SkDebugf("\n"
"Usage: \n"
"     %s <inputDir> <outputDir> \n\n"
, argv0);
    SkDebugf(
"     inputDir:  directory to read the serialized SkPicture files.\n");
    SkDebugf(
"     outputDir: directory to write the rendered images.\n");
}

static void make_filepath(SkString* path, const char* dir,
                          const SkString& name) {
    size_t len = strlen(dir);
    path->set(dir);
    if (0 < len  && '/' != dir[len - 1]) {
        path->append("/");
    }
    path->append(name);
}

static void open_picture_stream(const char* inputDir,
                                const SkString& inputFilename,
                                SkFILEStream* inputStream) {
    SkString inputPath;
    make_filepath(&inputPath, inputDir, inputFilename);
    inputStream->setPath(inputPath.c_str());
    if (!inputStream->isValid()) {
        SkDebugf("Could not open file %s\n", inputPath.c_str());
    }
}

static void make_output_filepath(SkString* path, const char* dir,
                                 const SkString& name) {
    make_filepath(path, dir, name);
    path->remove(path->size() - 3, 3);
    path->append("png");
}

static void setup_bitmap(SkPicture& picture, SkBitmap* bitmap) {
    bitmap->setConfig(SkBitmap::kARGB_8888_Config, picture.width(),
                     picture.height());
    bitmap->allocPixels();
    bitmap->eraseColor(0);
}

static void generate_image_from_picture(SkPicture& pict, SkBitmap* bitmap) {
    setup_bitmap(pict, bitmap);
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

static void render_picture(const char* inputDir, const char* outputDir,
                           const SkString& inputFilename) {
    SkFILEStream inputStream;
    open_picture_stream(inputDir, inputFilename, &inputStream);
    SkPicture picture(&inputStream);
    SkBitmap bitmap;
    generate_image_from_picture(picture, &bitmap);
    write_output(outputDir, inputFilename, bitmap);
}

int main(int argc, char* const argv[]) {
    const char* inputDir;
    const char* outputDir;
    if (argc != 3) {
        usage(argv[0]);
    }

    inputDir = argv[1];
    outputDir = argv[2];

    SkOSFile::Iter iter(inputDir, "skp");
    SkString inputFilename;

    while(iter.next(&inputFilename)) {
        render_picture(inputDir, outputDir, inputFilename);
    }
}
