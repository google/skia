
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNWayCanvas_DEFINED
#define SkNWayCanvas_DEFINED

#include "SkCanvas.h"

class SK_API SkNWayCanvas : public SkCanvas {
public:
    SkNWayCanvas(int width, int height);
    virtual ~SkNWayCanvas();

    virtual void addCanvas(SkCanvas*);
    virtual void removeCanvas(SkCanvas*);
    virtual void removeAll();

    ///////////////////////////////////////////////////////////////////////////
    // These are forwarded to the N canvases we're referencing

    SkDrawFilter* setDrawFilter(SkDrawFilter*) SK_OVERRIDE;

    void beginCommentGroup(const char* description) SK_OVERRIDE;
    void addComment(const char* kywd, const char* value) SK_OVERRIDE;
    void endCommentGroup() SK_OVERRIDE;

protected:
    SkTDArray<SkCanvas*> fList;

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
    virtual void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                             const SkPoint texCoords[4], SkXfermode* xmode,
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

    class Iter;

private:
    typedef SkCanvas INHERITED;
};


#endif
