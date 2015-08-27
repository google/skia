/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecorder_DEFINED
#define SkRecorder_DEFINED

#include "SkBigPicture.h"
#include "SkCanvas.h"
#include "SkMiniRecorder.h"
#include "SkRecord.h"
#include "SkRecords.h"
#include "SkTDArray.h"

class SkBBHFactory;

class SkDrawableList : SkNoncopyable {
public:
    SkDrawableList() {}
    ~SkDrawableList();

    int count() const { return fArray.count(); }
    SkDrawable* const* begin() const { return fArray.begin(); }

    void append(SkDrawable* drawable);

    // Return a new or ref'd array of pictures that were snapped from our drawables.
    SkBigPicture::SnapshotArray* newDrawableSnapshot();

private:
    SkTDArray<SkDrawable*> fArray;
};

// SkRecorder provides an SkCanvas interface for recording into an SkRecord.

class SkRecorder : public SkCanvas {
public:
    // Does not take ownership of the SkRecord.
    SkRecorder(SkRecord*, int width, int height, SkMiniRecorder* = nullptr);   // legacy version
    SkRecorder(SkRecord*, const SkRect& bounds, SkMiniRecorder* = nullptr);

    enum DrawPictureMode { Record_DrawPictureMode, Playback_DrawPictureMode };
    void reset(SkRecord*, const SkRect& bounds, DrawPictureMode, SkMiniRecorder* = nullptr);

    size_t approxBytesUsedBySubPictures() const { return fApproxBytesUsedBySubPictures; }

    SkDrawableList* getDrawableList() const { return fDrawableList.get(); }
    SkDrawableList* detachDrawableList() { return fDrawableList.detach(); }

    // Make SkRecorder forget entirely about its SkRecord*; all calls to SkRecorder will fail.
    void forgetRecord();

    void willSave() override;
    SaveLayerStrategy willSaveLayer(const SkRect*, const SkPaint*, SkCanvas::SaveFlags) override;
    void willRestore() override {}
    void didRestore() override;

    void didConcat(const SkMatrix&) override;
    void didSetMatrix(const SkMatrix&) override;

    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;
    void onDrawDrawable(SkDrawable*, const SkMatrix*) override;
    void onDrawText(const void* text,
                    size_t byteLength,
                    SkScalar x,
                    SkScalar y,
                    const SkPaint& paint) override;
    void onDrawPosText(const void* text,
                       size_t byteLength,
                       const SkPoint pos[],
                       const SkPaint& paint) override;
    void onDrawPosTextH(const void* text,
                        size_t byteLength,
                        const SkScalar xpos[],
                        SkScalar constY,
                        const SkPaint& paint) override;
    void onDrawTextOnPath(const void* text,
                          size_t byteLength,
                          const SkPath& path,
                          const SkMatrix* matrix,
                          const SkPaint& paint) override;
    void onDrawTextBlob(const SkTextBlob* blob,
                        SkScalar x,
                        SkScalar y,
                        const SkPaint& paint) override;
    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                     const SkPoint texCoords[4], SkXfermode* xmode,
                     const SkPaint& paint) override;

    void onDrawPaint(const SkPaint&) override;
    void onDrawPoints(PointMode, size_t count, const SkPoint pts[], const SkPaint&) override;
    void onDrawRect(const SkRect&, const SkPaint&) override;
    void onDrawOval(const SkRect&, const SkPaint&) override;
    void onDrawRRect(const SkRRect&, const SkPaint&) override;
    void onDrawPath(const SkPath&, const SkPaint&) override;
    void onDrawBitmap(const SkBitmap&, SkScalar left, SkScalar top, const SkPaint*) override;
    void onDrawBitmapRect(const SkBitmap&, const SkRect* src, const SkRect& dst, const SkPaint*,
                          SrcRectConstraint) override;
    void onDrawImage(const SkImage*, SkScalar left, SkScalar top, const SkPaint*) override;
    void onDrawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                         const SkPaint*, SrcRectConstraint) override;
    void onDrawImageNine(const SkImage*, const SkIRect& center, const SkRect& dst,
                         const SkPaint*) override;
    void onDrawBitmapNine(const SkBitmap&, const SkIRect& center, const SkRect& dst,
                          const SkPaint*) override;
    void onDrawSprite(const SkBitmap&, int left, int top, const SkPaint*) override;
    void onDrawVertices(VertexMode vmode, int vertexCount,
                        const SkPoint vertices[], const SkPoint texs[],
                        const SkColor colors[], SkXfermode* xmode,
                        const uint16_t indices[], int indexCount,
                        const SkPaint&) override;
    void onDrawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[],
                     int count, SkXfermode::Mode, const SkRect* cull, const SkPaint*) override;

    void onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle) override;
    void onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) override;
    void onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) override;
    void onClipRegion(const SkRegion& deviceRgn, SkRegion::Op op) override;

    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;

    SkSurface* onNewSurface(const SkImageInfo&, const SkSurfaceProps&) override { return nullptr; }

    void flushMiniRecorder();

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

    DrawPictureMode fDrawPictureMode;
    size_t fApproxBytesUsedBySubPictures;
    SkRecord* fRecord;
    SkAutoTDelete<SkDrawableList> fDrawableList;

    SkMiniRecorder* fMiniRecorder;
};

#endif//SkRecorder_DEFINED
