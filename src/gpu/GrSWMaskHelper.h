/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSWMaskHelper_DEFINED
#define GrSWMaskHelper_DEFINED

#include "GrTypesPriv.h"
#include "SkAutoPixmapStorage.h"
#include "SkDraw.h"
#include "SkMatrix.h"
#include "SkRasterClip.h"
#include "SkRegion.h"
#include "SkTypes.h"

class GrShape;
class GrRecordingContext;
class GrTextureProxy;

/**
 * The GrSWMaskHelper helps generate clip masks using the software rendering
 * path. It is intended to be used as:
 *
 *   GrSWMaskHelper helper(context);
 *   helper.init(...);
 *
 *      draw one or more paths/rects specifying the required boolean ops
 *
 *   toTextureProxy();   // to get it from the internal bitmap to the GPU
 *
 * The result of this process will be the final mask (on the GPU) in the
 * upper left hand corner of the texture.
 */
class GrSWMaskHelper : SkNoncopyable {
public:
    GrSWMaskHelper(SkAutoPixmapStorage* pixels = nullptr)
            : fPixels(pixels ? pixels : &fPixelsStorage) { }

    // set up the internal state in preparation for draws. Since many masks
    // may be accumulated in the helper during creation, "resultBounds"
    // allows the caller to specify the region of interest - to limit the
    // amount of work.
    bool init(const SkIRect& resultBounds);

    // Draw a single rect into the accumulation bitmap using the specified op
    void drawRect(const SkRect& rect, const SkMatrix& matrix, SkRegion::Op op, GrAA, uint8_t alpha);

    // Draw a single path into the accumuation bitmap using the specified op
    void drawShape(const GrShape&, const SkMatrix& matrix, SkRegion::Op op, GrAA, uint8_t alpha);

    sk_sp<GrTextureProxy> toTextureProxy(GrRecordingContext*, SkBackingFit fit);

    // Reset the internal bitmap
    void clear(uint8_t alpha) {
        fPixels->erase(SkColorSetARGB(alpha, 0xFF, 0xFF, 0xFF));
    }

private:
    SkVector             fTranslate;
    SkAutoPixmapStorage* fPixels;
    SkAutoPixmapStorage  fPixelsStorage;
    SkDraw               fDraw;
    SkRasterClip         fRasterClip;

    typedef SkNoncopyable INHERITED;
};

#endif // GrSWMaskHelper_DEFINED
