
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBitmapHeap_DEFINED
#define SkBitmapHeap_DEFINED

#include "SkBitmap.h"
#include "SkFlattenable.h"
#include "SkRefCnt.h"
#include "SkTDArray.h"
#include "SkThread.h"
#include "SkTRefArray.h"

/**
 * SkBitmapHeapEntry provides users of SkBitmapHeap (using internal storage) with a means to...
 *  (1) get access a bitmap in the heap
 *  (2) indicate they are done with bitmap by releasing their reference (if they were an owner).
 */
class SkBitmapHeapEntry : SkNoncopyable {
public:
    ~SkBitmapHeapEntry();

    int32_t getSlot() { return fSlot; }

    SkBitmap* getBitmap() { return &fBitmap; }

    void releaseRef() {
        sk_atomic_dec(&fRefCount);
    }

private:
    SkBitmapHeapEntry();

    void addReferences(int count);

    int32_t fSlot;
    int32_t fRefCount;

    SkBitmap fBitmap;
    // Keep track of the bytes allocated for this bitmap. When replacing the
    // bitmap or removing this HeapEntry we know how much memory has been
    // reclaimed.
    size_t fBytesAllocated;

    friend class SkBitmapHeap;
    friend class SkBitmapHeapTester;
};


class SkBitmapHeapReader : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkBitmapHeapReader)

    SkBitmapHeapReader() : INHERITED() {}
    virtual SkBitmap* getBitmap(int32_t slot) const = 0;
    virtual void releaseRef(int32_t slot) = 0;
private:
    typedef SkRefCnt INHERITED;
};


/**
 * TODO: stores immutable bitmaps into a heap
 */
class SkBitmapHeap : public SkBitmapHeapReader {
public:
    class ExternalStorage : public SkRefCnt {
     public:
        SK_DECLARE_INST_COUNT(ExternalStorage)

        virtual bool insert(const SkBitmap& bitmap, int32_t slot) = 0;

     private:
        typedef SkRefCnt INHERITED;
    };

    static const int32_t UNLIMITED_SIZE = -1;
    static const int32_t IGNORE_OWNERS  = -1;
    static const int32_t INVALID_SLOT   = -1;

    /**
     * Constructs a heap that is responsible for allocating and managing its own storage.  In the
     * case where we choose to allow the heap to grow indefinitely (i.e. UNLIMITED_SIZE) we
     * guarantee that once allocated in the heap a bitmap's index in the heap is immutable.
     * Otherwise we guarantee the bitmaps placement in the heap until its owner count goes to zero.
     *
     * @param preferredSize  Specifies the preferred maximum number of bitmaps to store. This is
     *   not a hard limit as it can grow larger if the number of bitmaps in the heap with active
     *   owners exceeds this limit.
     * @param ownerCount  The number of owners to assign to each inserted bitmap. NOTE: while a
     *   bitmap in the heap has a least one owner it can't be removed.
     */
    SkBitmapHeap(int32_t preferredSize = UNLIMITED_SIZE, int32_t ownerCount = IGNORE_OWNERS);

    /**
     * Constructs a heap that defers the responsibility of storing the bitmaps to an external
     * function. This is especially useful if the bitmaps will be used in a separate process as the
     * external storage can ensure the data is properly shuttled to the appropriate processes.
     *
     * Our LRU implementation assumes that inserts into the external storage are consumed in the
     * order that they are inserted (i.e. SkPipe). This ensures that we don't need to query the
     * external storage to see if a slot in the heap is eligible to be overwritten.
     *
     * @param externalStorage  The class responsible for storing the bitmaps inserted into the heap
     * @param heapSize  The maximum size of the heap. Because of the sequential limitation imposed
     *   by our LRU implementation we can guarantee that the heap will never grow beyond this size.
     */
    SkBitmapHeap(ExternalStorage* externalStorage, int32_t heapSize = UNLIMITED_SIZE);

    ~SkBitmapHeap();

    /**
     * Makes a shallow copy of all bitmaps currently in the heap and returns them as an array. The
     * array indices match their position in the heap.
     *
     * @return  a ptr to an array of bitmaps or NULL if external storage is being used.
     */
    SkTRefArray<SkBitmap>* extractBitmaps() const;

    /**
     * Retrieves the bitmap from the specified slot in the heap
     *
     * @return  The bitmap located at that slot or NULL if external storage is being used.
     */
    virtual SkBitmap* getBitmap(int32_t slot) const SK_OVERRIDE {
        SkASSERT(fExternalStorage == NULL);
        SkBitmapHeapEntry* entry = getEntry(slot);
        if (entry) {
            return &entry->fBitmap;
        }
        return NULL;
    }

    /**
     * Retrieves the bitmap from the specified slot in the heap
     *
     * @return  The bitmap located at that slot or NULL if external storage is being used.
     */
    virtual void releaseRef(int32_t slot) SK_OVERRIDE {
        SkASSERT(fExternalStorage == NULL);
        if (fOwnerCount != IGNORE_OWNERS) {
            SkBitmapHeapEntry* entry = getEntry(slot);
            if (entry) {
                entry->releaseRef();
            }
        }
    }

    /**
     * Inserts a bitmap into the heap. The stored version of bitmap is guaranteed to be immutable
     * and is not dependent on the lifecycle of the provided bitmap.
     *
     * @param bitmap  the bitmap to be inserted into the heap
     * @return  the slot in the heap where the bitmap is stored or INVALID_SLOT if the bitmap could
     *          not be added to the heap. If it was added the slot will remain valid...
     *            (1) indefinitely if no owner count has been specified.
     *            (2) until all owners have called releaseRef on the appropriate SkBitmapHeapEntry*
     */
    int32_t insert(const SkBitmap& bitmap);

