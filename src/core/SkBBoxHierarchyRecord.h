
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRTreeCanvas_DEFINED
#define SkRTreeCanvas_DEFINED

#include "SkBBoxHierarchy.h"
#include "SkBBoxRecord.h"

/**
 * This records bounding box information into an SkBBoxHierarchy, and clip/transform information
 * into an SkPictureStateTree to allow for efficient culling and correct playback of draws.
 */
class SkBBoxHierarchyRecord : public SkBBoxRecord, public SkBBoxHierarchyClient {
public:
    /** This will take a ref of h */
    SkBBoxHierarchyRecord(const SkISize& size, uint32_t recordFlags, SkBBoxHierarchy* h);

    virtual void handleBBox(const SkRect& bounds) SK_OVERRIDE;

    // Implementation of the SkBBoxHierarchyClient interface
    virtual bool shouldRewind(void* data) SK_OVERRIDE;

protected:
    virtual void willSave(SaveFlags) SK_OVERRIDE;
    virtual SaveLayerStrategy willSaveLayer(const SkRect*, const SkPaint*, SaveFlags) SK_OVERRIDE;
    virtual void willRestore() SK_OVERRIDE;

    virtual void didTranslate(SkScalar, SkScalar) SK_OVERRIDE;
    virtual void didScale(SkScalar, SkScalar) SK_OVERRIDE;
    virtual void didRotate(SkScalar) SK_OVERRIDE;
    virtual void didSkew(SkScalar, SkScalar) SK_OVERRIDE;
    virtual void didConcat(const SkMatrix&) SK_OVERRIDE;
    virtual void didSetMatrix(const SkMatrix&) SK_OVERRIDE;

    virtual void onClipRect(const SkRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipRRect(const SkRRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipPath(const SkPath&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipRegion(const SkRegion&, SkRegion::Op) SK_OVERRIDE;

private:
    typedef SkBBoxRecord INHERITED;
};

#endif
