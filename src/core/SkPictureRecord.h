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
#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
#include "SkMatrixClipStateMgr.h"
#endif
#include "SkPathHeap.h"
#include "SkPicture.h"
#include "SkPictureData.h"
#include "SkPictureFlat.h"
#include "SkTemplates.h"
#include "SkWriter32.h"

class SkBBoxHierarchy;
class SkPictureStateTree;

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

    virtual void clear(SkColor) SK_OVERRIDE;
    virtual void drawPaint(const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPoints(PointMode, size_t count, const SkPoint pts[],
                            const SkPaint&) SK_OVERRIDE;
    virtual void drawOval(const SkRect&, const SkPaint&) SK_OVERRIDE;
    virtual void drawRect(const SkRect&, const SkPaint&) SK_OVERRIDE;
    virtual void drawRRect(const SkRRect&, const SkPaint&) SK_OVERRIDE;
    virtual void drawPath(const SkPath& path, const SkPaint&) SK_OVERRIDE;
    virtual void drawBitmap(const SkBitmap&, SkScalar left, SkScalar top,
                            const SkPaint*) SK_OVERRIDE;
    virtual void drawBitmapRectToRect(const SkBitmap&, const SkRect* src,
                                      const SkRect& dst, const SkPaint* paint,
                                      DrawBitmapRectFlags flags) SK_OVERRIDE;
    virtual void drawBitmapMatrix(const SkBitmap&, const SkMatrix&,
                                  const SkPaint*) SK_OVERRIDE;
    virtual void drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                const SkRect& dst, const SkPaint*) SK_OVERRIDE;
    virtual void drawSprite(const SkBitmap&, int left, int top,
                            const SkPaint*) SK_OVERRIDE;
    virtual void drawVertices(VertexMode, int vertexCount,
                          const SkPoint vertices[], const SkPoint texs[],
                          const SkColor colors[], SkXfermode*,
                          const uint16_t indices[], int indexCount,
                              const SkPaint&) SK_OVERRIDE;
    virtual void drawData(const void*, size_t) SK_OVERRIDE;
    virtual void beginCommentGroup(const char* description) SK_OVERRIDE;
    virtual void addComment(const char* kywd, const char* value) SK_OVERRIDE;
    virtual void endCommentGroup() SK_OVERRIDE;
    virtual bool isDrawingToLayer() const SK_OVERRIDE;

    void addFontMetricsTopBottom(const SkPaint& paint, const SkFlatData&,
                                 SkScalar minY, SkScalar maxY);

    const SkTDArray<const SkPicture* >& getPictureRefs() const {
        return fPictureRefs;
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

    const SkPathHeap* pathHeap() const {
        return fPathHeap.get();
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

    void internalOnly_EnableOpts(bool optsEnabled) {
        fOptsEnabled = optsEnabled;
    }

protected:
    void addNoOp();

private:
    void handleOptimization(int opt);
    size_t recordRestoreOffsetPlaceholder(SkRegion::Op);
    void fillRestoreOffsetPlaceholdersForCurrentStackLevel(uint32_t restoreOffset);

#ifndef SK_COLLAPSE_MATRIX_CLIP_STATE
    SkTDArray<int32_t> fRestoreOffsetStack;
    int fFirstSavedLayerIndex;
    enum {
        kNoSavedLayerIndex = -1
    };
#endif

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

    #ifdef SK_DEBUG_TRACE
        SkDebugf("add %s\n", DrawTypeToString(drawType));
    #endif

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

    // The command at 'offset' in the skp uses the specified bitmap
    int addBitmap(const SkBitmap& bitmap);
    void addMatrix(const SkMatrix& matrix);
    const SkFlatData* addPaint(const SkPaint& paint) { return this->addPaintPtr(&paint); }
    const SkFlatData* addPaintPtr(const SkPaint* paint);
    void addFlatPaint(const SkFlatData* flatPaint);
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

    int find(const SkBitmap& bitmap);

#ifdef SK_DEBUG_DUMP
public:
    void dumpMatrices();
    void dumpPaints();
#endif

#ifdef SK_DEBUG_SIZE
public:
    size_t size() const;
    int bitmaps(size_t* size) const;
    int matrices(size_t* size) const;
    int paints(size_t* size) const;
    int paths(size_t* size) const;
    int regions(size_t* size) const;
    size_t streamlen() const;

    size_t fPointBytes, fRectBytes, fTextBytes;
    int fPointWrites, fRectWrites, fTextWrites;
#endif

#ifdef SK_DEBUG_VALIDATE
public:
    void validate(size_t initialOffset, uint32_t size) const;
private:
    void validateBitmaps() const;
    void validateMatrices() const;
    void validatePaints() const;
    void validatePaths() const;
    void validateRegions() const;
#else
public:
    void validate(size_t initialOffset, size_t size) const {
        SkASSERT(fWriter.bytesWritten() == initialOffset + size);
    }
#endif

protected:
    virtual SkSurface* onNewSurface(const SkImageInfo&) SK_OVERRIDE;
    const void* onPeekPixels(SkImageInfo*, size_t*) SK_OVERRIDE {
        return NULL;
    }

    virtual void willSave() SK_OVERRIDE;
    virtual SaveLayerStrategy willSaveLayer(const SkRect*, const SkPaint*, SaveFlags) SK_OVERRIDE;
    virtual void willRestore() SK_OVERRIDE;

    virtual void didConcat(const SkMatrix&) SK_OVERRIDE;
    virtual void didSetMatrix(const SkMatrix&) SK_OVERRIDE;

    virtual void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) SK_OVERRIDE;
    virtual void onPushCull(const SkRect&) SK_OVERRIDE;
    virtual void onPopCull() SK_OVERRIDE;

    virtual void onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                            const SkPaint&) SK_OVERRIDE;
    virtual void onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                               const SkPaint&) SK_OVERRIDE;
    virtual void onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                                SkScalar constY, const SkPaint&) SK_OVERRIDE;
    virtual void onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                  const SkMatrix* matrix, const SkPaint&) SK_OVERRIDE;

    virtual void onClipRect(const SkRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipRRect(const SkRRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipPath(const SkPath&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipRegion(const SkRegion&, SkRegion::Op) SK_OVERRIDE;

    virtual void onDrawPicture(const SkPicture* picture) SK_OVERRIDE;

    // Return fontmetrics.fTop,fBottom in topbot[0,1], after they have been
    // tweaked by paint.computeFastBounds().
    static void ComputeFontMetricsTopBottom(const SkPaint& paint, SkScalar topbot[2]);

    // Make sure that flat has fTopBot written.
    static void WriteTopBot(const SkPaint& paint, const SkFlatData& flat) {
        if (!flat.isTopBotWritten()) {
            ComputeFontMetricsTopBottom(paint, flat.writableTopBot());
            SkASSERT(flat.isTopBotWritten());
        }
    }
    // Will return a cached version when possible.
    const SkFlatData* getFlatPaintData(const SkPaint& paint);
    /**
     * SkBBoxRecord::drawPosTextH gets a flat paint and uses it,
     * then it calls this, using the extra parameter, to avoid duplication.
     */
    void drawPosTextHImpl(const void* text, size_t byteLength,
                          const SkScalar xpos[], SkScalar constY,
                          const SkPaint& paint, const SkFlatData* flatPaintData);

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

    // These are set to NULL in our constructor, but may be changed by
    // subclasses, in which case they will be SkSafeUnref'd in our destructor.
    SkBBoxHierarchy* fBoundingHierarchy;
    SkPictureStateTree* fStateTree;

    // Allocated in the constructor and managed by this class.
    SkBitmapHeap* fBitmapHeap;

private:
    friend class MatrixClipState; // for access to *Impl methods
    friend class SkMatrixClipStateMgr; // for access to *Impl methods

    SkPictureContentInfo fContentInfo;
    SkAutoTUnref<SkPathHeap> fPathHeap;

    SkChunkFlatController fFlattenableHeap;

    SkPaintDictionary fPaints;

    SkWriter32 fWriter;

    // we ref each item in these arrays
    SkTDArray<const SkPicture*> fPictureRefs;

    uint32_t fRecordFlags;
    bool     fOptsEnabled;
    int      fInitialSaveCount;

    friend class SkPictureData;   // for SkPictureData's SkPictureRecord-based constructor
    friend class SkPictureTester; // for unit testing

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
    SkMatrixClipStateMgr fMCMgr;
#endif

    typedef SkCanvas INHERITED;
};

#endif
