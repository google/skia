
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPictureFlat_DEFINED
#define SkPictureFlat_DEFINED

//#define SK_DEBUG_SIZE

#include "SkBitmap.h"
#include "SkBitmapHeap.h"
#include "SkChecksum.h"
#include "SkChunkAlloc.h"
#include "SkMatrix.h"
#include "SkOrderedReadBuffer.h"
#include "SkOrderedWriteBuffer.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkPtrRecorder.h"
#include "SkRegion.h"
#include "SkTDynamicHash.h"
#include "SkTRefArray.h"
#include "SkTSearch.h"

enum DrawType {
    UNUSED,
    CLIP_PATH,
    CLIP_REGION,
    CLIP_RECT,
    CLIP_RRECT,
    CONCAT,
    DRAW_BITMAP,
    DRAW_BITMAP_MATRIX,
    DRAW_BITMAP_NINE,
    DRAW_BITMAP_RECT_TO_RECT,
    DRAW_CLEAR,
    DRAW_DATA,
    DRAW_OVAL,
    DRAW_PAINT,
    DRAW_PATH,
    DRAW_PICTURE,
    DRAW_POINTS,
    DRAW_POS_TEXT,
    DRAW_POS_TEXT_TOP_BOTTOM, // fast variant of DRAW_POS_TEXT
    DRAW_POS_TEXT_H,
    DRAW_POS_TEXT_H_TOP_BOTTOM, // fast variant of DRAW_POS_TEXT_H
    DRAW_RECT,
    DRAW_RRECT,
    DRAW_SPRITE,
    DRAW_TEXT,
    DRAW_TEXT_ON_PATH,
    DRAW_TEXT_TOP_BOTTOM,   // fast variant of DRAW_TEXT
    DRAW_VERTICES,
    RESTORE,
    ROTATE,
    SAVE,
    SAVE_LAYER,
    SCALE,
    SET_MATRIX,
    SKEW,
    TRANSLATE,
    NOOP,
    BEGIN_COMMENT_GROUP,
    COMMENT,
    END_COMMENT_GROUP,

    LAST_DRAWTYPE_ENUM = END_COMMENT_GROUP
};

// In the 'match' method, this constant will match any flavor of DRAW_BITMAP*
static const int kDRAW_BITMAP_FLAVOR = LAST_DRAWTYPE_ENUM+1;

enum DrawVertexFlags {
    DRAW_VERTICES_HAS_TEXS    = 0x01,
    DRAW_VERTICES_HAS_COLORS  = 0x02,
    DRAW_VERTICES_HAS_INDICES = 0x04
};

///////////////////////////////////////////////////////////////////////////////
// clipparams are packed in 5 bits
//  doAA:1 | regionOp:4

static inline uint32_t ClipParams_pack(SkRegion::Op op, bool doAA) {
    unsigned doAABit = doAA ? 1 : 0;
    return (doAABit << 4) | op;
}

static inline SkRegion::Op ClipParams_unpackRegionOp(uint32_t packed) {
    return (SkRegion::Op)(packed & 0xF);
}

static inline bool ClipParams_unpackDoAA(uint32_t packed) {
    return SkToBool((packed >> 4) & 1);
}

///////////////////////////////////////////////////////////////////////////////

class SkTypefacePlayback {
public:
    SkTypefacePlayback();
    virtual ~SkTypefacePlayback();

    int count() const { return fCount; }

    void reset(const SkRefCntSet*);

    void setCount(int count);
    SkRefCnt* set(int index, SkRefCnt*);

    void setupBuffer(SkOrderedReadBuffer& buffer) const {
        buffer.setTypefaceArray((SkTypeface**)fArray, fCount);
    }

protected:
    int fCount;
    SkRefCnt** fArray;
};

class SkFactoryPlayback {
public:
    SkFactoryPlayback(int count) : fCount(count) {
        fArray = SkNEW_ARRAY(SkFlattenable::Factory, count);
    }

    ~SkFactoryPlayback() {
        SkDELETE_ARRAY(fArray);
    }

    SkFlattenable::Factory* base() const { return fArray; }

