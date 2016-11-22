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
#include "SkColorSpace_XYZ.h"
#include "SkCommandLineFlags.h"
#include "SkForceLinking.h"
#include "SkImageEncoder.h"
#include "SkMatrix44.h"
#include "SkOSFile.h"

#include "sk_tool_utils.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_string(input, "input.png", "A path to the input image or icc profile.");
DEFINE_string(output, "output.png", "A path to the output image.");
DEFINE_bool(sRGB, false, "Draws the sRGB gamut.");
DEFINE_bool(adobeRGB, false, "Draws the Adobe RGB gamut.");
DEFINE_string(uncorrected, "", "A path to reencode the uncorrected input image.");

static void dump_transfer_fn(SkColorSpace_XYZ* colorSpace) {
    switch (colorSpace->gammaNamed()) {
        case kSRGB_SkGammaNamed:
            SkDebugf("Transfer Function: sRGB\n");
            return;
        case k2Dot2Curve_SkGammaNamed:
            SkDebugf("Exponential Transfer Function: Exponent 2.2\n");
            return;
        case kLinear_SkGammaNamed:
            SkDebugf("Transfer Function: Linear\n");
            return;
        default:
            break;
    }

    static const char* kChannels[] = { "Red  ", "Green", "Blue ", };
    const SkGammas* gammas = colorSpace->gammas();
    for (int i = 0; i < 3; i++) {
        if (gammas->isNamed(i)) {
            switch (gammas->data(i).fNamed) {
                case kSRGB_SkGammaNamed:
                    SkDebugf("%s Transfer Function: sRGB\n", kChannels[i]);
                    return;
                case k2Dot2Curve_SkGammaNamed:
                    SkDebugf("%s Transfer Function: Exponent 2.2\n", kChannels[i]);
                    return;
                case kLinear_SkGammaNamed:
                    SkDebugf("%s Transfer Function: Linear\n", kChannels[i]);
                    return;
                default:
                    SkASSERT(false);
                    continue;
            }
        } else if (gammas->isValue(i)) {
            SkDebugf("%s Transfer Function: Exponent %.3f\n", kChannels[i], gammas->data(i).fValue);
        } else if (gammas->isParametric(i)) {
            const SkColorSpaceTransferFn& fn = gammas->data(i).params(gammas);
            SkDebugf("%s Transfer Function: Parametric A = %.3f, B = %.3f, C = %.3f, D = %.3f, "
                     "E = %.3f, F = %.3f, G = %.3f\n", kChannels[i], fn.fA, fn.fB, fn.fC, fn.fD,
                     fn.fE, fn.fF, fn.fG);
        } else {
            SkASSERT(gammas->isTable(i));
            SkDebugf("%s Transfer Function: Table (%d entries)\n", kChannels[i],
                    gammas->data(i).fTable.fSize);
        }
    }
}

/**
 *  Loads the triangular gamut as a set of three points.
 */
