// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkAutoDrawLooper_DEFINED
#define SkAutoDrawLooper_DEFINED

#include "SkArenaAlloc.h"
#include "SkDrawLooper.h"

class SkAutoDrawLooper {
public:
    SkAutoDrawLooper(SkCanvas* canvas, const SkPaint* paint)
        : fCanvas(canvas), fPaint(MakeCopy(paint)) {}
    struct Iter {
        void operator++() { this->doNext(); }
        const SkPaint& operator*() const { return *fPaintPerLoop.get(); }
        bool operator!=(const Iter& rhs) const { return fCanvas != rhs.fCanvas; }
        SkCanvas* fCanvas = nullptr;
        const SkPaint* fOriginalPaint = nullptr;
        SkDrawLooper::Context* fLooperContext = nullptr;
        SkTCopyOnFirstWrite<SkPaint> fPaintPerLoop;
        SkArenaAlloc fAlloc = SkArenaAlloc(0);
        void doNext() {
            if (fCanvas) {
                fPaintPerLoop = SkTCopyOnFirstWrite<SkPaint>(fOriginalPaint);
                if (!fLooperContext) {
                    fCanvas = nullptr;
                } else {
                    fPaintPerLoop.writable()->setDrawLooper(nullptr);
                    if (!fLooperContext->next(fCanvas, fPaintPerLoop.writable())) {
                        fCanvas = nullptr;  // Mark as done.
                    }
                }
            }
        }
    };
    Iter begin() {
        Iter itr;
        itr.fCanvas = fCanvas;
        itr.fOriginalPaint = fPaint.get();
        if (SkDrawLooper* looper = fPaint.get()->getDrawLooper()) {
            itr.fLooperContext = looper->makeContext(fCanvas, &itr.fAlloc);
            itr.doNext();
        } else {
            itr.fPaintPerLoop = SkTCopyOnFirstWrite<SkPaint>(fPaint.get());
        }
        return itr;
    }
    Iter end() { return Iter{}; }
    template <typename T>
    static SkTCopyOnFirstWrite<T> MakeCopy(const T* ptr = nullptr) {
        if (ptr) {
            return SkTCopyOnFirstWrite<T>(*ptr);
        }
        const T obj;
        SkTCopyOnFirstWrite<T> copy(obj);
        (void)copy.writable(); // trigger copy
        return std::move(copy);
    }

private:
    SkCanvas* fCanvas;
    SkTCopyOnFirstWrite<SkPaint> fPaint;
};
#endif  // SkAutoDrawLooper_DEFINED
