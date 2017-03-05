/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkTextBlob.h"
#include "SkTextBox.h"

static const char gText[] =
    "When in the Course of human events it becomes necessary for one people "
    "to dissolve the political bands which have connected them with another "
    "and to assume among the powers of the earth, the separate and equal "
    "station to which the Laws of Nature and of Nature's God entitle them, "
    "a decent respect to the opinions of mankind requires that they should "
    "declare the causes which impel them to the separation.";

class TextBoxGM : public skiagm::GM {
    struct Rec {
        sk_sp<SkTextBlob>   fBlob;
        SkPoint             fOrigin;
    };
    SkTArray<Rec> fRecs;

public:
    TextBoxGM() {}

protected:
    SkString onShortName() override {
        return SkString("textbox");
    }

    SkISize onISize() override { return SkISize::Make(1024, 768); }

    void onOnceBeforeDraw() override {
        const SkScalar w = 1024;
        const SkScalar h = 768;
        const SkScalar margin = 20;

        SkTextBox tbox;
        tbox.setMode(SkTextBox::kLineBreak_Mode);
        tbox.setBox(margin, margin, w - margin, h - margin);
        tbox.setSpacing(1, 0);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);
        tbox.setText(gText, strlen(gText), paint);

        SkScalar x = 0, y = 0;
        for (int i = 9; i < 26; i += 2) {
            paint.setTextSize(SkIntToScalar(i));
            fRecs.push_back({ tbox.snapshotTextBlob(nullptr), { x, y } });
            y += tbox.getTextHeight() + paint.getFontSpacing();
        }
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        for (auto& r : fRecs) {
            canvas->drawTextBlob(r.fBlob, r.fOrigin.fX, r.fOrigin.fY, paint);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new TextBoxGM; )

