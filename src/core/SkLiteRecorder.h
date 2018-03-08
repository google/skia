/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLiteRecorder_DEFINED
#define SkLiteRecorder_DEFINED

#include "SkNoDrawCanvas.h"

class SkLiteDL;

class SkLiteRecorder final : public SkNoDrawCanvas {
public:
    SkLiteRecorder();
    void reset(SkLiteDL*, const SkIRect& bounds);

    sk_sp<SkSurface> onNewSurface(const SkImageInfo&, const SkSurfaceProps&) override;

#ifdef SK_SUPPORT_LEGACY_DRAWFILTER
    SkDrawFilter* setDrawFilter(SkDrawFilter*) override;
#endif

    void willSave() override;
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) override;
    void willRestore() override;

    void onFlush() override;

    void didConcat(const SkMatrix&) override;
    void didSetMatrix(const SkMatrix&) override;
    void didTranslate(SkScalar, SkScalar) override;

    void onClipRect  (const   SkRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipRRect (const  SkRRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipPath  (const   SkPath&, SkClipOp, ClipEdgeStyle) override;
    void onClipRegion(const SkRegion&, SkClipOp) override;

#define X(func, ...) void func(__VA_ARGS__) override;
#include "SkCanvasDrawVirtuals.inc"

    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;

private:
    typedef SkNoDrawCanvas INHERITED;

    SkLiteDL* fDL;
};

#endif//SkLiteRecorder_DEFINED
