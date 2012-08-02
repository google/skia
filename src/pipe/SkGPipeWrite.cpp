
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "SkCanvas.h"
#include "SkData.h"
#include "SkDevice.h"
#include "SkPaint.h"
#include "SkPathEffect.h"
#include "SkGPipe.h"
#include "SkGPipePriv.h"
#include "SkImageFilter.h"
#include "SkStream.h"
#include "SkTSearch.h"
#include "SkTypeface.h"
#include "SkWriter32.h"
#include "SkColorFilter.h"
#include "SkDrawLooper.h"
#include "SkMaskFilter.h"
#include "SkRasterizer.h"
#include "SkShader.h"
#include "SkOrderedWriteBuffer.h"
#include "SkPictureFlat.h"

static bool isCrossProcess(uint32_t flags) {
    return SkToBool(flags & SkGPipeWriter::kCrossProcess_Flag);
}

static SkFlattenable* get_paintflat(const SkPaint& paint, unsigned paintFlat) {
    SkASSERT(paintFlat < kCount_PaintFlats);
    switch (paintFlat) {
        case kColorFilter_PaintFlat:    return paint.getColorFilter();
        case kDrawLooper_PaintFlat:     return paint.getLooper();
        case kMaskFilter_PaintFlat:     return paint.getMaskFilter();
        case kPathEffect_PaintFlat:     return paint.getPathEffect();
        case kRasterizer_PaintFlat:     return paint.getRasterizer();
        case kShader_PaintFlat:         return paint.getShader();
        case kImageFilter_PaintFlat:    return paint.getImageFilter();
        case kXfermode_PaintFlat:       return paint.getXfermode();
    }
    SkDEBUGFAIL("never gets here");
    return NULL;
}

static size_t writeTypeface(SkWriter32* writer, SkTypeface* typeface) {
    SkASSERT(typeface);
    SkDynamicMemoryWStream stream;
    typeface->serialize(&stream);
    size_t size = stream.getOffset();
    if (writer) {
        writer->write32(size);
        SkAutoDataUnref data(stream.copyToData());
        writer->writePad(data->data(), size);
    }
    return 4 + SkAlign4(size);
}

///////////////////////////////////////////////////////////////////////////////

class FlattenableHeap : public SkFlatController {
public:
    FlattenableHeap(int numFlatsToKeep, SkNamedFactorySet* fset)
    : fNumFlatsToKeep(numFlatsToKeep) {
        this->setNamedFactorySet(fset);
    }

    ~FlattenableHeap() {
        fPointers.freeAll();
    }

    virtual void* allocThrow(size_t bytes) SK_OVERRIDE;

    virtual void unalloc(void* ptr) SK_OVERRIDE;

    const SkFlatData* flatToReplace() const;

    // Mark an SkFlatData as one that should not be returned by flatToReplace.
    // Takes the result of SkFlatData::index() as its parameter.
    void markFlatForKeeping(int index) {
        *fFlatsThatMustBeKept.append() = index;
    }

    void markAllFlatsSafeToDelete() {
        fFlatsThatMustBeKept.reset();
    }

private:
    // Keep track of the indices (i.e. the result of SkFlatData::index()) of
    // flats that must be kept, since they are on the current paint.
    SkTDArray<int>   fFlatsThatMustBeKept;
    SkTDArray<void*> fPointers;
    const int        fNumFlatsToKeep;
};

void FlattenableHeap::unalloc(void* ptr) {
    int indexToRemove = fPointers.rfind(ptr);
    if (indexToRemove >= 0) {
        sk_free(ptr);
        fPointers.remove(indexToRemove);
    }
}

void* FlattenableHeap::allocThrow(size_t bytes) {
    void* ptr = sk_malloc_throw(bytes);
    *fPointers.append() = ptr;
    return ptr;
}

