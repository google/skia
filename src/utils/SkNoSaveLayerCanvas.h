/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNoSaveLayerCanvas_DEFINED
#define SkNoSaveLayerCanvas_DEFINED

#include "SkCanvas.h"
#include "SkRRect.h"

// The NoSaveLayerCanvas is used to play back SkPictures when the saveLayer
// functionality isn't required (e.g., during analysis of the draw calls).
// It also simplifies the clipping calls to only use rectangles.
class SkNoSaveLayerCanvas : public SkCanvas {
public:
    SkNoSaveLayerCanvas(SkBaseDevice* device) : INHERITED(device) {}

    // turn saveLayer() into save() for speed, should not affect correctness.
    virtual int saveLayer(const SkRect* bounds,
                          const SkPaint* paint,
                          SaveFlags flags) SK_OVERRIDE {

        // Like SkPictureRecord, we don't want to create layers, but we do need
        // to respect the save and (possibly) its rect-clip.
        int count = this->INHERITED::save(flags);
        if (NULL != bounds) {
            this->INHERITED::clipRectBounds(bounds, flags, NULL);
        }
        return count;
    }

    // disable aa for speed
    virtual bool clipRect(const SkRect& rect,
                          SkRegion::Op op,
                          bool doAA) SK_OVERRIDE {
        return this->INHERITED::clipRect(rect, op, false);
    }

    // for speed, just respect the bounds, and disable AA. May give us a few
    // false positives and negatives.
    virtual bool clipPath(const SkPath& path,
                          SkRegion::Op op,
                          bool doAA) SK_OVERRIDE {
        return this->updateClipConservativelyUsingBounds(path.getBounds(), op,
                                                         path.isInverseFillType());
    }
    virtual bool clipRRect(const SkRRect& rrect,
                           SkRegion::Op op,
                           bool doAA) SK_OVERRIDE {
        return this->updateClipConservativelyUsingBounds(rrect.getBounds(), op, false);
    }

private:
    typedef SkCanvas INHERITED;
};

#endif // SkNoSaveLayerCanvas_DEFINED
