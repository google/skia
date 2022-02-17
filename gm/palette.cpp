/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <string.h>

namespace skiagm {

namespace {
const char kColrCpalTestFontPath[] = "fonts/more_samples-glyf_colr_1.ttf";

constexpr SkFontArguments::Palette::Override kColorOverridesAll[] = {
        // A gradient of dark to light purple for the circle palette test glyph.
        {0, 0xff310b55},
        {1, 0xff510970},
        {2, 0xff76078f},
        {3, 0xff9606aa},
        {4, 0xffb404c4},
        {5, 0xffd802e2},
        {6, 0xfffa00ff},
        {7, 0xff888888},
        {8, 0xff888888},
        {9, 0xff888888},
        {10, 0xff888888},
        {11, 0xff888888}};

constexpr SkFontArguments::Palette::Override kColorOverridesOne[] = {
        {2, 0xff02dfe2},
};

constexpr SkFontArguments::Palette kLightPaletteOverride{2, nullptr, 0};
constexpr SkFontArguments::Palette kDarkPaletteOverride{1, nullptr, 0};
constexpr SkFontArguments::Palette kOnePaletteOverride{
        0, kColorOverridesOne, SK_ARRAY_COUNT(kColorOverridesOne)};
constexpr SkFontArguments::Palette kAllPaletteOverride{
        0, kColorOverridesAll, SK_ARRAY_COUNT(kColorOverridesAll)};

constexpr uint16_t kTestGlyphs[] = {56, 57};

}  // namespace

class FontPaletteGM : public GM {
public:
    FontPaletteGM(const char* test_name,
                  const SkFontArguments::Palette& paletteOverride)
            : fName(test_name), fPalette(paletteOverride) {}

protected:
    sk_sp<SkTypeface> fTypefaceDefault;
    sk_sp<SkTypeface> fTypefaceFromStream;
    sk_sp<SkTypeface> fTypefaceCloned;

    void onOnceBeforeDraw() override {
        SkFontArguments paletteArguments;
        paletteArguments.setPalette(fPalette);

        fTypefaceDefault = MakeResourceAsTypeface(kColrCpalTestFontPath);
        fTypefaceCloned =
                fTypefaceDefault ? fTypefaceDefault->makeClone(paletteArguments) : nullptr;

        fTypefaceFromStream = SkFontMgr::RefDefault()->makeFromStream(
                GetResourceAsStream(kColrCpalTestFontPath), paletteArguments);
    }

    SkString onShortName() override {
        SkString gm_name = SkStringPrintf("font_palette_%s", fName.c_str());
        return gm_name;
    }

    SkISize onISize() override { return SkISize::Make(1000, 400); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        canvas->drawColor(SK_ColorWHITE);
        SkPaint paint;

        canvas->translate(10, 20);

        if (!fTypefaceCloned || !fTypefaceFromStream) {
            *errorMsg = "Did not recognize COLR v1 test font format.";
            return DrawResult::kSkip;
        }

        SkFontMetrics metrics;
        SkScalar y = 0;
        SkScalar textSize = 200;
        for (auto& typeface : { fTypefaceFromStream, fTypefaceCloned} ) {
            SkFont defaultFont(fTypefaceDefault);
            SkFont paletteFont(typeface);
            defaultFont.setSize(textSize);
            paletteFont.setSize(textSize);

            defaultFont.getMetrics(&metrics);
            y += -metrics.fAscent;
            // Set a recognizable foreground color which is not to be overriden.
            paint.setColor(SK_ColorGRAY);
            // Draw the default palette on the left, for COLRv0 and COLRv1.
            canvas->drawSimpleText(kTestGlyphs,
                                   SK_ARRAY_COUNT(kTestGlyphs) * sizeof(uint16_t),
                                   SkTextEncoding::kGlyphID,
                                   0,
                                   y,
                                   defaultFont,
                                   paint);
            // Draw the overriden palette on the right.
            canvas->drawSimpleText(kTestGlyphs,
                                   SK_ARRAY_COUNT(kTestGlyphs) * sizeof(uint16_t),
                                   SkTextEncoding::kGlyphID,
                                   440,
                                   y,
                                   paletteFont,
                                   paint);
            y += metrics.fDescent + metrics.fLeading;
        }
        return DrawResult::kOk;
    }

private:
    using INHERITED = GM;
    SkString fName;
    SkFontArguments::Palette fPalette;
};

DEF_GM(return new FontPaletteGM("default", SkFontArguments::Palette()));
DEF_GM(return new FontPaletteGM("light", kLightPaletteOverride));
DEF_GM(return new FontPaletteGM("dark", kDarkPaletteOverride));
DEF_GM(return new FontPaletteGM("one", kOnePaletteOverride));
DEF_GM(return new FontPaletteGM("all", kAllPaletteOverride));

}  // namespace skiagm