    void setupBuffer(SkOrderedReadBuffer& buffer) const {
        buffer.setFactoryPlayback(fArray, fCount);
    }

private:
    int fCount;
    SkFlattenable::Factory* fArray;
};

///////////////////////////////////////////////////////////////////////////////
//
//
// The following templated classes provide an efficient way to store and compare
// objects that have been flattened (i.e. serialized in an ordered binary
// format).
//
// SkFlatData:       is a simple indexable container for the flattened data
//                   which is agnostic to the type of data is is indexing. It is
//                   also responsible for flattening/unflattening objects but
//                   details of that operation are hidden in the provided procs
// SkFlatDictionary: is an abstract templated dictionary that maintains a
//                   searchable set of SkFlatData objects of type T.
// SkFlatController: is an interface provided to SkFlatDictionary which handles
//                   allocation (and unallocation in some cases). It also holds
//                   ref count recorders and the like.
//
// NOTE: any class that wishes to be used in conjunction with SkFlatDictionary
// must subclass the dictionary and provide the necessary flattening procs.
// The end of this header contains dictionary subclasses for some common classes
// like SkBitmap, SkMatrix, SkPaint, and SkRegion. SkFlatController must also
// be implemented, or SkChunkFlatController can be used to use an
// SkChunkAllocator and never do replacements.
//
//
///////////////////////////////////////////////////////////////////////////////

class SkFlatData;

class SkFlatController : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkFlatController)

    SkFlatController();
    virtual ~SkFlatController();
    /**
     * Return a new block of memory for the SkFlatDictionary to use.
     * This memory is owned by the controller and has the same lifetime unless you
     * call unalloc(), in which case it may be freed early.
     */
    virtual void* allocThrow(size_t bytes) = 0;

    /**
     * Hint that this block, which was allocated with allocThrow, is no longer needed.
     * The implementation may choose to free this memory any time beteween now and destruction.
     */
    virtual void unalloc(void* ptr) = 0;

    /**
     * Used during creation and unflattening of SkFlatData objects. If the
     * objects being flattened contain bitmaps they are stored in this heap
     * and the flattenable stores the index to the bitmap on the heap.
     * This should be set by the protected setBitmapHeap.
     */
    SkBitmapHeap* getBitmapHeap() { return fBitmapHeap; }

    /**
     * Used during creation of SkFlatData objects. If a typeface recorder is
     * required to flatten the objects being flattened (i.e. for SkPaints), this
     * should be set by the protected setTypefaceSet.
     */
    SkRefCntSet* getTypefaceSet() { return fTypefaceSet; }

    /**
     * Used during unflattening of the SkFlatData objects in the
     * SkFlatDictionary. Needs to be set by the protected setTypefacePlayback
     * and needs to be reset to the SkRefCntSet passed to setTypefaceSet.
     */
    SkTypefacePlayback* getTypefacePlayback() { return fTypefacePlayback; }

    /**
     * Optional factory recorder used during creation of SkFlatData objects. Set
     * using the protected method setNamedFactorySet.
     */
    SkNamedFactorySet* getNamedFactorySet() { return fFactorySet; }

    /**
     * Flags to use during creation of SkFlatData objects. Defaults to zero.
     */
    uint32_t getWriteBufferFlags() { return fWriteBufferFlags; }

protected:
    /**
     * Set an SkBitmapHeap to be used to store/read SkBitmaps. Ref counted.
     */
    void setBitmapHeap(SkBitmapHeap*);

    /**
     * Set an SkRefCntSet to be used to store SkTypefaces during flattening. Ref
     * counted.
     */
    void setTypefaceSet(SkRefCntSet*);

    /**
     * Set an SkTypefacePlayback to be used to find references to SkTypefaces
     * during unflattening. Should be reset to the set provided to
     * setTypefaceSet.
     */
    void setTypefacePlayback(SkTypefacePlayback*);

    /**
     * Set an SkNamedFactorySet to be used to store Factorys and their
     * corresponding names during flattening. Ref counted. Returns the same
     * set as a convenience.
     */
    SkNamedFactorySet* setNamedFactorySet(SkNamedFactorySet*);

    /**
     * Set the flags to be used during flattening.
     */
    void setWriteBufferFlags(uint32_t flags) { fWriteBufferFlags = flags; }

