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
#include "SkColorSpace_A2B.h"
#include "SkColorSpace_XYZ.h"
#include "SkColorSpacePriv.h"
#include "SkCommandLineFlags.h"
#include "SkImageEncoder.h"
#include "SkMatrix44.h"
#include "SkOSFile.h"

#include "sk_tool_utils.h"

DEFINE_string(input, "input.png", "A path to the input image or icc profile.");
DEFINE_string(gamut_output, "gamut_output.png", "A path to the output gamut image.");
DEFINE_string(gamma_output, "gamma_output.png", "A path to the output gamma image.");
DEFINE_bool(sRGB_gamut, false, "Draws the sRGB gamut on the gamut visualization.");
DEFINE_bool(adobeRGB, false, "Draws the Adobe RGB gamut on the gamut visualization.");
DEFINE_bool(sRGB_gamma, false, "Draws the sRGB gamma on all gamma output images.");
DEFINE_string(uncorrected, "", "A path to reencode the uncorrected input image.");

static const char* kRGBChannelNames[3] = {
    "Red  ", "Green", "Blue "
};

static const SkColor kRGBChannelColors[3] = {
    SkColorSetARGB(164, 255, 32, 32),
    SkColorSetARGB(164, 32, 255, 32),
    SkColorSetARGB(164, 32, 32, 255)
};