static void load_gamut(SkPoint rgb[], const SkMatrix44& xyz) {
    // rx = rX / (rX + rY + rZ)
    // ry = rX / (rX + rY + rZ)
    // gx, gy, bx, and gy are calulcated similarly.
    float rSum = xyz.get(0, 0) + xyz.get(1, 0) + xyz.get(2, 0);
    float gSum = xyz.get(0, 1) + xyz.get(1, 1) + xyz.get(2, 1);
    float bSum = xyz.get(0, 2) + xyz.get(1, 2) + xyz.get(2, 2);
    rgb[0].fX = xyz.get(0, 0) / rSum;
    rgb[0].fY = xyz.get(1, 0) / rSum;
    rgb[1].fX = xyz.get(0, 1) / gSum;
    rgb[1].fY = xyz.get(1, 1) / gSum;
    rgb[2].fX = xyz.get(0, 2) / bSum;
    rgb[2].fY = xyz.get(1, 2) / bSum;
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
    SkDebugf("       R     G     B\n");
    SkDebugf("X  %.3f %.3f %.3f\n", xyz.get(0, 0), xyz.get(0, 1), xyz.get(0, 2));
    SkDebugf("Y  %.3f %.3f %.3f\n", xyz.get(1, 0), xyz.get(1, 1), xyz.get(1, 2));
    SkDebugf("Z  %.3f %.3f %.3f\n", xyz.get(2, 0), xyz.get(2, 1), xyz.get(2, 2));

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
            "Usage: colorspaceinfo --input <path to input image or icc profile> "
                                  "--output <path to output image> "
                                  "--sRGB <draw canonical sRGB gamut> "
                                  "--adobeRGB <draw canonical Adobe RGB gamut> "
                                  "--uncorrected <path to reencoded, uncorrected input image>\n"
            "Description: Writes a visualization of the color space to the output image  ."
                         "Also, if a path is provided, writes uncorrected bytes to an unmarked "
                         "png, for comparison with the input image.\n");
    SkCommandLineFlags::Parse(argc, argv);
    const char* input = FLAGS_input[0];
    const char* output = FLAGS_output[0];
    if (!input || !output) {
        SkCommandLineFlags::PrintUsage();
        return -1;
    }

    sk_sp<SkData> data(SkData::MakeFromFileName(input));
    if (!data) {
        SkDebugf("Cannot find input image.\n");
        return -1;
    }
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(data));
    sk_sp<SkColorSpace> colorSpace = nullptr;
    if (codec) {
        colorSpace = sk_ref_sp(codec->getInfo().colorSpace());
    } else {
        colorSpace = SkColorSpace::MakeICC(data->bytes(), data->size());
    }

    if (!colorSpace) {
        SkDebugf("Cannot create codec or icc profile from input file.\n");
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
        sk_sp<SkColorSpace> sRGBSpace = SkColorSpace::MakeNamed(SkColorSpace::kSRGB_Named);
        const SkMatrix44* mat = as_CSB(sRGBSpace)->toXYZD50();
        SkASSERT(mat);
        draw_gamut(&canvas, *mat, "sRGB", 0xFFFF9394, false);
    }

    // Draw the Adobe RGB gamut if requested.
    if (FLAGS_adobeRGB) {
        sk_sp<SkColorSpace> adobeRGBSpace = SkColorSpace::MakeNamed(SkColorSpace::kAdobeRGB_Named);
        const SkMatrix44* mat = as_CSB(adobeRGBSpace)->toXYZD50();
        SkASSERT(mat);
        draw_gamut(&canvas, *mat, "Adobe RGB", 0xFF31a9e1, false);
    }

    if (SkColorSpace_Base::Type::kXYZ == as_CSB(colorSpace)->type()) {
        const SkMatrix44* mat = as_CSB(colorSpace)->toXYZD50();
        SkASSERT(mat);
        draw_gamut(&canvas, *mat, input, 0xFF000000, true);
        dump_transfer_fn((SkColorSpace_XYZ*) colorSpace.get());
    } else {
        SkDebugf("Color space is defined using an A2B tag.  It cannot be represented by "
                 "a transfer function and to D50 matrix.\n");
    }

    // Finally, encode the result to the output file.
    sk_sp<SkData> out = sk_tool_utils::EncodeImageToData(gamut, SkEncodedImageFormat::kPNG, 100);
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
    if (!FLAGS_uncorrected.isEmpty() && codec) {
        SkBitmap bitmap;
        int width = codec->getInfo().width();
        int height = codec->getInfo().height();
        bitmap.allocN32Pixels(width, height, kOpaque_SkAlphaType == codec->getInfo().alphaType());
        SkImageInfo decodeInfo = SkImageInfo::MakeN32(width, height, kUnpremul_SkAlphaType);
        if (SkCodec::kSuccess != codec->getPixels(decodeInfo, bitmap.getPixels(),
                                                  bitmap.rowBytes())) {
            SkDebugf("Could not decode input image.\n");
            return -1;
        }
        out = sk_tool_utils::EncodeImageToData(bitmap, SkEncodedImageFormat::kPNG, 100);
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