private:
    SkBitmapHeap*       fBitmapHeap;
    SkRefCntSet*        fTypefaceSet;
    SkTypefacePlayback* fTypefacePlayback;
    SkNamedFactorySet*  fFactorySet;
    uint32_t            fWriteBufferFlags;

    typedef SkRefCnt INHERITED;
};

class SkFlatData {
public:
    // Flatten obj into an SkFlatData with this index.  controller owns the SkFlatData*.
    static SkFlatData* Create(SkFlatController* controller,
                              const void* obj,
                              int index,
                              void (*flattenProc)(SkOrderedWriteBuffer&, const void*));

    // Unflatten this into result, using bitmapHeap and facePlayback for bitmaps and fonts if given.
    void unflatten(void* result,
                   void (*unflattenProc)(SkOrderedReadBuffer&, void*),
                   SkBitmapHeap* bitmapHeap = NULL,
                   SkTypefacePlayback* facePlayback = NULL) const;

    // Do these contain the same data?  Ignores index() and topBot().
    bool operator==(const SkFlatData& that) const {
        if (this->checksum() != that.checksum() || this->flatSize() != that.flatSize()) {
            return false;
        }
        return memcmp(this->data(), that.data(), this->flatSize()) == 0;
    }

    int index() const { return fIndex; }
    const uint8_t* data() const { return (const uint8_t*)this + sizeof(*this); }
    size_t flatSize() const { return fFlatSize; }
    uint32_t checksum() const { return fChecksum; }

    // Returns true if fTopBot[] has been recorded.
    bool isTopBotWritten() const {
        return !SkScalarIsNaN(fTopBot[0]);
    }

    // Returns fTopBot array, so it can be passed to a routine to compute them.
    // For efficiency, we assert that fTopBot have not been recorded yet.
    SkScalar* writableTopBot() const {
        SkASSERT(!this->isTopBotWritten());
        return fTopBot;
    }

    // Return the topbot[] after it has been recorded.
    const SkScalar* topBot() const {
        SkASSERT(this->isTopBotWritten());
        return fTopBot;
    }

private:
    // For SkTDynamicHash.
    static const SkFlatData& Identity(const SkFlatData& flat) { return flat; }
    static uint32_t Hash(const SkFlatData& flat) { return flat.checksum(); }
    static bool Equal(const SkFlatData& a, const SkFlatData& b) { return a == b; }

    void setIndex(int index) { fIndex = index; }
    uint8_t* data() { return (uint8_t*)this + sizeof(*this); }

    // This assumes the payload flat data has already been written and does not modify it.
    void stampHeader(int index, int32_t size) {
        SkASSERT(SkIsAlign4(size));
        fIndex     = index;
        fFlatSize  = size;
        fTopBot[0] = SK_ScalarNaN;  // Mark as unwritten.
        fChecksum  = SkChecksum::Compute((uint32_t*)this->data(), size);
    }

    int fIndex;
    int32_t fFlatSize;
    uint32_t fChecksum;
    mutable SkScalar fTopBot[2];  // Cache of FontMetrics fTop, fBottom.  Starts as [NaN,?].
    // uint32_t flattenedData[] implicitly hangs off the end.

    template <class T> friend class SkFlatDictionary;
};

template <class T>
class SkFlatDictionary {
    static const size_t kWriteBufferGrowthBytes = 1024;

public:
    SkFlatDictionary(SkFlatController* controller, size_t scratchSizeGuess = 0)
    : fFlattenProc(NULL)
    , fUnflattenProc(NULL)
    , fController(SkRef(controller))
    , fScratchSize(scratchSizeGuess)
    , fScratch(AllocScratch(fScratchSize))
    , fWriteBuffer(kWriteBufferGrowthBytes)
    , fWriteBufferReady(false) {
        this->reset();
    }