static void dump_transfer_fn(SkGammaNamed gammaNamed) {
    switch (gammaNamed) {
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

}

static void dump_transfer_fn(const SkGammas& gammas) {
    SkASSERT(gammas.channels() == 3);
    for (int i = 0; i < gammas.channels(); i++) {
        if (gammas.isNamed(i)) {
            switch (gammas.data(i).fNamed) {
                case kSRGB_SkGammaNamed:
                    SkDebugf("%s Transfer Function: sRGB\n", kRGBChannelNames[i]);
                    return;
                case k2Dot2Curve_SkGammaNamed:
                    SkDebugf("%s Transfer Function: Exponent 2.2\n", kRGBChannelNames[i]);
                    return;
                case kLinear_SkGammaNamed:
                    SkDebugf("%s Transfer Function: Linear\n", kRGBChannelNames[i]);
                    return;
                default:
                    SkASSERT(false);
                    continue;
            }
        } else if (gammas.isValue(i)) {
            SkDebugf("%s Transfer Function: Exponent %.3f\n", kRGBChannelNames[i],
                     gammas.data(i).fValue);
        } else if (gammas.isParametric(i)) {
            const SkColorSpaceTransferFn& fn = gammas.data(i).params(&gammas);
            SkDebugf("%s Transfer Function: Parametric A = %.3f, B = %.3f, C = %.3f, D = %.3f, "
                     "E = %.3f, F = %.3f, G = %.3f\n", kRGBChannelNames[i], fn.fA, fn.fB, fn.fC,
                     fn.fD, fn.fE, fn.fF, fn.fG);
        } else {
            SkASSERT(gammas.isTable(i));
            SkDebugf("%s Transfer Function: Table (%d entries)\n", kRGBChannelNames[i],
                    gammas.data(i).fTable.fSize);
        }
    }
}

static inline float parametric(const SkColorSpaceTransferFn& fn, float x) {
    return x >= fn.fD ? powf(fn.fA*x + fn.fB, fn.fG) + fn.fC
                      : fn.fE*x + fn.fF;
}

static void draw_transfer_fn(SkCanvas* canvas, SkGammaNamed gammaNamed, const SkGammas* gammas,
                             SkColor color, int col) {
    SkColorSpaceTransferFn fn[4];
    struct TableInfo {
        const float* fTable;
        int          fSize;
    };
    TableInfo table[4];
    bool isTable[4] = {false, false, false, false};
    const int channels = gammas ? gammas->channels() : 1;
    SkASSERT(channels <= 4);
    if (kNonStandard_SkGammaNamed != gammaNamed) {
        dump_transfer_fn(gammaNamed);
        for (int i = 0; i < channels; ++i) {
            named_to_parametric(&fn[i], gammaNamed);
        }
    } else {
        SkASSERT(gammas);
        dump_transfer_fn(*gammas);
        for (int i = 0; i < channels; ++i) {
            if (gammas->isTable(i)) {
                table[i].fTable = gammas->table(i);
                table[i].fSize = gammas->data(i).fTable.fSize;
                isTable[i] = true;
            } else {
                switch (gammas->type(i)) {
                    case SkGammas::Type::kNamed_Type:
                        named_to_parametric(&fn[i], gammas->data(i).fNamed);
                        break;
                    case SkGammas::Type::kValue_Type:
                        value_to_parametric(&fn[i], gammas->data(i).fValue);
                        break;
                    case SkGammas::Type::kParam_Type:
                        fn[i] = gammas->params(i);
                        break;
                    default:
                        SkASSERT(false);
                }
            }
        }
    }
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(color);
    paint.setStrokeWidth(2.0f);
    // note: gamma has positive values going up in this image so this origin is
    //       the bottom left and we must subtract y instead of adding.
    const float gap         = 16.0f;
    const float cellWidth   = 500.0f;
    const float cellHeight  = 500.0f;
    const float gammaWidth  = cellWidth - 2 * gap;
    const float gammaHeight = cellHeight - 2 * gap;
    // gamma origin point
    const float ox = gap + cellWidth * col;
    const float oy = gap + gammaHeight;
    for (int i = 0; i < channels; ++i) {
        if (kNonStandard_SkGammaNamed == gammaNamed) {
            paint.setColor(kRGBChannelColors[i]);
        } else {
            paint.setColor(color);
        }
        if (isTable[i]) {
            auto tx = [&table,i](int index) {
                return index / (table[i].fSize - 1.0f);
            };
            for (int ti = 1; ti < table[i].fSize; ++ti) {
                canvas->drawLine(ox + gammaWidth * tx(ti - 1),
                                 oy - gammaHeight * table[i].fTable[ti - 1],
                                 ox + gammaWidth * tx(ti),
                                 oy - gammaHeight * table[i].fTable[ti],
                                 paint);
            }
        } else {
            const float step = 0.01f;
            float yPrev = parametric(fn[i], 0.0f);
            for (float x = step; x <= 1.0f; x += step) {
                const float y = parametric(fn[i], x);
                canvas->drawLine(ox + gammaWidth * (x - step), oy - gammaHeight * yPrev,
                                 ox + gammaWidth * x, oy - gammaHeight * y,
                                 paint);
                yPrev = y;
            }
        }
    }
    paint.setColor(0xFF000000);
    paint.setStrokeWidth(3.0f);
    canvas->drawRectCoords(ox, oy - gammaHeight, ox + gammaWidth, oy, paint);
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
                                  "--gamma_output <path to output gamma image> "
                                  "--gamut_output <path to output gamut image>"
                                  "--sRGB <draw canonical sRGB gamut> "
                                  "--adobeRGB <draw canonical Adobe RGB gamut> "
                                  "--uncorrected <path to reencoded, uncorrected input image>\n"
            "Description: Writes visualizations of the color space to the output image(s)  ."
                         "Also, if a path is provided, writes uncorrected bytes to an unmarked "
                         "png, for comparison with the input image.\n");
    SkCommandLineFlags::Parse(argc, argv);
    const char* input = FLAGS_input[0];
    const char* gamut_output = FLAGS_gamut_output[0];
    const char* gamma_output = FLAGS_gamma_output[0];
    if (!input || !gamut_output || !gamma_output) {
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
    const bool isImage = (codec != nullptr);
    if (isImage) {
        colorSpace = sk_ref_sp(codec->getInfo().colorSpace());
    } else {
        colorSpace = SkColorSpace::MakeICC(data->bytes(), data->size());
    }

    if (!colorSpace) {
        SkDebugf("Cannot create codec or icc profile from input file.\n");
        return -1;
    }

    // Load a graph of the CIE XYZ color gamut.
    SkBitmap gamutCanvasBitmap;
    if (!GetResourceAsBitmap("gamut.png", &gamutCanvasBitmap)) {
        SkDebugf("Program failure.\n");
        return -1;
    }
    SkCanvas gamutCanvas(gamutCanvasBitmap);

    SkBitmap gammaCanvasBitmap;
    gammaCanvasBitmap.allocN32Pixels(500, 500);
    SkCanvas gammaCanvas(gammaCanvasBitmap);

    // Draw the sRGB gamut if requested.
    if (FLAGS_sRGB_gamut) {
        sk_sp<SkColorSpace> sRGBSpace = SkColorSpace::MakeNamed(SkColorSpace::kSRGB_Named);
        const SkMatrix44* mat = as_CSB(sRGBSpace)->toXYZD50();
        SkASSERT(mat);
        draw_gamut(&gamutCanvas, *mat, "sRGB", 0xFFFF9394, false);
    }

    // Draw the Adobe RGB gamut if requested.
    if (FLAGS_adobeRGB) {
        sk_sp<SkColorSpace> adobeRGBSpace = SkColorSpace::MakeNamed(SkColorSpace::kAdobeRGB_Named);
        const SkMatrix44* mat = as_CSB(adobeRGBSpace)->toXYZD50();
        SkASSERT(mat);
        draw_gamut(&gamutCanvas, *mat, "Adobe RGB", 0xFF31a9e1, false);
    }

    int gammaCol = 0;
    if (SkColorSpace_Base::Type::kXYZ == as_CSB(colorSpace)->type()) {
        const SkMatrix44* mat = as_CSB(colorSpace)->toXYZD50();
        SkASSERT(mat);
        auto xyz = static_cast<SkColorSpace_XYZ*>(colorSpace.get());
        draw_gamut(&gamutCanvas, *mat, input, 0xFF000000, true);
        if (FLAGS_sRGB_gamma) {
            draw_transfer_fn(&gammaCanvas, kSRGB_SkGammaNamed, nullptr, 0xFFFF9394, gammaCol);
        }
        draw_transfer_fn(&gammaCanvas, xyz->gammaNamed(), xyz->gammas(), 0xFF000000, gammaCol++);
    } else {
        SkDebugf("Color space is defined using an A2B tag.  It cannot be represented by "
                 "a transfer function and to D50 matrix.\n");
        return -1;
    }

    // marker to tell the web-tool the names of all images output
    SkDebugf("=========\n");
    auto saveCanvasBitmap = [](const SkBitmap& bitmap, const char *fname) {
        // Finally, encode the result to the output file.
        sk_sp<SkData> out = sk_tool_utils::EncodeImageToData(bitmap, SkEncodedImageFormat::kPNG,
                                                             100);
        if (!out) {
            SkDebugf("Failed to encode %s output.\n", fname);
            return false;
        }
        SkFILEWStream stream(fname);
        if (!stream.write(out->data(), out->size())) {
            SkDebugf("Failed to write %s output.\n", fname);
            return false;
        }
        // record name of canvas
        SkDebugf("%s\n", fname);
        return true;
    };

    // only XYZ images have a gamut visualization since the matrix in A2B is not
    // a gamut adjustment from RGB->XYZ always (or ever)
    if (SkColorSpace_Base::Type::kXYZ == as_CSB(colorSpace)->type() &&
        !saveCanvasBitmap(gamutCanvasBitmap, gamut_output)) {
        return -1;
    }
    if (gammaCol > 0 && !saveCanvasBitmap(gammaCanvasBitmap, gamma_output)) {
        return -1;
    }

    if (isImage) {
        SkDebugf("%s\n", input);
    }
    // Also, if requested, decode and reencode the uncorrected input image.
    if (!FLAGS_uncorrected.isEmpty() && isImage) {
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
        sk_sp<SkData> out = sk_tool_utils::EncodeImageToData(bitmap, SkEncodedImageFormat::kPNG,
                                                             100);
        if (!out) {
            SkDebugf("Failed to encode uncorrected image.\n");
            return -1;
        }
        SkFILEWStream bitmapStream(FLAGS_uncorrected[0]);
        if (!bitmapStream.write(out->data(), out->size())) {
            SkDebugf("Failed to write uncorrected image output.\n");
            return -1;
        }
        SkDebugf("%s\n", FLAGS_uncorrected[0]);
    }

    return 0;
}
