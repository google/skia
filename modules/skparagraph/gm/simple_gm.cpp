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
    kShowVisitor    = 1 << 2,
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
        if (fFlags & kShowVisitor) {
            name.append("_visitor");
        }
        return name;
    }

    SkISize onISize() override {
        if (fFlags & kShowVisitor) {
            return SkISize::Make(810, 420);
        }
        return SkISize::Make(412, 420);
    }

    void drawFromVisitor(SkCanvas* canvas, skia::textlayout::Paragraph* para) const {
        SkPaint p, p2;
        p.setColor(0xFF0000FF);
        p2.setColor(0xFFFF0000);
        p2.setStrokeWidth(4);
        p2.setStrokeCap(SkPaint::kSquare_Cap);
        SkPaint underp;
        underp.setStroke(true);
        underp.setStrokeWidth(2);
        underp.setAntiAlias(true);
        underp.setColor(p.getColor());
        const SkScalar GAP = 2;

        para->visit([&](int, const skia::textlayout::Paragraph::VisitorInfo* info) {
            if (!info) {
                return;
            }
            canvas->drawGlyphs(info->count, info->glyphs, info->positions, info->origin,
                               info->font, p);

            if (fFlags & kUseUnderline) {
                // Need to modify positions to roll-in the orign
                std::vector<SkPoint> pos;
                for (int i = 0; i < info->count; ++i) {
                    pos.push_back({info->origin.fX + info->positions[i].fX,
                                   info->origin.fY + info->positions[i].fY});
                }

                const SkScalar X0 = pos[0].fX;
                const SkScalar X1 = X0 + info->advanceX;
                const SkScalar Y  = pos[0].fY;
                auto sects = info->font.getIntercepts(info->glyphs, info->count, pos.data(),
                                                      Y+1, Y+3);

                SkScalar x0 = X0;
                for (size_t i = 0; i < sects.size(); i += 2) {
                    SkScalar x1 = sects[i] - GAP;
                    if (x0 < x1) {
                        canvas->drawLine(x0, Y+2, x1, Y+2, underp);
                    }
                    x0 = sects[i+1] + GAP;
                }
                canvas->drawLine(x0, Y+2, X1, Y+2, underp);
            }

            if (info->utf8Starts && false) {
                SkString str;
                for (int i = 0; i < info->count; ++i) {
                    str.appendUnichar(gSpeach[info->utf8Starts[i]]);
                }
                SkDebugf("'%s'\n", str.c_str());
            }

            if (false) {    // show position points
            for (int i = 0; i < info->count; ++i) {
                auto pos = info->positions[i];
                canvas->drawPoint(pos.fX + info->origin.fX, pos.fY + info->origin.fY, p2);
            }
            }
        });
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        if (nullptr == fPara) {
            return DrawResult::kSkip;
        }

        if (fFlags & kShowVisitor) {
            canvas->clear(SK_ColorWHITE);
            fPara->layout(400);
            fPara->paint(canvas, 10, 10);
            canvas->translate(400+10, 10);
            this->drawFromVisitor(canvas, fPara.get());
            return DrawResult::kOk;
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
        return false;
    }

private:
    using INHERITED = skiagm::GM;
};
DEF_GM(return new ParagraphGM(0);)
DEF_GM(return new ParagraphGM(kTimeLayout);)
DEF_GM(return new ParagraphGM(kUseUnderline);)
DEF_GM(return new ParagraphGM(kShowVisitor);)
DEF_GM(return new ParagraphGM(kShowVisitor | kUseUnderline);)
