/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkArithmeticMode.h"
#include "SkOffsetImageFilter.h"
#include "SkXfermodeImageFilter.h"
#include "SkBitmapSource.h"

#define WIDTH 600
#define HEIGHT 600
#define MARGIN 12

namespace skiagm {

class XfermodeImageFilterGM : public GM {
public:
    XfermodeImageFilterGM(){
        this->setBGColor(0xFF000000);
    }

protected:
    SkString onShortName() override {
        return SkString("xfermodeimagefilter");
    }

    void make_bitmap() {
        fBitmap.allocN32Pixels(80, 80);
        SkCanvas canvas(fBitmap);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setColor(0xD000D000);
        paint.setTextSize(SkIntToScalar(96));
        const char* str = "e";
        canvas.drawText(str, strlen(str), SkIntToScalar(15), SkIntToScalar(65), paint);
    }

    SkISize onISize() override {
        return SkISize::Make(WIDTH, HEIGHT);
    }

    static void drawClippedBitmap(SkCanvas* canvas, const SkBitmap& bitmap, const SkPaint& paint,
                           int x, int y) {
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        canvas->clipRect(SkRect::MakeWH(
            SkIntToScalar(bitmap.width()), SkIntToScalar(bitmap.height())));
        canvas->drawBitmap(bitmap, 0, 0, &paint);
        canvas->restore();
    }

    static void drawClippedPaint(SkCanvas* canvas, const SkRect& rect, const SkPaint& paint,
                          int x, int y) {
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        canvas->clipRect(rect);
        canvas->drawPaint(paint);
        canvas->restore();
    }

    void onOnceBeforeDraw() override {
        make_bitmap();

        fCheckerboard.allocN32Pixels(80, 80);
        SkCanvas checkerboardCanvas(fCheckerboard);
        sk_tool_utils::draw_checkerboard(&checkerboardCanvas,
                sk_tool_utils::color_to_565(0xFFA0A0A0),
                sk_tool_utils::color_to_565(0xFF404040), 8);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);
        SkPaint paint;

        const struct {
            SkXfermode::Mode  fMode;
            const char*         fLabel;
        } gModes[] = {
            { SkXfermode::kClear_Mode,    "Clear"     },
            { SkXfermode::kSrc_Mode,      "Src"       },
            { SkXfermode::kDst_Mode,      "Dst"       },
            { SkXfermode::kSrcOver_Mode,  "SrcOver"   },
            { SkXfermode::kDstOver_Mode,  "DstOver"   },
            { SkXfermode::kSrcIn_Mode,    "SrcIn"     },
            { SkXfermode::kDstIn_Mode,    "DstIn"     },
            { SkXfermode::kSrcOut_Mode,   "SrcOut"    },
            { SkXfermode::kDstOut_Mode,   "DstOut"    },
            { SkXfermode::kSrcATop_Mode,  "SrcATop"   },
            { SkXfermode::kDstATop_Mode,  "DstATop"   },
            { SkXfermode::kXor_Mode,      "Xor"       },

            { SkXfermode::kPlus_Mode,         "Plus"          },
            { SkXfermode::kModulate_Mode,     "Modulate"      },
            { SkXfermode::kScreen_Mode,       "Screen"        },
            { SkXfermode::kOverlay_Mode,      "Overlay"       },
            { SkXfermode::kDarken_Mode,       "Darken"        },
            { SkXfermode::kLighten_Mode,      "Lighten"       },
            { SkXfermode::kColorDodge_Mode,   "ColorDodge"    },
            { SkXfermode::kColorBurn_Mode,    "ColorBurn"     },
            { SkXfermode::kHardLight_Mode,    "HardLight"     },
            { SkXfermode::kSoftLight_Mode,    "SoftLight"     },
            { SkXfermode::kDifference_Mode,   "Difference"    },
            { SkXfermode::kExclusion_Mode,    "Exclusion"     },
            { SkXfermode::kMultiply_Mode,     "Multiply"      },
            { SkXfermode::kHue_Mode,          "Hue"           },
            { SkXfermode::kSaturation_Mode,   "Saturation"    },
            { SkXfermode::kColor_Mode,        "Color"         },
            { SkXfermode::kLuminosity_Mode,   "Luminosity"    },
        };

