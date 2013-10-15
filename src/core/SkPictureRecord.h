
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
#include "SkPathHeap.h"
#include "SkPicture.h"
#include "SkPictureFlat.h"
#include "SkTemplates.h"
#include "SkWriter32.h"

class SkPictureStateTree;
class SkBBoxHierarchy;

// These macros help with packing and unpacking a single byte value and
// a 3 byte value into/out of a uint32_t
#define MASK_24 0x00FFFFFF
#define UNPACK_8_24(combined, small, large)             \
    small = (combined >> 24) & 0xFF;                    \
    large = combined & MASK_24;
#define PACK_8_24(small, large) ((small << 24) | large)


class SkPictureRecord : public SkCanvas {
public:
    SkPictureRecord(uint32_t recordFlags, SkBaseDevice*);
    virtual ~SkPictureRecord();

    virtual SkBaseDevice* setDevice(SkBaseDevice* device) SK_OVERRIDE;

    virtual int save(SaveFlags) SK_OVERRIDE;
    virtual int saveLayer(const SkRect* bounds, const SkPaint*, SaveFlags) SK_OVERRIDE;
    virtual void restore() SK_OVERRIDE;
    virtual bool translate(SkScalar dx, SkScalar dy) SK_OVERRIDE;
    virtual bool scale(SkScalar sx, SkScalar sy) SK_OVERRIDE;
    virtual bool rotate(SkScalar degrees) SK_OVERRIDE;
    virtual bool skew(SkScalar sx, SkScalar sy) SK_OVERRIDE;
    virtual bool concat(const SkMatrix& matrix) SK_OVERRIDE;
    virtual void setMatrix(const SkMatrix& matrix) SK_OVERRIDE;
    virtual bool clipRect(const SkRect&, SkRegion::Op, bool) SK_OVERRIDE;
    virtual bool clipRRect(const SkRRect&, SkRegion::Op, bool) SK_OVERRIDE;
    virtual bool clipPath(const SkPath&, SkRegion::Op, bool) SK_OVERRIDE;
    virtual bool clipRegion(const SkRegion& region, SkRegion::Op op) SK_OVERRIDE;
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
    virtual void drawText(const void* text, size_t byteLength, SkScalar x,
                          SkScalar y, const SkPaint&) SK_OVERRIDE;
    virtual void drawPosText(const void* text, size_t byteLength,
                             const SkPoint pos[], const SkPaint&) SK_OVERRIDE;
    virtual void drawPosTextH(const void* text, size_t byteLength,
                      const SkScalar xpos[], SkScalar constY, const SkPaint&) SK_OVERRIDE;
    virtual void drawTextOnPath(const void* text, size_t byteLength,
                            const SkPath& path, const SkMatrix* matrix,
                                const SkPaint&) SK_OVERRIDE;
    virtual void drawPicture(SkPicture& picture) SK_OVERRIDE;
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

    const SkTDArray<SkPicture* >& getPictureRefs() const {
        return fPictureRefs;
    }

    void setFlags(uint32_t recordFlags) {
        fRecordFlags = recordFlags;
    }

    const SkWriter32& writeStream() const {
        return fWriter;
    }

    void beginRecording();
    void endRecording();

private:
    void handleOptimization(int opt);
    void recordRestoreOffsetPlaceholder(SkRegion::Op);
    void fillRestoreOffsetPlaceholdersForCurrentStackLevel(
        uint32_t restoreOffset);

    SkTDArray<int32_t> fRestoreOffsetStack;
    int fFirstSavedLayerIndex;
    enum {
        kNoSavedLayerIndex = -1
    };

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
    size_t addDraw(DrawType drawType, uint32_t* size) {
        size_t offset = fWriter.size();

        this->predrawNotify();

    #ifdef SK_DEBUG_TRACE
        SkDebugf("add %s\n", DrawTypeToString(drawType));
    #endif

        SkASSERT(0 != *size);
        SkASSERT(((uint8_t) drawType) == drawType);

        if (0 != (*size & ~MASK_24) || *size == MASK_24) {
            fWriter.writeInt(PACK_8_24(drawType, MASK_24));
            *size += 1;
            fWriter.writeInt(*size);
        } else {
            fWriter.writeInt(PACK_8_24(drawType, *size));
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
    void addMatrixPtr(const SkMatrix* matrix);
    const SkFlatData* addPaint(const SkPaint& paint) { return this->addPaintPtr(&paint); }
    const SkFlatData* addPaintPtr(const SkPaint* paint);
    void addFlatPaint(const SkFlatData* flatPaint);
    void addPath(const SkPath& path);
    void addPicture(SkPicture& picture);
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
    void validate(size_t initialOffset, uint32_t size) const {
        SkASSERT(fWriter.size() == initialOffset + size);
    }
#endif

protected:
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

    // These are set to NULL in our constructor, but may be changed by
    // subclasses, in which case they will be SkSafeUnref'd in our destructor.
    SkBBoxHierarchy* fBoundingHierarchy;
    SkPictureStateTree* fStateTree;

    // Allocated in the constructor and managed by this class.
    SkBitmapHeap* fBitmapHeap;

private:
    SkChunkFlatController fFlattenableHeap;

    SkMatrixDictionary fMatrices;
    SkPaintDictionary fPaints;
    SkRegionDictionary fRegions;

    SkPathHeap* fPathHeap;  // reference counted
    SkWriter32 fWriter;

    // we ref each item in these arrays
    SkTDArray<SkPicture*> fPictureRefs;

    uint32_t fRecordFlags;
    int fInitialSaveCount;

    friend class SkPicturePlayback;
    friend class SkPictureTester; // for unit testing

    typedef SkCanvas INHERITED;
};

#endif
