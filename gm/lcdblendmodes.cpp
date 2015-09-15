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

namespace skiagm {

static const int kColWidth = 180;
static const int kNumCols = 4;
static const int kWidth = kColWidth * kNumCols;
static const int kHeight = 750;

static SkShader* make_shader(const SkRect& bounds) {
    const SkPoint pts[] = {
        { bounds.left(), bounds.top() },
        { bounds.right(), bounds.bottom() },
    };
    const SkColor colors[] = {
        SK_ColorRED, SK_ColorGREEN,
    };
    return SkGradientShader::CreateLinear(pts,
                                          colors, nullptr, SK_ARRAY_COUNT(colors),
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
        SkString name("lcdblendmodes");
        name.append(sk_tool_utils::major_platform_os_name());
        return name;
    }
    
    SkISize onISize() override { return SkISize::Make(kWidth, kHeight); }
    
    void onDraw(SkCanvas* canvas) override {
        this->drawColumn(canvas, SK_ColorBLACK, SK_ColorWHITE, false);
        canvas->translate(SkIntToScalar(kColWidth), 0);
        this->drawColumn(canvas, SK_ColorWHITE, SK_ColorBLACK, false);
        canvas->translate(SkIntToScalar(kColWidth), 0);
        this->drawColumn(canvas, SK_ColorGREEN, SK_ColorMAGENTA, false);
        canvas->translate(SkIntToScalar(kColWidth), 0);
        this->drawColumn(canvas, SK_ColorCYAN, SK_ColorMAGENTA, true);
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
            SkAutoTUnref<SkXfermode> xfermode(SkXfermode::Create(gModes[m].fMode));
            SkPaint paint;
            paint.setColor(textColor);
            paint.setAntiAlias(true);
            paint.setSubpixelText(true);
            paint.setLCDRenderText(true);
            paint.setTextSize(fTextHeight);
            paint.setXfermode(xfermode);
            sk_tool_utils::set_portable_typeface(&paint);
            if (useGrad) {
                SkRect r;
                r.setXYWH(0, y - fTextHeight, SkIntToScalar(kColWidth), fTextHeight);
                paint.setShader(make_shader(r))->unref();
            }
            SkString string(gModes[m].fLabel);
            canvas->drawText(gModes[m].fLabel, string.size(), 0, y, paint);
            y+=fTextHeight;
        }
    }
    
private:
    SkScalar fTextHeight;
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new LcdBlendGM; )
}
