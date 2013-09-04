
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
    virtual void removeAll() SK_OVERRIDE;

    /*
     * The following add/remove canvas methods are overrides from SkNWayCanvas
     * that do not make sense in the context of our CanvasStack, but since we
     * can share most of the other implementation of NWay we override those
     * methods to be no-ops.
     */
    virtual void addCanvas(SkCanvas*) SK_OVERRIDE { SkDEBUGFAIL("Invalid Op"); }
    virtual void removeCanvas(SkCanvas*) SK_OVERRIDE { SkDEBUGFAIL("Invalid Op"); }

    virtual void setMatrix(const SkMatrix& matrix) SK_OVERRIDE;
    virtual bool clipRect(const SkRect&, SkRegion::Op, bool) SK_OVERRIDE;
    virtual bool clipRRect(const SkRRect&, SkRegion::Op, bool) SK_OVERRIDE;
    virtual bool clipPath(const SkPath&, SkRegion::Op, bool) SK_OVERRIDE;
    virtual bool clipRegion(const SkRegion& deviceRgn,
                            SkRegion::Op) SK_OVERRIDE;

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
