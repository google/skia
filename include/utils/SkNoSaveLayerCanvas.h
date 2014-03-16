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
class SK_API SkNoSaveLayerCanvas : public SkCanvas {
public:
    SkNoSaveLayerCanvas(SkBaseDevice* device) : INHERITED(device) {}

protected:
    virtual SaveLayerStrategy willSaveLayer(const SkRect* bounds, const SkPaint* paint,
                                            SaveFlags flags) SK_OVERRIDE {
        this->INHERITED::willSaveLayer(bounds, paint, flags);
        return kNoLayer_SaveLayerStrategy;
    }

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
