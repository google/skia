/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecordDraw_DEFINED
#define SkRecordDraw_DEFINED

#include "include/core/SkBBHFactory.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkM44.h"
#include "include/core/SkPicture.h"
#include "include/private/base/SkNoncopyable.h"

class SkDrawable;
class SkRecord;
struct SkRect;

// Calculate conservative identity space bounds for each op in the record.
void SkRecordFillBounds(const SkRect& cullRect, const SkRecord&,
                        SkRect bounds[], SkBBoxHierarchy::Metadata[]);

// Draw an SkRecord into an SkCanvas.  A convenience wrapper around SkRecords::Draw.
void SkRecordDraw(const SkRecord&, SkCanvas*, SkPicture const* const drawablePicts[],
                  SkDrawable* const drawables[], int drawableCount,
                  const SkBBoxHierarchy*, SkPicture::AbortCallback*);

namespace SkRecords {

// This is an SkRecord visitor that will draw that SkRecord to an SkCanvas.
class Draw : SkNoncopyable {
public:
    explicit Draw(SkCanvas* canvas, SkPicture const* const drawablePicts[],
                  SkDrawable* const drawables[], int drawableCount,
                  const SkM44* initialCTM = nullptr)
        : fInitialCTM(initialCTM ? *initialCTM : canvas->getLocalToDevice())
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

    const SkM44 fInitialCTM;
    SkCanvas* fCanvas;
    SkPicture const* const* fDrawablePicts;
    SkDrawable* const* fDrawables;
    int fDrawableCount;
};

}  // namespace SkRecords

#endif//SkRecordDraw_DEFINED
