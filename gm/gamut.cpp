/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkSurface.h"
#include "SkGradientShader.h"
#include "SkPM4fPriv.h"

static const int gRectSize = 50;
static const SkScalar gScalarSize = SkIntToScalar(gRectSize);
static const int gTestWidth = 700;
static const int gTestHeight = 300;

struct CellRenderer {
    virtual void draw(SkCanvas* canvas) = 0;
    virtual const char* label() = 0;
    virtual ~CellRenderer() {}
};

struct PaintColorCellRenderer : public CellRenderer {
    PaintColorCellRenderer(SkColor color) : fColor(color) {}
    void draw(SkCanvas* canvas) override {
        canvas->drawColor(fColor);
    }
    const char* label() override {
        return "Paint Color";
    }
protected:
    SkColor fColor;
};

struct BitmapCellRenderer : public CellRenderer {
    BitmapCellRenderer(SkColor color, SkFilterQuality quality, float scale = 1.0f)
        : fQuality(quality) {
        int scaledSize = SkFloatToIntRound(scale * gRectSize);
        fBitmap.allocPixels(SkImageInfo::MakeS32(scaledSize, scaledSize, kPremul_SkAlphaType));
        fBitmap.eraseColor(color);
        const char* qualityNames[] = { "None", "Low", "Medium", "High" };
        fLabel = SkStringPrintf("Bitmap (%s)", qualityNames[quality]);
    }
    void draw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setFilterQuality(fQuality);
        canvas->drawBitmapRect(fBitmap, SkRect::MakeIWH(gRectSize, gRectSize), &paint);
    }
    const char* label() override {
        return fLabel.c_str();
    }
protected:
    SkFilterQuality fQuality;
    SkBitmap        fBitmap;
    SkString        fLabel;
};

struct GradientCellRenderer : public CellRenderer {
    GradientCellRenderer(SkColor colorOne, SkColor colorTwo, bool manyStops) {
        fColors[0] = colorOne;
        fColors[1] = colorTwo;
        fManyStops = manyStops;
    }
    void draw(SkCanvas* canvas) override {
        SkPoint points[2] = {
            SkPoint::Make(0, 0),
            SkPoint::Make(0, gScalarSize)
        };
        SkPaint paint;
        if (fManyStops) {
            SkColor colors[4] ={
                fColors[0], fColors[0], fColors[1], fColors[1]
            };
            paint.setShader(SkGradientShader::MakeLinear(points, colors, nullptr, 4,
                                                         SkShader::kClamp_TileMode));
        } else {
            paint.setShader(SkGradientShader::MakeLinear(points, fColors, nullptr, 2,
                                                         SkShader::kClamp_TileMode));
        }
        canvas->drawPaint(paint);
    }
    const char* label() override {
        return "Linear Gradient";
    }
protected:
    SkColor fColors[2];
    bool fManyStops;
};

struct VerticesCellRenderer : public CellRenderer {
    VerticesCellRenderer(SkColor colorOne, SkColor colorTwo) {
        fColors[0] = fColors[1] = colorOne;
        fColors[2] = fColors[3] = colorTwo;
    }
    void draw(SkCanvas* canvas) override {
        SkPaint paint;
        SkPoint vertices[4] = {
            SkPoint::Make(0, 0),
            SkPoint::Make(gScalarSize, 0),
            SkPoint::Make(gScalarSize, gScalarSize),
            SkPoint::Make(0, gScalarSize)
        };
        canvas->drawVertices(SkCanvas::kTriangleFan_VertexMode, 4, vertices, nullptr, fColors,
                             nullptr, nullptr, 0, paint);
    }
    const char* label() override {
        return "Vertices";
    }
protected:
    SkColor fColors[4];
};

