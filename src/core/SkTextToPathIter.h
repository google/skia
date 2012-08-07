/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextToPathIter_DEFINED
#define SkTextToPathIter_DEFINED

#include "SkAutoKern.h"
#include "SkPaint.h"

class SkGlyphCache;

class SkTextToPathIter {
public:
    SkTextToPathIter(const char text[], size_t length, const SkPaint& paint,
                     bool applyStrokeAndPathEffects);
    ~SkTextToPathIter();

    const SkPaint&  getPaint() const { return fPaint; }
    SkScalar        getPathScale() const { return fScale; }

    const SkPath*   next(SkScalar* xpos);   //!< returns nil when there are no more paths

private:
    SkGlyphCache*   fCache;
    SkPaint         fPaint;
    SkScalar        fScale;
    SkFixed         fPrevAdvance;
    const char*     fText;
    const char*     fStop;
    SkMeasureCacheProc fGlyphCacheProc;

    const SkPath*   fPath;      // returned in next
    SkScalar        fXPos;      // accumulated xpos, returned in next
    SkAutoKern      fAutoKern;
    int             fXYIndex;   // cache for horizontal -vs- vertical text
};

#endif
