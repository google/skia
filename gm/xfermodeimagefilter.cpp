/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
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
    XfermodeImageFilterGM() : fInitialized(false) {
        this->setBGColor(0xFF000000);
    }

protected:
    virtual SkString onShortName() {
        return SkString("xfermodeimagefilter");
    }

    void make_bitmap() {
        fBitmap.setConfig(SkBitmap::kARGB_8888_Config, 80, 80);
        fBitmap.allocPixels();
        SkDevice device(fBitmap);
        SkCanvas canvas(&device);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xD000D000);
        paint.setTextSize(SkIntToScalar(96));
        const char* str = "e";
        canvas.drawText(str, strlen(str), SkIntToScalar(15), SkIntToScalar(65), paint);
    }

    void make_checkerboard() {
        fCheckerboard.setConfig(SkBitmap::kARGB_8888_Config, 80, 80);
        fCheckerboard.allocPixels();
        SkDevice device(fCheckerboard);
        SkCanvas canvas(&device);
        canvas.clear(0x00000000);
        SkPaint darkPaint;
        darkPaint.setColor(0xFF404040);
        SkPaint lightPaint;
        lightPaint.setColor(0xFFA0A0A0);
        for (int y = 0; y < 80; y += 16) {
          for (int x = 0; x < 80; x += 16) {
            canvas.save();
            canvas.translate(SkIntToScalar(x), SkIntToScalar(y));
            canvas.drawRect(SkRect::MakeXYWH(0, 0, 8, 8), darkPaint);
            canvas.drawRect(SkRect::MakeXYWH(8, 0, 8, 8), lightPaint);
            canvas.drawRect(SkRect::MakeXYWH(0, 8, 8, 8), lightPaint);
            canvas.drawRect(SkRect::MakeXYWH(8, 8, 8, 8), darkPaint);
            canvas.restore();
          }
        }
    }

    virtual SkISize onISize() {
        return make_isize(WIDTH, HEIGHT);
    }

    void drawClippedBitmap(SkCanvas* canvas, const SkBitmap& bitmap, const SkPaint& paint,
                           SkScalar x, SkScalar y) {
        canvas->save();
        canvas->clipRect(SkRect::MakeXYWH(x, y,
            SkIntToScalar(bitmap.width()), SkIntToScalar(bitmap.height())));
        canvas->drawBitmap(bitmap, x, y, &paint);
        canvas->restore();
    }

    virtual void onDraw(SkCanvas* canvas) {
        if (!fInitialized) {
            make_bitmap();
            make_checkerboard();
            fInitialized = true;
        }
        canvas->clear(0x00000000);
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
        SkAutoTUnref<SkImageFilter> background(SkNEW_ARGS(SkBitmapSource, (fCheckerboard)));
        for (size_t i = 0; i < SK_ARRAY_COUNT(gModes); i++) {
            SkAutoTUnref<SkXfermode> mode(SkXfermode::Create(gModes[i].fMode));
            SkAutoTUnref<SkImageFilter> filter(SkNEW_ARGS(
                SkXfermodeImageFilter, (mode, background)));
            paint.setImageFilter(filter);
            drawClippedBitmap(canvas, fBitmap, paint, SkIntToScalar(x), SkIntToScalar(y));
            x += fBitmap.width() + MARGIN;
            if (x + fBitmap.width() > WIDTH) {
                x = 0;
                y += fBitmap.height() + MARGIN;
            }
        }
        // Test arithmetic mode as image filter
        SkAutoTUnref<SkXfermode> mode(SkArithmeticMode::Create(0, SK_Scalar1, SK_Scalar1, 0));
        SkAutoTUnref<SkImageFilter> filter(SkNEW_ARGS(SkXfermodeImageFilter, (mode, background)));
        paint.setImageFilter(filter);
        drawClippedBitmap(canvas, fBitmap, paint, SkIntToScalar(x), SkIntToScalar(y));
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        // Test NULL mode
        filter.reset(SkNEW_ARGS(SkXfermodeImageFilter, (NULL, background)));
        paint.setImageFilter(filter);
        drawClippedBitmap(canvas, fBitmap, paint, SkIntToScalar(x), SkIntToScalar(y));
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        // Test offsets on SrcMode (uses fixed-function blend)
        SkAutoTUnref<SkImageFilter> foreground(SkNEW_ARGS(SkBitmapSource, (fBitmap)));
        SkAutoTUnref<SkImageFilter> offsetForeground(SkNEW_ARGS(SkOffsetImageFilter,
            (SkIntToScalar(4), SkIntToScalar(-4), foreground)));
        SkAutoTUnref<SkImageFilter> offsetBackground(SkNEW_ARGS(SkOffsetImageFilter,
            (SkIntToScalar(4), SkIntToScalar(4), background)));
        mode.reset(SkXfermode::Create(SkXfermode::kSrcOver_Mode));
        filter.reset(SkNEW_ARGS(SkXfermodeImageFilter,
            (mode, offsetBackground, offsetForeground)));
        paint.setImageFilter(filter);
        canvas->save();
        canvas->clipRect(SkRect::MakeXYWH(SkIntToScalar(x),
                                          SkIntToScalar(y),
                                          SkIntToScalar(fBitmap.width() + 4),
                                          SkIntToScalar(fBitmap.height() + 4)));
        canvas->drawPaint(paint);
        canvas->restore();
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        // Test offsets on Darken (uses shader blend)
        mode.reset(SkXfermode::Create(SkXfermode::kDarken_Mode));
        filter.reset(SkNEW_ARGS(SkXfermodeImageFilter, (mode, offsetBackground, offsetForeground)));
        paint.setImageFilter(filter);
        canvas->save();
        canvas->clipRect(SkRect::MakeXYWH(SkIntToScalar(x),
                                          SkIntToScalar(y),
                                          SkIntToScalar(fBitmap.width() + 4),
                                          SkIntToScalar(fBitmap.height() + 4)));
        canvas->drawPaint(paint);
        canvas->restore();
    }
private:
    typedef GM INHERITED;
    SkBitmap fBitmap, fCheckerboard;
    bool fInitialized;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new XfermodeImageFilterGM; }
static GMRegistry reg(MyFactory);

}
