/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkCodec.h"
#include "SkColorSpace.h"
#include "SkCommandLineFlags.h"
#include "SkForceLinking.h"
#include "SkImageEncoder.h"
#include "SkMatrix44.h"
#include "SkOSFile.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_string2(input, i, "input.png", "A path to the input image.");
DEFINE_string2(output, o, "output.png", "A path to the output image.");

/**
 *  Loads the triangular gamut as a set of three points.
 */
static void load_gamut(SkPoint rgb[], const SkMatrix44& xyz) {
    // rx = rX / (rX + rY + rZ)
    // ry = rX / (rX + rY + rZ)
    // gx, gy, bx, and gy are calulcated similarly.
    float rSum = xyz.get(0, 0) + xyz.get(0, 1) + xyz.get(0, 2);
    float gSum = xyz.get(1, 0) + xyz.get(1, 1) + xyz.get(1, 2);
    float bSum = xyz.get(2, 0) + xyz.get(2, 1) + xyz.get(2, 2);
    rgb[0].fX = xyz.get(0, 0) / rSum;
    rgb[0].fY = xyz.get(0, 1) / rSum;
    rgb[1].fX = xyz.get(1, 0) / gSum;
    rgb[1].fY = xyz.get(1, 1) / gSum;
    rgb[2].fX = xyz.get(2, 0) / bSum;
    rgb[2].fY = xyz.get(2, 1) / bSum;
}

/**
 *  Calculates the area of the triangular gamut.
 */
float calculate_area(SkPoint abc[]) {
    SkPoint a = abc[0];
    SkPoint b = abc[1];
    SkPoint c = abc[2];
    return 0.5f * SkTAbs(a.fX*b.fY + b.fX*c.fY - a.fX*c.fY - c.fX*b.fY - b.fX*a.fY);
}

int main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage(
            "Usage: visualize_color_gamut --input <path to input image>"
                                         "--output <path to output image>\n"
            "Description: Writes a visualization of the color gamut to the output image\n");
    SkCommandLineFlags::Parse(argc, argv);
    const char* input = FLAGS_input[0];
    const char* output = FLAGS_output[0];
    if (!input || !output) {
        SkCommandLineFlags::PrintUsage();
        return -1;
    }

    SkAutoTUnref<SkData> data(SkData::NewFromFileName(input));
    if (!data) {
        SkDebugf("Cannot find input image.\n");
        return -1;
    }
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(data));
    if (!codec) {
        SkDebugf("Invalid input image.\n");
        return -1;
    }

    // Load a graph of the CIE XYZ color gamut.
    SkBitmap bitmap;
    if (!GetResourceAsBitmap("gamut.png", &bitmap)) {
        SkDebugf("Program failure.\n");
        return -1;
    }
    SkCanvas canvas(bitmap);

    sk_sp<SkColorSpace> colorSpace = sk_ref_sp(codec->getColorSpace());
    if (!colorSpace) {
        SkDebugf("Image had no embedded color space information.  Defaulting to sRGB.\n");
        colorSpace = SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named);
    }

    // Calculate the points in the gamut from the XYZ values.
    SkMatrix44 xyz = colorSpace->xyz();
    SkPoint rgb[4];
    load_gamut(rgb, xyz);

    // Report the XYZ values.
    SkDebugf("      X    Y    Z\n");
    SkDebugf("Red   %.2f %.2f %.2f\n", xyz.get(0, 0), xyz.get(0, 1), xyz.get(0, 2));
    SkDebugf("Green %.2f %.2f %.2f\n", xyz.get(1, 0), xyz.get(1, 1), xyz.get(1, 2));
    SkDebugf("Blue  %.2f %.2f %.2f\n", xyz.get(2, 0), xyz.get(2, 1), xyz.get(2, 2));

    // Report the area of the gamut.
    SkDebugf("Area of Gamut: %g\n", calculate_area(rgb));

    // Now transform the points so they can be drawn on our canvas.  We use 1000 pixels
    // to represent the space from 0 to 1.  Note that the graph is at an offset of (50, 50).
    // Also note that y increases as we move down the canvas.
    rgb[0].fX = 50 + 1000*rgb[0].fX;
    rgb[0].fY = 50 + 1000*(1 - rgb[0].fY);
    rgb[1].fX = 50 + 1000*rgb[1].fX;
    rgb[1].fY = 50 + 1000*(1 - rgb[1].fY);
    rgb[2].fX = 50 + 1000*rgb[2].fX;
    rgb[2].fY = 50 + 1000*(1 - rgb[2].fY);

    // Repeat the first point to connect the polygon.
    rgb[3] = rgb[0];

    SkPaint paint;
    canvas.drawPoints(SkCanvas::kPolygon_PointMode, 4, rgb, paint);

    // Finally, encode the result to out.png.
    SkAutoTUnref<SkData> out(SkImageEncoder::EncodeData(bitmap, SkImageEncoder::kPNG_Type, 100));
    if (!out) {
        SkDebugf("Failed to encode output.\n");
        return -1;
    }
    SkFILEWStream stream(output);
    bool result = stream.write(out->data(), out->size());
    if (!result) {
        SkDebugf("Failed to write output.\n");
        return -1;
    }

    return 0;
}
