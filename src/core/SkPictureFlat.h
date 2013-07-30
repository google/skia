
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPictureFlat_DEFINED
#define SkPictureFlat_DEFINED

//#define SK_DEBUG_SIZE

#include "SkChunkAlloc.h"
#include "SkBitmap.h"
#include "SkBitmapHeap.h"
#include "SkOrderedReadBuffer.h"
#include "SkOrderedWriteBuffer.h"
#include "SkPicture.h"
#include "SkPtrRecorder.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRegion.h"
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
    /**
     *  Compare two SkFlatData ptrs, returning -1, 0, 1 to allow them to be
     *  sorted.
     *
     *  Note: this assumes that a and b have different sentinel values, either
     *  InCache or AsCandidate, otherwise the loop will go beyond the end of
     *  the buffers.
     *
     *  dataToCompare() returns 2 fields before the flattened data:
     *      - checksum
     *      - size
     *  This ensures that if we see two blocks of different length, we will
     *  notice that right away, and not read any further. It also ensures that
     *  we see the checksum right away, so that most of the time it is enough
     *  to short-circuit our comparison.
     */
    static int Compare(const SkFlatData& a, const SkFlatData& b) {
        const uint32_t* stop = a.dataStop();
        const uint32_t* a_ptr = a.dataToCompare() - 1;
        const uint32_t* b_ptr = b.dataToCompare() - 1;
        // We use -1 above, so we can pre-increment our pointers in the loop
        while (*++a_ptr == *++b_ptr) {}

        if (a_ptr == stop) {    // sentinel
            SkASSERT(b.dataStop() == b_ptr);
            return 0;
        }
        SkASSERT(a_ptr < a.dataStop());
        SkASSERT(b_ptr < b.dataStop());
        return (*a_ptr < *b_ptr) ? -1 : 1;
    }

    // Adapts Compare to be used with SkTSearch
    static bool Less(const SkFlatData& a, const SkFlatData& b) {
        return Compare(a, b) < 0;
    }

    int index() const { return fIndex; }
    const void* data() const { return (const char*)this + sizeof(*this); }
    void* data() { return (char*)this + sizeof(*this); }
    // Our data is always 32bit aligned, so we can offer this accessor
    uint32_t* data32() { return (uint32_t*)this->data(); }
    // Returns the size of the flattened data.
    size_t flatSize() const { return fFlatSize; }

    void setSentinelInCache() {
        this->setSentinel(kInCache_Sentinel);
    }
    void setSentinelAsCandidate() {
        this->setSentinel(kCandidate_Sentinel);
    }

    uint32_t checksum() const { return fChecksum; }

#ifdef SK_DEBUG_SIZE
    // returns the logical size of our data. Does not return any sentinel or
    // padding we might have.
    size_t size() const {
        return sizeof(SkFlatData) + fFlatSize;
    }
#endif

    static SkFlatData* Create(SkFlatController* controller, const void* obj, int index,
                              void (*flattenProc)(SkOrderedWriteBuffer&, const void*));

    void unflatten(void* result,
                   void (*unflattenProc)(SkOrderedReadBuffer&, void*),
                   SkBitmapHeap* bitmapHeap = NULL,
                   SkTypefacePlayback* facePlayback = NULL) const;

    // When we purge an entry, we want to reuse an old index for the new entry,
    // so we expose this setter.
    void setIndex(int index) { fIndex = index; }

    // for unittesting
    friend bool operator==(const SkFlatData& a, const SkFlatData& b) {
        size_t N = (const char*)a.dataStop() - (const char*)a.dataToCompare();
        return !memcmp(a.dataToCompare(), b.dataToCompare(), N);
    }

    // returns true if fTopBot[] has been recorded
    bool isTopBotWritten() const {
        return !SkScalarIsNaN(fTopBot[0]);
    }

    // Returns fTopBot array, so it can be passed to a routine to compute them.
    // For efficiency, we assert that fTopBot have not been recorded yet.
    SkScalar* writableTopBot() const {
        SkASSERT(!this->isTopBotWritten());
        return fTopBot;
    }

    // return the topbot[] after it has been recorded
    const SkScalar* topBot() const {
        SkASSERT(this->isTopBotWritten());
        return fTopBot;
    }

