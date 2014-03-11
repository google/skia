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

protected:
    // disable aa for speed
    virtual void onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle) SK_OVERRIDE {
        this->INHERITED::onClipRect(rect, op, kHard_ClipEdgeStyle);
    }

    // for speed, just respect the bounds, and disable AA. May give us a few
    // false positives and negatives.
    virtual void onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle) SK_OVERRIDE {
        this->updateClipConservativelyUsingBounds(path.getBounds(), op,
                                                  path.isInverseFillType());
    }
    virtual void onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle) SK_OVERRIDE {
        this->updateClipConservativelyUsingBounds(rrect.getBounds(), op, false);
    }

private:
    typedef SkCanvas INHERITED;
};

#endif // SkNoSaveLayerCanvas_DEFINED
