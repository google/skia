/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterClipStack_DEFINED
#define SkRasterClipStack_DEFINED

#include "include/core/SkClipOp.h"
#include "include/private/SkDeque.h"
#include "src/core/SkRasterClip.h"
#include <new>

template <typename T> class SkTStack {
public:
    SkTStack(void* storage, size_t size) : fDeque(sizeof(T), storage, size), fTop(nullptr) {}
    ~SkTStack() {
        while (!fDeque.empty()) {
            ((T*)fDeque.back())->~T();
            fDeque.pop_back();
        }
    }

    bool empty() const { return fDeque.empty(); }

    int count() const { return fDeque.count(); }

    const T& top() const {
        SkASSERT(fTop);
        return *fTop;
    }

    T& top() {
        SkASSERT(fTop);
        return *fTop;
    }

    T* push_raw() { return (T*)fDeque.push_back(); }
    T& push() {
        fTop = this->push_raw();
        new (fTop) T();
        return *fTop;
    }
    T& push(const T& src) {
        fTop = this->push_raw();
        new (fTop) T(src);
        return *fTop;
    }

    void pop() {
        fTop->~T();
        fDeque.pop_back();
        fTop = fDeque.empty() ? nullptr : (T*)fDeque.back();
    }

private:
    SkDeque fDeque;
    T*      fTop;
};

class SkRasterClipStack : SkNoncopyable {
    int fCounter = 0;
public:
    SkRasterClipStack(int width, int height)
        : fStack(fStorage, sizeof(fStorage))
        , fRootBounds(SkIRect::MakeWH(width, height))
    {
        Rec& rec = fStack.push();
        rec.fRC.setRect(fRootBounds);
        rec.fDeferredCount = 0;
        SkASSERT(fStack.count() == 1);
    }

    void setNewSize(int w, int h) {
        fRootBounds.setXYWH(0, 0, w, h);

        SkASSERT(fStack.count() == 1);
        Rec& rec = fStack.top();
        SkASSERT(rec.fDeferredCount == 0);
        rec.fRC.setRect(fRootBounds);
    }

    const SkRasterClip& rc() const { return fStack.top().fRC; }

    void save() {
        fCounter += 1;
        SkASSERT(fStack.top().fDeferredCount >= 0);
        fStack.top().fDeferredCount += 1;
    }

    void restore() {
        fCounter -= 1; SkASSERT(fCounter >= 0);
        if (--fStack.top().fDeferredCount < 0) {
            SkASSERT(fStack.top().fDeferredCount == -1);
            SkASSERT(fStack.count() > 1);
            fStack.pop();
        }
    }

    void clipRect(const SkMatrix& ctm, const SkRect& rect, SkClipOp op, bool aa) {
        this->writable_rc().op(rect, ctm, fRootBounds, (SkRegion::Op)op, aa);
        this->trimIfExpanding(op);
        this->validate();
    }

    void clipRRect(const SkMatrix& ctm, const SkRRect& rrect, SkClipOp op, bool aa) {
        this->writable_rc().op(rrect, ctm, fRootBounds, (SkRegion::Op)op, aa);
        this->trimIfExpanding(op);
        this->validate();
    }

    void clipPath(const SkMatrix& ctm, const SkPath& path, SkClipOp op, bool aa) {
        this->writable_rc().op(path, ctm, fRootBounds, (SkRegion::Op)op, aa);
        this->trimIfExpanding(op);
        this->validate();
    }

    void clipShader(sk_sp<SkShader> sh) {
        this->writable_rc().op(std::move(sh));
        this->validate();
    }

    void clipRegion(const SkRegion& rgn, SkClipOp op) {
        this->writable_rc().op(rgn, (SkRegion::Op)op);
        this->trimIfExpanding(op);
        this->validate();
    }

    void setDeviceClipRestriction(SkIRect* mutableClipRestriction) {
        this->writable_rc().setDeviceClipRestriction(mutableClipRestriction);
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
        SkRasterClip    fRC;
        int             fDeferredCount; // 0 for a "normal" entry
    };

    enum {
        ELEM_COUNT = 16,
        PTR_COUNT = ELEM_COUNT * sizeof(Rec) / sizeof(void*)
    };
    void*           fStorage[PTR_COUNT];
    SkTStack<Rec>   fStack;
    SkIRect         fRootBounds;

    SkRasterClip& writable_rc() {
        SkASSERT(fStack.top().fDeferredCount >= 0);
        if (fStack.top().fDeferredCount > 0) {
            fStack.top().fDeferredCount -= 1;
            fStack.push(fStack.top());
            fStack.top().fDeferredCount = 0;
        }
        return fStack.top().fRC;
    }

    void trimIfExpanding(SkClipOp op) {
        if ((int)op > (int)SkClipOp::kIntersect) {
            Rec& rec = fStack.top();
            SkASSERT(rec.fDeferredCount == 0);
            rec.fRC.op(fRootBounds, SkRegion::kIntersect_Op);
        }
    }
};

#endif
