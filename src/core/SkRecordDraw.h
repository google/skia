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
#include "SkMatrix.h"
#include "SkRecord.h"

// Fill a BBH to be used by SkRecordDraw to accelerate playback.
void SkRecordFillBounds(const SkRecord&, SkBBoxHierarchy*);

// Draw an SkRecord into an SkCanvas.  A convenience wrapper around SkRecords::Draw.
void SkRecordDraw(const SkRecord&, SkCanvas*, const SkBBoxHierarchy*, SkDrawPictureCallback*);

// Draw a portion of an SkRecord into an SkCanvas while replacing clears with drawRects.
// When drawing a portion of an SkRecord the CTM on the passed in canvas must be
// the composition of the replay matrix with the record-time CTM (for the portion
// of the record that is being replayed). For setMatrix calls to behave correctly
// the initialCTM parameter must set to just the replay matrix.
void SkRecordPartialDraw(const SkRecord&, SkCanvas*, const SkRect&, unsigned start, unsigned stop,
                         const SkMatrix& initialCTM);

namespace SkRecords {

// This is an SkRecord visitor that will draw that SkRecord to an SkCanvas.
class Draw : SkNoncopyable {
public:
    explicit Draw(SkCanvas* canvas, const SkMatrix* initialCTM = NULL)
        : fInitialCTM(initialCTM ? *initialCTM : canvas->getTotalMatrix())
        , fCanvas(canvas) {}

    template <typename T> void operator()(const T& r) {
        this->draw(r);
    }

private:
    // No base case, so we'll be compile-time checked that we implement all possibilities.
    template <typename T> void draw(const T&);

    const SkMatrix fInitialCTM;
    SkCanvas* fCanvas;
};

// Used by SkRecordPartialDraw.
class PartialDraw : public Draw {
public:
    PartialDraw(SkCanvas* canvas, const SkRect& clearRect, const SkMatrix& initialCTM)
        : INHERITED(canvas, &initialCTM), fClearRect(clearRect) {}

    // Same as Draw for all ops except Clear.
    template <typename T> void operator()(const T& r) {
        this->INHERITED::operator()(r);
    }
    void operator()(const Clear& c) {
        SkPaint p;
        p.setColor(c.color);
        DrawRect drawRect(p, fClearRect);
        this->INHERITED::operator()(drawRect);
    }

private:
    const SkRect fClearRect;
    typedef Draw INHERITED;
};

}  // namespace SkRecords

#endif//SkRecordDraw_DEFINED
