/*
* Copyright 2014 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_RectanizerPow2_DEFINED
#define skgpu_RectanizerPow2_DEFINED

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMalloc.h"
#include "src/base/SkMathPriv.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/Rectanizer.h"

#include <cstdint>

namespace skgpu {

// This Rectanizer quantizes the incoming rects to powers of 2. Each power
// of two can have, at most, one active row/shelf. Once a row/shelf for
// a particular power of two gets full its fRows entry is recycled to point
// to a new row.
// The skyline algorithm almost always provides a better packing.
//
// Mark this class final in an effort to avoid the vtable when this subclass is used explicitly.
class RectanizerPow2 final : public Rectanizer {
public:
    RectanizerPow2(int w, int h) : Rectanizer(w, h) {
        this->reset();
    }

    ~RectanizerPow2() final {}

    void reset() final {
        fNextStripY = 0;
        fAreaSoFar = 0;
        sk_bzero(fRows, sizeof(fRows));
    }

    bool addRect(int w, int h, SkIPoint16* loc) final;

    float percentFull() const final {
        return fAreaSoFar / ((float)this->width() * this->height());
    }

private:
    static const int kMIN_HEIGHT_POW2 = 2;
    static const int kMaxExponent = 16;

    struct Row {
        SkIPoint16  fLoc;
        // fRowHeight is actually known by this struct's position in fRows
        // but it is used to signal if there exists an open row of this height
        int         fRowHeight;

        bool canAddWidth(int width, int containerWidth) const {
            return fLoc.fX + width <= containerWidth;
        }
    };

    Row fRows[kMaxExponent];    // 0-th entry will be unused

    int fNextStripY;
    int32_t fAreaSoFar;

    static int HeightToRowIndex(int height) {
        SkASSERT(height >= kMIN_HEIGHT_POW2);
        int index = 32 - SkCLZ(height - 1);
        SkASSERT(index < kMaxExponent);
        return index;
    }

    bool canAddStrip(int height) const {
        return fNextStripY + height <= this->height();
    }

    void initRow(Row* row, int rowHeight) {
        row->fLoc.set(0, fNextStripY);
        row->fRowHeight = rowHeight;
        fNextStripY += rowHeight;
    }
};

} // End of namespace skgpu

#endif
