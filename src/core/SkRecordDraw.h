/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecordDraw_DEFINED
#define SkRecordDraw_DEFINED

#include "SkBBoxHierarchy.h"
#include "SkCanvas.h"
#include "SkDrawPictureCallback.h"
#include "SkRecord.h"

// Fill a BBH to be used by SkRecordDraw to accelerate playback.
void SkRecordFillBounds(const SkRecord&, SkBBoxHierarchy*);

// Draw an SkRecord into an SkCanvas.  A convenience wrapper around SkRecords::Draw.
void SkRecordDraw(const SkRecord&, SkCanvas*, const SkBBoxHierarchy*, SkDrawPictureCallback*);

namespace SkRecords {

// This is an SkRecord visitor that will draw that SkRecord to an SkCanvas.
class Draw : SkNoncopyable {
public:
    explicit Draw(SkCanvas* canvas)
        : fInitialCTM(canvas->getTotalMatrix()), fCanvas(canvas), fIndex(0) {}

    unsigned index() const { return fIndex; }
    void next() { ++fIndex; }

    template <typename T> void operator()(const T& r) {
        this->draw(r);
    }

private:
    // No base case, so we'll be compile-time checked that we implement all possibilities.
    template <typename T> void draw(const T&);

    const SkMatrix fInitialCTM;
    SkCanvas* fCanvas;
    unsigned fIndex;
};

}  // namespace SkRecords

#endif//SkRecordDraw_DEFINED
