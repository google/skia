/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecordDraw_DEFINED
#define SkRecordDraw_DEFINED

#include "SkBBoxHierarchy.h"
#include "SkBigPicture.h"
#include "SkCanvas.h"
#include "SkMatrix.h"
#include "SkRecord.h"

class SkDrawable;
class SkLayerInfo;

// Calculate conservative identity space bounds for each op in the record.
void SkRecordFillBounds(const SkRect& cullRect, const SkRecord&, SkRect bounds[]);

// SkRecordFillBounds(), and gathers information about saveLayers and stores it for later
// use (e.g., layer hoisting). The gathered information is sufficient to determine
// where each saveLayer will land and which ops in the picture it represents.
void SkRecordComputeLayers(const SkRect& cullRect, const SkRecord&, SkRect bounds[],
                           const SkBigPicture::SnapshotArray*, SkLayerInfo* data);

// Draw an SkRecord into an SkCanvas.  A convenience wrapper around SkRecords::Draw.
void SkRecordDraw(const SkRecord&, SkCanvas*, SkPicture const* const drawablePicts[],
                  SkDrawable* const drawables[], int drawableCount,
                  const SkBBoxHierarchy*, SkPicture::AbortCallback*);

// Draw a portion of an SkRecord into an SkCanvas.
// When drawing a portion of an SkRecord the CTM on the passed in canvas must be
// the composition of the replay matrix with the record-time CTM (for the portion
// of the record that is being replayed). For setMatrix calls to behave correctly
// the initialCTM parameter must set to just the replay matrix.
void SkRecordPartialDraw(const SkRecord&, SkCanvas*,
                         SkPicture const* const drawablePicts[], int drawableCount,
                         int start, int stop, const SkMatrix& initialCTM);

namespace SkRecords {

// This is an SkRecord visitor that will draw that SkRecord to an SkCanvas.
class Draw : SkNoncopyable {
public:
    explicit Draw(SkCanvas* canvas, SkPicture const* const drawablePicts[],
                  SkDrawable* const drawables[], int drawableCount,
                  const SkMatrix* initialCTM = nullptr)
        : fInitialCTM(initialCTM ? *initialCTM : canvas->getTotalMatrix())
        , fCanvas(canvas)
        , fDrawablePicts(drawablePicts)
        , fDrawables(drawables)
        , fDrawableCount(drawableCount)
    {}

    // This operator calls methods on the |canvas|. The various draw() wrapper
    // methods around SkCanvas are defined by the DRAW() macro in
    // SkRecordDraw.cpp.
    template <typename T> void operator()(const T& r) {
        this->draw(r);
    }

protected:
    SkPicture const* const* drawablePicts() const { return fDrawablePicts; }
    int drawableCount() const { return fDrawableCount; }

private:
    // No base case, so we'll be compile-time checked that we implement all possibilities.
    template <typename T> void draw(const T&);

    const SkMatrix fInitialCTM;
    SkCanvas* fCanvas;
    SkPicture const* const* fDrawablePicts;
    SkDrawable* const* fDrawables;
    int fDrawableCount;
};

}  // namespace SkRecords

#endif//SkRecordDraw_DEFINED
