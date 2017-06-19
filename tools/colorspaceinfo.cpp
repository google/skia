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
#include "SkICCPriv.h"
#include "SkImageEncoder.h"
#include "SkMatrix44.h"
#include "SkOSFile.h"

#include "sk_tool_utils.h"

#include <sstream>
#include <string>
#include <vector>

DEFINE_string(input, "input.png", "A path to the input image (or icc profile with --icc).");
DEFINE_string(output, ".", "A path to the output image directory.");
DEFINE_bool(icc, false, "Indicates that the input is an icc profile.");
DEFINE_bool(sRGB_gamut, false, "Draws the sRGB gamut on the gamut visualization.");
DEFINE_bool(adobeRGB, false, "Draws the Adobe RGB gamut on the gamut visualization.");
DEFINE_bool(sRGB_gamma, false, "Draws the sRGB gamma on all gamma output images.");
DEFINE_string(uncorrected, "", "A path to reencode the uncorrected input image.");


//-------------------------------------------------------------------------------------------------
//------------------------------------ Gamma visualizations ---------------------------------------

static const char* kRGBChannelNames[3] = {
    "Red  ",
    "Green",
    "Blue "
};
static const SkColor kRGBChannelColors[3] = {
    SkColorSetARGB(128, 255, 0, 0),
    SkColorSetARGB(128, 0, 255, 0),
    SkColorSetARGB(128, 0, 0, 255)
};

static const char* kGrayChannelNames[1] = { "Gray"};
static const SkColor kGrayChannelColors[1] = { SkColorSetRGB(128, 128, 128) };

static const char* kCMYKChannelNames[4] = {
    "Cyan   ",
    "Magenta",
    "Yellow ",
    "Black  "
};
static const SkColor kCMYKChannelColors[4] = {
    SkColorSetARGB(128, 0, 255, 255),
    SkColorSetARGB(128, 255, 0, 255),
    SkColorSetARGB(128, 255, 255, 0),
    SkColorSetARGB(128, 16, 16, 16)
};

