/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "tools/ToolUtils.h"

#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"

static const char* gSpeach = "Five score years ago, a great American, in whose symbolic shadow we stand today, signed the Emancipation Proclamation. This momentous decree came as a great beacon light of hope to millions of Negro slaves who had been seared in the flames of withering injustice. It came as a joyous daybreak to end the long night of their captivity.";

class ParagraphGM : public skiagm::GM {
    std::unique_ptr<skia::textlayout::Paragraph> fPara;

public:
    ParagraphGM() {
        skia::textlayout::TextStyle style;
        style.setForegroundColor(SkPaint());
        style.setFontFamilies({SkString("sans-serif")});
        style.setFontSize(30);

        skia::textlayout::ParagraphStyle paraStyle;
        paraStyle.setTextStyle(style);

        auto collection = sk_make_sp<skia::textlayout::FontCollection>();
        collection->setDefaultFontManager(SkFontMgr::RefDefault());
        auto builder = skia::textlayout::ParagraphBuilder::make(paraStyle, collection);

        builder->addText(gSpeach, strlen(gSpeach));

        fPara = builder->Build();
        fPara->layout(400);
    }

protected:

    SkString onShortName() override {
        return SkString("paragraph");
    }

    SkISize onISize() override {
        return SkISize::Make(600, 420);
    }

    void onDraw(SkCanvas* canvas) override {
        for (int i = 0; i < 100; ++i) {
            SkAutoCanvasRestore acr(canvas, true);

        fPara->layout(400 + fParity);
        fParity = (fParity + 1) & 1;

        fPara->paint(canvas, 20, 20);
        }
    }

    bool runAsBench() const override { return true; }

    bool onAnimate(double /*nanos*/) override {
        return true;
    }

private:
    int fParity = 0;

    typedef skiagm::GM INHERITED;
};
DEF_GM(return new ParagraphGM;)