private:
    // This is *not* part of the key for search/sort
    int fIndex;

    // Cache of paint's FontMetrics fTop,fBottom
    // initialied to [NaN,NaN] as a sentinel that they have not been recorded yet
    //
    // This is *not* part of the key for search/sort
    mutable SkScalar fTopBot[2];

    // marks fTopBot[] as unrecorded
    void setTopBotUnwritten() {
        this->fTopBot[0] = SK_ScalarNaN; // initial to sentinel values
    }

    // From here down is the data we look at in the search/sort. We always begin
    // with the checksum and then length.
    uint32_t fChecksum;
    int32_t  fFlatSize;  // size of flattened data
    // uint32_t flattenedData[]
    // uint32_t sentinelValue

    const uint32_t* dataToCompare() const {
        return (const uint32_t*)&fChecksum;
    }
    const uint32_t* dataStop() const {
        SkASSERT(SkIsAlign4(fFlatSize));
        return (const uint32_t*)((const char*)this->data() + fFlatSize);
    }

    enum {
        kInCache_Sentinel = 0,
        kCandidate_Sentinel = ~0U,
    };
    void setSentinel(uint32_t value) {
        SkASSERT(SkIsAlign4(fFlatSize));
        this->data32()[fFlatSize >> 2] = value;
    }

    // This does not modify the payload flat data, in case it's already been written.
    void stampHeaderAndSentinel(int index, int32_t size);
    template <class T> friend class SkFlatDictionary;  // For stampHeaderAndSentinel().
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
    , fWriteBufferReady(false)
    , fNextIndex(1)  { // set to 1 since returning a zero from find() indicates failure
        sk_bzero(fHash, sizeof(fHash));
        // index 0 is always empty since it is used as a signal that find failed
        fIndexedData.push(NULL);
    }

    ~SkFlatDictionary() {
        sk_free(fScratch);
    }

    int count() const {
        SkASSERT(fIndexedData.count() == fSortedData.count()+1);
        return fSortedData.count();
    }

    const SkFlatData*  operator[](int index) const {
        SkASSERT(index >= 0 && index < fSortedData.count());
        return fSortedData[index];
    }

    /**
     * Clears the dictionary of all entries. However, it does NOT free the
     * memory that was allocated for each entry.
     */
    void reset() {
        fSortedData.reset();
        fIndexedData.rewind();
        // index 0 is always empty since it is used as a signal that find failed
        fIndexedData.push(NULL);
        fNextIndex = 1;
        sk_bzero(fHash, sizeof(fHash));
    }

    /**
     * Similar to find. Allows the caller to specify an SkFlatData to replace in
     * the case of an add. Also tells the caller whether a new SkFlatData was
     * added and whether the old one was replaced. The parameters added and
     * replaced are required to be non-NULL. Rather than returning the index of
     * the entry in the dictionary, it returns the actual SkFlatData.
     */
    const SkFlatData* findAndReplace(const T& element,
                                     const SkFlatData* toReplace, bool* added,
                                     bool* replaced) {
        SkASSERT(added != NULL && replaced != NULL);
        int oldCount = fSortedData.count();
        const SkFlatData* flat = this->findAndReturnFlat(element);
        *added = fSortedData.count() == oldCount + 1;
        *replaced = false;
        if (*added && toReplace != NULL) {
            // First, find the index of the one to replace
            int indexToReplace = fSortedData.find(toReplace);
            if (indexToReplace >= 0) {
                // findAndReturnFlat set the index to fNextIndex and increased
                // fNextIndex by one. Reuse the index from the one being
                // replaced and reset fNextIndex to the proper value.
                int oldIndex = flat->index();
                const_cast<SkFlatData*>(flat)->setIndex(toReplace->index());
                fIndexedData[toReplace->index()] = flat;
                fNextIndex--;
                // Remove from the arrays.
                fSortedData.remove(indexToReplace);
                fIndexedData.remove(oldIndex);
                // Remove from the hash table.
                int oldHash = ChecksumToHashIndex(toReplace->checksum());
                if (fHash[oldHash] == toReplace) {
                    fHash[oldHash] = NULL;
                }
                // Delete the actual object.
                fController->unalloc((void*)toReplace);
                *replaced = true;
                SkASSERT(fIndexedData.count() == fSortedData.count()+1);
            }
        }
        return flat;
    }

    /**
     * Given an element of type T return its 1-based index in the dictionary. If
     * the element wasn't previously in the dictionary it is automatically
     * added.
     *
     * To make the Compare function fast, we write a sentinel value at the end
     * of each block. The blocks in our fSortedData[] all have a 0 sentinel. The
     * newly created block we're comparing against has a -1 in the sentinel.
     *
     * This trick allows Compare to always loop until failure. If it fails on
     * the sentinal value, we know the blocks are equal.
     */
    int find(const T& element) {
        return this->findAndReturnFlat(element)->index();
    }

    /**
     *  Unflatten the objects and return them in SkTRefArray, or return NULL
     *  if there no objects (instead of an empty array).
     */
    SkTRefArray<T>* unflattenToArray() const {
        int count = fSortedData.count();
        SkTRefArray<T>* array = NULL;
        if (count > 0) {
            array = SkTRefArray<T>::Create(count);
            this->unflattenIntoArray(&array->writableAt(0));
        }
        return array;
    }

    /**
     * Unflatten the specific object at the given index
     */
    T* unflatten(int index) const {
        SkASSERT(fIndexedData.count() == fSortedData.count()+1);
        const SkFlatData* element = fIndexedData[index];
        SkASSERT(index == element->index());

        T* dst = new T;
        this->unflatten(dst, element);
        return dst;
    }

    const SkFlatData* findAndReturnFlat(const T& element) {
        // Only valid until the next call to resetScratch().
        const SkFlatData& scratch = this->resetScratch(element, fNextIndex);

        // See if we have it in the hash?
        const int hashIndex = ChecksumToHashIndex(scratch.checksum());
        const SkFlatData* candidate = fHash[hashIndex];
        if (candidate != NULL && SkFlatData::Compare(scratch, *candidate) == 0) {
            return candidate;
        }

        // See if we have it at all?
        const int index = SkTSearch<const SkFlatData, SkFlatData::Less>(fSortedData.begin(),
                                                                        fSortedData.count(),
                                                                        &scratch,
                                                                        sizeof(&scratch));
        if (index >= 0) {
            // Found.  Update hash before we return.
            fHash[hashIndex] = fSortedData[index];
            return fSortedData[index];
        }

        // We don't have it.  Add it.
        SkFlatData* detached = this->detachScratch();
        // detached will live beyond the next call to resetScratch(), but is owned by fController.
        *fSortedData.insert(~index) = detached;  // SkTSearch returned bit-not of where to insert.
        *fIndexedData.insert(detached->index()) = detached;
        fHash[hashIndex] = detached;

        SkASSERT(detached->index() == fNextIndex);
        SkASSERT(fSortedData.count() == fNextIndex);
        SkASSERT(fIndexedData.count() == fNextIndex+1);
        fNextIndex++;

        return detached;
    }