static const char*const*const kChannelNames[4] = {
    kGrayChannelNames,
    kRGBChannelNames,
    kRGBChannelNames,
    kCMYKChannelNames
};
static const SkColor*const kChannelColors[4] = {
    kGrayChannelColors,
    kRGBChannelColors,
    kRGBChannelColors,
    kCMYKChannelColors
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

static constexpr int kGammaImageWidth = 500;
static constexpr int kGammaImageHeight = 500;

static void dump_transfer_fn(const SkGammas& gammas) {
    SkASSERT(gammas.channels() <= 4);
    const char*const*const channels = kChannelNames[gammas.channels() - 1];
    for (int i = 0; i < gammas.channels(); i++) {
        if (gammas.isNamed(i)) {
            switch (gammas.data(i).fNamed) {
                case kSRGB_SkGammaNamed:
                    SkDebugf("%s Transfer Function: sRGB\n", channels[i]);
                    return;
                case k2Dot2Curve_SkGammaNamed:
                    SkDebugf("%s Transfer Function: Exponent 2.2\n", channels[i]);
                    return;
                case kLinear_SkGammaNamed:
                    SkDebugf("%s Transfer Function: Linear\n", channels[i]);
                    return;
                default:
                    SkASSERT(false);
                    continue;
            }
        } else if (gammas.isValue(i)) {
            SkDebugf("%s Transfer Function: Exponent %.3f\n", channels[i], gammas.data(i).fValue);
        } else if (gammas.isParametric(i)) {
            const SkColorSpaceTransferFn& fn = gammas.data(i).params(&gammas);
            SkDebugf("%s Transfer Function: Parametric A = %.3f, B = %.3f, C = %.3f, D = %.3f, "
                     "E = %.3f, F = %.3f, G = %.3f\n", channels[i], fn.fA, fn.fB, fn.fC, fn.fD,
                     fn.fE, fn.fF, fn.fG);
        } else {
            SkASSERT(gammas.isTable(i));
            SkDebugf("%s Transfer Function: Table (%d entries)\n", channels[i],
                    gammas.data(i).fTable.fSize);
        }
    }
}

static inline float parametric(const SkColorSpaceTransferFn& fn, float x) {
    return x >= fn.fD ? powf(fn.fA*x + fn.fB, fn.fG) + fn.fE
                      : fn.fC*x + fn.fF;
}

static void draw_transfer_fn(SkCanvas* canvas, SkGammaNamed gammaNamed, const SkGammas* gammas,
                             SkColor color) {
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
    const float gammaWidth  = kGammaImageWidth - 2 * gap;
    const float gammaHeight = kGammaImageHeight - 2 * gap;
    // gamma origin point
    const float ox = gap;
    const float oy = gap + gammaHeight;
    for (int i = 0; i < channels; ++i) {
        if (kNonStandard_SkGammaNamed == gammaNamed) {
            paint.setColor(kChannelColors[channels - 1][i]);
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
    canvas->drawRect({ ox, oy - gammaHeight, ox + gammaWidth, oy }, paint);
}

//-------------------------------------------------------------------------------------------------
//------------------------------------ CLUT visualizations ----------------------------------------
static void dump_clut(const SkColorLookUpTable& clut) {
    SkDebugf("CLUT: ");
    for (int i = 0; i < clut.inputChannels(); ++i) {
        SkDebugf("[%d]", clut.gridPoints(i));
    }
    SkDebugf(" -> [%d]\n", clut.outputChannels());
}

constexpr int kClutGap = 8;
constexpr float kClutCanvasSize = 2000;

static inline int usedGridPoints(const SkColorLookUpTable& clut, int dimension) {
    const int gp = clut.gridPoints(dimension);
    return gp <= 16 ? gp : 16;
}

// how many rows of cross-section cuts to display
static inline int cut_rows(const SkColorLookUpTable& clut, int dimOrder[4]) {
    // and vertical ones for the 4th dimension (if applicable)
    return clut.inputChannels() >= 4 ? usedGridPoints(clut, dimOrder[3]) : 1;
}

// how many columns of cross-section cuts to display
static inline int cut_cols(const SkColorLookUpTable& clut, int dimOrder[4]) {
    // do horizontal cuts for the 3rd dimension (if applicable)
    return clut.inputChannels() >= 3 ? usedGridPoints(clut, dimOrder[2]) : 1;
}

// gets the width/height to use for cross-sections of a CLUT
static int cut_size(const SkColorLookUpTable& clut, int dimOrder[4]) {
    const int rows = cut_rows(clut, dimOrder);
    const int cols = cut_cols(clut, dimOrder);
    // make sure the cross-section CLUT cuts are square still by using the
    // smallest of the width/height, then adjust the gaps between accordingly
    const int cutWidth = (kClutCanvasSize - kClutGap * (1 + cols)) / cols;
    const int cutHeight = (kClutCanvasSize - kClutGap * (1 + rows)) / rows;
    return cutWidth < cutHeight ? cutWidth : cutHeight;
}

static void draw_clut(SkCanvas* canvas, const SkColorLookUpTable& clut, int dimOrder[4]) {
    dump_clut(clut);

    const int cutSize = cut_size(clut, dimOrder);
    const int rows = cut_rows(clut, dimOrder);
    const int cols = cut_cols(clut, dimOrder);
    const int cutHorizGap = (kClutCanvasSize - cutSize * cols) / (1 + cols);
    const int cutVertGap = (kClutCanvasSize - cutSize * rows) / (1 + rows);

    SkPaint paint;
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            // make sure to move at least one pixel, but otherwise move per-gridpoint
            const float xStep = 1.0f / (SkTMin(cutSize, clut.gridPoints(dimOrder[0])) - 1);
            const float yStep = 1.0f / (SkTMin(cutSize, clut.gridPoints(dimOrder[1])) - 1);
            const float ox = clut.inputChannels() >= 3 ? (1 + col) * cutHorizGap + col * cutSize
                                                       : kClutGap;
            const float oy = clut.inputChannels() >= 4 ? (1 + row) * cutVertGap + row * cutSize
                                                       : kClutGap;
            // for each cross-section cut, draw a bunch of squares whose colour is the top-left's
            // colour in the CLUT (usually this will just draw the gridpoints)
            for (float x = 0.0f; x < 1.0f; x += xStep) {
                for (float y = 0.0f; y < 1.0f; y += yStep) {
                    const float z = col / (cols - 1.0f);
                    const float w = row / (rows - 1.0f);
                    const float input[4] = {x, y, z, w};
                    float output[3];
                    clut.interp(output, input);
                    paint.setColor(SkColorSetRGB(255*output[0], 255*output[1], 255*output[2]));
                    canvas->drawRect(SkRect::MakeLTRB(ox + cutSize * x, oy + cutSize * y,
                                                      ox + cutSize * (x + xStep),
                                                      oy + cutSize * (y + yStep)), paint);
                }
            }
        }
    }
}


//-------------------------------------------------------------------------------------------------
//------------------------------------ Gamut visualizations ---------------------------------------
static void dump_matrix(const SkMatrix44& m) {
    for (int r = 0; r < 4; ++r) {
        SkDebugf("|");
        for (int c = 0; c < 4; ++c) {
            SkDebugf(" %f ", m.get(r, c));
        }
        SkDebugf("|\n");
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
        canvas->drawString("R", rgb[0].fX + 5.0f, rgb[0].fY + 75.0f, paint);
        canvas->drawString("G", rgb[1].fX + 5.0f, rgb[1].fY - 5.0f, paint);
        canvas->drawString("B", rgb[2].fX - 75.0f, rgb[2].fY - 5.0f, paint);
    }
}


//-------------------------------------------------------------------------------------------------
//----------------------------------------- Main code ---------------------------------------------
static SkBitmap transparentBitmap(int width, int height) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(width, height);
    bitmap.eraseColor(SkColorSetARGB(0, 0, 0, 0));
    return bitmap;
}

