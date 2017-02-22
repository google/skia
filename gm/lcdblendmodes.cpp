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

constexpr int kColWidth = 180;
constexpr int kNumCols = 4;
constexpr int kWidth = kColWidth * kNumCols;
constexpr int kHeight = 750;

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
        surfPaint.setBlendMode(SkBlendMode::kSrcOver);
        surface->draw(canvas, 0, 0, &surfPaint);
    }

    void drawColumn(SkCanvas* canvas, SkColor backgroundColor, SkColor textColor, bool useGrad) {
        const struct {
            SkBlendMode fMode;
            const char* fLabel;
        } gModes[] = {
            { SkBlendMode::kClear,        "Clear"       },
            { SkBlendMode::kSrc,          "Src"         },
            { SkBlendMode::kDst,          "Dst"         },
            { SkBlendMode::kSrcOver,      "SrcOver"     },
            { SkBlendMode::kDstOver,      "DstOver"     },
            { SkBlendMode::kSrcIn,        "SrcIn"       },
            { SkBlendMode::kDstIn,        "DstIn"       },
            { SkBlendMode::kSrcOut,       "SrcOut"      },
            { SkBlendMode::kDstOut,       "DstOut"      },
            { SkBlendMode::kSrcATop,      "SrcATop"     },
            { SkBlendMode::kDstATop,      "DstATop"     },
            { SkBlendMode::kXor,          "Xor"         },
            { SkBlendMode::kPlus,         "Plus"        },
            { SkBlendMode::kModulate,     "Modulate"    },
            { SkBlendMode::kScreen,       "Screen"      },
            { SkBlendMode::kOverlay,      "Overlay"     },
            { SkBlendMode::kDarken,       "Darken"      },
            { SkBlendMode::kLighten,      "Lighten"     },
            { SkBlendMode::kColorDodge,   "ColorDodge"  },
            { SkBlendMode::kColorBurn,    "ColorBurn"   },
            { SkBlendMode::kHardLight,    "HardLight"   },
            { SkBlendMode::kSoftLight,    "SoftLight"   },
            { SkBlendMode::kDifference,   "Difference"  },
            { SkBlendMode::kExclusion,    "Exclusion"   },
            { SkBlendMode::kMultiply,     "Multiply"    },
            { SkBlendMode::kHue,          "Hue"         },
            { SkBlendMode::kSaturation,   "Saturation"  },
            { SkBlendMode::kColor,        "Color"       },
            { SkBlendMode::kLuminosity,   "Luminosity"  },
        };
        // Draw background rect
        SkPaint backgroundPaint;
        backgroundPaint.setColor(backgroundColor);
        canvas->drawRect(SkRect::MakeIWH(kColWidth, kHeight), backgroundPaint);
        SkScalar y = fTextHeight;
        for (size_t m = 0; m < SK_ARRAY_COUNT(gModes); m++) {
            SkPaint paint;
            paint.setColor(textColor);
            paint.setAntiAlias(true);
            paint.setSubpixelText(true);
            paint.setLCDRenderText(true);
            paint.setTextSize(fTextHeight);
            paint.setBlendMode(gModes[m].fMode);
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
