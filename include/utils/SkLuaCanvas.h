/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLuaCanvas_DEFINED
#define SkLuaCanvas_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkString.h"

struct lua_State;

class SkLuaCanvas : public SkCanvas {
public:
    void pushThis();

    SkLuaCanvas(int width, int height, lua_State*, const char function[]);
    ~SkLuaCanvas() override;

protected:
    void willSave() override;
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) override;
    bool onDoSaveBehind(const SkRect*) override;
    void willRestore() override;

    void didConcat44(const SkM44&) override;
    void didConcat(const SkMatrix&) override;
    void didSetMatrix(const SkMatrix&) override;
    void didScale(SkScalar, SkScalar) override;
    void didTranslate(SkScalar, SkScalar) override;

    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;
    virtual void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                const SkPaint& paint) override;

    void onDrawPaint(const SkPaint&) override;
    void onDrawPoints(PointMode, size_t count, const SkPoint pts[], const SkPaint&) override;
    void onDrawRect(const SkRect&, const SkPaint&) override;
    void onDrawOval(const SkRect&, const SkPaint&) override;
    void onDrawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override;
    void onDrawRRect(const SkRRect&, const SkPaint&) override;
    void onDrawPath(const SkPath&, const SkPaint&) override;

    void onDrawImage(const SkImage*, SkScalar left, SkScalar top, const SkPaint*) override;
    void onDrawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                         const SkPaint*, SrcRectConstraint) override;
    void onDrawVerticesObject(const SkVertices*, SkBlendMode, const SkPaint&) override;
    void onClipRect(const SkRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipRRect(const SkRRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipPath(const SkPath&, SkClipOp, ClipEdgeStyle) override;
    void onClipShader(sk_sp<SkShader>, SkClipOp) override;
    void onClipRegion(const SkRegion&, SkClipOp) override;

    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;
    void onDrawDrawable(SkDrawable*, const SkMatrix*) override;

private:
    lua_State*  fL;
    SkString    fFunc;

    void sendverb(const char verb[]);

    typedef SkCanvas INHERITED;
};

#endif
