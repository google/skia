/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/private/SkTDArray.h"
#include "tools/ToolUtils.h"

static const char gText[] = "Call me Ishmael. Some years ago—never mind how long precisely";

class DrawGlyphsGM : public skiagm::GM {
public:
    void onOnceBeforeDraw() override {
        fTypeface = ToolUtils::create_portable_typeface("serif", SkFontStyle());
        fFont = SkFont(fTypeface);
        fFont.setSubpixel(true);
        fFont.setSize(18);
        size_t txtLen = strlen(gText);
        fGlyphCount = fFont.countText(gText, txtLen, SkTextEncoding::kUTF8);

        fGlyphs.append(fGlyphCount);
        fFont.textToGlyphs(gText, txtLen, SkTextEncoding::kUTF8, fGlyphs.begin(), fGlyphCount);

        fPositions.append(fGlyphCount);
        fFont.getPos(fGlyphs.begin(), fGlyphCount, fPositions.begin());
    }

    SkString onShortName() override {
        return SkString("drawglyphs");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawGlyphs(fGlyphCount, fGlyphs.begin(), fPositions.begin(), {50, 100}, fFont,
                           SkPaint{});

        canvas->drawGlyphs(fGlyphCount, fGlyphs.begin(), fPositions.begin(), {50, 120}, fFont,
                           SkPaint{});

        // Check bounding box calculation.
        for (auto& pos : fPositions) {
            pos += {0, -500};
        }
        canvas->drawGlyphs(fGlyphCount, fGlyphs.begin(), fPositions.begin(), {50, 640}, fFont,
                           SkPaint{});

        // TODO: add tests for cluster versions of drawGlyphs.
    }

private:
    sk_sp<SkTypeface>   fTypeface;
    SkFont fFont;
    SkTDArray<SkGlyphID> fGlyphs;
    SkTDArray<SkPoint>  fPositions;
    int fGlyphCount;
};

DEF_GM(return new DrawGlyphsGM{};)