static void draw_gamut_grid(SkCanvas* canvas, SkTArray<SkAutoTDelete<CellRenderer>>& renderers) {
    // We want our colors in our wide gamut to be obviously visibly distorted from sRGB, so we use
    // Wide Gamut RGB (with sRGB gamma, for HW acceleration) as the working space for this test:
    const float gWideGamutRGB_toXYZD50[]{
        0.7161046f, 0.1009296f, 0.1471858f,  // -> X
        0.2581874f, 0.7249378f, 0.0168748f,  // -> Y
        0.0000000f, 0.0517813f, 0.7734287f,  // -> Z
    };

    SkMatrix44 wideGamutRGB_toXYZD50(SkMatrix44::kUninitialized_Constructor);
    wideGamutRGB_toXYZD50.set3x3RowMajorf(gWideGamutRGB_toXYZD50);

    // Use the original canvas' color type, but account for gamma requirements
    SkColorType colorType = canvas->imageInfo().colorType();
    if (kUnknown_SkColorType == colorType) {
        // This can happen when recording (in which case we'll just use 8888 offscreen surfaces)
        colorType = kN32_SkColorType;
    }
    auto srgbCS = SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named);
    auto wideCS = SkColorSpace::NewRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                       wideGamutRGB_toXYZD50);
    switch (colorType) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            break;
        case kRGBA_F16_SkColorType:
            srgbCS = srgbCS->makeLinearGamma();
            wideCS = wideCS->makeLinearGamma();
            break;
        default:
            return;
    }

    // Make our two working surfaces (one sRGB, one Adobe)
    SkImageInfo srgbGamutInfo = SkImageInfo::Make(gRectSize, gRectSize, colorType,
                                                  kPremul_SkAlphaType, srgbCS);
    SkImageInfo wideGamutInfo = SkImageInfo::Make(gRectSize, gRectSize, colorType,
                                                  kPremul_SkAlphaType, wideCS);
    // readPixels doesn't do color conversion (yet), so we can use it to see the raw (wide) data
    SkImageInfo dstInfo = srgbGamutInfo.makeColorSpace(nullptr);
    sk_sp<SkSurface> srgbGamutSurface = canvas->makeSurface(srgbGamutInfo);
    if (!srgbGamutSurface) {
        srgbGamutSurface = SkSurface::MakeRaster(srgbGamutInfo);
    }
    sk_sp<SkSurface> wideGamutSurface = canvas->makeSurface(wideGamutInfo);
    if (!wideGamutSurface) {
        wideGamutSurface = SkSurface::MakeRaster(wideGamutInfo);
    }
    SkCanvas* srgbGamutCanvas = srgbGamutSurface->getCanvas();
    SkCanvas* wideGamutCanvas = wideGamutSurface->getCanvas();

    SkPaint textPaint;
    textPaint.setAntiAlias(true);
    textPaint.setColor(SK_ColorWHITE);
    sk_tool_utils::set_portable_typeface(&textPaint);

    SkScalar x = 0, y = 0;
    SkScalar textHeight = textPaint.getFontSpacing();

    for (const auto& renderer : renderers) {
        srgbGamutCanvas->clear(SK_ColorBLACK);
        renderer->draw(srgbGamutCanvas);
        wideGamutCanvas->clear(SK_ColorBLACK);
        renderer->draw(wideGamutCanvas);

        canvas->drawText(renderer->label(), strlen(renderer->label()), x, y + textHeight,
                         textPaint);

        SkBitmap srgbBitmap;
        srgbBitmap.setInfo(dstInfo);
        srgbGamutCanvas->readPixels(&srgbBitmap, 0, 0);
        canvas->drawBitmap(srgbBitmap, x, y + textHeight + 5);
        x += (gScalarSize + 1);

        SkBitmap wideBitmap;
        wideBitmap.setInfo(dstInfo);
        wideGamutCanvas->readPixels(&wideBitmap, 0, 0);
        canvas->drawBitmap(wideBitmap, x, y + textHeight + 5);
        x += (gScalarSize + 10);

        if (x + (2 * gScalarSize + 1) > gTestWidth) {
            x = 0;
            y += (textHeight + gScalarSize + 10);
        }
    }
}

DEF_SIMPLE_GM_BG(gamut, canvas, gTestWidth, gTestHeight, SK_ColorBLACK) {
    SkTArray<SkAutoTDelete<CellRenderer>> renderers;

    // sRGB primaries, rendered as paint color
    renderers.push_back(new PaintColorCellRenderer(SK_ColorRED));
    renderers.push_back(new PaintColorCellRenderer(SK_ColorGREEN));

    // sRGB primaries, rendered as bitmaps
    renderers.push_back(new BitmapCellRenderer(SK_ColorRED, kNone_SkFilterQuality));
    renderers.push_back(new BitmapCellRenderer(SK_ColorGREEN, kLow_SkFilterQuality));
    // Larger bitmap to trigger mipmaps
    renderers.push_back(new BitmapCellRenderer(SK_ColorRED, kMedium_SkFilterQuality, 2.0f));
    // Smaller bitmap to trigger bicubic
    renderers.push_back(new BitmapCellRenderer(SK_ColorGREEN, kHigh_SkFilterQuality, 0.5f));

    // Various gradients involving sRGB primaries and white/black

    // First with just two stops (implemented with uniforms on GPU)
    renderers.push_back(new GradientCellRenderer(SK_ColorRED, SK_ColorGREEN, false));
    renderers.push_back(new GradientCellRenderer(SK_ColorGREEN, SK_ColorBLACK, false));
    renderers.push_back(new GradientCellRenderer(SK_ColorGREEN, SK_ColorWHITE, false));

    // ... and then with four stops (implemented with textures on GPU)
    renderers.push_back(new GradientCellRenderer(SK_ColorRED, SK_ColorGREEN, true));
    renderers.push_back(new GradientCellRenderer(SK_ColorGREEN, SK_ColorBLACK, true));
    renderers.push_back(new GradientCellRenderer(SK_ColorGREEN, SK_ColorWHITE, true));

    // Vertex colors
    renderers.push_back(new VerticesCellRenderer(SK_ColorRED, SK_ColorRED));
    renderers.push_back(new VerticesCellRenderer(SK_ColorRED, SK_ColorGREEN));

    draw_gamut_grid(canvas, renderers);
}