protected:
    void (*fFlattenProc)(SkOrderedWriteBuffer&, const void*);
    void (*fUnflattenProc)(SkOrderedReadBuffer&, void*);

private:
    // Layout: [ SkFlatData header, 20 bytes ] [ data ..., 4-byte aligned ] [ sentinel, 4 bytes]
    static size_t SizeWithPadding(size_t flatDataSize) {
        SkASSERT(SkIsAlign4(flatDataSize));
        return sizeof(SkFlatData) + flatDataSize + sizeof(uint32_t);
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

        // The data is in fScratch now, but we need to stamp its header and trailing sentinel.
        fScratch->stampHeaderAndSentinel(index, bytesWritten);
        return *fScratch;
    }

    // This result is owned by fController and lives as long as it does (unless unalloc'd).
    SkFlatData* detachScratch() {
        // Allocate a new SkFlatData exactly big enough to hold our current scratch.
        // We use the controller for this allocation to extend the allocation's lifetime and allow
        // the controller to do whatever memory management it wants.
        const size_t paddedSize = SizeWithPadding(fScratch->flatSize());
        SkFlatData* detached = (SkFlatData*)fController->allocThrow(paddedSize);

        // Copy scratch into the new SkFlatData, setting the sentinel for cache storage.
        memcpy(detached, fScratch, paddedSize);
        detached->setSentinelInCache();

        // We can now reuse fScratch, and detached will live until fController dies.
        return detached;
    }

    void unflatten(T* dst, const SkFlatData* element) const {
        element->unflatten(dst, fUnflattenProc,
                           fController->getBitmapHeap(),
                           fController->getTypefacePlayback());
    }

    void unflattenIntoArray(T* array) const {
        const int count = fSortedData.count();
        SkASSERT(fIndexedData.count() == fSortedData.count()+1);
        const SkFlatData* const* iter = fSortedData.begin();
        for (int i = 0; i < count; ++i) {
            const SkFlatData* element = iter[i];
            int index = element->index() - 1;
            SkASSERT((unsigned)index < (unsigned)count);
            unflatten(&array[index], element);
        }
    }

    SkAutoTUnref<SkFlatController> fController;
    size_t fScratchSize;  // How many bytes fScratch has allocated for data itself.
    SkFlatData* fScratch;  // Owned, must be freed with sk_free.
    SkOrderedWriteBuffer fWriteBuffer;
    bool fWriteBufferReady;

    // SkFlatDictionary has two copies of the data one indexed by the
    // SkFlatData's index and the other sorted. The sorted data is used
    // for finding and uniquification while the indexed copy is used
    // for standard array-style lookups based on the SkFlatData's index
    // (as in 'unflatten').
    int fNextIndex;
    SkTDArray<const SkFlatData*> fIndexedData;
    // fSortedData is sorted by checksum/size/data.
    SkTDArray<const SkFlatData*> fSortedData;

    enum {
        // Determined by trying diff values on picture-recording benchmarks
        // (e.g. PictureRecordBench.cpp), choosing the smallest value that
        // showed a big improvement. Even better would be to benchmark diff
        // values on recording representative web-pages or other "real" content.
        HASH_BITS   = 7,
        HASH_MASK   = (1 << HASH_BITS) - 1,
        HASH_COUNT  = 1 << HASH_BITS
    };
    const SkFlatData* fHash[HASH_COUNT];

    static int ChecksumToHashIndex(uint32_t checksum) {
        int n = checksum;
        if (HASH_BITS < 32) {
            n ^= n >> 16;
        }
        if (HASH_BITS < 16) {
            n ^= n >> 8;
        }
        if (HASH_BITS < 8) {
            n ^= n >> 4;
        }
        return n & HASH_MASK;
    }
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
