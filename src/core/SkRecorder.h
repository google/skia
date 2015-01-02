/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecorder_DEFINED
#define SkRecorder_DEFINED

#include "SkCanvas.h"
#include "SkRecord.h"
#include "SkRecords.h"
#include "SkTDArray.h"

class SkBBHFactory;

class SkCanvasDrawableList : SkNoncopyable {
public:
    ~SkCanvasDrawableList();

    int count() const { return fArray.count(); }
    SkCanvasDrawable* const* begin() const { return fArray.begin(); }

    void append(SkCanvasDrawable* drawable);

    // Return a new or ref'd array of pictures that were snapped from our drawables.
    SkPicture::SnapshotArray* newDrawableSnapshot();

private:
    SkTDArray<SkCanvasDrawable*> fArray;
};

// SkRecorder provides an SkCanvas interface for recording into an SkRecord.

class SkRecorder : public SkCanvas {
public:
    // Does not take ownership of the SkRecord.
    SkRecorder(SkRecord*, int width, int height);   // legacy version
    SkRecorder(SkRecord*, const SkRect& bounds);

    SkCanvasDrawableList* getDrawableList() const { return fDrawableList.get(); }
    SkCanvasDrawableList* detachDrawableList() { return fDrawableList.detach(); }

    // Make SkRecorder forget entirely about its SkRecord*; all calls to SkRecorder will fail.
    void forgetRecord();

    void drawPaint(const SkPaint& paint) SK_OVERRIDE;
    void drawPoints(PointMode mode,
                    size_t count,
                    const SkPoint pts[],
                    const SkPaint& paint) SK_OVERRIDE;
    void drawRect(const SkRect& rect, const SkPaint& paint) SK_OVERRIDE;
    void drawOval(const SkRect& oval, const SkPaint&) SK_OVERRIDE;
    void drawRRect(const SkRRect& rrect, const SkPaint& paint) SK_OVERRIDE;
    void drawPath(const SkPath& path, const SkPaint& paint) SK_OVERRIDE;
    void drawBitmap(const SkBitmap& bitmap,
                    SkScalar left,
                    SkScalar top,
                    const SkPaint* paint = NULL) SK_OVERRIDE;
    void drawBitmapRectToRect(const SkBitmap& bitmap,
                              const SkRect* src,
                              const SkRect& dst,
                              const SkPaint* paint = NULL,
                              DrawBitmapRectFlags flags = kNone_DrawBitmapRectFlag) SK_OVERRIDE;
    void drawBitmapNine(const SkBitmap& bitmap,
                        const SkIRect& center,
                        const SkRect& dst,
                        const SkPaint* paint = NULL) SK_OVERRIDE;
    virtual void drawImage(const SkImage* image, SkScalar left, SkScalar top,
                           const SkPaint* paint = NULL) SK_OVERRIDE;
    virtual void drawImageRect(const SkImage* image, const SkRect* src,
                               const SkRect& dst,
                               const SkPaint* paint = NULL) SK_OVERRIDE;
    void drawSprite(const SkBitmap& bitmap,
                    int left,
                    int top,
                    const SkPaint* paint = NULL) SK_OVERRIDE;
    void drawVertices(VertexMode vmode,
                      int vertexCount,
                      const SkPoint vertices[],
                      const SkPoint texs[],
                      const SkColor colors[],
                      SkXfermode* xmode,
                      const uint16_t indices[],
                      int indexCount,
                      const SkPaint& paint) SK_OVERRIDE;

    void willSave() SK_OVERRIDE;
    SaveLayerStrategy willSaveLayer(const SkRect*, const SkPaint*, SkCanvas::SaveFlags) SK_OVERRIDE;
    void willRestore() SK_OVERRIDE {}
    void didRestore() SK_OVERRIDE;

    void didConcat(const SkMatrix&) SK_OVERRIDE;
    void didSetMatrix(const SkMatrix&) SK_OVERRIDE;

    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) SK_OVERRIDE;
    void onDrawDrawable(SkCanvasDrawable*) SK_OVERRIDE;
    void onDrawText(const void* text,
                    size_t byteLength,
                    SkScalar x,
                    SkScalar y,
                    const SkPaint& paint) SK_OVERRIDE;
    void onDrawPosText(const void* text,
                       size_t byteLength,
                       const SkPoint pos[],
                       const SkPaint& paint) SK_OVERRIDE;
    void onDrawPosTextH(const void* text,
                        size_t byteLength,
                        const SkScalar xpos[],
                        SkScalar constY,
                        const SkPaint& paint) SK_OVERRIDE;
    void onDrawTextOnPath(const void* text,
                          size_t byteLength,
                          const SkPath& path,
                          const SkMatrix* matrix,
                          const SkPaint& paint) SK_OVERRIDE;
    void onDrawTextBlob(const SkTextBlob* blob,
                        SkScalar x,
                        SkScalar y,
                        const SkPaint& paint) SK_OVERRIDE;
    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                     const SkPoint texCoords[4], SkXfermode* xmode,
                     const SkPaint& paint) SK_OVERRIDE;

    void onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle) SK_OVERRIDE;
    void onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) SK_OVERRIDE;
    void onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) SK_OVERRIDE;
    void onClipRegion(const SkRegion& deviceRgn, SkRegion::Op op) SK_OVERRIDE;

    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) SK_OVERRIDE;

    void beginCommentGroup(const char*) SK_OVERRIDE;
    void addComment(const char*, const char*) SK_OVERRIDE;
    void endCommentGroup() SK_OVERRIDE;

    SkSurface* onNewSurface(const SkImageInfo&, const SkSurfaceProps&) SK_OVERRIDE { return NULL; }

private:
    template <typename T>
    T* copy(const T*);

    template <typename T>
    T* copy(const T[], size_t count);

    SkIRect devBounds() const {
        SkIRect devBounds;
        this->getClipDeviceBounds(&devBounds);
        return devBounds;
    }

    SkRecord* fRecord;

    SkAutoTDelete<SkCanvasDrawableList> fDrawableList;
};

#endif//SkRecorder_DEFINED
