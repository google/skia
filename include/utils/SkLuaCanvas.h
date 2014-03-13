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

    virtual void drawPaint(const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPoints(PointMode mode, size_t count, const SkPoint pts[],
                            const SkPaint& paint) SK_OVERRIDE;
    virtual void drawOval(const SkRect&, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawRect(const SkRect&, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawRRect(const SkRRect&, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPath(const SkPath& path, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                            const SkPaint* paint) SK_OVERRIDE;
    virtual void drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                                      const SkRect& dst, const SkPaint* paint,
                                      DrawBitmapRectFlags flags) SK_OVERRIDE;
    virtual void drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& m,
                                  const SkPaint* paint) SK_OVERRIDE;
    virtual void drawSprite(const SkBitmap& bitmap, int left, int top,
                            const SkPaint* paint) SK_OVERRIDE;
    virtual void drawText(const void* text, size_t byteLength, SkScalar x,
                          SkScalar y, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPosText(const void* text, size_t byteLength,
                             const SkPoint pos[], const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPosTextH(const void* text, size_t byteLength,
                              const SkScalar xpos[], SkScalar constY,
                              const SkPaint& paint) SK_OVERRIDE;
    virtual void drawTextOnPath(const void* text, size_t byteLength,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPicture(SkPicture&) SK_OVERRIDE;
    virtual void drawVertices(VertexMode vmode, int vertexCount,
                              const SkPoint vertices[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint) SK_OVERRIDE;
    virtual void drawData(const void* data, size_t length) SK_OVERRIDE;

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

    virtual void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) SK_OVERRIDE;

    virtual void onClipRect(const SkRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipRRect(const SkRRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipPath(const SkPath&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipRegion(const SkRegion&, SkRegion::Op) SK_OVERRIDE;

private:
    lua_State*  fL;
    SkString    fFunc;

    void sendverb(const char verb[]);

    typedef SkCanvas INHERITED;
};

#endif
