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

    SkM44 matrix() const { return fM; }
    void setMatrix(const SkM44& m) { fM = m; }

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
    void onDraw(SkCanvas* canvas) override {
        canvas->drawRRect(fRRect, fPaint);
    }

    void onDrawHilite(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(0xFFFFFF00);
        canvas->drawRRect(fRRect, paint);
    }

    bool onHitTest(SkV2 p) override {
        // todo: how to take into account styles like stroking (and path-effects)?
        return fRRect.contains({p.x-1, p.y-1, p.x+1, p.y+1});
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class Doc {
    std::vector<std::unique_ptr<Shape>> fShapes;
    Shape* fSelected = nullptr;

public:
    template <typename V> void visit_b2f(V visitor) {
        for (auto& s : fShapes) {
            visitor(s.get());
        }
    }

    // returns -1 on failure
    template <typename P> int find_f2b_index(P pred) {
        for (int i = fShapes.size() - 1; i >= 0; --i) {
            if (pred(fShapes[i].get())) {
                return i;
            }
        }
        return -1;
    }

    // returns nullptr on failure
    template <typename P> Shape* find_f2b(P pred) {
        int index = this->find_f2b_index(pred);
        return index >= 0 ? fShapes[index].get() : nullptr;
    }

    Shape* selected() const { return fSelected; }

    void setSelected(Shape* s) {
        fSelected = s;
    }

    void add(std::unique_ptr<Shape> s) {
        fShapes.push_back(std::move(s));
    }

    void remove(Shape* s) {
        if (!s) {
            return;
        }
        if (s == fSelected) {
            this->setSelected(nullptr);
        }

        int index = this->find_f2b_index([s](Shape* aShape) {
            return s == aShape;
        });
        SkASSERT(index >= 0);
        fShapes.erase(fShapes.begin() + index);
    }
};

static SkRandom gRand;

class SampleDraw : public Sample {
    Doc fDoc;

protected:
    SkString name() override { return SkString("skdraw"); }

public:
    void onDrawContent(SkCanvas* canvas) override {
        fDoc.visit_b2f([canvas](Shape* s) {
            s->draw(canvas);
        });
        if (auto s = fDoc.selected()) {
            s->drawHilite(canvas);
        }
    }

    bool onChar(SkUnichar uni) override {
        switch (uni) {
            case 'r': {
                auto s = std::make_unique<RRectShape>();
                SkRect r = SkRect::MakeXYWH(gRand.nextF() * 640, gRand.nextF() * 500, 300, 200);
                s->set(SkRRect::MakeRectXY(r, 20, 20));
                s->fPaint.setColor(gRand.nextU() | (0x80 << 24));
                fDoc.add(std::move(s));
                return true;
            } break;
            case 'x':   // delete code?
                fDoc.remove(fDoc.selected());
                return true;
        }
        return this->INHERITED::onChar(uni);
    }

    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        auto shape = fDoc.find_f2b([x,y](Shape* sh) {
            return sh->hitTest({x, y});
        });
        fDoc.setSelected(shape);

        if (shape) {
            auto mx = shape->matrix();
            return new Click([shape, mx](Click* c) {
                shape->setMatrix(SkM44::Translate(c->fCurr.fX - c->fOrig.fX,
                                                  c->fCurr.fY - c->fOrig.fY) * mx);
                return true;
            });
        }
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

    bool onAnimate(double nanos) override {
        return false;
    }

private:
    typedef Sample INHERITED;
};
DEF_SAMPLE( return new SampleDraw; )
