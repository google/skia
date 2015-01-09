/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLuaCanvas_DEFINED
#define SkLuaCanvas_DEFINED

#include "SkCanvas.h"
#include "SkString.h"

struct lua_State;

class SkLuaCanvas : public SkCanvas {
public:
    void pushThis();

    SkLuaCanvas(int width, int height, lua_State*, const char function[]);
    virtual ~SkLuaCanvas();

protected:
    void willSave() SK_OVERRIDE;
    SaveLayerStrategy willSaveLayer(const SkRect*, const SkPaint*, SaveFlags) SK_OVERRIDE;
    void willRestore() SK_OVERRIDE;

    void didConcat(const SkMatrix&) SK_OVERRIDE;
    void didSetMatrix(const SkMatrix&) SK_OVERRIDE;

    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) SK_OVERRIDE;
    virtual void onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                            const SkPaint&) SK_OVERRIDE;
    virtual void onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                               const SkPaint&) SK_OVERRIDE;
    virtual void onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                                SkScalar constY, const SkPaint&) SK_OVERRIDE;
    virtual void onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                  const SkMatrix* matrix, const SkPaint&) SK_OVERRIDE;
    virtual void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                const SkPaint& paint) SK_OVERRIDE;

    void onDrawPaint(const SkPaint&) SK_OVERRIDE;
    void onDrawPoints(PointMode, size_t count, const SkPoint pts[], const SkPaint&) SK_OVERRIDE;
    void onDrawRect(const SkRect&, const SkPaint&) SK_OVERRIDE;
    void onDrawOval(const SkRect&, const SkPaint&) SK_OVERRIDE;
    void onDrawRRect(const SkRRect&, const SkPaint&) SK_OVERRIDE;
    void onDrawPath(const SkPath&, const SkPaint&) SK_OVERRIDE;
    void onDrawBitmap(const SkBitmap&, SkScalar left, SkScalar top, const SkPaint*) SK_OVERRIDE;
    void onDrawBitmapRect(const SkBitmap&, const SkRect* src, const SkRect& dst, const SkPaint*,
                          DrawBitmapRectFlags flags) SK_OVERRIDE;
    void onDrawImage(const SkImage*, SkScalar left, SkScalar top, const SkPaint*) SK_OVERRIDE;
    void onDrawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                         const SkPaint*) SK_OVERRIDE;
    void onDrawBitmapNine(const SkBitmap&, const SkIRect& center, const SkRect& dst,
                          const SkPaint*) SK_OVERRIDE;
    void onDrawSprite(const SkBitmap&, int left, int top, const SkPaint*) SK_OVERRIDE;
    void onDrawVertices(VertexMode vmode, int vertexCount,
                        const SkPoint vertices[], const SkPoint texs[],
                        const SkColor colors[], SkXfermode* xmode,
                        const uint16_t indices[], int indexCount,
                        const SkPaint&) SK_OVERRIDE;

    void onClipRect(const SkRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    void onClipRRect(const SkRRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    void onClipPath(const SkPath&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    void onClipRegion(const SkRegion&, SkRegion::Op) SK_OVERRIDE;

    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) SK_OVERRIDE;

private:
    lua_State*  fL;
    SkString    fFunc;

    void sendverb(const char verb[]);

    typedef SkCanvas INHERITED;
};

#endif
