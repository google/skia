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

DEFINE_string(input, "input.png", "A path to the input image.");
DEFINE_string(output, "output.png", "A path to the output image.");
DEFINE_bool(sRGB, false, "Draws the sRGB gamut.");
DEFINE_bool(adobeRGB, false, "Draws the Adobe RGB gamut.");
DEFINE_string(uncorrected, "", "A path to reencode the uncorrected input image.");

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
static float calculate_area(SkPoint abc[]) {
    SkPoint a = abc[0];
    SkPoint b = abc[1];
    SkPoint c = abc[2];
    return 0.5f * SkTAbs(a.fX*b.fY + b.fX*c.fY - a.fX*c.fY - c.fX*b.fY - b.fX*a.fY);
}

static void draw_gamut(SkCanvas* canvas, const SkMatrix44& xyz, const char* name, SkColor color,
                       bool label) {
    // Report the XYZ values.
    SkDebugf("%s\n", name);
    SkDebugf("          X     Y     Z\n");
    SkDebugf("Red   %.3f %.3f %.3f\n", xyz.get(0, 0), xyz.get(0, 1), xyz.get(0, 2));
    SkDebugf("Green %.3f %.3f %.3f\n", xyz.get(1, 0), xyz.get(1, 1), xyz.get(1, 2));
    SkDebugf("Blue  %.3f %.3f %.3f\n", xyz.get(2, 0), xyz.get(2, 1), xyz.get(2, 2));

    // Calculate the points in the gamut from the XYZ values.
    SkPoint rgb[4];
    load_gamut(rgb, xyz);

    // Report the area of the gamut.
    SkDebugf("Area of Gamut: %.3f\n\n", calculate_area(rgb));

    // Magic constants that help us place the gamut triangles in the appropriate position
    // on the canvas.
    const float xScale = 2071.25f;  // Num pixels from 0 to 1 in x
    const float xOffset = 241.0f;   // Num pixels until start of x-axis
    const float yScale = 2067.78f;  // Num pixels from 0 to 1 in y
    const float yOffset = -144.78f; // Num pixels until start of y-axis
                                    // (negative because y extends beyond image bounds)

    // Now transform the points so they can be drawn on our canvas.
    // Note that y increases as we move down the canvas.
    rgb[0].fX = xOffset + xScale * rgb[0].fX;
    rgb[0].fY = yOffset + yScale * (1.0f - rgb[0].fY);
    rgb[1].fX = xOffset + xScale * rgb[1].fX;
    rgb[1].fY = yOffset + yScale * (1.0f - rgb[1].fY);
    rgb[2].fX = xOffset + xScale * rgb[2].fX;
    rgb[2].fY = yOffset + yScale * (1.0f - rgb[2].fY);

    // Repeat the first point to connect the polygon.
    rgb[3] = rgb[0];
    SkPaint paint;
    paint.setColor(color);
    paint.setStrokeWidth(6.0f);
    paint.setTextSize(75.0f);
    canvas->drawPoints(SkCanvas::kPolygon_PointMode, 4, rgb, paint);
    if (label) {
        canvas->drawText("R", 1, rgb[0].fX + 5.0f, rgb[0].fY + 75.0f, paint);
        canvas->drawText("G", 1, rgb[1].fX + 5.0f, rgb[1].fY - 5.0f, paint);
        canvas->drawText("B", 1, rgb[2].fX - 75.0f, rgb[2].fY - 5.0f, paint);
    }
}

int main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage(
            "Usage: visualize_color_gamut --input <path to input image> "
                                         "--output <path to output image> "
                                         "--sRGB <draw canonical sRGB gamut> "
                                         "--adobeRGB <draw canonical Adobe RGB gamut> "
                                         "--uncorrected <path to reencoded, uncorrected "
                                         "               input image>\n"
            "Description: Writes a visualization of the color gamut to the output image  ."
                         "Also, if a path is provided, writes uncorrected bytes to an unmarked "
                         "png, for comparison with the input image.\n");
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
    SkBitmap gamut;
    if (!GetResourceAsBitmap("gamut.png", &gamut)) {
        SkDebugf("Program failure.\n");
        return -1;
    }
    SkCanvas canvas(gamut);

    // Draw the sRGB gamut if requested.
    if (FLAGS_sRGB) {
        sk_sp<SkColorSpace> sRGBSpace = SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named);
        draw_gamut(&canvas, sRGBSpace->xyz(), "sRGB", 0xFFFF9394, false);
    }

    // Draw the Adobe RGB gamut if requested.
    if (FLAGS_adobeRGB) {
        sk_sp<SkColorSpace> adobeRGBSpace = SkColorSpace::NewNamed(SkColorSpace::kAdobeRGB_Named);
        draw_gamut(&canvas, adobeRGBSpace->xyz(), "Adobe RGB", 0xFF31a9e1, false);
    }

    // Draw gamut for the input image.
    sk_sp<SkColorSpace> colorSpace = sk_ref_sp(codec->getColorSpace());
    if (!colorSpace) {
        SkDebugf("Image had no embedded color space information.  Defaulting to sRGB.\n");
        colorSpace = SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named);
    }
    draw_gamut(&canvas, colorSpace->xyz(), input, 0xFF000000, true);

    // Finally, encode the result to the output file.
    SkAutoTUnref<SkData> out(SkImageEncoder::EncodeData(gamut, SkImageEncoder::kPNG_Type, 100));
    if (!out) {
        SkDebugf("Failed to encode gamut output.\n");
        return -1;
    }
    SkFILEWStream stream(output);
    bool result = stream.write(out->data(), out->size());
    if (!result) {
        SkDebugf("Failed to write gamut output.\n");
        return -1;
    }

    // Also, if requested, decode and reencode the uncorrected input image.
    if (!FLAGS_uncorrected.isEmpty()) {
        SkBitmap bitmap;
        int width = codec->getInfo().width();
        int height = codec->getInfo().height();
        SkAlphaType alphaType = codec->getInfo().alphaType();
        bitmap.allocN32Pixels(width, height, kOpaque_SkAlphaType == alphaType);
        SkImageInfo decodeInfo = SkImageInfo::MakeN32(width, height, alphaType);
        if (SkCodec::kSuccess != codec->getPixels(decodeInfo, bitmap.getPixels(),
                                                  bitmap.rowBytes())) {
            SkDebugf("Could not decode input image.\n");
            return -1;
        }
        out.reset(SkImageEncoder::EncodeData(bitmap, SkImageEncoder::kPNG_Type, 100));
        if (!out) {
            SkDebugf("Failed to encode uncorrected image.\n");
            return -1;
        }
        SkFILEWStream bitmapStream(FLAGS_uncorrected[0]);
        result = bitmapStream.write(out->data(), out->size());
        if (!result) {
            SkDebugf("Failed to write uncorrected image output.\n");
            return -1;
        }
    }

    return 0;
}
