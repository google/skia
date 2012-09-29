/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSWMaskHelper_DEFINED
#define GrSWMaskHelper_DEFINED

#include "GrColor.h"
#include "GrMatrix.h"
#include "GrNoncopyable.h"
#include "SkBitmap.h"
#include "SkDraw.h"
#include "SkRasterClip.h"
#include "SkRegion.h"
#include "GrDrawState.h"

class GrAutoScratchTexture;
class GrContext;
class GrTexture;
class SkPath;
class GrDrawTarget;

/**
 * The GrSWMaskHelper helps generate clip masks using the software rendering
 * path. It is intended to be used as:
 *
 *   GrSWMaskHelper helper(context);
 *   helper.init(...);
 *
 *      draw one or more paths/rects specifying the required boolean ops
 *
 *   toTexture();   // to get it from the internal bitmap to the GPU
 *
 * The result of this process will be the final mask (on the GPU) in the
 * upper left hand corner of the texture.
 */
class GrSWMaskHelper : public GrNoncopyable {
public:
    GrSWMaskHelper(GrContext* context)
    : fContext(context) {
    }

    // set up the internal state in preparation for draws. Since many masks
    // may be accumulated in the helper during creation, "resultBounds"
    // allows the caller to specify the region of interest - to limit the
    // amount of work.
    bool init(const GrIRect& resultBounds, const GrMatrix* matrix);

    // Draw a single rect into the accumulation bitmap using the specified op
    void draw(const GrRect& rect, SkRegion::Op op,
              bool antiAlias, uint8_t alpha);

    // Draw a single path into the accumuation bitmap using the specified op
    void draw(const SkPath& path, SkRegion::Op op,
              GrPathFill fill, bool antiAlias, uint8_t alpha);

    // Helper function to get a scratch texture suitable for capturing the
    // result (i.e., right size & format)
    bool getTexture(GrAutoScratchTexture* texture);

    // Move the mask generation results from the internal bitmap to the gpu.
    // The space outside of the mask is cleared using "alpha"
    void toTexture(GrTexture* texture, uint8_t alpha);

    // Reset the internal bitmap
    void clear(uint8_t alpha) {
        fBM.eraseColor(SkColorSetARGB(alpha, alpha, alpha, alpha));
    }

    // Canonical usage utility that draws a single path and uploads it
    // to the GPU. The result is returned in "result".
    static GrTexture* DrawPathMaskToTexture(GrContext* context,
                                            const SkPath& path,
                                            const GrIRect& resultBounds,
                                            GrPathFill fill,
                                            bool antiAlias,
                                            GrMatrix* matrix);

    // This utility routine is used to add a path's mask to some other draw.
    // The ClipMaskManager uses it to accumulate clip masks while the
    // GrSoftwarePathRenderer uses it to fulfill a drawPath call.
    // It draws with "texture" as a path mask into "target" using "rect" as
    // geometry and the current drawState. The current drawState is altered to
    // accommodate the mask.
    // Note that this method assumes that the GrPaint::kTotalStages slot in
    // the draw state can be used to hold the mask texture stage.
    // This method is really only intended to be used with the
    // output of DrawPathMaskToTexture.
    static void DrawToTargetWithPathMask(GrTexture* texture,
                                         GrDrawTarget* target,
                                         const GrIRect& rect);

protected:
private:
    GrContext*      fContext;
    GrMatrix        fMatrix;
    SkBitmap        fBM;
    SkDraw          fDraw;
    SkRasterClip    fRasterClip;

    typedef GrNoncopyable INHERITED;
};

#endif // GrSWMaskHelper_DEFINED
