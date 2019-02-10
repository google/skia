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

#include "../modules/sksg/include/SkSGDraw.h"
#include "../modules/sksg/include/SkSGColor.h"
#include "../modules/sksg/include/SkSGGroup.h"
#include "../modules/sksg/include/SkSGRect.h"
#include "../modules/sksg/include/SkSGScene.h"

class SampleSG : public Sample {
    sk_sp<sksg::Group> fGroup;
public:
    SampleSG() {
        fGroup = sksg::Group::Make();

        auto r = sksg::Rect::Make({20, 20, 400, 300});
        auto p = sksg::Color::Make(SK_ColorRED);
        auto d = sksg::Draw::Make(r, p);
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
        fGroup->render(canvas);
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