    /**
     * Retrieves an entry from the heap at a given slot.
     *
     * @param slot  the slot in the heap where a bitmap was stored.
     * @return  a SkBitmapHeapEntry that wraps the bitmap or NULL if external storage is used.
     */
    SkBitmapHeapEntry* getEntry(int32_t slot) const {
        SkASSERT(slot <= fStorage.count());
        if (fExternalStorage != NULL) {
            return NULL;
        }
        return fStorage[slot];
    }

    /**
     * Returns a count of the number of items currently in the heap
     */
    int count() const {
        SkASSERT(fExternalStorage != NULL ||
                 fStorage.count() - fUnusedSlots.count() == fLookupTable.count());
        return fLookupTable.count();
    }

    /**
     * Returns the total number of bytes allocated by the bitmaps in the heap
     */
    size_t bytesAllocated() const {
        return fBytesAllocated;
    }

    /**
     * Attempt to reduce the storage allocated.
     * @param bytesToFree minimum number of bytes that should be attempted to
     *   be freed.
     * @return number of bytes actually freed.
     */
    size_t freeMemoryIfPossible(size_t bytesToFree);

    /**
     * Defer any increments of owner counts until endAddingOwnersDeferral is called. So if an
     * existing SkBitmap is inserted into the SkBitmapHeap, its corresponding SkBitmapHeapEntry will
     * not have addReferences called on it, and the client does not need to make a corresponding
     * call to releaseRef. Only meaningful if this SkBitmapHeap was created with an owner count not
     * equal to IGNORE_OWNERS.
     */
    void deferAddingOwners();

    /**
     * Resume adding references when duplicate SkBitmaps are inserted.
     * @param add If true, add references to the SkBitmapHeapEntrys whose SkBitmaps were re-inserted
     *            while deferring.
     */
    void endAddingOwnersDeferral(bool add);

private:
    struct LookupEntry {
        LookupEntry(const SkBitmap& bm)
        : fGenerationId(bm.getGenerationID())
        , fPixelOffset(bm.pixelRefOffset())
        , fWidth(bm.width())
        , fHeight(bm.height())
        , fMoreRecentlyUsed(NULL)
        , fLessRecentlyUsed(NULL){}

        const uint32_t fGenerationId; // SkPixelRef GenerationID.
        const size_t   fPixelOffset;
        const uint32_t fWidth;
        const uint32_t fHeight;

        // TODO: Generalize the LRU caching mechanism
        LookupEntry* fMoreRecentlyUsed;
        LookupEntry* fLessRecentlyUsed;

        uint32_t fStorageSlot; // slot of corresponding bitmap in fStorage.

        /**
         * Compare two LookupEntry pointers, returning -1, 0, 1 for sorting.
         */
        static int Compare(const LookupEntry* a, const LookupEntry* b);
    };

    /**
     * Remove the entry from the lookup table. Also deletes the entry pointed
     * to by the table. Therefore, if a pointer to that one was passed in, the
     * pointer should no longer be used, since the object to which it points has
     * been deleted.
     * @return The index in the lookup table of the entry before removal.
     */
    int removeEntryFromLookupTable(LookupEntry*);

    /**
     * Searches for the bitmap in the lookup table and returns the bitmaps index within the table.
     * If the bitmap was not already in the table it is added.
     *
     * @param key    The key to search the lookup table, created from a bitmap.
     * @param entry  A pointer to a SkBitmapHeapEntry* that if non-null AND the bitmap is found
     *               in the lookup table is populated with the entry from the heap storage.
     */
    int findInLookupTable(const LookupEntry& key, SkBitmapHeapEntry** entry);

    LookupEntry* findEntryToReplace(const SkBitmap& replacement);
    bool copyBitmap(const SkBitmap& originalBitmap, SkBitmap& copiedBitmap);

    /**
     * Remove a LookupEntry from the LRU, in preparation for either deleting or appending as most
     * recent. Points the LookupEntry's old neighbors at each other, and sets fLeastRecentlyUsed
     * (if there is still an entry left). Sets LookupEntry's fMoreRecentlyUsed to NULL and leaves
     * its fLessRecentlyUsed unmodified.
     */
    void removeFromLRU(LookupEntry* entry);

    /**
     * Append a LookupEntry to the end of the LRU cache, marking it as the most
     * recently used. Assumes that the LookupEntry is already in fLookupTable,
     * but is not in the LRU cache. If it is in the cache, removeFromLRU should
     * be called first.
     */
    void appendToLRU(LookupEntry*);

    // searchable index that maps to entries in the heap
    SkTDArray<LookupEntry*> fLookupTable;

    // heap storage
    SkTDArray<SkBitmapHeapEntry*> fStorage;
    // Used to mark slots in fStorage as deleted without actually deleting
    // the slot so as not to mess up the numbering.
    SkTDArray<int> fUnusedSlots;
    ExternalStorage* fExternalStorage;

    LookupEntry* fMostRecentlyUsed;
    LookupEntry* fLeastRecentlyUsed;

    const int32_t fPreferredCount;
    const int32_t fOwnerCount;
    size_t fBytesAllocated;

    bool fDeferAddingOwners;
    SkTDArray<int> fDeferredEntries;

    typedef SkBitmapHeapReader INHERITED;
};

#endif // SkBitmapHeap_DEFINED
