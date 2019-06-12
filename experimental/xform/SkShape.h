/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShape_DEFINED
#define SkShape_DEFINED

#include "experimental/xform/SkXform.h"
#include "include/core/SkPaint.h"

class SkCanvas;

class XContext {
public:
    virtual ~XContext() {}

    void push(Xform* parentXform) { this->onPush(parentXform); }
    void pop() { this->onPop(); }

    void drawRect(const SkRect&, const SkPaint&, Xform* localXform);

    static std::unique_ptr<XContext> Make(SkCanvas*);

protected:
    virtual void onPush(Xform*) = 0;
    virtual void onPop() = 0;

    virtual void onDrawRect(const SkRect&, const SkPaint&, Xform*) = 0;
};

class Shape : public SkRefCnt {
    sk_sp<Xform>    fXform;

public:
    Shape(sk_sp<Xform> x = nullptr) : fXform(std::move(x)) {}

    Xform* xform() const { return fXform.get(); }
    void setXform(sk_sp<Xform> x) {
        fXform = std::move(x);
    }

    virtual void draw(XContext*) {}
};

class GeoShape : public Shape {
    SkRect  fRect;
    SkPaint fPaint;

    GeoShape(sk_sp<Xform> x, const SkRect& r, SkColor c) : Shape(std::move(x)), fRect(r) {
        fPaint.setColor(c);
    }

public:
    static sk_sp<Shape> Make(sk_sp<Xform> x, const SkRect& r, SkColor c) {
        return sk_sp<Shape>(new GeoShape(std::move(x), r, c));
    }

    void draw(XContext*) override;
};

class GroupShape : public Shape {
    SkTDArray<Shape*> fArray;

    GroupShape(sk_sp<Xform> x) : Shape(std::move(x)) {}

public:
    static sk_sp<GroupShape> Make(sk_sp<Xform> x = nullptr) {
        return sk_sp<GroupShape>(new GroupShape(std::move(x)));
    }

    static sk_sp<GroupShape> Make(sk_sp<Xform> x, sk_sp<Shape> s) {
        auto g = sk_sp<GroupShape>(new GroupShape(std::move(x)));
        g->append(std::move(s));
        return g;
    }

    ~GroupShape() override {
        fArray.unrefAll();
    }

    int count() const { return fArray.count(); }
    Shape* get(int index) const { return fArray[index]; }
    void set(int index, sk_sp<Shape> s) {
        fArray[index] = s.release();
    }

    void append(sk_sp<Shape> s) {
        *fArray.append() = s.release();
    }
    void insert(int index, sk_sp<Shape> s) {
        *fArray.insert(index) = s.release();
    }
    void remove(int index) {
        SkSafeUnref(fArray[index]);
        fArray.remove(index);
    }

    void draw(XContext*) override ;
};

#endif
