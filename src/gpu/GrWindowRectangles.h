/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrWindowRectangles_DEFINED
#define GrWindowRectangles_DEFINED

#include "GrNonAtomicRef.h"
#include "SkRect.h"

class GrWindowRectangles {
public:
    constexpr static int kMaxWindows = 8;

    GrWindowRectangles() : fCount(0) {}
    GrWindowRectangles(const GrWindowRectangles& that) : fCount(0) { *this = that; }
    ~GrWindowRectangles() { SkSafeUnref(this->rec()); }

    bool empty() const { return !fCount; }
    int count() const { return fCount; }
    const SkIRect* data() const;

    void reset();
    GrWindowRectangles& operator=(const GrWindowRectangles&);

    SkIRect& addWindow(const SkIRect& window) { return this->addWindow() = window; }
    SkIRect& addWindow();

    bool operator!=(const GrWindowRectangles& that) const { return !(*this == that); }
    bool operator==(const GrWindowRectangles&) const;

private:
    constexpr static int kNumLocalWindows = 1;
    struct Rec;

    const Rec* rec() const { return fCount <= kNumLocalWindows ? nullptr : fRec; }

    int fCount;
    union {
        SkIRect   fLocalWindows[kNumLocalWindows]; // If fCount <= kNumLocalWindows.
        Rec*      fRec;                            // If fCount > kNumLocalWindows.
    };
};

struct GrWindowRectangles::Rec : public GrNonAtomicRef<Rec> {
    Rec(const SkIRect* windows, int numWindows) {
        SkASSERT(numWindows < kMaxWindows);
        memcpy(fData, windows, sizeof(SkIRect) * numWindows);
    }

    SkIRect fData[kMaxWindows];
};

inline const SkIRect* GrWindowRectangles::data() const {
    return fCount <= kNumLocalWindows ? fLocalWindows : fRec->fData;
}

inline void GrWindowRectangles::reset() {
    SkSafeUnref(this->rec());
    fCount = 0;
}

inline GrWindowRectangles& GrWindowRectangles::operator=(const GrWindowRectangles& that) {
    SkSafeUnref(this->rec());
    fCount = that.fCount;
    if (fCount <= kNumLocalWindows) {
        memcpy(fLocalWindows, that.fLocalWindows, fCount * sizeof(SkIRect));
    } else {
        fRec = SkRef(that.fRec);
    }
    return *this;
}

inline SkIRect& GrWindowRectangles::addWindow() {
    SkASSERT(fCount < kMaxWindows);
    if (fCount < kNumLocalWindows) {
        return fLocalWindows[fCount++];
    }
    if (fCount == kNumLocalWindows) {
        fRec = new Rec(fLocalWindows, kNumLocalWindows);
    } else if (!fRec->unique()) { // Simple copy-on-write.
        fRec->unref();
        fRec = new Rec(fRec->fData, fCount);
    }
    return fRec->fData[fCount++];
}

inline bool GrWindowRectangles::operator==(const GrWindowRectangles& that) const {
    if (fCount != that.fCount) {
        return false;
    }
    if (fCount > kNumLocalWindows && fRec == that.fRec) {
        return true;
    }
    return !fCount || !memcmp(this->data(), that.data(), sizeof(SkIRect) * fCount);
}

#endif
