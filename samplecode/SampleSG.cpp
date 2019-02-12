/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sample.h"
#include "SkCanvas.h"
#include "SkFont.h"
#include "SkFontMetrics.h"
#include "SkPath.h"

#include "SkSGDraw.h"
#include "SkSGColor.h"
#include "SkSGGroup.h"
#include "SkSGRect.h"
#include "SkSGScene.h"

class SampleSG : public Sample {
    sk_sp<sksg::Group> fGroup;
    std::unique_ptr<sksg::Scene> fScene;
public:
    SampleSG() {
        fGroup = sksg::Group::Make();

        fScene = sksg::Scene::Make(fGroup, sksg::AnimatorList());

        auto r = sksg::Rect::Make({20, 20, 400, 300});
        auto p = sksg::Color::Make(SK_ColorRED);
        auto d = sksg::Draw::Make(r, p);
        fGroup->addChild(d);

        r = sksg::Rect::Make({60, 70, 300, 400});
        p = sksg::Color::Make(SK_ColorBLUE);
        d = sksg::Draw::Make(r, p);
        fGroup->addChild(d);
    }

protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "SceneGraph");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        fScene->render(canvas);
    }

    virtual Sample::Click* onFindClickHandler(SkScalar x, SkScalar y,
                                              unsigned modi) override {
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

    bool onClick(Click* click) override {
        return false;
    }

private:

    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new SampleSG(); )
