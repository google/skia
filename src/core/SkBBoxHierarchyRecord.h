
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

    virtual int save(SaveFlags flags = kMatrixClip_SaveFlag) SK_OVERRIDE;
    virtual int saveLayer(const SkRect* bounds, const SkPaint* paint,
                          SaveFlags flags = kARGB_ClipLayer_SaveFlag) SK_OVERRIDE;
    virtual void restore() SK_OVERRIDE;

    virtual bool translate(SkScalar dx, SkScalar dy) SK_OVERRIDE;
    virtual bool scale(SkScalar sx, SkScalar sy) SK_OVERRIDE;
    virtual bool rotate(SkScalar degrees) SK_OVERRIDE;
    virtual bool skew(SkScalar sx, SkScalar sy) SK_OVERRIDE;
    virtual bool concat(const SkMatrix& matrix) SK_OVERRIDE;
    virtual void setMatrix(const SkMatrix& matrix) SK_OVERRIDE;

    // Implementation of the SkBBoxHierarchyClient interface
    virtual bool shouldRewind(void* data) SK_OVERRIDE;

protected:
    virtual void onClipRect(const SkRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipRRect(const SkRRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipPath(const SkPath&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipRegion(const SkRegion&, SkRegion::Op) SK_OVERRIDE;

private:
    typedef SkBBoxRecord INHERITED;
};

#endif
