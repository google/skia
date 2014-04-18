/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Simple tool to generate SKP files for testing.
 */

#include "SkCanvas.h"
#include "SkColor.h"
#include "SkCommandLineFlags.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkScalar.h"
#include "SkStream.h"

// Flags used by this file, alphabetically:
DEFINE_int32(blue, 128, "Value of blue color channel in image, 0-255.");
DEFINE_int32(border, 4, "Width of the black border around the image.");
DEFINE_int32(green, 128, "Value of green color channel in image, 0-255.");
DEFINE_int32(height, 200, "Height of canvas to create.");
DEFINE_int32(red, 128, "Value of red color channel in image, 0-255.");
DEFINE_int32(width, 300, "Width of canvas to create.");
DEFINE_string(writePath, "", "Filepath to write the SKP into.");

static void skpmaker(int width, int height, int border, SkColor color,
                     const char *writePath) {
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(width, height, NULL, 0);
    SkPaint paint;
    paint.setStyle(SkPaint::kFill_Style);
    paint.setColor(SK_ColorBLACK);
    canvas->drawRectCoords(0, 0, SkIntToScalar(width), SkIntToScalar(height), paint);
    paint.setColor(color);
    canvas->drawRectCoords(SkIntToScalar(border), SkIntToScalar(border),
                           SkIntToScalar(width - border*2), SkIntToScalar(height - border*2),
                           paint);
    SkAutoTUnref<SkPicture> pict(recorder.endRecording());
    SkFILEWStream stream(writePath);
    pict->serialize(&stream);
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage("Creates a simple .skp file for testing.");
    SkCommandLineFlags::Parse(argc, argv);

    // Validate flags.
    if ((FLAGS_blue < 0) || (FLAGS_blue > 255)) {
        SkDebugf("--blue must be within range [0,255]\n");
        exit(-1);
    }
    if ((FLAGS_green < 0) || (FLAGS_green > 255)) {
        SkDebugf("--green must be within range [0,255]\n");
        exit(-1);
    }
    if (FLAGS_height <= 0) {
        SkDebugf("--height must be >0\n");
        exit(-1);
    }
    if ((FLAGS_red < 0) || (FLAGS_red > 255)) {
        SkDebugf("--red must be within range [0,255]\n");
        exit(-1);
    }
    if (FLAGS_width <= 0) {
        SkDebugf("--width must be >0\n");
        exit(-1);
    }
    if (FLAGS_writePath.isEmpty()) {
        SkDebugf("--writePath must be nonempty\n");
        exit(-1);
    }

    SkColor color = SkColorSetRGB(FLAGS_red, FLAGS_green, FLAGS_blue);
    skpmaker(FLAGS_width, FLAGS_height, FLAGS_border, color, FLAGS_writePath[0]);
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
