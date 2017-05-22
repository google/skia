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
#include "SkTArray.h"
#include "SkTDArray.h"
#include "SkTHash.h"
#include "SkVertices.h"
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
    ~SkPictureRecord() override;

    const SkTDArray<const SkPicture* >& getPictureRefs() const {
        return fPictureRefs;
    }

    const SkTDArray<SkDrawable* >& getDrawableRefs() const {
        return fDrawableRefs;
    }

    const SkTDArray<const SkTextBlob* >& getTextBlobRefs() const {
        return fTextBlobRefs;
    }

    const SkTDArray<const SkVertices* >& getVerticesRefs() const {
        return fVerticesRefs;
    }

   const SkTDArray<const SkImage* >& getImageRefs() const {
        return fImageRefs;
    }

    sk_sp<SkData> opData() const {
        this->validate(fWriter.bytesWritten(), 0);

        if (fWriter.bytesWritten() == 0) {
            return SkData::MakeEmpty();
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
    size_t recordRestoreOffsetPlaceholder(SkClipOp);
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

    void addImage(const SkImage*);
    void addMatrix(const SkMatrix& matrix);
    void addPaint(const SkPaint& paint) { this->addPaintPtr(&paint); }
    void addPaintPtr(const SkPaint* paint);
    void addPatch(const SkPoint cubics[12]);
    void addPath(const SkPath& path);
    void addPicture(const SkPicture* picture);
    void addDrawable(SkDrawable* picture);
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
    void addVertices(const SkVertices*);

    int find(const SkBitmap& bitmap);

protected:
    void validate(size_t initialOffset, size_t size) const {
        SkASSERT(fWriter.bytesWritten() == initialOffset + size);
    }

    sk_sp<SkSurface> onNewSurface(const SkImageInfo&, const SkSurfaceProps&) override;
    bool onPeekPixels(SkPixmap*) override { return false; }

    void willSave() override;
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) override;
    void willRestore() override;

    void didConcat(const SkMatrix&) override;
    void didSetMatrix(const SkMatrix&) override;

    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;

    void onDrawText(const void* text, size_t, SkScalar x, SkScalar y, const SkPaint&) override;
    void onDrawPosText(const void* text, size_t, const SkPoint pos[], const SkPaint&) override;
    void onDrawPosTextH(const void* text, size_t, const SkScalar xpos[], SkScalar constY,
                        const SkPaint&) override;
    void onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                  const SkMatrix* matrix, const SkPaint&) override;
    void onDrawTextRSXform(const void* text, size_t byteLength, const SkRSXform xform[],
                           const SkRect* cull, const SkPaint&) override;
    void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                const SkPaint& paint) override;

    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                     const SkPoint texCoords[4], SkBlendMode, const SkPaint& paint) override;
    void onDrawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[], int,
                     SkBlendMode, const SkRect*, const SkPaint*) override;

    void onDrawPaint(const SkPaint&) override;
    void onDrawPoints(PointMode, size_t count, const SkPoint pts[], const SkPaint&) override;
    void onDrawRect(const SkRect&, const SkPaint&) override;
    void onDrawRegion(const SkRegion&, const SkPaint&) override;
    void onDrawOval(const SkRect&, const SkPaint&) override;
    void onDrawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override;
    void onDrawRRect(const SkRRect&, const SkPaint&) override;
    void onDrawPath(const SkPath&, const SkPaint&) override;
    void onDrawImage(const SkImage*, SkScalar left, SkScalar top, const SkPaint*) override;
    void onDrawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                         const SkPaint*, SrcRectConstraint) override;
    void onDrawImageNine(const SkImage*, const SkIRect& center, const SkRect& dst,
                         const SkPaint*) override;
    void onDrawImageLattice(const SkImage*, const SkCanvas::Lattice& lattice, const SkRect& dst,
                            const SkPaint*) override;
    void onDrawShadowRec(const SkPath&, const SkDrawShadowRec&) override;
    void onDrawVerticesObject(const SkVertices*, SkBlendMode, const SkPaint&) override;

    void onClipRect(const SkRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipRRect(const SkRRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipPath(const SkPath&, SkClipOp, ClipEdgeStyle) override;
    void onClipRegion(const SkRegion&, SkClipOp) override;

    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;

    void onDrawDrawable(SkDrawable*, const SkMatrix*) override;
    void onDrawAnnotation(const SkRect&, const char[], SkData*) override;

    int addPathToHeap(const SkPath& path);  // does not write to ops stream

    // These entry points allow the writing of matrices, clips, saves &
    // restores to be deferred (e.g., if the MC state is being collapsed and
    // only written out as needed).
    void recordConcat(const SkMatrix& matrix);
    void recordTranslate(const SkMatrix& matrix);
    void recordScale(const SkMatrix& matrix);
    size_t recordClipRect(const SkRect& rect, SkClipOp op, bool doAA);
    size_t recordClipRRect(const SkRRect& rrect, SkClipOp op, bool doAA);
    size_t recordClipPath(int pathID, SkClipOp op, bool doAA);
    size_t recordClipRegion(const SkRegion& region, SkClipOp op);
    void recordSave();
    void recordSaveLayer(const SaveLayerRec&);
    void recordRestore(bool fillInSkips = true);

    // SHOULD NEVER BE CALLED
    void onDrawBitmap(const SkBitmap&, SkScalar left, SkScalar top, const SkPaint*) override {
        sk_throw();
    }
    void onDrawBitmapRect(const SkBitmap&, const SkRect* src, const SkRect& dst, const SkPaint*,
                          SrcRectConstraint) override {
        sk_throw();
    }
    void onDrawBitmapNine(const SkBitmap&, const SkIRect& center, const SkRect& dst,
                          const SkPaint*) override {
        sk_throw();
    }
    void onDrawBitmapLattice(const SkBitmap&, const SkCanvas::Lattice& lattice, const SkRect& dst,
                             const SkPaint*) override {
        sk_throw();
    }

private:
    SkPictureContentInfo fContentInfo;

    SkTArray<SkPaint>  fPaints;

    struct PathHash {
        uint32_t operator()(const SkPath& p) { return p.getGenerationID(); }
    };
    SkTHashMap<SkPath, int, PathHash> fPaths;

    SkWriter32 fWriter;

    // we ref each item in these arrays
    SkTDArray<const SkImage*>    fImageRefs;
    SkTDArray<const SkPicture*>  fPictureRefs;
    SkTDArray<SkDrawable*>       fDrawableRefs;
    SkTDArray<const SkTextBlob*> fTextBlobRefs;
    SkTDArray<const SkVertices*> fVerticesRefs;

    uint32_t fRecordFlags;
    int      fInitialSaveCount;

    friend class SkPictureData;   // for SkPictureData's SkPictureRecord-based constructor

    typedef SkCanvas INHERITED;
};

#endif
