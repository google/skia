/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkM44.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkVertices.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "tools/Resources.h"

static SkM44 inv(const SkM44& m) {
    SkM44 im;
    SkAssertResult(m.invert(&im));
    return im;
}

static SkV2 operator*(const SkM44& m, SkV2 v) {
    SkV4 v2 = m.map(v.x, v.y, 0, 1);
    return {v2.x, v2.y};
}

class Shape {
    SkM44   fM;
public:
    SkPaint fPaint;

    Shape() {
        fPaint.setAntiAlias(true);
    }
    virtual ~Shape() {}

    void draw(SkCanvas* canvas) {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(fM);
        this->onDraw(canvas);
    }

    void drawHilite(SkCanvas* canvas) {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(fM);
        this->onDrawHilite(canvas);
    }

    bool hitTest(SkV2 p) {
        return this->onHitTest(inv(fM) * p);
    }

protected:
    virtual void onDraw(SkCanvas*) {}
    virtual void onDrawHilite(SkCanvas*) {}
    virtual bool onHitTest(SkV2) { return false; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class RRectShape : public Shape {
    SkRRect fRRect;
public:
    RRectShape() {}

    SkRRect get() const { return fRRect; }
    void set(SkRRect rr) { fRRect = rr; }

protected:
    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawRRect(fRRect, fPaint);
    }

    virtual void onDrawHilite(SkCanvas*) {}
    virtual bool onHitTest(SkV2) { return false; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

static SkRandom gRand;

class SampleDraw : public Sample {
    std::vector<std::unique_ptr<Shape>> fShapes;

protected:
    SkString name() override { return SkString("skdraw"); }

public:
    void onDrawContent(SkCanvas* canvas) override {
        for (auto& sh : fShapes) {
            sh->draw(canvas);
        }
    }

    bool onChar(SkUnichar uni) override {
        switch (uni) {
            case 'r': {
                auto s = std::make_unique<RRectShape>();
                SkRect r = SkRect::MakeXYWH(gRand.nextF() * 640, gRand.nextF() * 500, 300, 200);
                s->set(SkRRect::MakeRectXY(r, 20, 20));
                s->fPaint.setColor(gRand.nextU() | (0x80 << 24));
                fShapes.push_back(std::move(s));
                return true;
            } break;
        }
        return this->INHERITED::onChar(uni);
    }

    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }
    bool onClick(Click* click) override {
        return true;
    }

    bool onAnimate(double nanos) override {
        return false;
    }

private:
    typedef Sample INHERITED;
};
DEF_SAMPLE( return new SampleDraw; )