    /**
     * Clears the dictionary of all entries. However, it does NOT free the
     * memory that was allocated for each entry (that's owned by controller).
     */
    void reset() {
        fIndexedData.rewind();
        // TODO(mtklein): There's no reason to have the index start from 1.  Clean this up.
        // index 0 is always empty since it is used as a signal that find failed
        fIndexedData.push(NULL);
        fNextIndex = 1;
    }

    ~SkFlatDictionary() {
        sk_free(fScratch);
    }

    int count() const {
        SkASSERT(fIndexedData.count() == fNextIndex);
        SkASSERT(fHash.count() == fNextIndex - 1);
        return fNextIndex - 1;
    }

    // For testing only.  Index is zero-based.
    const SkFlatData* operator[](int index) {
        return fIndexedData[index+1];
    }

    /**
     * Given an element of type T return its 1-based index in the dictionary. If
     * the element wasn't previously in the dictionary it is automatically
     * added.
     *
     */
    int find(const T& element) {
        return this->findAndReturnFlat(element)->index();
    }

    /**
     * Similar to find. Allows the caller to specify an SkFlatData to replace in
     * the case of an add. Also tells the caller whether a new SkFlatData was
     * added and whether the old one was replaced. The parameters added and
     * replaced are required to be non-NULL. Rather than returning the index of
     * the entry in the dictionary, it returns the actual SkFlatData.
     */
    const SkFlatData* findAndReplace(const T& element,
                                     const SkFlatData* toReplace,
                                     bool* added,
                                     bool* replaced) {
        SkASSERT(added != NULL && replaced != NULL);

        const int oldCount = this->count();
        SkFlatData* flat = this->findAndReturnMutableFlat(element);
        *added = this->count() > oldCount;

        // If we don't want to replace anything, we're done.
        if (!*added || toReplace == NULL) {
            *replaced = false;
            return flat;
        }

        // If we don't have the thing to replace, we're done.
        const SkFlatData* found = fHash.find(*toReplace);
        if (found == NULL) {
            *replaced = false;
            return flat;
        }

        // findAndReturnMutableFlat gave us index (fNextIndex-1), but we'll use the old one.
        fIndexedData.remove(flat->index());
        fNextIndex--;
        flat->setIndex(found->index());
        fIndexedData[flat->index()] = flat;

        // findAndReturnMutableFlat already called fHash.add(), so we just clean up the old entry.
        fHash.remove(*found);
        fController->unalloc((void*)found);
        SkASSERT(this->count() == oldCount);

        *replaced = true;
        return flat;
    }

    /**
     *  Unflatten the objects and return them in SkTRefArray, or return NULL
     *  if there no objects.  Caller takes ownership of result.
     */
    SkTRefArray<T>* unflattenToArray() const {
        const int count = this->count();
        if (count == 0) {
            return NULL;
        }
        SkTRefArray<T>* array = SkTRefArray<T>::Create(count);
        for (int i = 0; i < count; i++) {
            this->unflatten(&array->writableAt(i), fIndexedData[i+1]);
        }
        return array;
    }

    /**
     * Unflatten the specific object at the given index.
     * Caller takes ownership of the result.
     */
    T* unflatten(int index) const {
        const SkFlatData* element = fIndexedData[index];
        SkASSERT(index == element->index());

        T* dst = new T;
        this->unflatten(dst, element);
        return dst;
    }

    /**
     * Find or insert a flattened version of element into the dictionary.
     * Caller does not take ownership of the result.  This will not return NULL.
     */
    const SkFlatData* findAndReturnFlat(const T& element) {
        return this->findAndReturnMutableFlat(element);
    }

protected:
    void (*fFlattenProc)(SkOrderedWriteBuffer&, const void*);
    void (*fUnflattenProc)(SkOrderedReadBuffer&, void*);

private:
    // Layout: [ SkFlatData header, 20 bytes ] [ data ..., 4-byte aligned ]
    static size_t SizeWithPadding(size_t flatDataSize) {
        SkASSERT(SkIsAlign4(flatDataSize));
        return sizeof(SkFlatData) + flatDataSize;
    }

