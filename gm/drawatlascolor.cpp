/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkRSXform.h"
#include "SkSurface.h"

// Create a square atlas of:
//   opaque white  |     opaque red
//  ------------------------------------
//   opaque green  |  transparent black
//
static sk_sp<SkImage> make_atlas(SkCanvas* caller, int atlasSize) {
    const int kBlockSize = atlasSize/2;

    SkImageInfo info = SkImageInfo::MakeN32Premul(atlasSize, atlasSize);
    auto surface(caller->makeSurface(info));
    if (nullptr == surface) {
        surface = SkSurface::MakeRaster(info);
    }
    SkCanvas* canvas = surface->getCanvas();

    SkPaint paint;
    paint.setXfermode(SkXfermode::Make(SkXfermode::kSrc_Mode));

    paint.setColor(SK_ColorWHITE);
    SkRect r = SkRect::MakeXYWH(0, 0,
                                SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));
    canvas->drawRect(r, paint);

    paint.setColor(SK_ColorRED);
    r = SkRect::MakeXYWH(SkIntToScalar(kBlockSize), 0,
                         SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));
    canvas->drawRect(r, paint);

    paint.setColor(SK_ColorGREEN);
    r = SkRect::MakeXYWH(0, SkIntToScalar(kBlockSize),
                         SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));
    canvas->drawRect(r, paint);

    paint.setColor(SK_ColorTRANSPARENT);
    r = SkRect::MakeXYWH(SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize),
                         SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));
    canvas->drawRect(r, paint);

    return surface->makeImageSnapshot();
}

// This GM tests the drawAtlas API with colors, different xfer modes
// and transparency in the atlas image
class DrawAtlasColorsGM : public skiagm::GM {
public:
    DrawAtlasColorsGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:
    SkString onShortName() override {
        return SkString("draw-atlas-colors");
    }

    SkISize onISize() override {
        return SkISize::Make(kNumXferModes * (kAtlasSize + kPad) + kPad,
                             2 * kNumColors * (kAtlasSize + kPad) + kTextPad + kPad);
    }

    void onDraw(SkCanvas* canvas) override {
        const SkRect target = SkRect::MakeWH(SkIntToScalar(kAtlasSize), SkIntToScalar(kAtlasSize));

        if (nullptr == fAtlas) {
            fAtlas = make_atlas(canvas, kAtlasSize);
        }

        const struct {
            SkXfermode::Mode fMode;
            const char*      fLabel;
        } gModes[] = {
            { SkXfermode::kClear_Mode,      "Clear"     },
            { SkXfermode::kSrc_Mode,        "Src"       },
            { SkXfermode::kDst_Mode,        "Dst"       },
            { SkXfermode::kSrcOver_Mode,    "SrcOver"   },
            { SkXfermode::kDstOver_Mode,    "DstOver"   },
            { SkXfermode::kSrcIn_Mode,      "SrcIn"     },
            { SkXfermode::kDstIn_Mode,      "DstIn"     },
            { SkXfermode::kSrcOut_Mode,     "SrcOut"    },
            { SkXfermode::kDstOut_Mode,     "DstOut"    },
            { SkXfermode::kSrcATop_Mode,    "SrcATop"   },
            { SkXfermode::kDstATop_Mode,    "DstATop"   },
            { SkXfermode::kXor_Mode,        "Xor"       },
            { SkXfermode::kPlus_Mode,       "Plus"      },
            { SkXfermode::kModulate_Mode,   "Mod"       },
            { SkXfermode::kScreen_Mode,     "Screen"    },
            { SkXfermode::kOverlay_Mode,    "Overlay"   },
            { SkXfermode::kDarken_Mode,     "Darken"    },
            { SkXfermode::kLighten_Mode,    "Lighten"   },
            { SkXfermode::kColorDodge_Mode, "Dodge"     },
            { SkXfermode::kColorBurn_Mode,  "Burn"      },
            { SkXfermode::kHardLight_Mode,  "Hard"      },
            { SkXfermode::kSoftLight_Mode,  "Soft"      },
            { SkXfermode::kDifference_Mode, "Diff"      },
            { SkXfermode::kExclusion_Mode,  "Exclusion" },
            { SkXfermode::kMultiply_Mode,   "Multiply"  },
            { SkXfermode::kHue_Mode,        "Hue"       },
            { SkXfermode::kSaturation_Mode, "Sat"       },
            { SkXfermode::kColor_Mode,      "Color"     },
            { SkXfermode::kLuminosity_Mode, "Luminosity"},
        };

        SkColor gColors[] = {
            SK_ColorWHITE,
            SK_ColorRED,
            0x88888888,         // transparent grey
            0x88000088          // transparent blue
        };

        const int numModes = SK_ARRAY_COUNT(gModes);
        SkASSERT(numModes == kNumXferModes);
        const int numColors = SK_ARRAY_COUNT(gColors);
        SkASSERT(numColors == kNumColors);
        SkRSXform xforms[numColors];
        SkRect rects[numColors];
        SkColor quadColors[numColors];

        SkPaint paint;
        paint.setAntiAlias(true);

        for (int i = 0; i < numColors; ++i) {
            xforms[i].set(1.0f, 0.0f, SkIntToScalar(kPad), i*(target.width()+kPad));
            rects[i] = target;
            quadColors[i] = gColors[i];
        }

        SkPaint textP;
        textP.setTextSize(SkIntToScalar(kTextPad));
        textP.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&textP, nullptr);

        for (int i = 0; i < numModes; ++i) {
            canvas->drawText(gModes[i].fLabel, strlen(gModes[i].fLabel),
                             i*(target.width()+kPad)+kPad, SkIntToScalar(kTextPad),
                             textP);
        }

        for (int i = 0; i < numModes; ++i) {
            canvas->save();
            canvas->translate(SkIntToScalar(i*(target.height()+kPad)),
                              SkIntToScalar(kTextPad+kPad));
            // w/o a paint
            canvas->drawAtlas(fAtlas.get(), xforms, rects, quadColors, numColors,
                              gModes[i].fMode, nullptr, nullptr);
            canvas->translate(0.0f, numColors*(target.height()+kPad));
            // w a paint
            canvas->drawAtlas(fAtlas.get(), xforms, rects, quadColors, numColors,
                              gModes[i].fMode, nullptr, &paint);
            canvas->restore();
        }
    }

private:
    static const int kNumXferModes = 29;
    static const int kNumColors = 4;
    static const int kAtlasSize = 30;
    static const int kPad = 2;
    static const int kTextPad = 8;


    sk_sp<SkImage> fAtlas;

    typedef GM INHERITED;
};
DEF_GM( return new DrawAtlasColorsGM; )