        int x = 0, y = 0;
        SkAutoTUnref<SkImageFilter> background(SkBitmapSource::Create(fCheckerboard));
        for (size_t i = 0; i < SK_ARRAY_COUNT(gModes); i++) {
            SkAutoTUnref<SkXfermode> mode(SkXfermode::Create(gModes[i].fMode));
            SkAutoTUnref<SkImageFilter> filter(SkXfermodeImageFilter::Create(mode, background));
            paint.setImageFilter(filter);
            drawClippedBitmap(canvas, fBitmap, paint, x, y);
            x += fBitmap.width() + MARGIN;
            if (x + fBitmap.width() > WIDTH) {
                x = 0;
                y += fBitmap.height() + MARGIN;
            }
        }
        // Test arithmetic mode as image filter
        SkAutoTUnref<SkXfermode> mode(SkArithmeticMode::Create(0, SK_Scalar1, SK_Scalar1, 0));
        SkAutoTUnref<SkImageFilter> filter(SkXfermodeImageFilter::Create(mode, background));
        paint.setImageFilter(filter);
        drawClippedBitmap(canvas, fBitmap, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        // Test NULL mode
        filter.reset(SkXfermodeImageFilter::Create(NULL, background));
        paint.setImageFilter(filter);
        drawClippedBitmap(canvas, fBitmap, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        SkRect clipRect = SkRect::MakeWH(SkIntToScalar(fBitmap.width() + 4),
                                         SkIntToScalar(fBitmap.height() + 4));
        // Test offsets on SrcMode (uses fixed-function blend)
        SkAutoTUnref<SkImageFilter> foreground(SkBitmapSource::Create(fBitmap));
        SkAutoTUnref<SkImageFilter> offsetForeground(SkOffsetImageFilter::Create(
            SkIntToScalar(4), SkIntToScalar(-4), foreground));
        SkAutoTUnref<SkImageFilter> offsetBackground(SkOffsetImageFilter::Create(
            SkIntToScalar(4), SkIntToScalar(4), background));
        mode.reset(SkXfermode::Create(SkXfermode::kSrcOver_Mode));
        filter.reset(SkXfermodeImageFilter::Create(mode, offsetBackground, offsetForeground));
        paint.setImageFilter(filter);
        drawClippedPaint(canvas, clipRect, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        // Test offsets on Darken (uses shader blend)
        mode.reset(SkXfermode::Create(SkXfermode::kDarken_Mode));
        filter.reset(SkXfermodeImageFilter::Create(mode, offsetBackground, offsetForeground));
        paint.setImageFilter(filter);
        drawClippedPaint(canvas, clipRect, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        // Test cropping
        static const size_t nbSamples = 3;
        SkXfermode::Mode sampledModes[nbSamples] = {SkXfermode::kOverlay_Mode,
                                                    SkXfermode::kSrcOver_Mode,
                                                    SkXfermode::kPlus_Mode};
        int offsets[nbSamples][4] = {{ 10,  10, -16, -16},
                                     { 10,  10,  10,  10},
                                     {-10, -10,  -6,  -6}};
        for (size_t i = 0; i < nbSamples; ++i) {
            SkIRect cropRect = SkIRect::MakeXYWH(offsets[i][0],
                                                 offsets[i][1],
                                                 fBitmap.width()  + offsets[i][2],
                                                 fBitmap.height() + offsets[i][3]);
            SkImageFilter::CropRect rect(SkRect::Make(cropRect));
            mode.reset(SkXfermode::Create(sampledModes[i]));
            filter.reset(SkXfermodeImageFilter::Create(
                                    mode, offsetBackground, offsetForeground, &rect));
            paint.setImageFilter(filter);
            drawClippedPaint(canvas, clipRect, paint, x, y);
            x += fBitmap.width() + MARGIN;
            if (x + fBitmap.width() > WIDTH) {
                x = 0;
                y += fBitmap.height() + MARGIN;
            }
        }
    }
private:
    SkBitmap fBitmap, fCheckerboard;
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new XfermodeImageFilterGM; );

}
