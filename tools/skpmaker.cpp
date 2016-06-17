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

#include <stdlib.h>

// Flags used by this file, alphabetically:
DEFINE_int32(blue, 128, "Value of blue color channel in image, 0-255.");
DEFINE_int32(border, 4, "Width of the black border around the image.");
DEFINE_int32(green, 128, "Value of green color channel in image, 0-255.");
DEFINE_int32(height, 200, "Height of canvas to create.");
DEFINE_int32(red, 128, "Value of red color channel in image, 0-255.");
DEFINE_int32(width, 300, "Width of canvas to create.");
DEFINE_string(writePath, "", "Filepath to write the SKP into.");

// Create a 'width' by 'height' skp with a 'border'-wide black border around
// a 'color' rectangle.
static void make_skp(SkScalar width, SkScalar height, SkScalar border, SkColor color,
                     const char *writePath) {
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(width, height, nullptr, 0);
    SkPaint paint;
    paint.setStyle(SkPaint::kFill_Style);
    paint.setColor(SK_ColorBLACK);
    SkRect r = SkRect::MakeWH(width, height);
    canvas->drawRect(r, paint);
    paint.setColor(color);
    r.inset(border, border);
    canvas->drawRect(r, paint);
    SkFILEWStream stream(writePath);
    recorder.finishRecordingAsPicture()->serialize(&stream);
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
    make_skp(SkIntToScalar(FLAGS_width),
             SkIntToScalar(FLAGS_height),
             SkIntToScalar(FLAGS_border),
             color, FLAGS_writePath[0]);
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
