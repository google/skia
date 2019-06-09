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

class RasterClipCache : public ClipCache {
public:
    RasterClipCache(const SkRasterClip& rc) : fRC(std::move(rc)) {}

    SkRasterClip fRC;
};

class RasterXformResolver : public XformResolver {
public:
    RasterXformResolver(const SkIRect& bounds)
        : fCTM(SkMatrix::I())
        , fRC(bounds)
        , fBounds(bounds)
    {}

    void concat(const SkMatrix& m) override {
        fCTM.preConcat(m);
    }

    void clipRect(const SkRect& r, SkClipOp op) override {
        fRC.op(r, fCTM, fBounds, (SkRegion::Op)op, false);
        fCache.reset(nullptr);
    }

    void clipRRect(const SkRRect& rr, SkClipOp op) override {
        fRC.op(rr, fCTM, fBounds, (SkRegion::Op)op, false);
        fCache.reset(nullptr);
    }
    void clipPath(const SkPath& p, SkClipOp op) override {
        fRC.op(p, fCTM, fBounds, (SkRegion::Op)op, false);
        fCache.reset(nullptr);
    }

    const SkMatrix& ctm() const { return fCTM; }

    sk_sp<ClipCache> snapCache() {
        if (!fCache) {
            fCache = sk_sp<ClipCache>(new RasterClipCache(fRC));
        }
        return fCache;
    }

private:
    const SkIRect   fBounds;
    SkMatrix        fCTM;
    SkRasterClip    fRC;
    sk_sp<ClipCache> fCache;
};

void XContext::drawRect(const SkRect& r, const SkPaint& p, Xform* x) {
    this->onDrawRect(r, p, x);
}

class CanvasXContext : public XContext {
public:
    CanvasXContext(SkCanvas* canvas) : fCanvas(canvas) {
        fBounds = {
            0, 0, canvas->getBaseLayerSize().width(), canvas->getBaseLayerSize().height()
        };
    }

protected:
    static int count_nodes(const Xform* x) {
        int n = 0;
        for (; x; x = x->parent()) {
            n += 1;
        }
        return n;
    }

    void onPush(Xform* x) override {
        int n = count_nodes(x);
        fCounts.push_back(n);
        if (n) {
            int prevCount = fStack.count();
            // now push the x tree such that we get [... grandparent, parent, x] in the array
            Xform** ptr = fStack.append(n) + n;
            while (n --> 0) {
                *--ptr = x;
                x = x->parent();
            }
#if 0
            // init with the old tail
            if (prevCount > 0) {
                RasterXformResolver res(fBounds, fStack[prevCount - 1]);
                for (int i = prevCount; i < fStack.count(); ++i) {
                    fStack[i]->visit(&res);
                    fStack[i]->setCache(res.ctm(), res.snapCache());
                }
            }
#endif
        }
        SkASSERT(!x);
    }

    void onPop() override {
        int n = fCounts.top();
        fCounts.pop();
        if (n) {
            fStack.setCount(fStack.count() - n);
        }
    }

    void onDrawRect(const SkRect& r, const SkPaint& p, Xform* x) override {
        this->push(x);

        RasterXformResolver res(fBounds);
        for (Xform** iter = fStack.begin(); iter < fStack.end(); ++iter) {
            (*iter)->visit(&res);
        }

        fCanvas->save();
        fCanvas->concat(res.ctm());
//        fCanvas->clipRegion(res.fRC.bwRgn());
        fCanvas->drawRect(r, p);
        fCanvas->restore();

        this->pop();
    }

private:
    SkTDArray<Xform*> fStack;
    SkTDArray<int>    fCounts;

    SkCanvas* fCanvas;    // bare pointer
    SkIRect   fBounds;
};

std::unique_ptr<XContext> XContext::Make(SkCanvas* canvas) {
    return std::unique_ptr<XContext>(new CanvasXContext(canvas));
}
