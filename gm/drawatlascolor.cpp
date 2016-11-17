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
    paint.setBlendMode(SkBlendMode::kSrc);

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

        auto atlas = make_atlas(canvas, kAtlasSize);

        const struct {
            SkBlendMode fMode;
            const char* fLabel;
        } gModes[] = {
            { SkBlendMode::kClear,      "Clear"     },
            { SkBlendMode::kSrc,        "Src"       },
            { SkBlendMode::kDst,        "Dst"       },
            { SkBlendMode::kSrcOver,    "SrcOver"   },
            { SkBlendMode::kDstOver,    "DstOver"   },
            { SkBlendMode::kSrcIn,      "SrcIn"     },
            { SkBlendMode::kDstIn,      "DstIn"     },
            { SkBlendMode::kSrcOut,     "SrcOut"    },
            { SkBlendMode::kDstOut,     "DstOut"    },
            { SkBlendMode::kSrcATop,    "SrcATop"   },
            { SkBlendMode::kDstATop,    "DstATop"   },
            { SkBlendMode::kXor,        "Xor"       },
            { SkBlendMode::kPlus,       "Plus"      },
            { SkBlendMode::kModulate,   "Mod"       },
            { SkBlendMode::kScreen,     "Screen"    },
            { SkBlendMode::kOverlay,    "Overlay"   },
            { SkBlendMode::kDarken,     "Darken"    },
            { SkBlendMode::kLighten,    "Lighten"   },
            { SkBlendMode::kColorDodge, "Dodge"     },
            { SkBlendMode::kColorBurn,  "Burn"      },
            { SkBlendMode::kHardLight,  "Hard"      },
            { SkBlendMode::kSoftLight,  "Soft"      },
            { SkBlendMode::kDifference, "Diff"      },
            { SkBlendMode::kExclusion,  "Exclusion" },
            { SkBlendMode::kMultiply,   "Multiply"  },
            { SkBlendMode::kHue,        "Hue"       },
            { SkBlendMode::kSaturation, "Sat"       },
            { SkBlendMode::kColor,      "Color"     },
            { SkBlendMode::kLuminosity, "Luminosity"},
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
            canvas->drawAtlas(atlas.get(), xforms, rects, quadColors, numColors,
                              gModes[i].fMode, nullptr, nullptr);
            canvas->translate(0.0f, numColors*(target.height()+kPad));
            // w a paint
            canvas->drawAtlas(atlas.get(), xforms, rects, quadColors, numColors,
                              gModes[i].fMode, nullptr, &paint);
            canvas->restore();
        }
    }

private:
    static constexpr int kNumXferModes = 29;
    static constexpr int kNumColors = 4;
    static constexpr int kAtlasSize = 30;
    static constexpr int kPad = 2;
    static constexpr int kTextPad = 8;

    typedef GM INHERITED;
};
DEF_GM( return new DrawAtlasColorsGM; )
