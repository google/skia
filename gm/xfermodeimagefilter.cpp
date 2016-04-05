/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkArithmeticMode.h"
#include "SkImage.h"
#include "SkImageSource.h"
#include "SkOffsetImageFilter.h"
#include "SkXfermodeImageFilter.h"

#define WIDTH 600
#define HEIGHT 700
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

    SkISize onISize() override {
        return SkISize::Make(WIDTH, HEIGHT);
    }

    void onOnceBeforeDraw() override {
        fBitmap = sk_tool_utils::create_string_bitmap(80, 80, 0xD000D000, 15, 65, 96, "e");

        fCheckerboard = SkImage::MakeFromBitmap(
            sk_tool_utils::create_checkerboard_bitmap(80, 80,
                                                      sk_tool_utils::color_to_565(0xFFA0A0A0),
                                                      sk_tool_utils::color_to_565(0xFF404040),
                                                      8));
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
        sk_sp<SkImageFilter> background(SkImageSource::Make(fCheckerboard));
        for (size_t i = 0; i < SK_ARRAY_COUNT(gModes); i++) {
            paint.setImageFilter(SkXfermodeImageFilter::Make(SkXfermode::Make(gModes[i].fMode),
                                                             background));
            DrawClippedBitmap(canvas, fBitmap, paint, x, y);
            x += fBitmap.width() + MARGIN;
            if (x + fBitmap.width() > WIDTH) {
                x = 0;
                y += fBitmap.height() + MARGIN;
            }
        }
        // Test arithmetic mode as image filter
        paint.setImageFilter(SkXfermodeImageFilter::Make(
                         SkArithmeticMode::Make(0, SK_Scalar1, SK_Scalar1, 0),
                         background));
        DrawClippedBitmap(canvas, fBitmap, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        // Test nullptr mode
        paint.setImageFilter(SkXfermodeImageFilter::Make(nullptr, background));
        DrawClippedBitmap(canvas, fBitmap, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        SkRect clipRect = SkRect::MakeWH(SkIntToScalar(fBitmap.width() + 4),
                                         SkIntToScalar(fBitmap.height() + 4));
        // Test offsets on SrcMode (uses fixed-function blend)
        sk_sp<SkImage> bitmapImage(SkImage::MakeFromBitmap(fBitmap));
        sk_sp<SkImageFilter> foreground(SkImageSource::Make(std::move(bitmapImage)));
        sk_sp<SkImageFilter> offsetForeground(SkOffsetImageFilter::Make(SkIntToScalar(4),
                                                                        SkIntToScalar(-4),
                                                                        foreground));
        sk_sp<SkImageFilter> offsetBackground(SkOffsetImageFilter::Make(SkIntToScalar(4),
                                                                        SkIntToScalar(4),
                                                                        background));
        paint.setImageFilter(SkXfermodeImageFilter::Make(
                                                     SkXfermode::Make(SkXfermode::kSrcOver_Mode),
                                                     offsetBackground,
                                                     offsetForeground,
                                                     nullptr));
        DrawClippedPaint(canvas, clipRect, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        // Test offsets on Darken (uses shader blend)
        paint.setImageFilter(SkXfermodeImageFilter::Make(SkXfermode::Make(SkXfermode::kDarken_Mode),
                                                         offsetBackground,
                                                         offsetForeground,
                                                         nullptr));
        DrawClippedPaint(canvas, clipRect, paint, x, y);
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
            paint.setImageFilter(SkXfermodeImageFilter::Make(SkXfermode::Make(sampledModes[i]),
                                                             offsetBackground,
                                                             offsetForeground,
                                                             &rect));
            DrawClippedPaint(canvas, clipRect, paint, x, y);
            x += fBitmap.width() + MARGIN;
            if (x + fBitmap.width() > WIDTH) {
                x = 0;
                y += fBitmap.height() + MARGIN;
            }
        }
        // Test small bg, large fg with Screen (uses shader blend)
        auto mode = SkXfermode::Make(SkXfermode::kScreen_Mode);
        SkImageFilter::CropRect cropRect(SkRect::MakeXYWH(10, 10, 60, 60));
        sk_sp<SkImageFilter> cropped(SkOffsetImageFilter::Make(0, 0, foreground, &cropRect));
        paint.setImageFilter(SkXfermodeImageFilter::Make(mode, cropped, background, nullptr));
        DrawClippedPaint(canvas, clipRect, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        // Test small fg, large bg with Screen (uses shader blend)
        paint.setImageFilter(SkXfermodeImageFilter::Make(mode, background, cropped, nullptr));
        DrawClippedPaint(canvas, clipRect, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
        // Test small fg, large bg with SrcIn with a crop that forces it to full size.
        // This tests that SkXfermodeImageFilter correctly applies the compositing mode to
        // the region outside the foreground.
        mode = SkXfermode::Make(SkXfermode::kSrcIn_Mode);
        SkImageFilter::CropRect cropRectFull(SkRect::MakeXYWH(0, 0, 80, 80));
        paint.setImageFilter(SkXfermodeImageFilter::Make(mode, background,
                                                         cropped, &cropRectFull));
        DrawClippedPaint(canvas, clipRect, paint, x, y);
        x += fBitmap.width() + MARGIN;
        if (x + fBitmap.width() > WIDTH) {
            x = 0;
            y += fBitmap.height() + MARGIN;
        }
    }

private:
    static void DrawClippedBitmap(SkCanvas* canvas, const SkBitmap& bitmap, const SkPaint& paint,
                           int x, int y) {
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        canvas->clipRect(SkRect::MakeWH(
            SkIntToScalar(bitmap.width()), SkIntToScalar(bitmap.height())));
        canvas->drawBitmap(bitmap, 0, 0, &paint);
        canvas->restore();
    }

    static void DrawClippedPaint(SkCanvas* canvas, const SkRect& rect, const SkPaint& paint,
                          int x, int y) {
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        canvas->clipRect(rect);
        canvas->drawPaint(paint);
        canvas->restore();
    }

    SkBitmap        fBitmap;
    sk_sp<SkImage>  fCheckerboard;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new XfermodeImageFilterGM; );

}
