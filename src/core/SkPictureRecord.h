/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureRecord_DEFINED
#define SkPictureRecord_DEFINED

#include "SkCanvas.h"
#include "SkFlattenable.h"
#include "SkPicture.h"
#include "SkPictureData.h"
#include "SkTemplates.h"
#include "SkWriter32.h"

// These macros help with packing and unpacking a single byte value and
// a 3 byte value into/out of a uint32_t
#define MASK_24 0x00FFFFFF
#define UNPACK_8_24(combined, small, large)             \
    small = (combined >> 24) & 0xFF;                    \
    large = combined & MASK_24;
#define PACK_8_24(small, large) ((small << 24) | large)


class SkPictureRecord : public SkCanvas {
public:
    SkPictureRecord(const SkISize& dimensions, uint32_t recordFlags);
    virtual ~SkPictureRecord();

    void beginCommentGroup(const char* description) SK_OVERRIDE;
    void addComment(const char* kywd, const char* value) SK_OVERRIDE;
    void endCommentGroup() SK_OVERRIDE;

    const SkTDArray<const SkPicture* >& getPictureRefs() const {
        return fPictureRefs;
    }

    const SkTDArray<const SkTextBlob* >& getTextBlobRefs() const {
        return fTextBlobRefs;
    }

    SkData* opData(bool deepCopy) const {
        this->validate(fWriter.bytesWritten(), 0);

        if (fWriter.bytesWritten() == 0) {
            return SkData::NewEmpty();
        }

        if (deepCopy) {
            return SkData::NewWithCopy(fWriter.contiguousArray(), fWriter.bytesWritten());
        }

        return fWriter.snapshotAsData();
    }

    const SkPictureContentInfo& contentInfo() const {
        return fContentInfo;
    }

    void setFlags(uint32_t recordFlags) {
        fRecordFlags = recordFlags;
    }

    const SkWriter32& writeStream() const {
        return fWriter;
    }

    void beginRecording();
    void endRecording();

protected:
    void addNoOp();

private:
    void handleOptimization(int opt);
    size_t recordRestoreOffsetPlaceholder(SkRegion::Op);
    void fillRestoreOffsetPlaceholdersForCurrentStackLevel(uint32_t restoreOffset);

    SkTDArray<int32_t> fRestoreOffsetStack;

    SkTDArray<uint32_t> fCullOffsetStack;

    /*
     * Write the 'drawType' operation and chunk size to the skp. 'size'
     * can potentially be increased if the chunk size needs its own storage
     * location (i.e., it overflows 24 bits).
     * Returns the start offset of the chunk. This is the location at which
     * the opcode & size are stored.
     * TODO: since we are handing the size into here we could call reserve
     * and then return a pointer to the memory storage. This could decrease
     * allocation overhead but could lead to more wasted space (the tail
     * end of blocks could go unused). Possibly add a second addDraw that
     * operates in this manner.
     */
    size_t addDraw(DrawType drawType, size_t* size) {
        size_t offset = fWriter.bytesWritten();

        this->predrawNotify();
        fContentInfo.addOperation();

        SkASSERT(0 != *size);
        SkASSERT(((uint8_t) drawType) == drawType);

        if (0 != (*size & ~MASK_24) || *size == MASK_24) {
            fWriter.writeInt(PACK_8_24(drawType, MASK_24));
            *size += 1;
            fWriter.writeInt(SkToU32(*size));
        } else {
            fWriter.writeInt(PACK_8_24(drawType, SkToU32(*size)));
        }

        return offset;
    }

    void addInt(int value) {
        fWriter.writeInt(value);
    }
    void addScalar(SkScalar scalar) {
        fWriter.writeScalar(scalar);
    }

    void addBitmap(const SkBitmap& bitmap);
    void addMatrix(const SkMatrix& matrix);
    void addPaint(const SkPaint& paint) { this->addPaintPtr(&paint); }
    void addPaintPtr(const SkPaint* paint);
    void addPatch(const SkPoint cubics[12]);
    void addPath(const SkPath& path);
    void addPicture(const SkPicture* picture);
    void addPoint(const SkPoint& point);
    void addPoints(const SkPoint pts[], int count);
    void addRect(const SkRect& rect);
    void addRectPtr(const SkRect* rect);
    void addIRect(const SkIRect& rect);
    void addIRectPtr(const SkIRect* rect);
    void addRRect(const SkRRect&);
    void addRegion(const SkRegion& region);
    void addText(const void* text, size_t byteLength);
    void addTextBlob(const SkTextBlob* blob);

    int find(const SkBitmap& bitmap);

protected:
    void validate(size_t initialOffset, size_t size) const {
        SkASSERT(fWriter.bytesWritten() == initialOffset + size);
    }

    SkSurface* onNewSurface(const SkImageInfo&, const SkSurfaceProps&) SK_OVERRIDE;
    const void* onPeekPixels(SkImageInfo*, size_t*) SK_OVERRIDE {
        return NULL;
    }

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
#if 0
    // rely on conversion to bitmap (for now)
    void onDrawImage(const SkImage*, SkScalar left, SkScalar top, const SkPaint*) SK_OVERRIDE;
    void onDrawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                         const SkPaint*) SK_OVERRIDE;
#endif
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

    int addPathToHeap(const SkPath& path);  // does not write to ops stream

    // These entry points allow the writing of matrices, clips, saves &
    // restores to be deferred (e.g., if the MC state is being collapsed and
    // only written out as needed).
    void recordConcat(const SkMatrix& matrix);
    void recordTranslate(const SkMatrix& matrix);
    void recordScale(const SkMatrix& matrix);
    size_t recordClipRect(const SkRect& rect, SkRegion::Op op, bool doAA);
    size_t recordClipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA);
    size_t recordClipPath(int pathID, SkRegion::Op op, bool doAA);
    size_t recordClipRegion(const SkRegion& region, SkRegion::Op op);
    void recordSave();
    void recordSaveLayer(const SkRect* bounds, const SkPaint* paint, SaveFlags flags);
    void recordRestore(bool fillInSkips = true);

private:
    SkPictureContentInfo fContentInfo;

    SkTArray<SkBitmap> fBitmaps;
    SkTArray<SkPaint>  fPaints;
    SkTArray<SkPath>   fPaths;

    SkWriter32 fWriter;

    // we ref each item in these arrays
    SkTDArray<const SkPicture*>  fPictureRefs;
    SkTDArray<const SkTextBlob*> fTextBlobRefs;

    uint32_t fRecordFlags;
    int      fInitialSaveCount;

    friend class SkPictureData;   // for SkPictureData's SkPictureRecord-based constructor

    typedef SkCanvas INHERITED;
};

#endif