    // Allocate a new scratch SkFlatData.  Must be sk_freed.
    static SkFlatData* AllocScratch(size_t scratchSize) {
        return (SkFlatData*) sk_malloc_throw(SizeWithPadding(scratchSize));
    }

    // We have to delay fWriteBuffer's initialization until its first use; fController might not
    // be fully set up by the time we get it in the constructor.
    void lazyWriteBufferInit() {
        if (fWriteBufferReady) {
            return;
        }
        // Without a bitmap heap, we'll flatten bitmaps into paints.  That's never what you want.
        SkASSERT(fController->getBitmapHeap() != NULL);
        fWriteBuffer.setBitmapHeap(fController->getBitmapHeap());
        fWriteBuffer.setTypefaceRecorder(fController->getTypefaceSet());
        fWriteBuffer.setNamedFactoryRecorder(fController->getNamedFactorySet());
        fWriteBuffer.setFlags(fController->getWriteBufferFlags());
        fWriteBufferReady = true;
    }

    // As findAndReturnFlat, but returns a mutable pointer for internal use.
    SkFlatData* findAndReturnMutableFlat(const T& element) {
        // Only valid until the next call to resetScratch().
        const SkFlatData& scratch = this->resetScratch(element, fNextIndex);

        SkFlatData* candidate = fHash.find(scratch);
        if (candidate != NULL) return candidate;

        SkFlatData* detached = this->detachScratch();
        fHash.add(detached);
        *fIndexedData.insert(fNextIndex) = detached;
        fNextIndex++;
        return detached;
    }

    // This reference is valid only until the next call to resetScratch() or detachScratch().
    const SkFlatData& resetScratch(const T& element, int index) {
        this->lazyWriteBufferInit();

        // Flatten element into fWriteBuffer (using fScratch as storage).
        fWriteBuffer.reset(fScratch->data(), fScratchSize);
        fFlattenProc(fWriteBuffer, &element);
        const size_t bytesWritten = fWriteBuffer.bytesWritten();

        // If all the flattened bytes fit into fScratch, we can skip a call to writeToMemory.
        if (!fWriteBuffer.wroteOnlyToStorage()) {
            SkASSERT(bytesWritten > fScratchSize);
            // It didn't all fit.  Copy into a larger replacement SkFlatData.
            // We can't just realloc because it might move the pointer and confuse writeToMemory.
            SkFlatData* larger = AllocScratch(bytesWritten);
            fWriteBuffer.writeToMemory(larger->data());

            // Carry on with this larger scratch to minimize the likelihood of future resizing.
            sk_free(fScratch);
            fScratchSize = bytesWritten;
            fScratch = larger;
        }

        // The data is in fScratch now but we need to stamp its header.
        fScratch->stampHeader(index, bytesWritten);
        return *fScratch;
    }

    // This result is owned by fController and lives as long as it does (unless unalloc'd).
    SkFlatData* detachScratch() {
        // Allocate a new SkFlatData exactly big enough to hold our current scratch.
        // We use the controller for this allocation to extend the allocation's lifetime and allow
        // the controller to do whatever memory management it wants.
        const size_t paddedSize = SizeWithPadding(fScratch->flatSize());
        SkFlatData* detached = (SkFlatData*)fController->allocThrow(paddedSize);

        // Copy scratch into the new SkFlatData.
        memcpy(detached, fScratch, paddedSize);

        // We can now reuse fScratch, and detached will live until fController dies.
        return detached;
    }

    void unflatten(T* dst, const SkFlatData* element) const {
        element->unflatten(dst,
                           fUnflattenProc,
                           fController->getBitmapHeap(),
                           fController->getTypefacePlayback());
    }

    // All SkFlatData* stored in fIndexedData and fHash are owned by the controller.
    SkAutoTUnref<SkFlatController> fController;
    size_t fScratchSize;  // How many bytes fScratch has allocated for data itself.
    SkFlatData* fScratch;  // Owned, must be freed with sk_free.
    SkOrderedWriteBuffer fWriteBuffer;
    bool fWriteBufferReady;

