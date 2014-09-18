/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRecordReplaceDraw_DEFINED
#define GrRecordReplaceDraw_DEFINED

#include "SkDrawPictureCallback.h"
#include "SkRect.h"
#include "SkTDArray.h"

class SkBBoxHierarchy;
class SkBitmap;
class SkCanvas;
class SkImage;
class SkPaint;
class SkRecord;

// GrReplacements collects op ranges that can be replaced with
// a single drawBitmap call (using a precomputed bitmap).
class GrReplacements {
public:
    // All the operations between fStart and fStop (inclusive) will be replaced with
    // a single drawBitmap call using fPos, fBM and fPaint.
    struct ReplacementInfo {
        unsigned        fStart;
        unsigned        fStop;
        SkIPoint        fPos;
        SkImage*        fImage;  // Owns a ref
        const SkPaint*  fPaint;  // Owned by this object

        SkIRect         fSrcRect;
    };

    ~GrReplacements() { this->freeAll(); }

    // Add a new replacement range. The replacement ranges should be
    // sorted in increasing order and non-overlapping (esp. no nested
    // saveLayers).
    ReplacementInfo* push();

    // look up a replacement range by its start offset.
    // lastLookedUp is an in/out parameter that is used to speed up the search.
    // It should be initialized to 0 on the first call and then passed back in
    // unmodified on subsequent calls.
    const ReplacementInfo* lookupByStart(size_t start, int* lastLookedUp) const;

private:
    SkTDArray<ReplacementInfo> fReplacements;

    void freeAll();

#ifdef SK_DEBUG
    void validate() const;
#endif
};

// Draw an SkRecord into an SkCanvas replacing saveLayer/restore blocks with
// drawBitmap calls.  A convenience wrapper around SkRecords::Draw.
void GrRecordReplaceDraw(const SkRecord&, 
                         SkCanvas*,
                         const SkBBoxHierarchy*,
                         const GrReplacements*,
                         SkDrawPictureCallback*);

#endif // GrRecordReplaceDraw_DEFINED
