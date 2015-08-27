/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkBitmap.h"
#include "SkGradientShader.h"
#include "SkShader.h"
#include "SkXfermode.h"
#include "../src/fonts/SkGScalerContext.h"

namespace skiagm {

static uint16_t gData[] = { 0xFFFF, 0xCCCF, 0xCCCF, 0xFFFF };

class ColorTypeXfermodeGM : public GM {
public:
    const static int W = 64;
    const static int H = 64;
    ColorTypeXfermodeGM()
        : fColorType(nullptr) {
    }

    virtual ~ColorTypeXfermodeGM() {
        SkSafeUnref(fColorType);
    }

protected:
    void onOnceBeforeDraw() override {
        const SkColor colors[] = {
            SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE,
            SK_ColorMAGENTA, SK_ColorCYAN, SK_ColorYELLOW
        };
        SkMatrix local;
        local.setRotate(180);
        SkShader* s = SkGradientShader::CreateSweep(0,0, colors, nullptr,
                                                    SK_ARRAY_COUNT(colors), 0, &local);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setShader(s)->unref();

        SkTypeface* orig = sk_tool_utils::create_portable_typeface("serif",
                                                            SkTypeface::kBold);
        if (nullptr == orig) {
            orig = SkTypeface::RefDefault();
        }
        fColorType = new SkGTypeface(orig, paint);
        orig->unref();

        fBG.installPixels(SkImageInfo::Make(2, 2, kARGB_4444_SkColorType,
                                            kOpaque_SkAlphaType), gData, 4);
    }

    virtual SkString onShortName() override {
        return SkString("colortype_xfermodes");
    }

    virtual SkISize onISize() override {
        return SkISize::Make(400, 640);
    }

    virtual void onDraw(SkCanvas* canvas) override {
        canvas->translate(SkIntToScalar(10), SkIntToScalar(20));

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

        const SkScalar w = SkIntToScalar(W);
        const SkScalar h = SkIntToScalar(H);
        SkMatrix m;
        m.setScale(SkIntToScalar(6), SkIntToScalar(6));
        SkShader* s = SkShader::CreateBitmapShader(fBG,
                                                   SkShader::kRepeat_TileMode,
                                                   SkShader::kRepeat_TileMode,
                                                   &m);

        SkPaint labelP;
        labelP.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&labelP);
        labelP.setTextAlign(SkPaint::kCenter_Align);

        SkPaint textP;
        textP.setAntiAlias(true);
        textP.setTypeface(fColorType);
        textP.setTextSize(SkIntToScalar(70));

        const int W = 5;

        SkScalar x0 = 0;
        SkScalar y0 = 0;
        SkScalar x = x0, y = y0;
        for (size_t i = 0; i < SK_ARRAY_COUNT(gModes); i++) {
            SkXfermode* mode = SkXfermode::Create(gModes[i].fMode);
            SkAutoUnref aur(mode);
            SkRect r;
            r.set(x, y, x+w, y+h);

            SkPaint p;
            p.setStyle(SkPaint::kFill_Style);
            p.setShader(s);
            canvas->drawRect(r, p);

            r.inset(-SK_ScalarHalf, -SK_ScalarHalf);
            p.setStyle(SkPaint::kStroke_Style);
            p.setShader(nullptr);
            canvas->drawRect(r, p);

            textP.setXfermode(mode);
            canvas->drawText("H", 1, x+ w/10.f, y + 7.f*h/8.f, textP);
#if 1
            canvas->drawText(gModes[i].fLabel, strlen(gModes[i].fLabel),
                             x + w/2, y - labelP.getTextSize()/2, labelP);
#endif
            x += w + SkIntToScalar(10);
            if ((i % W) == W - 1) {
                x = x0;
                y += h + SkIntToScalar(30);
            }
        }
        s->unref();
    }

private:
    SkBitmap    fBG;
    SkTypeface* fColorType;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ColorTypeXfermodeGM; }
static GMRegistry reg(MyFactory);

}