const SkFlatData* FlattenableHeap::flatToReplace() const {
    // First, determine whether we should replace one.
    if (fPointers.count() > fNumFlatsToKeep) {
        // Look through the flattenable heap.
        // TODO: Return the LRU flat.
        for (int i = 0; i < fPointers.count(); i++) {
            SkFlatData* potential = (SkFlatData*)fPointers[i];
            // Make sure that it is not one that must be kept.
            bool mustKeep = false;
            for (int j = 0; j < fFlatsThatMustBeKept.count(); j++) {
                if (potential->index() == fFlatsThatMustBeKept[j]) {
                    mustKeep = true;
                    break;
                }
            }
            if (!mustKeep) {
                return potential;
            }
        }
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

class FlatDictionary : public SkFlatDictionary<SkFlattenable> {
public:
    FlatDictionary(FlattenableHeap* heap)
            : SkFlatDictionary<SkFlattenable>(heap) {
        fFlattenProc = &flattenFlattenableProc;
        // No need to define fUnflattenProc since the writer will never
        // unflatten the data.
    }
    static void flattenFlattenableProc(SkOrderedWriteBuffer& buffer,
                                       const void* obj) {
        buffer.writeFlattenable((SkFlattenable*)obj);
    }

};

///////////////////////////////////////////////////////////////////////////////

/*
 * Shared heap for storing large things that can be shared, for a stream
 * used by multiple readers.
 * TODO: Make the allocations all come from cross process safe address space
 * TODO: Store paths (others?)
 * TODO: Generalize the LRU caching mechanism
 */
class SharedHeap {
public:
    SharedHeap(bool shallow, int numOfReaders)
        : fBitmapCount(0)
        , fMostRecentlyUsed(NULL)
        , fLeastRecentlyUsed(NULL)
        , fCanDoShallowCopies(shallow)
        , fNumberOfReaders(numOfReaders)
        , fBytesAllocated(0) {}
    ~SharedHeap() {
        BitmapInfo* iter = fMostRecentlyUsed;
        while (iter != NULL) {
            SkDEBUGCODE(fBytesAllocated -= (iter->fBytesAllocated + sizeof(BitmapInfo)));
            BitmapInfo* next = iter->fLessRecentlyUsed;
            SkDELETE(iter);
            fBitmapCount--;
            iter = next;
        }
        SkASSERT(0 == fBitmapCount);
        SkASSERT(0 == fBytesAllocated);
    }

    /*
     * Get the approximate number of bytes allocated.
     *
     * Not exact. Some SkBitmaps may share SkPixelRefs, in which case only one
     * SkBitmap will take the size of the SkPixelRef into account (the first
     * one). It is possible that the one which accounts for the SkPixelRef has
     * been removed, in which case we will no longer be counting those bytes.
     */
    size_t bytesAllocated() { return fBytesAllocated; }

    /*
     * Add a copy of a bitmap to the heap.
     * @param bm The SkBitmap to be copied and placed in the heap.
     * @return void* Pointer to the BitmapInfo stored in the heap, which
     *               contains a copy of the SkBitmap. If NULL,
     *               the bitmap could not be copied.
     */
    const void* addBitmap(const SkBitmap& orig) {
        const uint32_t genID = orig.getGenerationID();
        SkPixelRef* sharedPixelRef = NULL;
        // When looking to see if we've previously used this bitmap, start at
        // the end, assuming that the caller is more likely to reuse a recent
        // one.
        BitmapInfo* iter = fMostRecentlyUsed;
        while (iter != NULL) {
            if (genID == iter->fGenID) {
                SkBitmap* storedBitmap = iter->fBitmap;
                // TODO: Perhaps we can share code with
                // SkPictureRecord::PixelRefDictionaryEntry/
                // BitmapIndexCacheEntry so we can do a binary search for a
                // matching bitmap
                if (orig.pixelRefOffset() != storedBitmap->pixelRefOffset()
                    || orig.width() != storedBitmap->width()
                    || orig.height() != storedBitmap->height()) {
                    // In this case, the bitmaps share a pixelRef, but have
                    // different offsets or sizes. Keep track of the other
                    // bitmap so that instead of making another copy of the
                    // pixelRef we can use the copy we already made.
                    sharedPixelRef = storedBitmap->pixelRef();
                    break;
                }
                iter->addDraws(fNumberOfReaders);
                this->setMostRecentlyUsed(iter);
                return iter;
            }
            iter = iter->fLessRecentlyUsed;
        }
        SkAutoRef ar((SkRefCnt*)sharedPixelRef);
        BitmapInfo* replace = this->bitmapToReplace(orig);
        SkBitmap* copy;
        // If the bitmap is mutable, we still need to do a deep copy, since the
        // caller may modify it afterwards. That said, if the bitmap is mutable,
        // but has no pixelRef, the copy constructor actually does a deep copy.
        if (fCanDoShallowCopies && (orig.isImmutable() || !orig.pixelRef())) {
            if (NULL == replace) {
                copy = SkNEW_ARGS(SkBitmap, (orig));
            } else {
                *replace->fBitmap = orig;
            }
        } else {
            if (sharedPixelRef != NULL) {
                if (NULL == replace) {
                    // Do a shallow copy of the bitmap to get the width, height, etc
                    copy = SkNEW_ARGS(SkBitmap, (orig));
                    // Replace the pixelRef with the copy that was already made, and
                    // use the appropriate offset.
                    copy->setPixelRef(sharedPixelRef, orig.pixelRefOffset());
                } else {
                    *replace->fBitmap = orig;
                    replace->fBitmap->setPixelRef(sharedPixelRef, orig.pixelRefOffset());
                }
            } else {
                if (NULL == replace) {
                    copy = SkNEW(SkBitmap);
                    if (!orig.copyTo(copy, orig.getConfig())) {
                        delete copy;
                        return NULL;
                    }
                } else {
                    if (!orig.copyTo(replace->fBitmap, orig.getConfig())) {
                        return NULL;
                    }
                }
            }
        }
        BitmapInfo* info;
        if (NULL == replace) {
            fBytesAllocated += sizeof(BitmapInfo);
            info = SkNEW_ARGS(BitmapInfo, (copy, genID, fNumberOfReaders));
            fBitmapCount++;
        } else {
            fBytesAllocated -= replace->fBytesAllocated;
            replace->fGenID = genID;
            replace->addDraws(fNumberOfReaders);
            info = replace;
        }
        // Always include the size of the SkBitmap struct.
        info->fBytesAllocated = sizeof(SkBitmap);
        // If the SkBitmap does not share an SkPixelRef with an SkBitmap already
        // in the SharedHeap, also include the size of its pixels.
        if (NULL == sharedPixelRef) {
            info->fBytesAllocated += orig.getSize();
        }
        fBytesAllocated += info->fBytesAllocated;
        this->setMostRecentlyUsed(info);
        return info;
    }
private:
    void setMostRecentlyUsed(BitmapInfo* info);
    BitmapInfo* bitmapToReplace(const SkBitmap& bm) const;

    int         fBitmapCount;
    BitmapInfo* fLeastRecentlyUsed;
    BitmapInfo* fMostRecentlyUsed;
    const bool  fCanDoShallowCopies;
    const int   fNumberOfReaders;
    size_t      fBytesAllocated;
};

// We just "used" info. Update our LRU accordingly
void SharedHeap::setMostRecentlyUsed(BitmapInfo* info) {
    SkASSERT(info != NULL);
    if (info == fMostRecentlyUsed) {
        return;
    }
    // Remove info from its prior place, and make sure to cover the hole.
    if (fLeastRecentlyUsed == info) {
        SkASSERT(info->fMoreRecentlyUsed != NULL);
        fLeastRecentlyUsed = info->fMoreRecentlyUsed;
    }
    if (info->fMoreRecentlyUsed != NULL) {
        SkASSERT(fMostRecentlyUsed != info);
        info->fMoreRecentlyUsed->fLessRecentlyUsed = info->fLessRecentlyUsed;
    }
    if (info->fLessRecentlyUsed != NULL) {
        SkASSERT(fLeastRecentlyUsed != info);
        info->fLessRecentlyUsed->fMoreRecentlyUsed = info->fMoreRecentlyUsed;
    }
    info->fMoreRecentlyUsed = NULL;
    // Set up the head and tail pointers properly.
    if (fMostRecentlyUsed != NULL) {
        SkASSERT(NULL == fMostRecentlyUsed->fMoreRecentlyUsed);
        fMostRecentlyUsed->fMoreRecentlyUsed = info;
        info->fLessRecentlyUsed = fMostRecentlyUsed;
    }
    fMostRecentlyUsed = info;
    if (NULL == fLeastRecentlyUsed) {
        fLeastRecentlyUsed = info;
    }
}

/**
 * Given a new bitmap to be added to the cache, return an existing one that
 * should be removed to make room, or NULL if there is already room.
 */
BitmapInfo* SharedHeap::bitmapToReplace(const SkBitmap& bm) const {
    // Arbitrarily set a limit of 5. We should test to find the best tradeoff
    // between time and space. A lower limit means that we use less space, but
    // it also means that we may have to insert the same bitmap into the heap
    // multiple times (depending on the input), potentially taking more time.
    // On the other hand, a lower limit also means searching through our stored
    // bitmaps takes less time.
    if (fBitmapCount > 5) {
        BitmapInfo* iter = fLeastRecentlyUsed;
        while (iter != NULL) {
            if (iter->drawCount() > 0) {
                // If the least recently used bitmap has not been drawn by some
                // reader, then a more recently used one will not have been
                // drawn yet either.
                return NULL;
            }
            if (bm.pixelRef() != NULL
                    && bm.pixelRef() == iter->fBitmap->pixelRef()) {
                // Do not replace a bitmap with a new one using the same
                // pixel ref. Instead look for a different one that will
                // potentially free up more space.
                iter = iter->fMoreRecentlyUsed;
            } else {
                return iter;
            }
        }
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

class SkGPipeCanvas : public SkCanvas {
public:
    SkGPipeCanvas(SkGPipeController*, SkWriter32*, uint32_t flags);
    virtual ~SkGPipeCanvas();

    void finish() {
        if (!fDone) {
            if (this->needOpBytes()) {
                this->writeOp(kDone_DrawOp);
                this->doNotify();
            }
            fDone = true;
        }
    }

    void flushRecording(bool detachCurrentBlock);

    size_t storageAllocatedForRecording() {
        return fSharedHeap.bytesAllocated();
    }

    // overrides from SkCanvas
    virtual int save(SaveFlags) SK_OVERRIDE;
    virtual int saveLayer(const SkRect* bounds, const SkPaint*,
                          SaveFlags) SK_OVERRIDE;
    virtual void restore() SK_OVERRIDE;
    virtual bool isDrawingToLayer() const SK_OVERRIDE;
    virtual bool translate(SkScalar dx, SkScalar dy) SK_OVERRIDE;
    virtual bool scale(SkScalar sx, SkScalar sy) SK_OVERRIDE;
    virtual bool rotate(SkScalar degrees) SK_OVERRIDE;
    virtual bool skew(SkScalar sx, SkScalar sy) SK_OVERRIDE;
    virtual bool concat(const SkMatrix& matrix) SK_OVERRIDE;
    virtual void setMatrix(const SkMatrix& matrix) SK_OVERRIDE;
    virtual bool clipRect(const SkRect& rect, SkRegion::Op op,
                          bool doAntiAlias = false) SK_OVERRIDE;
    virtual bool clipPath(const SkPath& path, SkRegion::Op op,
                          bool doAntiAlias = false) SK_OVERRIDE;
    virtual bool clipRegion(const SkRegion& region, SkRegion::Op op) SK_OVERRIDE;
    virtual void clear(SkColor) SK_OVERRIDE;
    virtual void drawPaint(const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPoints(PointMode, size_t count, const SkPoint pts[],
                            const SkPaint&) SK_OVERRIDE;
    virtual void drawRect(const SkRect& rect, const SkPaint&) SK_OVERRIDE;
    virtual void drawPath(const SkPath& path, const SkPaint&) SK_OVERRIDE;
    virtual void drawBitmap(const SkBitmap&, SkScalar left, SkScalar top,
                            const SkPaint*) SK_OVERRIDE;
    virtual void drawBitmapRect(const SkBitmap&, const SkIRect* src,
                                const SkRect& dst, const SkPaint*) SK_OVERRIDE;
    virtual void drawBitmapMatrix(const SkBitmap&, const SkMatrix&,
                                  const SkPaint*) SK_OVERRIDE;
    virtual void drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                const SkRect& dst, const SkPaint* paint = NULL) SK_OVERRIDE;
    virtual void drawSprite(const SkBitmap&, int left, int top,
                            const SkPaint*) SK_OVERRIDE;
    virtual void drawText(const void* text, size_t byteLength, SkScalar x,
                          SkScalar y, const SkPaint&) SK_OVERRIDE;
    virtual void drawPosText(const void* text, size_t byteLength,
                             const SkPoint pos[], const SkPaint&) SK_OVERRIDE;
    virtual void drawPosTextH(const void* text, size_t byteLength,
                              const SkScalar xpos[], SkScalar constY,
                              const SkPaint&) SK_OVERRIDE;
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

private:
    enum {
        kNoSaveLayer = -1,
    };
    SkNamedFactorySet* fFactorySet;
    int                fFirstSaveLayerStackLevel;
    SharedHeap         fSharedHeap;
    SkGPipeController* fController;
    SkWriter32&        fWriter;
    size_t             fBlockSize; // amount allocated for writer
    size_t             fBytesNotified;
    bool               fDone;
    uint32_t           fFlags;

    SkRefCntSet        fTypefaceSet;

    uint32_t getTypefaceID(SkTypeface*);

    inline void writeOp(DrawOps op, unsigned flags, unsigned data) {
        fWriter.write32(DrawOp_packOpFlagData(op, flags, data));
    }

    inline void writeOp(DrawOps op) {
        fWriter.write32(DrawOp_packOpFlagData(op, 0, 0));
    }

    bool needOpBytes(size_t size = 0);

    inline void doNotify() {
        if (!fDone) {
            size_t bytes = fWriter.size() - fBytesNotified;
            if (bytes > 0) {
                fController->notifyWritten(bytes);
                fBytesNotified += bytes;
            }
        }
    }

    // Should be called after any calls to an SkFlatDictionary::findAndReplace
    // if a new SkFlatData was added when in cross process mode
    void flattenFactoryNames();

    // These are only used when in cross process, but with no shared address
    // space, so bitmaps are flattened.
    FlattenableHeap    fBitmapHeap;
    SkBitmapDictionary fBitmapDictionary;
    int flattenToIndex(const SkBitmap&);

    FlattenableHeap fFlattenableHeap;
    FlatDictionary  fFlatDictionary;
    int fCurrFlatIndex[kCount_PaintFlats];
    int flattenToIndex(SkFlattenable* obj, PaintFlats);

    // Common code used by drawBitmap* when flattening.
    bool commonDrawBitmapFlatten(const SkBitmap& bm, DrawOps op, unsigned flags,
                                 size_t opBytesNeeded, const SkPaint* paint);
    // Common code used by drawBitmap* when storing in the heap.
    bool commonDrawBitmapHeap(const SkBitmap& bm, DrawOps op, unsigned flags,
                              size_t opBytesNeeded, const SkPaint* paint);
    // Convenience type for function pointer
    typedef bool (SkGPipeCanvas::*BitmapCommonFunction)(const SkBitmap&,
                                                        DrawOps, unsigned,
                                                        size_t, const SkPaint*);

    SkPaint fPaint;
    void writePaint(const SkPaint&);

    class AutoPipeNotify {
    public:
        AutoPipeNotify(SkGPipeCanvas* canvas) : fCanvas(canvas) {}
        ~AutoPipeNotify() { fCanvas->doNotify(); }
    private:
        SkGPipeCanvas* fCanvas;
    };
    friend class AutoPipeNotify;

    typedef SkCanvas INHERITED;
};

void SkGPipeCanvas::flattenFactoryNames() {
    const char* name;
    while ((name = fFactorySet->getNextAddedFactoryName()) != NULL) {
        size_t len = strlen(name);
        if (this->needOpBytes(len)) {
            this->writeOp(kDef_Factory_DrawOp);
            fWriter.writeString(name, len);
        }
    }
}

int SkGPipeCanvas::flattenToIndex(const SkBitmap & bitmap) {
    SkASSERT(shouldFlattenBitmaps(fFlags));
    uint32_t flags = SkFlattenableWriteBuffer::kCrossProcess_Flag;
    bool added, replaced;
    const SkFlatData* flat = fBitmapDictionary.findAndReplace(
        bitmap, flags, fBitmapHeap.flatToReplace(), &added, &replaced);

    int index = flat->index();
    if (added) {
        this->flattenFactoryNames();
        size_t flatSize = flat->flatSize();
        if (this->needOpBytes(flatSize)) {
            this->writeOp(kDef_Bitmap_DrawOp, 0, index);
            fWriter.write(flat->data(), flatSize);
        }
    }
    return index;
}

// return 0 for NULL (or unflattenable obj), or index-base-1
// return ~(index-base-1) if an old flattenable was replaced
int SkGPipeCanvas::flattenToIndex(SkFlattenable* obj, PaintFlats paintflat) {
    if (NULL == obj) {
        return 0;
    }

    uint32_t writeBufferFlags;
    if (isCrossProcess(fFlags)) {
        writeBufferFlags =  SkFlattenableWriteBuffer::kCrossProcess_Flag;
    } else {
        // Needed for bitmap shaders.
        writeBufferFlags = SkFlattenableWriteBuffer::kForceFlattenBitmapPixels_Flag;
    }

    bool added, replaced;
    const SkFlatData* flat = fFlatDictionary.findAndReplace(
            *obj, writeBufferFlags, fFlattenableHeap.flatToReplace(), &added, &replaced);
    int index = flat->index();
    if (added) {
        if (isCrossProcess(fFlags)) {
            this->flattenFactoryNames();
        }
        size_t flatSize = flat->flatSize();
        if (this->needOpBytes(flatSize)) {
            this->writeOp(kDef_Flattenable_DrawOp, paintflat, index);
            fWriter.write(flat->data(), flatSize);
        }
    }
    if (replaced) {
        index = ~index;
    }
    return index;
}

///////////////////////////////////////////////////////////////////////////////

#define MIN_BLOCK_SIZE  (16 * 1024)
#define BITMAPS_TO_KEEP 5
#define FLATTENABLES_TO_KEEP 10

SkGPipeCanvas::SkGPipeCanvas(SkGPipeController* controller,
                             SkWriter32* writer, uint32_t flags)
: fFactorySet(isCrossProcess(flags) ? SkNEW(SkNamedFactorySet) : NULL)
, fSharedHeap(!isCrossProcess(flags), controller->numberOfReaders())
, fWriter(*writer)
, fFlags(flags)
, fBitmapHeap(BITMAPS_TO_KEEP, fFactorySet)
, fBitmapDictionary(&fBitmapHeap)
, fFlattenableHeap(FLATTENABLES_TO_KEEP, fFactorySet)
, fFlatDictionary(&fFlattenableHeap) {
    fController = controller;
    fDone = false;
    fBlockSize = 0; // need first block from controller
    fBytesNotified = 0;
    fFirstSaveLayerStackLevel = kNoSaveLayer;
    sk_bzero(fCurrFlatIndex, sizeof(fCurrFlatIndex));

    // we need a device to limit our clip
    // should the caller give us the bounds?
    // We don't allocate pixels for the bitmap
    SkBitmap bitmap;
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, 32767, 32767);
    SkDevice* device = SkNEW_ARGS(SkDevice, (bitmap));
    this->setDevice(device)->unref();
    // Tell the reader the appropriate flags to use.
    if (this->needOpBytes()) {
        this->writeOp(kReportFlags_DrawOp, fFlags, 0);
    }
}

SkGPipeCanvas::~SkGPipeCanvas() {
    this->finish();
    SkSafeUnref(fFactorySet);
}

bool SkGPipeCanvas::needOpBytes(size_t needed) {
    if (fDone) {
        return false;
    }

    needed += 4;  // size of DrawOp atom
    if (fWriter.size() + needed > fBlockSize) {
        // Before we wipe out any data that has already been written, read it
        // out.
        this->doNotify();
        size_t blockSize = SkMax32(MIN_BLOCK_SIZE, needed);
        void* block = fController->requestBlock(blockSize, &fBlockSize);
        if (NULL == block) {
            fDone = true;
            return false;
        }
        fWriter.reset(block, fBlockSize);
        fBytesNotified = 0;
    }
    return true;
}

uint32_t SkGPipeCanvas::getTypefaceID(SkTypeface* face) {
    uint32_t id = 0; // 0 means default/null typeface
    if (face) {
        id = fTypefaceSet.find(face);
        if (0 == id) {
            id = fTypefaceSet.add(face);
            size_t size = writeTypeface(NULL, face);
            if (this->needOpBytes(size)) {
                this->writeOp(kDef_Typeface_DrawOp);
                writeTypeface(&fWriter, face);
            }
        }
    }
    return id;
}

///////////////////////////////////////////////////////////////////////////////

#define NOTIFY_SETUP(canvas)    \
    AutoPipeNotify apn(canvas)

int SkGPipeCanvas::save(SaveFlags flags) {
    NOTIFY_SETUP(this);
    if (this->needOpBytes()) {
        this->writeOp(kSave_DrawOp, 0, flags);
    }
    return this->INHERITED::save(flags);
}

int SkGPipeCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint,
                             SaveFlags saveFlags) {
    NOTIFY_SETUP(this);
    size_t size = 0;
    unsigned opFlags = 0;

    if (bounds) {
        opFlags |= kSaveLayer_HasBounds_DrawOpFlag;
        size += sizeof(SkRect);
    }
    if (paint) {
        opFlags |= kSaveLayer_HasPaint_DrawOpFlag;
        this->writePaint(*paint);
    }

    if (this->needOpBytes(size)) {
        this->writeOp(kSaveLayer_DrawOp, opFlags, saveFlags);
        if (bounds) {
            fWriter.writeRect(*bounds);
        }
    }

    if (kNoSaveLayer == fFirstSaveLayerStackLevel){
        fFirstSaveLayerStackLevel = this->getSaveCount();
    }
    // we just pass on the save, so we don't create a layer
    return this->INHERITED::save(saveFlags);
}

void SkGPipeCanvas::restore() {
    NOTIFY_SETUP(this);
    if (this->needOpBytes()) {
        this->writeOp(kRestore_DrawOp);
    }

    this->INHERITED::restore();

    if (this->getSaveCount() == fFirstSaveLayerStackLevel){
        fFirstSaveLayerStackLevel = kNoSaveLayer;
    }
}

bool SkGPipeCanvas::isDrawingToLayer() const {
    return kNoSaveLayer != fFirstSaveLayerStackLevel;
}

bool SkGPipeCanvas::translate(SkScalar dx, SkScalar dy) {
    if (dx || dy) {
        NOTIFY_SETUP(this);
        if (this->needOpBytes(2 * sizeof(SkScalar))) {
            this->writeOp(kTranslate_DrawOp);
            fWriter.writeScalar(dx);
            fWriter.writeScalar(dy);
        }
    }
    return this->INHERITED::translate(dx, dy);
}

bool SkGPipeCanvas::scale(SkScalar sx, SkScalar sy) {
    if (sx || sy) {
        NOTIFY_SETUP(this);
        if (this->needOpBytes(2 * sizeof(SkScalar))) {
            this->writeOp(kScale_DrawOp);
            fWriter.writeScalar(sx);
            fWriter.writeScalar(sy);
        }
    }
    return this->INHERITED::scale(sx, sy);
}

bool SkGPipeCanvas::rotate(SkScalar degrees) {
    if (degrees) {
        NOTIFY_SETUP(this);
        if (this->needOpBytes(sizeof(SkScalar))) {
            this->writeOp(kRotate_DrawOp);
            fWriter.writeScalar(degrees);
        }
    }
    return this->INHERITED::rotate(degrees);
}

bool SkGPipeCanvas::skew(SkScalar sx, SkScalar sy) {
    if (sx || sy) {
        NOTIFY_SETUP(this);
        if (this->needOpBytes(2 * sizeof(SkScalar))) {
            this->writeOp(kSkew_DrawOp);
            fWriter.writeScalar(sx);
            fWriter.writeScalar(sy);
        }
    }
    return this->INHERITED::skew(sx, sy);
}

bool SkGPipeCanvas::concat(const SkMatrix& matrix) {
    if (!matrix.isIdentity()) {
        NOTIFY_SETUP(this);
        if (this->needOpBytes(matrix.writeToMemory(NULL))) {
            this->writeOp(kConcat_DrawOp);
            fWriter.writeMatrix(matrix);
        }
    }
    return this->INHERITED::concat(matrix);
}

void SkGPipeCanvas::setMatrix(const SkMatrix& matrix) {
    NOTIFY_SETUP(this);
    if (this->needOpBytes(matrix.writeToMemory(NULL))) {
        this->writeOp(kSetMatrix_DrawOp);
        fWriter.writeMatrix(matrix);
    }
    this->INHERITED::setMatrix(matrix);
}

bool SkGPipeCanvas::clipRect(const SkRect& rect, SkRegion::Op rgnOp,
                             bool doAntiAlias) {
    NOTIFY_SETUP(this);
    if (this->needOpBytes(sizeof(SkRect)) + sizeof(bool)) {
        this->writeOp(kClipRect_DrawOp, 0, rgnOp);
        fWriter.writeRect(rect);
        fWriter.writeBool(doAntiAlias);
    }
    return this->INHERITED::clipRect(rect, rgnOp, doAntiAlias);
}

bool SkGPipeCanvas::clipPath(const SkPath& path, SkRegion::Op rgnOp,
                             bool doAntiAlias) {
    NOTIFY_SETUP(this);
    if (this->needOpBytes(path.writeToMemory(NULL)) + sizeof(bool)) {
        this->writeOp(kClipPath_DrawOp, 0, rgnOp);
        fWriter.writePath(path);
        fWriter.writeBool(doAntiAlias);
    }
    // we just pass on the bounds of the path
    return this->INHERITED::clipRect(path.getBounds(), rgnOp, doAntiAlias);
}

bool SkGPipeCanvas::clipRegion(const SkRegion& region, SkRegion::Op rgnOp) {
    NOTIFY_SETUP(this);
    if (this->needOpBytes(region.writeToMemory(NULL))) {
        this->writeOp(kClipRegion_DrawOp, 0, rgnOp);
        fWriter.writeRegion(region);
    }
    return this->INHERITED::clipRegion(region, rgnOp);
}

///////////////////////////////////////////////////////////////////////////////

void SkGPipeCanvas::clear(SkColor color) {
    NOTIFY_SETUP(this);
    unsigned flags = 0;
    if (color) {
        flags |= kClear_HasColor_DrawOpFlag;
    }
    if (this->needOpBytes(sizeof(SkColor))) {
        this->writeOp(kDrawClear_DrawOp, flags, 0);
        if (color) {
            fWriter.write32(color);
        }
    }
}

void SkGPipeCanvas::drawPaint(const SkPaint& paint) {
    NOTIFY_SETUP(this);
    this->writePaint(paint);
    if (this->needOpBytes()) {
        this->writeOp(kDrawPaint_DrawOp);
    }
}

void SkGPipeCanvas::drawPoints(PointMode mode, size_t count,
                                   const SkPoint pts[], const SkPaint& paint) {
    if (count) {
        NOTIFY_SETUP(this);
        this->writePaint(paint);
        if (this->needOpBytes(4 + count * sizeof(SkPoint))) {
            this->writeOp(kDrawPoints_DrawOp, mode, 0);
            fWriter.write32(count);
            fWriter.write(pts, count * sizeof(SkPoint));
        }
    }
}

void SkGPipeCanvas::drawRect(const SkRect& rect, const SkPaint& paint) {
    NOTIFY_SETUP(this);
    this->writePaint(paint);
    if (this->needOpBytes(sizeof(SkRect))) {
        this->writeOp(kDrawRect_DrawOp);
        fWriter.writeRect(rect);
    }
}

void SkGPipeCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    NOTIFY_SETUP(this);
    this->writePaint(paint);
    if (this->needOpBytes(path.writeToMemory(NULL))) {
        this->writeOp(kDrawPath_DrawOp);
        fWriter.writePath(path);
    }
}

bool SkGPipeCanvas::commonDrawBitmapFlatten(const SkBitmap& bm, DrawOps op,
                                            unsigned flags,
                                            size_t opBytesNeeded,
                                            const SkPaint* paint) {
    if (paint != NULL) {
        flags |= kDrawBitmap_HasPaint_DrawOpsFlag;
        this->writePaint(*paint);
    }
    int bitmapIndex = this->flattenToIndex(bm);
    if (this->needOpBytes(opBytesNeeded)) {
        this->writeOp(op, flags, bitmapIndex);
        return true;
    }
    return false;
}

bool SkGPipeCanvas::commonDrawBitmapHeap(const SkBitmap& bm, DrawOps op,
                                         unsigned flags,
                                         size_t opBytesNeeded,
                                         const SkPaint* paint) {
    const void* ptr = fSharedHeap.addBitmap(bm);
    if (NULL == ptr) {
        return false;
    }
    if (paint != NULL) {
        flags |= kDrawBitmap_HasPaint_DrawOpsFlag;
        this->writePaint(*paint);
    }
    if (this->needOpBytes(opBytesNeeded + sizeof(void*))) {
        this->writeOp(op, flags, 0);
        fWriter.writePtr(const_cast<void*>(ptr));
        return true;
    }
    return false;
}

void SkGPipeCanvas::drawBitmap(const SkBitmap& bm, SkScalar left, SkScalar top,
                               const SkPaint* paint) {
    NOTIFY_SETUP(this);
    size_t opBytesNeeded = sizeof(SkScalar) * 2;

    BitmapCommonFunction bitmapCommon = shouldFlattenBitmaps(fFlags) ?
            &SkGPipeCanvas::commonDrawBitmapFlatten :
            &SkGPipeCanvas::commonDrawBitmapHeap;

    if ((*this.*bitmapCommon)(bm, kDrawBitmap_DrawOp, 0, opBytesNeeded, paint)) {
        fWriter.writeScalar(left);
        fWriter.writeScalar(top);
    }
}

void SkGPipeCanvas::drawBitmapRect(const SkBitmap& bm, const SkIRect* src,
                                   const SkRect& dst, const SkPaint* paint) {
    NOTIFY_SETUP(this);
    size_t opBytesNeeded = sizeof(SkRect);
    bool hasSrc = src != NULL;
    unsigned flags;
    if (hasSrc) {
        flags = kDrawBitmap_HasSrcRect_DrawOpsFlag;
        opBytesNeeded += sizeof(int32_t) * 4;
    } else {
        flags = 0;
    }

    BitmapCommonFunction bitmapCommon = shouldFlattenBitmaps(fFlags) ?
            &SkGPipeCanvas::commonDrawBitmapFlatten :
            &SkGPipeCanvas::commonDrawBitmapHeap;
    
    if ((*this.*bitmapCommon)(bm, kDrawBitmapRect_DrawOp, flags, opBytesNeeded, paint)) {
        if (hasSrc) {
            fWriter.write32(src->fLeft);
            fWriter.write32(src->fTop);
            fWriter.write32(src->fRight);
            fWriter.write32(src->fBottom);
        }
        fWriter.writeRect(dst);
    }
}

void SkGPipeCanvas::drawBitmapMatrix(const SkBitmap& bm, const SkMatrix& matrix,
                                     const SkPaint* paint) {
    NOTIFY_SETUP(this);
    size_t opBytesNeeded = matrix.writeToMemory(NULL);
    
    BitmapCommonFunction bitmapCommon = shouldFlattenBitmaps(fFlags) ?
        &SkGPipeCanvas::commonDrawBitmapFlatten :
        &SkGPipeCanvas::commonDrawBitmapHeap;

    if ((*this.*bitmapCommon)(bm, kDrawBitmapMatrix_DrawOp, 0, opBytesNeeded, paint)) {
        fWriter.writeMatrix(matrix);
    }
}

void SkGPipeCanvas::drawBitmapNine(const SkBitmap& bm, const SkIRect& center,
                                   const SkRect& dst, const SkPaint* paint) {
    NOTIFY_SETUP(this);
    size_t opBytesNeeded = sizeof(int32_t) * 4 + sizeof(SkRect);

    BitmapCommonFunction bitmapCommon = shouldFlattenBitmaps(fFlags) ?
            &SkGPipeCanvas::commonDrawBitmapFlatten :
            &SkGPipeCanvas::commonDrawBitmapHeap;

    if ((*this.*bitmapCommon)(bm, kDrawBitmapNine_DrawOp, 0, opBytesNeeded, paint)) {
        fWriter.write32(center.fLeft);
        fWriter.write32(center.fTop);
        fWriter.write32(center.fRight);
        fWriter.write32(center.fBottom);
        fWriter.writeRect(dst);
    }
}

void SkGPipeCanvas::drawSprite(const SkBitmap& bm, int left, int top,
                                   const SkPaint* paint) {
    NOTIFY_SETUP(this);
    size_t opBytesNeeded = sizeof(int32_t) * 2;

    BitmapCommonFunction bitmapCommon = shouldFlattenBitmaps(fFlags) ?
            &SkGPipeCanvas::commonDrawBitmapFlatten :
            &SkGPipeCanvas::commonDrawBitmapHeap;

    if ((*this.*bitmapCommon)(bm, kDrawSprite_DrawOp, 0, opBytesNeeded, paint)) {
        fWriter.write32(left);
        fWriter.write32(top);
    }
}

void SkGPipeCanvas::drawText(const void* text, size_t byteLength, SkScalar x,
                                 SkScalar y, const SkPaint& paint) {
    if (byteLength) {
        NOTIFY_SETUP(this);
        this->writePaint(paint);
        if (this->needOpBytes(4 + SkAlign4(byteLength) + 2 * sizeof(SkScalar))) {
            this->writeOp(kDrawText_DrawOp);
            fWriter.write32(byteLength);
            fWriter.writePad(text, byteLength);
            fWriter.writeScalar(x);
            fWriter.writeScalar(y);
        }
    }
}

void SkGPipeCanvas::drawPosText(const void* text, size_t byteLength,
                                const SkPoint pos[], const SkPaint& paint) {
    if (byteLength) {
        NOTIFY_SETUP(this);
        this->writePaint(paint);
        int count = paint.textToGlyphs(text, byteLength, NULL);
        if (this->needOpBytes(4 + SkAlign4(byteLength) + 4 + count * sizeof(SkPoint))) {
            this->writeOp(kDrawPosText_DrawOp);
            fWriter.write32(byteLength);
            fWriter.writePad(text, byteLength);
            fWriter.write32(count);
            fWriter.write(pos, count * sizeof(SkPoint));
        }
    }
}

void SkGPipeCanvas::drawPosTextH(const void* text, size_t byteLength,
                                 const SkScalar xpos[], SkScalar constY,
                                 const SkPaint& paint) {
    if (byteLength) {
        NOTIFY_SETUP(this);
        this->writePaint(paint);
        int count = paint.textToGlyphs(text, byteLength, NULL);
        if (this->needOpBytes(4 + SkAlign4(byteLength) + 4 + count * sizeof(SkScalar) + 4)) {
            this->writeOp(kDrawPosTextH_DrawOp);
            fWriter.write32(byteLength);
            fWriter.writePad(text, byteLength);
            fWriter.write32(count);
            fWriter.write(xpos, count * sizeof(SkScalar));
            fWriter.writeScalar(constY);
        }
    }
}

void SkGPipeCanvas::drawTextOnPath(const void* text, size_t byteLength,
                                   const SkPath& path, const SkMatrix* matrix,
                                   const SkPaint& paint) {
    if (byteLength) {
        NOTIFY_SETUP(this);
        unsigned flags = 0;
        size_t size = 4 + SkAlign4(byteLength) + path.writeToMemory(NULL);
        if (matrix) {
            flags |= kDrawTextOnPath_HasMatrix_DrawOpFlag;
            size += matrix->writeToMemory(NULL);
        }
        this->writePaint(paint);
        if (this->needOpBytes(size)) {
            this->writeOp(kDrawTextOnPath_DrawOp, flags, 0);

            fWriter.write32(byteLength);
            fWriter.writePad(text, byteLength);

            fWriter.writePath(path);
            if (matrix) {
                fWriter.writeMatrix(*matrix);
            }
        }
    }
}

void SkGPipeCanvas::drawPicture(SkPicture& picture) {
    // we want to playback the picture into individual draw calls
    this->INHERITED::drawPicture(picture);
}

void SkGPipeCanvas::drawVertices(VertexMode mode, int vertexCount,
                                 const SkPoint vertices[], const SkPoint texs[],
                                 const SkColor colors[], SkXfermode*,
                                 const uint16_t indices[], int indexCount,
                                 const SkPaint& paint) {
    if (0 == vertexCount) {
        return;
    }

    NOTIFY_SETUP(this);
    size_t size = 4 + vertexCount * sizeof(SkPoint);
    this->writePaint(paint);
    unsigned flags = 0;
    if (texs) {
        flags |= kDrawVertices_HasTexs_DrawOpFlag;
        size += vertexCount * sizeof(SkPoint);
    }
    if (colors) {
        flags |= kDrawVertices_HasColors_DrawOpFlag;
        size += vertexCount * sizeof(SkColor);
    }
    if (indices && indexCount > 0) {
        flags |= kDrawVertices_HasIndices_DrawOpFlag;
        size += 4 + SkAlign4(indexCount * sizeof(uint16_t));
    }

    if (this->needOpBytes(size)) {
        this->writeOp(kDrawVertices_DrawOp, flags, 0);
        fWriter.write32(mode);
        fWriter.write32(vertexCount);
        fWriter.write(vertices, vertexCount * sizeof(SkPoint));
        if (texs) {
            fWriter.write(texs, vertexCount * sizeof(SkPoint));
        }
        if (colors) {
            fWriter.write(colors, vertexCount * sizeof(SkColor));
        }

        // TODO: flatten xfermode

        if (indices && indexCount > 0) {
            fWriter.write32(indexCount);
            fWriter.writePad(indices, indexCount * sizeof(uint16_t));
        }
    }
}

void SkGPipeCanvas::drawData(const void* ptr, size_t size) {
    if (size && ptr) {
        NOTIFY_SETUP(this);
        unsigned data = 0;
        if (size < (1 << DRAWOPS_DATA_BITS)) {
            data = (unsigned)size;
        }
        if (this->needOpBytes(4 + SkAlign4(size))) {
            this->writeOp(kDrawData_DrawOp, 0, data);
            if (0 == data) {
                fWriter.write32(size);
            }
            fWriter.writePad(ptr, size);
        }
    }
}

void SkGPipeCanvas::flushRecording(bool detachCurrentBlock) {
    doNotify();
    if (detachCurrentBlock) {
        // force a new block to be requested for the next recorded command
        fBlockSize = 0; 
    }
}

///////////////////////////////////////////////////////////////////////////////

template <typename T> uint32_t castToU32(T value) {
    union {
        T           fSrc;
        uint32_t    fDst;
    } data;
    data.fSrc = value;
    return data.fDst;
}

void SkGPipeCanvas::writePaint(const SkPaint& paint) {
    SkPaint& base = fPaint;
    uint32_t storage[32];
    uint32_t* ptr = storage;

    if (base.getFlags() != paint.getFlags()) {
        *ptr++ = PaintOp_packOpData(kFlags_PaintOp, paint.getFlags());
        base.setFlags(paint.getFlags());
    }
    if (base.getColor() != paint.getColor()) {
        *ptr++ = PaintOp_packOp(kColor_PaintOp);
        *ptr++ = paint.getColor();
        base.setColor(paint.getColor());
    }
    if (base.getStyle() != paint.getStyle()) {
        *ptr++ = PaintOp_packOpData(kStyle_PaintOp, paint.getStyle());
        base.setStyle(paint.getStyle());
    }
    if (base.getStrokeJoin() != paint.getStrokeJoin()) {
        *ptr++ = PaintOp_packOpData(kJoin_PaintOp, paint.getStrokeJoin());
        base.setStrokeJoin(paint.getStrokeJoin());
    }
    if (base.getStrokeCap() != paint.getStrokeCap()) {
        *ptr++ = PaintOp_packOpData(kCap_PaintOp, paint.getStrokeCap());
        base.setStrokeCap(paint.getStrokeCap());
    }
    if (base.getStrokeWidth() != paint.getStrokeWidth()) {
        *ptr++ = PaintOp_packOp(kWidth_PaintOp);
        *ptr++ = castToU32(paint.getStrokeWidth());
        base.setStrokeWidth(paint.getStrokeWidth());
    }
    if (base.getStrokeMiter() != paint.getStrokeMiter()) {
        *ptr++ = PaintOp_packOp(kMiter_PaintOp);
        *ptr++ = castToU32(paint.getStrokeMiter());
        base.setStrokeMiter(paint.getStrokeMiter());
    }
    if (base.getTextEncoding() != paint.getTextEncoding()) {
        *ptr++ = PaintOp_packOpData(kEncoding_PaintOp, paint.getTextEncoding());
        base.setTextEncoding(paint.getTextEncoding());
    }
    if (base.getHinting() != paint.getHinting()) {
        *ptr++ = PaintOp_packOpData(kHinting_PaintOp, paint.getHinting());
        base.setHinting(paint.getHinting());
    }
    if (base.getTextAlign() != paint.getTextAlign()) {
        *ptr++ = PaintOp_packOpData(kAlign_PaintOp, paint.getTextAlign());
        base.setTextAlign(paint.getTextAlign());
    }
    if (base.getTextSize() != paint.getTextSize()) {
        *ptr++ = PaintOp_packOp(kTextSize_PaintOp);
        *ptr++ = castToU32(paint.getTextSize());
        base.setTextSize(paint.getTextSize());
    }
    if (base.getTextScaleX() != paint.getTextScaleX()) {
        *ptr++ = PaintOp_packOp(kTextScaleX_PaintOp);
        *ptr++ = castToU32(paint.getTextScaleX());
        base.setTextScaleX(paint.getTextScaleX());
    }
    if (base.getTextSkewX() != paint.getTextSkewX()) {
        *ptr++ = PaintOp_packOp(kTextSkewX_PaintOp);
        *ptr++ = castToU32(paint.getTextSkewX());
        base.setTextSkewX(paint.getTextSkewX());
    }

    if (!SkTypeface::Equal(base.getTypeface(), paint.getTypeface())) {
        if (isCrossProcess(fFlags)) {
            uint32_t id = this->getTypefaceID(paint.getTypeface());
            *ptr++ = PaintOp_packOpData(kTypeface_PaintOp, id);
        } else if (this->needOpBytes(sizeof(void*))) {
            // Add to the set for ref counting.
            fTypefaceSet.add(paint.getTypeface());
            // It is safe to write the typeface to the stream before the rest
            // of the paint unless we ever send a kReset_PaintOp, which we
            // currently never do.
            this->writeOp(kSetTypeface_DrawOp);
            fWriter.writePtr(paint.getTypeface());
        }
        base.setTypeface(paint.getTypeface());
    }

    // This is a new paint, so all old flats can be safely purged, if necessary.
    fFlattenableHeap.markAllFlatsSafeToDelete();
    for (int i = 0; i < kCount_PaintFlats; i++) {
        int index = this->flattenToIndex(get_paintflat(paint, i), (PaintFlats)i);
        bool replaced = index < 0;
        if (replaced) {
            index = ~index;
        }
        // Store the index of any flat that needs to be kept. 0 means no flat.
        if (index > 0) {
            fFlattenableHeap.markFlatForKeeping(index);
        }
        SkASSERT(index >= 0 && index <= fFlatDictionary.count());
        if (index != fCurrFlatIndex[i] || replaced) {
            *ptr++ = PaintOp_packOpFlagData(kFlatIndex_PaintOp, i, index);
            fCurrFlatIndex[i] = index;
        }
    }

    size_t size = (char*)ptr - (char*)storage;
    if (size && this->needOpBytes(size)) {
        this->writeOp(kPaintOp_DrawOp, 0, size);
        fWriter.write(storage, size);
        for (size_t i = 0; i < size/4; i++) {
//            SkDebugf("[%d] %08X\n", i, storage[i]);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "SkGPipe.h"

SkGPipeController::~SkGPipeController() {
    SkSafeUnref(fCanvas);
}

void SkGPipeController::setCanvas(SkGPipeCanvas* canvas) {
    SkRefCnt_SafeAssign(fCanvas, canvas);
}

///////////////////////////////////////////////////////////////////////////////

SkGPipeWriter::SkGPipeWriter()
: fWriter(0) {
    fCanvas = NULL;
}

SkGPipeWriter::~SkGPipeWriter() {
    this->endRecording();
}

SkCanvas* SkGPipeWriter::startRecording(SkGPipeController* controller, uint32_t flags) {
    if (NULL == fCanvas) {
        fWriter.reset(NULL, 0);
        fCanvas = SkNEW_ARGS(SkGPipeCanvas, (controller, &fWriter, flags));
    }
    controller->setCanvas(fCanvas);
    return fCanvas;
}

void SkGPipeWriter::endRecording() {
    if (fCanvas) {
        fCanvas->finish();
        fCanvas->unref();
        fCanvas = NULL;
    }
}

void SkGPipeWriter::flushRecording(bool detachCurrentBlock){
    fCanvas->flushRecording(detachCurrentBlock);
}

size_t SkGPipeWriter::storageAllocatedForRecording() {
    return NULL == fCanvas ? 0 : fCanvas->storageAllocatedForRecording();
}

