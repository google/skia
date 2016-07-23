/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


/*
 * Tests text rendering with LCD and the various blend modes.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkSurface.h"

namespace skiagm {

static const int kColWidth = 180;
static const int kNumCols = 4;
static const int kWidth = kColWidth * kNumCols;
static const int kHeight = 750;

static sk_sp<SkShader> make_shader(const SkRect& bounds) {
    const SkPoint pts[] = {
        { bounds.left(), bounds.top() },
        { bounds.right(), bounds.bottom() },
    };
    const SkColor colors[] = {
        SK_ColorRED, SK_ColorGREEN,
    };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, SK_ARRAY_COUNT(colors),
                                        SkShader::kRepeat_TileMode);
}

class LcdBlendGM : public skiagm::GM {
public:
    LcdBlendGM() {
        const int kPointSize = 25;
        fTextHeight = SkIntToScalar(kPointSize);
    }

protected:
    SkString onShortName() override {
        return SkString("lcdblendmodes");
    }

    void onOnceBeforeDraw() override {
        fCheckerboard = sk_tool_utils::create_checkerboard_shader(SK_ColorBLACK, SK_ColorWHITE, 4);
    }

    SkISize onISize() override { return SkISize::Make(kWidth, kHeight); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p;
        p.setAntiAlias(false);
        p.setStyle(SkPaint::kFill_Style);
        p.setShader(fCheckerboard);
        SkRect r = SkRect::MakeWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight));
        canvas->drawRect(r, p);

        SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
        auto surface(canvas->makeSurface(info));
        if (nullptr == surface) {
            surface = SkSurface::MakeRaster(info);
        }

        SkCanvas* surfCanvas = surface->getCanvas();
        this->drawColumn(surfCanvas, SK_ColorBLACK, SK_ColorWHITE, false);
        surfCanvas->translate(SkIntToScalar(kColWidth), 0);
        this->drawColumn(surfCanvas, SK_ColorWHITE, SK_ColorBLACK, false);
        surfCanvas->translate(SkIntToScalar(kColWidth), 0);
        this->drawColumn(surfCanvas, SK_ColorGREEN, SK_ColorMAGENTA, false);
        surfCanvas->translate(SkIntToScalar(kColWidth), 0);
        this->drawColumn(surfCanvas, SK_ColorCYAN, SK_ColorMAGENTA, true);

        SkPaint surfPaint;
        surfPaint.setXfermode(SkXfermode::Make(SkXfermode::kSrcOver_Mode));
        surface->draw(canvas, 0, 0, &surfPaint);
    }

    void drawColumn(SkCanvas* canvas, SkColor backgroundColor, SkColor textColor, bool useGrad) {
        const struct {
            SkXfermode::Mode  fMode;
            const char*       fLabel;
        } gModes[] = {
            { SkXfermode::kClear_Mode,        "Clear"       },
            { SkXfermode::kSrc_Mode,          "Src"         },
            { SkXfermode::kDst_Mode,          "Dst"         },
            { SkXfermode::kSrcOver_Mode,      "SrcOver"     },
            { SkXfermode::kDstOver_Mode,      "DstOver"     },
            { SkXfermode::kSrcIn_Mode,        "SrcIn"       },
            { SkXfermode::kDstIn_Mode,        "DstIn"       },
            { SkXfermode::kSrcOut_Mode,       "SrcOut"      },
            { SkXfermode::kDstOut_Mode,       "DstOut"      },
            { SkXfermode::kSrcATop_Mode,      "SrcATop"     },
            { SkXfermode::kDstATop_Mode,      "DstATop"     },
            { SkXfermode::kXor_Mode,          "Xor"         },
            { SkXfermode::kPlus_Mode,         "Plus"        },
            { SkXfermode::kModulate_Mode,     "Modulate"    },
            { SkXfermode::kScreen_Mode,       "Screen"      },
            { SkXfermode::kOverlay_Mode,      "Overlay"     },
            { SkXfermode::kDarken_Mode,       "Darken"      },
            { SkXfermode::kLighten_Mode,      "Lighten"     },
            { SkXfermode::kColorDodge_Mode,   "ColorDodge"  },
            { SkXfermode::kColorBurn_Mode,    "ColorBurn"   },
            { SkXfermode::kHardLight_Mode,    "HardLight"   },
            { SkXfermode::kSoftLight_Mode,    "SoftLight"   },
            { SkXfermode::kDifference_Mode,   "Difference"  },
            { SkXfermode::kExclusion_Mode,    "Exclusion"   },
            { SkXfermode::kMultiply_Mode,     "Multiply"    },
            { SkXfermode::kHue_Mode,          "Hue"         },
            { SkXfermode::kSaturation_Mode,   "Saturation"  },
            { SkXfermode::kColor_Mode,        "Color"       },
            { SkXfermode::kLuminosity_Mode,   "Luminosity"  },
        };
        // Draw background rect
        SkPaint backgroundPaint;
        backgroundPaint.setColor(backgroundColor);
        canvas->drawRectCoords(0, 0, SkIntToScalar(kColWidth), SkIntToScalar(kHeight),
                               backgroundPaint);
        SkScalar y = fTextHeight;
        for (size_t m = 0; m < SK_ARRAY_COUNT(gModes); m++) {
            SkPaint paint;
            paint.setColor(textColor);
            paint.setAntiAlias(true);
            paint.setSubpixelText(true);
            paint.setLCDRenderText(true);
            paint.setTextSize(fTextHeight);
            paint.setXfermode(SkXfermode::Make(gModes[m].fMode));
            sk_tool_utils::set_portable_typeface(&paint);
            if (useGrad) {
                SkRect r;
                r.setXYWH(0, y - fTextHeight, SkIntToScalar(kColWidth), fTextHeight);
                paint.setShader(make_shader(r));
            }
            SkString string(gModes[m].fLabel);
            canvas->drawText(gModes[m].fLabel, string.size(), 0, y, paint);
            y+=fTextHeight;
        }
    }

private:
    SkScalar fTextHeight;
    sk_sp<SkShader> fCheckerboard;
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new LcdBlendGM; )
}
