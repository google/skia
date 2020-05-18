/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/utils/SkCustomTypeface.h"
#include "tools/Resources.h"

static sk_sp<SkTypeface> make_tf() {
    SkCustomTypefaceBuilder builder;
    SkFont font;
    font.setSize(1.0f);
    font.setHinting(SkFontHinting::kNone);

    // Steal the first 128 chars from the default font
    for (SkGlyphID index = 0; index <= 127; ++index) {
        SkGlyphID glyph = font.unicharToGlyph(index);

        SkScalar width;
        font.getWidths(&glyph, 1, &width);
        SkPath path;
        font.getPath(glyph, &path);

        // we use the charcode to be our glyph index, since we have no cmap table
        builder.setGlyph(index, width, path);
    }

    return builder.detach();
}

#include "include/core/SkTextBlob.h"

class UserFontGM : public skiagm::GM {
    sk_sp<SkTypeface> fTF;

public:
    UserFontGM() {}

    void onOnceBeforeDraw() override {
        fTF = make_tf();
    }

    static sk_sp<SkTextBlob> make_blob(sk_sp<SkTypeface> tf, float size) {
        SkFont font(tf);
        font.setSize(size);
        font.setEdging(SkFont::Edging::kAntiAlias);
        return SkTextBlob::MakeFromString("Typeface", font);
    }

    bool runAsBench() const override { return true; }

    SkString onShortName() override { return SkString("user_typeface"); }

    SkISize onISize() override { return {810, 512}; }

    void onDraw(SkCanvas* canvas) override {
        auto waterfall = [&](sk_sp<SkTypeface> tf) {
            SkPaint paint;
            paint.setAntiAlias(true);

            float x = 20,
                  y = 16;
            for (float size = 9; size <= 100; size *= 1.25f) {
                auto blob = make_blob(tf, size);

                paint.setColor(0xFFCCCCCC);
                paint.setStyle(SkPaint::kStroke_Style);
                canvas->drawRect(blob->bounds().makeOffset(x, y), paint);

                paint.setStyle(SkPaint::kFill_Style);
                paint.setColor(SK_ColorBLACK);
                canvas->drawTextBlob(blob, x, y, paint);

                y += size * 1.5f;
            }
        };

        waterfall(nullptr);
        canvas->translate(400, 0);
        waterfall(fTF);
    }
};
DEF_GM(return new UserFontGM;)
