/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterClipStack_DEFINED
#define SkRasterClipStack_DEFINED

#include "SkRasterClip.h"
#include "SkTArray.h"

class SkRasterClipStack : SkNoncopyable {
public:
    SkRasterClipStack(int width, int height) : fRootBounds(SkIRect::MakeWH(width, height)) {
        Rec& back = fArray.push_back();
        back.fRC.setRect(fRootBounds);
        back.fDeferredCount = 0;
    }

    void save() { fArray.back().fDeferredCount += 1; }

    void restore() { fArray.pop_back(); }

    void clipRect(const SkMatrix& ctm, const SkRect& rect, SkClipOp op, bool aa) {
        this->writable_rc().op(rect, ctm, fRootBounds, (SkRegion::Op)op, aa);
    }

    void clipRRect(const SkMatrix& ctm, const SkRRect& rrect, SkClipOp op, bool aa) {
        this->writable_rc().op(rrect, ctm, fRootBounds, (SkRegion::Op)op, aa);
    }

    void clipPath(const SkMatrix& ctm, const SkPath& path, SkClipOp op, bool aa) {
        this->writable_rc().op(path, ctm, fRootBounds, (SkRegion::Op)op, aa);
    }

    void clipRegion(const SkRegion& rgn, SkClipOp op) {
        this->writable_rc().op(rgn, (SkRegion::Op)op);
    }

private:
    struct Rec {
        SkRasterClip    fRC;
        int             fDeferredCount; // 0 for a "normal" entry
    };
    SkTArray<Rec>   fArray;
    const SkIRect   fRootBounds;

    SkRasterClip& writable_rc() {
        Rec* back = &fArray.back();
        if (back->fDeferredCount > 0) {
            back->fDeferredCount -= 1;
            fArray.push_back(*back);
            back = &fArray.back();
            back->fDeferredCount = 0;
        }
        return back->fRC;
    }
};

#endif
