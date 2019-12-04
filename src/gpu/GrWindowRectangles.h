/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrWindowRectangles_DEFINED
#define GrWindowRectangles_DEFINED

#include "include/core/SkRect.h"
#include "src/gpu/GrNonAtomicRef.h"
#include <algorithm>

class GrWindowRectangles {
public:
    constexpr static int kMaxWindows = 8;

    GrWindowRectangles() { this->reset(); }

    int  count() const { return fCount; }
    bool empty() const { return this->count() == 0; }

    const SkIRect* data() const {
        return fHeap ? fHeap->windows
                     : &fInlineWindow;
    }

    void reset() {
        fCount        = 0;
        fInlineWindow = {0,0,0,0};
        fHeap         = nullptr;
    }

    SkIRect& addWindow(const SkIRect& window) { return (this->addWindow() = window); }
    SkIRect& addWindow() {
        SkASSERT(fCount < kMaxWindows);

        if (fCount == 0) {
            fCount = 1;
            return fInlineWindow;
        }

        if (!fHeap || !fHeap->unique()) {
            sk_sp<Rec> heap = sk_make_sp<Rec>();
            std::copy_n(this->data(), fCount, heap->windows);
            fHeap = std::move(heap);
        }
        return fHeap->windows[fCount++];
    }

    bool operator!=(const GrWindowRectangles& that) const { return !(*this == that); }
    bool operator==(const GrWindowRectangles& that) const {
        return this->count() == that.count()
            && std::equal(this->data(), this->data() + this->count(), that.data());
    }

    GrWindowRectangles makeOffset(int dx, int dy) const {
        if (dx == 0 && dy == 0) {
            return *this;
        }

        GrWindowRectangles result;
        for (int i = 0; i < this->count(); i++) {
            result.addWindow(this->data()[i].makeOffset(dx,dy));
        }
        return result;
    }

private:
    struct Rec : public GrNonAtomicRef<Rec> { SkIRect windows[kMaxWindows]; };

    int        fCount;
    SkIRect    fInlineWindow;
    sk_sp<Rec> fHeap;
};

#endif
