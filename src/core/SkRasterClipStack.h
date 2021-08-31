/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterClipStack_DEFINED
#define SkRasterClipStack_DEFINED

#include "include/core/SkClipOp.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkScan.h"
#include "src/core/SkTBlockList.h"

class SkRasterClipStack : SkNoncopyable {
public:
    SkRasterClipStack(int width, int height)
            : fRootBounds(SkIRect::MakeWH(width, height))
            , fDisableAA(SkScan::DowngradeClipAA(fRootBounds)) {
        fStack.emplace_back(SkRasterClip(fRootBounds));
        SkASSERT(fStack.count() == 1);
    }

    void setNewSize(int w, int h) {
        fRootBounds.setXYWH(0, 0, w, h);

        SkASSERT(fStack.count() == 1);
        Rec& rec = fStack.back();
        SkASSERT(rec.fDeferredCount == 0);
        rec.fRC.setRect(fRootBounds);
    }

    const SkRasterClip& rc() const { return fStack.back().fRC; }

    void save() {
        SkDEBUGCODE(fCounter += 1);
        SkASSERT(fStack.back().fDeferredCount >= 0);
        fStack.back().fDeferredCount += 1;
    }

    void restore() {
        SkDEBUGCODE(fCounter -= 1);
        SkASSERT(fCounter >= 0);

        if (--fStack.back().fDeferredCount < 0) {
            SkASSERT(fStack.back().fDeferredCount == -1);
            SkASSERT(fStack.count() > 1);
            fStack.pop_back();
        }
    }

    void clipRect(const SkMatrix& ctm, const SkRect& rect, SkClipOp op, bool aa) {
        this->writable_rc().op(rect, ctm, op, this->finalAA(aa));
        this->validate();
    }

    void clipRRect(const SkMatrix& ctm, const SkRRect& rrect, SkClipOp op, bool aa) {
        this->writable_rc().op(rrect, ctm, op, this->finalAA(aa));
        this->validate();
    }

    void clipPath(const SkMatrix& ctm, const SkPath& path, SkClipOp op, bool aa) {
        this->writable_rc().op(path, ctm, op, this->finalAA(aa));
        this->validate();
    }

    void clipShader(sk_sp<SkShader> sh) {
        this->writable_rc().op(std::move(sh));
        this->validate();
    }

    void clipRegion(const SkRegion& rgn, SkClipOp op) {
        this->writable_rc().op(rgn, op);
        this->validate();
    }

    void replaceClip(const SkIRect& rect) {
        SkIRect devRect = rect;
        if (!devRect.intersect(fRootBounds)) {
            this->writable_rc().setEmpty();
        } else {
            this->writable_rc().setRect(devRect);
        }
    }

    void validate() const {
#ifdef SK_DEBUG
        const SkRasterClip& clip = this->rc();
        if (fRootBounds.isEmpty()) {
            SkASSERT(clip.isEmpty());
        } else if (!clip.isEmpty()) {
            SkASSERT(fRootBounds.contains(clip.getBounds()));
        }
#endif
    }

private:
    struct Rec {
        SkRasterClip fRC;
        int          fDeferredCount; // 0 for a "normal" entry

        Rec(const SkRasterClip& rc) : fRC(rc), fDeferredCount(0) {}
    };

    SkTBlockList<Rec, 16> fStack;
    SkIRect fRootBounds;
    bool fDisableAA;
    SkDEBUGCODE(int fCounter = 0);

    SkRasterClip& writable_rc() {
        SkASSERT(fStack.back().fDeferredCount >= 0);
        if (fStack.back().fDeferredCount > 0) {
            fStack.back().fDeferredCount -= 1;
            fStack.emplace_back(fStack.back().fRC);
        }
        return fStack.back().fRC;
    }

    bool finalAA(bool aa) const { return aa && !fDisableAA; }
};

#endif
