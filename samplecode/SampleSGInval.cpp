/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkSGColor.h"
#include "SkSGDraw.h"
#include "SkSGGroup.h"
#include "SkSGInvalidationController.h"
#include "SkSGRect.h"
#include "SkSGTransform.h"
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

        fRoot = sksg::Group::Make();
        fRoot->addChild(sksg::Draw::Make(fRect1, fColor1));
        fRoot->addChild(sksg::Transform::Make(sksg::Draw::Make(fRect2, fColor2),
                                              SkMatrix::MakeScale(1.5f, 1.5f)));
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
        fRoot->revalidate(&ic, SkMatrix::I());

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
        if (!fRoot) {
            return true;
        }

        static constexpr SkScalar kSize = 50;
        static constexpr SkScalar kRate = 1.0f / 500;
        const auto t = timer.msec() * kRate;

        fRect1->setR(fRect1->getL() + kSize * (1 + std::sin(t)));
        fRect1->setB(fRect1->getT() + kSize * (1 + std::cos(t)));
        fRect2->setR(fRect2->getL() + kSize * (1 + std::cos(SK_ScalarPI / 2 + t)));
        fRect2->setB(fRect2->getT() + kSize * (1 + std::sin(SK_ScalarPI / 2 + t)));

        fColor1->setColor(SkColorSetARGB(128 * (1 + std::sin(t)), 0, 0x80, 0));
        fColor2->setColor(SkColorSetARGB(128 * (1 + std::cos(t)), 0, 0, 0x80));
        return true;
    }

private:
    typedef SampleView INHERITED;

    sk_sp<sksg::Rect>  fRect1,
                       fRect2;
    sk_sp<sksg::Color> fColor1,
                       fColor2;
    sk_sp<sksg::Group> fRoot;
};

static SkView* SGInvalFactory() { return new SGInvalView; }
static SkViewRegister reg(SGInvalFactory);
