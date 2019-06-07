/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/xform/SkShape.h"
#include "experimental/xform/SkXform.h"
#include "include/core/SkCanvas.h"

#include "src/core/SkRasterClip.h"

class RasterXformResolver : XformResolver {
public:
    RasterXformResolver(const SkIRect& bounds) : fBounds(bounds) {}

    void concat(const SkMatrix& m) override {
        fCTM.preConcat(m);
    }

    void clipRect(const SkRect& r, SkClipOp op) override {
        fRC.op(r, fCTM, fBounds, (SkRegion::Op)op, false);
    }

    void clipRRect(const SkRRect& rr, SkClipOp op) override {
        fRC.op(rr, fCTM, fBounds, (SkRegion::Op)op, false);
    }
    void clipPath(const SkPath& p, SkClipOp op) override {
        fRC.op(p, fCTM, fBounds, (SkRegion::Op)op, false);
    }

private:
    const SkIRect   fBounds;
    SkMatrix        fCTM;
    SkRasterClip    fRC;
};

void XContext::drawRect(const SkRect& r, const SkPaint& p, Xform* x) {
    this->onDrawRect(r, p, x);
}

class CanvasXContext : public XContext {
public:
    CanvasXContext(SkCanvas* canvas) : fCanvas(canvas) {}

protected:
    void onPush(Xform* x) override {
        fStack.push_back(x);
    }

    void onPop() override {
        fStack.pop();
    }

    void onDrawRect(const SkRect& r, const SkPaint& p, Xform* x) override {
        fCanvas->drawRect(r, p);
    }

private:
    SkTDArray<Xform*> fStack;

    SkCanvas* fCanvas;    // bare pointer
};

std::unique_ptr<XContext> XContext::Make(SkCanvas* canvas) {
    return std::unique_ptr<XContext>(new CanvasXContext(canvas));
}