class OutputCanvas {
public:
    OutputCanvas(SkBitmap&& bitmap)
        :fBitmap(bitmap)
        ,fCanvas(fBitmap)
    {}

    bool save(std::vector<std::string>* output, const std::string& filename) {
        // Finally, encode the result to the output file.
        sk_sp<SkData> out = sk_tool_utils::EncodeImageToData(fBitmap, SkEncodedImageFormat::kPNG,
                                                             100);
        if (!out) {
            SkDebugf("Failed to encode %s output.\n", filename.c_str());
            return false;
        }
        SkFILEWStream stream(filename.c_str());
        if (!stream.write(out->data(), out->size())) {
            SkDebugf("Failed to write %s output.\n", filename.c_str());
            return false;
        }
        // record name of canvas
        output->push_back(filename);
        return true;
    }

    SkCanvas* canvas() { return &fCanvas; }

private:
    SkBitmap fBitmap;
    SkCanvas fCanvas;
};

int main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage(
            "Usage: colorspaceinfo --input <path to input image (or icc profile with --icc)> "
                                  "--output <directory to output images> "
                                  "--icc <indicates that the input is an icc profile>"
                                  "--sRGB_gamut <draw canonical sRGB gamut> "
                                  "--adobeRGB <draw canonical Adobe RGB gamut> "
                                  "--sRGB_gamma <draw sRGB gamma> "
                                  "--uncorrected <path to reencoded, uncorrected input image>\n"
            "Description: Writes visualizations of the color space to the output image(s)  ."
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

    std::unique_ptr<SkCodec> codec = nullptr;
    sk_sp<SkColorSpace> colorSpace = nullptr;
    if (FLAGS_icc) {
        colorSpace = SkColorSpace::MakeICC(data->bytes(), data->size());
    } else {
        codec.reset(SkCodec::NewFromData(data));
        colorSpace = sk_ref_sp(codec->getInfo().colorSpace());
    }

    if (!colorSpace) {
        SkDebugf("Cannot create codec or icc profile from input file.\n");
        return -1;
    }

    {
        SkColorSpaceTransferFn colorSpaceTransferFn;
        SkMatrix44 toXYZD50;
        if (colorSpace->isNumericalTransferFn(&colorSpaceTransferFn) &&
            colorSpace->toXYZD50(&toXYZD50)) {
            SkString description = SkICCGetColorProfileTag(colorSpaceTransferFn, toXYZD50);
            SkDebugf("Color Profile Description: \"%s\"\n", description.c_str());
        }
    }

    // TODO: command line tweaking of this order
    int dimOrder[4] = {0, 1, 2, 3};

    std::vector<std::string> outputFilenames;

    auto createOutputFilename = [output](const char* category, int index) -> std::string {
        std::stringstream ss;
        ss << output << '/' << category << '_' << index << ".png";
        return ss.str();
    };

    if (SkColorSpace_Base::Type::kXYZ == as_CSB(colorSpace)->type()) {
        SkDebugf("XYZ/TRC color space\n");

        // Load a graph of the CIE XYZ color gamut.
        SkBitmap gamutCanvasBitmap;
        if (!GetResourceAsBitmap("gamut.png", &gamutCanvasBitmap)) {
            SkDebugf("Program failure (could not load gamut.png).\n");
            return -1;
        }
        OutputCanvas gamutCanvas(std::move(gamutCanvasBitmap));
        // Draw the sRGB gamut if requested.
        if (FLAGS_sRGB_gamut) {
            sk_sp<SkColorSpace> sRGBSpace = SkColorSpace::MakeSRGB();
            const SkMatrix44* mat = as_CSB(sRGBSpace)->toXYZD50();
            SkASSERT(mat);
            draw_gamut(gamutCanvas.canvas(), *mat, "sRGB", 0xFFFF9394, false);
        }

        // Draw the Adobe RGB gamut if requested.
        if (FLAGS_adobeRGB) {
            sk_sp<SkColorSpace> adobeRGBSpace = SkColorSpace::MakeRGB(
                    SkColorSpace::kSRGB_RenderTargetGamma, SkColorSpace::kAdobeRGB_Gamut);
            const SkMatrix44* mat = as_CSB(adobeRGBSpace)->toXYZD50();
            SkASSERT(mat);
            draw_gamut(gamutCanvas.canvas(), *mat, "Adobe RGB", 0xFF31a9e1, false);
        }
        const SkMatrix44* mat = as_CSB(colorSpace)->toXYZD50();
        SkASSERT(mat);
        auto xyz = static_cast<SkColorSpace_XYZ*>(colorSpace.get());
        draw_gamut(gamutCanvas.canvas(), *mat, input, 0xFF000000, true);
        if (!gamutCanvas.save(&outputFilenames, createOutputFilename("gamut", 0))) {
            return -1;
        }

        OutputCanvas gammaCanvas(transparentBitmap(kGammaImageWidth, kGammaImageHeight));
        if (FLAGS_sRGB_gamma) {
            draw_transfer_fn(gammaCanvas.canvas(), kSRGB_SkGammaNamed, nullptr, 0xFFFF9394);
        }
        draw_transfer_fn(gammaCanvas.canvas(), xyz->gammaNamed(), xyz->gammas(), 0xFF000000);
        if (!gammaCanvas.save(&outputFilenames, createOutputFilename("gamma", 0))) {
            return -1;
        }
    } else {
        SkDebugf("A2B color space");
        SkColorSpace_A2B* a2b = static_cast<SkColorSpace_A2B*>(colorSpace.get());
        SkDebugf("Conversion type: ");
        switch (a2b->iccType()) {
            case SkColorSpace_Base::kRGB_ICCTypeFlag:
                SkDebugf("RGB");
                break;
            case SkColorSpace_Base::kCMYK_ICCTypeFlag:
                SkDebugf("CMYK");
                break;
            case SkColorSpace_Base::kGray_ICCTypeFlag:
                SkDebugf("Gray");
                break;
            default:
                SkASSERT(false);
                break;

        }
        SkDebugf(" -> ");
        switch (a2b->pcs()) {
            case SkColorSpace_A2B::PCS::kXYZ:
                SkDebugf("XYZ\n");
                break;
            case SkColorSpace_A2B::PCS::kLAB:
                SkDebugf("LAB\n");
                break;
        }
        int clutCount = 0;
        int gammaCount = 0;
        for (int i = 0; i < a2b->count(); ++i) {
            const SkColorSpace_A2B::Element& e = a2b->element(i);
            switch (e.type()) {
                case SkColorSpace_A2B::Element::Type::kGammaNamed: {
                    OutputCanvas gammaCanvas(transparentBitmap(kGammaImageWidth,
                                                               kGammaImageHeight));
                    if (FLAGS_sRGB_gamma) {
                        draw_transfer_fn(gammaCanvas.canvas(), kSRGB_SkGammaNamed, nullptr,
                                         0xFFFF9394);
                    }
                    draw_transfer_fn(gammaCanvas.canvas(), e.gammaNamed(), nullptr,
                                     0xFF000000);
                    if (!gammaCanvas.save(&outputFilenames,
                                          createOutputFilename("gamma", gammaCount++))) {
                        return -1;
                    }
                }
                break;
                case SkColorSpace_A2B::Element::Type::kGammas: {
                    OutputCanvas gammaCanvas(transparentBitmap(kGammaImageWidth,
                                                               kGammaImageHeight));
                    if (FLAGS_sRGB_gamma) {
                        draw_transfer_fn(gammaCanvas.canvas(), kSRGB_SkGammaNamed, nullptr,
                                         0xFFFF9394);
                    }
                    draw_transfer_fn(gammaCanvas.canvas(), kNonStandard_SkGammaNamed,
                                     &e.gammas(), 0xFF000000);
                    if (!gammaCanvas.save(&outputFilenames,
                                          createOutputFilename("gamma", gammaCount++))) {
                        return -1;
                    }
                }
                break;
                case SkColorSpace_A2B::Element::Type::kCLUT: {
                    const SkColorLookUpTable& clut = e.colorLUT();
                    const int cutSize = cut_size(clut, dimOrder);
                    const int clutWidth = clut.inputChannels() >= 3 ? kClutCanvasSize
                                                                    : 2 * kClutGap + cutSize;
                    const int clutHeight = clut.inputChannels() >= 4 ? kClutCanvasSize
                                                                     : 2 * kClutGap + cutSize;
                    OutputCanvas clutCanvas(transparentBitmap(clutWidth, clutHeight));
                    draw_clut(clutCanvas.canvas(), e.colorLUT(), dimOrder);
                    if (!clutCanvas.save(&outputFilenames,
                                         createOutputFilename("clut", clutCount++))) {
                        return -1;
                    }
                }
                break;
                case SkColorSpace_A2B::Element::Type::kMatrix:
                    dump_matrix(e.matrix());
                    break;
            }
        }
    }

    // marker to tell the web-tool the names of all images output
    SkDebugf("=========\n");
    for (const std::string& filename : outputFilenames) {
        SkDebugf("%s\n", filename.c_str());
    }
    if (!FLAGS_icc) {
        SkDebugf("%s\n", input);
    }
    // Also, if requested, decode and reencode the uncorrected input image.
    if (!FLAGS_uncorrected.isEmpty() && !FLAGS_icc) {
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