    // We map between SkFlatData and a 1-based integer index.
    int fNextIndex;

    // For index -> SkFlatData.  fIndexedData[0] is always NULL.
    SkTDArray<const SkFlatData*> fIndexedData;

    // For SkFlatData -> cached SkFlatData, which has index().
    SkTDynamicHash<SkFlatData, SkFlatData,
                   SkFlatData::Identity, SkFlatData::Hash, SkFlatData::Equal> fHash;
};

///////////////////////////////////////////////////////////////////////////////
// Some common dictionaries are defined here for both reference and convenience
///////////////////////////////////////////////////////////////////////////////

template <class T>
static void SkFlattenObjectProc(SkOrderedWriteBuffer& buffer, const void* obj) {
    ((T*)obj)->flatten(buffer);
}

template <class T>
static void SkUnflattenObjectProc(SkOrderedReadBuffer& buffer, void* obj) {
    ((T*)obj)->unflatten(buffer);
}

class SkChunkFlatController : public SkFlatController {
public:
    SkChunkFlatController(size_t minSize)
    : fHeap(minSize)
    , fTypefaceSet(SkNEW(SkRefCntSet))
    , fLastAllocated(NULL) {
        this->setTypefaceSet(fTypefaceSet);
        this->setTypefacePlayback(&fTypefacePlayback);
    }

    virtual void* allocThrow(size_t bytes) SK_OVERRIDE {
        fLastAllocated = fHeap.allocThrow(bytes);
        return fLastAllocated;
    }

    virtual void unalloc(void* ptr) SK_OVERRIDE {
        // fHeap can only free a pointer if it was the last one allocated.  Otherwise, we'll just
        // have to wait until fHeap is destroyed.
        if (ptr == fLastAllocated) (void)fHeap.unalloc(ptr);
    }

    void setupPlaybacks() const {
        fTypefacePlayback.reset(fTypefaceSet.get());
    }

    void setBitmapStorage(SkBitmapHeap* heap) {
        this->setBitmapHeap(heap);
    }

private:
    SkChunkAlloc               fHeap;
    SkAutoTUnref<SkRefCntSet>  fTypefaceSet;
    void*                      fLastAllocated;
    mutable SkTypefacePlayback fTypefacePlayback;
};

class SkMatrixDictionary : public SkFlatDictionary<SkMatrix> {
 public:
    // All matrices fit in 36 bytes.
    SkMatrixDictionary(SkFlatController* controller)
    : SkFlatDictionary<SkMatrix>(controller, 36) {
        fFlattenProc = &flattenMatrix;
        fUnflattenProc = &unflattenMatrix;
    }

    static void flattenMatrix(SkOrderedWriteBuffer& buffer, const void* obj) {
        buffer.getWriter32()->writeMatrix(*(SkMatrix*)obj);
    }

    static void unflattenMatrix(SkOrderedReadBuffer& buffer, void* obj) {
        buffer.getReader32()->readMatrix((SkMatrix*)obj);
    }
};

class SkPaintDictionary : public SkFlatDictionary<SkPaint> {
 public:
    // The largest paint across ~60 .skps was 500 bytes.
    SkPaintDictionary(SkFlatController* controller)
    : SkFlatDictionary<SkPaint>(controller, 512) {
        fFlattenProc = &SkFlattenObjectProc<SkPaint>;
        fUnflattenProc = &SkUnflattenObjectProc<SkPaint>;
    }
};

class SkRegionDictionary : public SkFlatDictionary<SkRegion> {
 public:
    SkRegionDictionary(SkFlatController* controller)
    : SkFlatDictionary<SkRegion>(controller) {
        fFlattenProc = &flattenRegion;
        fUnflattenProc = &unflattenRegion;
    }

    static void flattenRegion(SkOrderedWriteBuffer& buffer, const void* obj) {
        buffer.getWriter32()->writeRegion(*(SkRegion*)obj);
    }

    static void unflattenRegion(SkOrderedReadBuffer& buffer, void* obj) {
        buffer.getReader32()->readRegion((SkRegion*)obj);
    }
};

#endif
