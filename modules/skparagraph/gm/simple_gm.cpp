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
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"

static const char* gSpeach = "Five score years ago, a great American, in whose symbolic shadow we stand today, signed the Emancipation Proclamation. This momentous decree came as a great beacon light of hope to millions of Negro slaves who had been seared in the flames of withering injustice. It came as a joyous daybreak to end the long night of their captivity.";

namespace {
enum ParaFlags {
    kTimeLayout     = 1 << 0,
    kUseUnderline   = 1 << 1,
};
}  // namespace

class ParagraphGM : public skiagm::GM {
    std::unique_ptr<skia::textlayout::Paragraph> fPara;
    const unsigned fFlags;

public:
    ParagraphGM(unsigned flags) : fFlags(flags) {}

    void buildParagraph() {
        skia::textlayout::TextStyle style;
        style.setForegroundColor(SkPaint());
        style.setFontFamilies({SkString("sans-serif")});
        style.setFontSize(30);

        if (fFlags & kUseUnderline) {
            style.setDecoration(skia::textlayout::TextDecoration::kUnderline);
            style.setDecorationMode(skia::textlayout::TextDecorationMode::kThrough);
            style.setDecorationColor(SK_ColorBLACK);
            style.setDecorationThicknessMultiplier(2);
        }

        skia::textlayout::ParagraphStyle paraStyle;
        paraStyle.setTextStyle(style);

        auto collection = sk_make_sp<skia::textlayout::FontCollection>();
        collection->setDefaultFontManager(SkFontMgr::RefDefault());
        auto builder = skia::textlayout::ParagraphBuilderImpl::make(paraStyle, collection);
        if (nullptr == builder) {
            fPara = nullptr;
            return;
        }

        builder->addText(gSpeach, strlen(gSpeach));

        fPara = builder->Build();
        fPara->layout(400);
    }

protected:
    void onOnceBeforeDraw() override {
        this->buildParagraph();
    }

    SkString onShortName() override {
        SkString name;
        name.printf("paragraph%s_%s",
                    fFlags & kTimeLayout   ? "_layout"    : "",
                    fFlags & kUseUnderline ? "_underline" : "");
        return name;
    }

    SkISize onISize() override { return SkISize::Make(412, 420); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        if (nullptr == fPara) {
            return DrawResult::kSkip;
        }
        const int loop = (this->getMode() == kGM_Mode) ? 1 : 50;

        int parity = 0;
        for (int i = 0; i < loop; ++i) {
            SkAutoCanvasRestore acr(canvas, true);

            if (fFlags & kTimeLayout) {
                fPara->layout(400 + parity);
                parity = (parity + 1) & 1;
            }
            fPara->paint(canvas, 10, 10);
        }
        // clean up if we've been looping
        if (loop > 1) {
            canvas->clear(SK_ColorWHITE);
            fPara->layout(400);
            fPara->paint(canvas, 10, 10);
        }

        if ((this->getMode() == kGM_Mode) && (fFlags & kTimeLayout)) {
            return DrawResult::kSkip;
        }
        return DrawResult::kOk;
    }

    bool runAsBench() const override { return true; }

    bool onAnimate(double /*nanos*/) override {
        return true;
    }

private:
    using INHERITED = skiagm::GM;
};
DEF_GM(return new ParagraphGM(0);)
DEF_GM(return new ParagraphGM(kTimeLayout);)
DEF_GM(return new ParagraphGM(kUseUnderline);)
