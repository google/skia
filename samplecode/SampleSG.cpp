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

#include "imgui.h"

struct PerNodeInfo {
    // key
    sksg::Draw* fDraw;

    // value(s)
    sksg::GeometryNode* fGeo;
    sksg::Color*        fPaint;
};

class SampleSG : public Sample {
    SkTDArray<PerNodeInfo> fSideCar;
    sk_sp<sksg::Group> fGroup;
    std::unique_ptr<sksg::Scene> fScene;
    // non-null if we are selected
    sksg::Draw* fSelected = nullptr;

    PerNodeInfo* findInfo(sksg::Draw* key) {
        for (int i = 0; i < fSideCar.count(); ++i) {
            if (fSideCar[i].fDraw == key) {
                return &fSideCar[i];
            }
        }
        return nullptr;
    }

    void appendNode(sk_sp<sksg::Draw> d, sk_sp<sksg::GeometryNode> g, sk_sp<sksg::Color> p) {
        fGroup->addChild(d);
        auto sc = fSideCar.append();
        sc->fDraw  = d.get();
        sc->fGeo   = g.get();
        sc->fPaint = p.get();
    }

    void updateColor() {
        if (fSelected) {
            auto info = this->findInfo(fSelected);
            SkColor4f c = SkColor4f::FromColor(info->fPaint->getColor());
            ImGui::ColorEdit4("Color", c.vec());
            info->fPaint->setColor(c.toSkColor());
        }
    }

public:
    SampleSG() {
        fGroup = sksg::Group::Make();

        fScene = sksg::Scene::Make(fGroup, sksg::AnimatorList());

        auto r = sksg::Rect::Make({20, 20, 400, 300});
        auto p = sksg::Color::Make(SK_ColorRED);
        auto d = sksg::Draw::Make(r, p);
        this->appendNode(d, r, p);

        r = sksg::Rect::Make({60, 70, 300, 400});
        p = sksg::Color::Make(SK_ColorBLUE);
        d = sksg::Draw::Make(r, p);
        this->appendNode(d, r, p);
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
        this->updateColor();

        fScene->render(canvas);
    }

    Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        if (auto node = fScene->nodeAt({x, y})) {
            SkASSERT(this->findInfo((sksg::Draw*)node) != nullptr);
            fSelected = (sksg::Draw*)node;
            Click* click = new Click(this);
            click->fMeta.setPtr("node", (void*)node);
            return click;
        }
        fSelected = nullptr;
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

    bool onClick(Click* click) override {
        sksg::Draw* node = nullptr;
        if (click->fMeta.findPtr("node", (void**)&node)) {
            auto info = this->findInfo(node);
            auto geo = info->fGeo;
            sksg::Rect* r = (sksg::Rect*)geo;
            SkScalar dx = click->fCurr.fX - click->fPrev.fX;
            SkScalar dy = click->fCurr.fY - click->fPrev.fY;
            r->setL(r->getL() + dx);
            r->setR(r->getR() + dx);
            r->setT(r->getT() + dy);
            r->setB(r->getB() + dy);
            return true;
        }
        return false;
    }

private:

    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new SampleSG(); )
