/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkSGColor.h"
#include "SkSGContainerNode.h"
#include "SkSGDraw.h"
#include "SkSGInvalidationController.h"
#include "SkSGRect.h"
#include "SkAnimTimer.h"

#include <cmath>

class SGInvalView final : public SampleView {
public:
    SGInvalView() {}

protected:
    void onOnceBeforeDraw() override {
        fRect1 = sksg::Rect::Make(SkRect::MakeLTRB(100, 100, 100, 100));
        fRect2 = sksg::Rect::Make(SkRect::MakeLTRB(300, 200, 300, 200));
        fColor1 = sksg::Color::Make(0);
        fColor2 = sksg::Color::Make(0);

        fRoot = sksg::ContainerNode::Make();
        fRoot->addChild(sksg::Draw::Make(fRect1, fColor1));
        fRoot->addChild(sksg::Draw::Make(fRect2, fColor2));
    }

    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "SGInval");
            return true;
        }

        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        sksg::InvalidationController ic;
        fRoot->revalidate(&ic);

        // TODO: clip/cull
        fRoot->render(canvas);

        SkPaint p;
        p.setColor(0xffff0000);
        p.setStyle(SkPaint::kStroke_Style);
        p.setAntiAlias(true);
        p.setStrokeWidth(0);

        for (const auto& r : ic) {
            canvas->drawRect(r, p);
        }
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        static constexpr SkScalar kSize = 100;
        static constexpr SkScalar kRate = 500;
        fRect1->set_r(fRect1->l() + kSize * 0.5f * (1 + std::sin(timer.msec() / kRate)));
        fRect1->set_b(fRect1->t() + kSize * 0.5f * (1 + std::cos(timer.msec() / kRate)));
        fRect2->set_r(fRect2->l() + kSize * 0.5f * (1 + std::cos(SK_ScalarPI / 2 +
                                                                 timer.msec() / kRate)));
        fRect2->set_b(fRect2->t() + kSize * 0.5f * (1 + std::sin(SK_ScalarPI / 2 +
                                                                 timer.msec() / kRate)));

        fColor1->set_color(SkColorSetARGB(128 * (1 + std::sin(timer.msec() / kRate)), 0, 0x80, 0));
        fColor2->set_color(SkColorSetARGB(128 * (1 + std::cos(timer.msec() / kRate)), 0, 0, 0x80));
        return true;
    }

private:
    typedef SampleView INHERITED;

    sk_sp<sksg::Rect>          fRect1,
                               fRect2;
    sk_sp<sksg::Color>         fColor1,
                               fColor2;
    sk_sp<sksg::ContainerNode> fRoot;
};

static SkView* SGInvalFactory() { return new SGInvalView; }
static SkViewRegister reg(SGInvalFactory);
