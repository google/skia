
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNWayCanvas_DEFINED
#define SkNWayCanvas_DEFINED

#include "../private/SkTDArray.h"
#include "SkNoDrawCanvas.h"

class SK_API SkNWayCanvas : public SkNoDrawCanvas {
public:
    SkNWayCanvas(int width, int height);
    ~SkNWayCanvas() override;

    virtual void addCanvas(SkCanvas*);
    virtual void removeCanvas(SkCanvas*);
    virtual void removeAll();

    ///////////////////////////////////////////////////////////////////////////
    // These are forwarded to the N canvases we're referencing

#ifdef SK_SUPPORT_LEGACY_DRAWFILTER
    SkDrawFilter* setDrawFilter(SkDrawFilter*) override;
#endif

protected:
    SkTDArray<SkCanvas*> fList;

    void willSave() override;
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) override;
    void willRestore() override;

    void didConcat(const SkMatrix&) override;
    void didSetMatrix(const SkMatrix&) override;

    // Override all onDraw virtuals to broadcast
#define X(func, ...) void func(__VA_ARGS__) override;
#include "SkCanvasDrawVirtuals.inc"

    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;

    void onClipRect(const SkRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipRRect(const SkRRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipPath(const SkPath&, SkClipOp, ClipEdgeStyle) override;
    void onClipRegion(const SkRegion&, SkClipOp) override;

    void onFlush() override;

    class Iter;

private:
    typedef SkNoDrawCanvas INHERITED;
};


#endif
