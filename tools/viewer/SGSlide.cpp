/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPath.h"
#include "include/private/base/SkTDArray.h"
#include "modules/sksg/include/SkSGDraw.h"
#include "modules/sksg/include/SkSGGroup.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGRect.h"
#include "modules/sksg/include/SkSGScene.h"
#include "tools/viewer/ClickHandlerSlide.h"

struct PerNodeInfo {
    // key
    sksg::Draw* fDraw;

    // value(s)
    sksg::GeometryNode* fGeo;
    sksg::PaintNode*    fPaint;
};

class SGSlide : public ClickHandlerSlide {
    // TODO(kjlubick) use a vector instead of our private SkTDArray
    SkTDArray<PerNodeInfo> fSideCar;
    sk_sp<sksg::Group> fGroup;
    std::unique_ptr<sksg::Scene> fScene;

    PerNodeInfo* findInfo(sksg::Draw* key) {
        for (int i = 0; i < fSideCar.size(); ++i) {
            if (fSideCar[i].fDraw == key) {
                return &fSideCar[i];
            }
        }
        return nullptr;
    }

    void appendNode(sk_sp<sksg::Draw> d, sk_sp<sksg::GeometryNode> g, sk_sp<sksg::PaintNode> p) {
        fGroup->addChild(d);
        auto sc = fSideCar.append();
        sc->fDraw  = d.get();
        sc->fGeo   = g.get();
        sc->fPaint = p.get();
    }

public:
    SGSlide() {
        fGroup = sksg::Group::Make();

        fScene = sksg::Scene::Make(fGroup);

        auto r = sksg::Rect::Make({20, 20, 400, 300});
        auto p = sksg::Color::Make(SK_ColorRED);
        auto d = sksg::Draw::Make(r, p);
        this->appendNode(d, r, p);

        r = sksg::Rect::Make({60, 70, 300, 400});
        p = sksg::Color::Make(SK_ColorBLUE);
        d = sksg::Draw::Make(r, p);
        this->appendNode(d, r, p);
        fName = "SceneGraph";
    }

    void draw(SkCanvas* canvas) override {
        fScene->render(canvas);
    }

protected:
    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        if (auto node = fScene->nodeAt({x, y})) {
            Click* click = new Click();
            click->fMeta.setPtr("node", (void*)node);
            return click;
        }
        return nullptr;
    }

    bool onClick(Click* click) override {
        sksg::Draw* node = nullptr;
        if (click->fMeta.findPtr("node", (void**)&node)) {
            if (auto info = this->findInfo(node)) {
                auto geo = info->fGeo;
                sksg::Rect* r = (sksg::Rect*)geo;
                SkScalar dx = click->fCurr.fX - click->fPrev.fX;
                SkScalar dy = click->fCurr.fY - click->fPrev.fY;
                r->setL(r->getL() + dx);
                r->setR(r->getR() + dx);
                r->setT(r->getT() + dy);
                r->setB(r->getB() + dy);
            }
            return true;
        }
        return false;
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_SLIDE( return new SGSlide(); )
