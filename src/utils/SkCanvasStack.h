
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCanvasStack_DEFINED
#define SkCanvasStack_DEFINED

#include "SkNWayCanvas.h"
#include "SkTArray.h"

class SkCanvasStack : public SkNWayCanvas {
public:
    SkCanvasStack(int width, int height);
    virtual ~SkCanvasStack();

    void pushCanvas(SkCanvas* canvas, const SkIPoint& origin);
    void removeAll() override;

    /*
     * The following add/remove canvas methods are overrides from SkNWayCanvas
     * that do not make sense in the context of our CanvasStack, but since we
     * can share most of the other implementation of NWay we override those
     * methods to be no-ops.
     */
    void addCanvas(SkCanvas*) override { SkDEBUGFAIL("Invalid Op"); }
    void removeCanvas(SkCanvas*) override { SkDEBUGFAIL("Invalid Op"); }

protected:
    void didSetMatrix(const SkMatrix&) override;

    void onClipRect(const SkRect&, SkRegion::Op, ClipEdgeStyle) override;
    void onClipRRect(const SkRRect&, SkRegion::Op, ClipEdgeStyle) override;
    void onClipPath(const SkPath&, SkRegion::Op, ClipEdgeStyle) override;
    void onClipRegion(const SkRegion&, SkRegion::Op) override;

private:
    void clipToZOrderedBounds();

    struct CanvasData {
        SkIPoint origin;
        SkRegion requiredClip;
    };

    SkTArray<CanvasData> fCanvasData;

    typedef SkNWayCanvas INHERITED;
};

#endif
