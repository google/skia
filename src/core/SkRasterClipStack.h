/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterClipStack_DEFINED
#define SkRasterClipStack_DEFINED

#include "SkDeque.h"
#include "SkRasterClip.h"

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
    T& push(T src) {
        fTop = this->push_raw();
        new (fTop) T(std::move(src));
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

    enum {
        ELEM_COUNT = 16,
        PTR_COUNT = ELEM_COUNT * sizeof(Rec) / sizeof(void*)
    };
    void*           fStorage[PTR_COUNT];
    SkTStack<Rec>   fStack;
    const SkIRect   fRootBounds;

    SkRasterClip& writable_rc() {
        SkASSERT(fStack.top().fDeferredCount >= 0);
        if (fStack.top().fDeferredCount > 0) {
            fStack.top().fDeferredCount -= 1;
            fStack.push(fStack.top());
            fStack.top().fDeferredCount = 0;
        }
        return fStack.top().fRC;
    }
};

#endif
