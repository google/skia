/*
 * Copyright 2014 Google, Inc
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSmallAllocator_DEFINED
#define SkSmallAllocator_DEFINED

#include "SkTDArray.h"
#include "SkTypes.h"

#include <new>

/*
 *  Template class for allocating small objects without additional heap memory
 *  allocations. kMaxObjects is a hard limit on the number of objects that can
 *  be allocated using this class. After that, attempts to create more objects
 *  with this class will assert and return nullptr.
 *
 *  kTotalBytes is the total number of bytes provided for storage for all
 *  objects created by this allocator. If an object to be created is larger
 *  than the storage (minus storage already used), it will be allocated on the
 *  heap. This class's destructor will handle calling the destructor for each
 *  object it allocated and freeing its memory.
 *
 *  Current the class always aligns each allocation to 16-bytes to be safe, but future
 *  may reduce this to only the alignment that is required per alloc.
 */
template<uint32_t kMaxObjects, size_t kTotalBytes>
class SkSmallAllocator : SkNoncopyable {
public:
    SkSmallAllocator()
    : fStorageUsed(0)
    , fNumObjects(0)
    {}

    ~SkSmallAllocator() {
        // Destruct in reverse order, in case an earlier object points to a
        // later object.
        while (fNumObjects > 0) {
            fNumObjects--;
            Rec* rec = &fRecs[fNumObjects];
            rec->fKillProc(rec->fObj);
            // Safe to do if fObj is in fStorage, since fHeapStorage will
            // point to nullptr.
            sk_free(rec->fHeapStorage);
        }
    }

    /*
     *  Create a new object of type T. Its lifetime will be handled by this
     *  SkSmallAllocator.
     *  Note: If kMaxObjects have been created by this SkSmallAllocator, nullptr
     *  will be returned.
     */
    template<typename T, typename... Args>
    T* createT(const Args&... args) {
        void* buf = this->reserveT<T>();
        if (nullptr == buf) {
            return nullptr;
        }
        return new (buf) T(args...);
    }

    /*
     *  Reserve a specified amount of space (must be enough space for one T).
     *  The space will be in fStorage if there is room, or on the heap otherwise.
     *  Either way, this class will call ~T() in its destructor and free the heap
     *  allocation if necessary.
     *  Unlike createT(), this method will not call the constructor of T.
     */
    template<typename T> void* reserveT(size_t storageRequired = sizeof(T)) {
        SkASSERT(fNumObjects < kMaxObjects);
        SkASSERT(storageRequired >= sizeof(T));
        if (kMaxObjects == fNumObjects) {
            return nullptr;
        }
        const size_t storageRemaining = sizeof(fStorage) - fStorageUsed;
        Rec* rec = &fRecs[fNumObjects];
        if (storageRequired > storageRemaining) {
            // Allocate on the heap. Ideally we want to avoid this situation,
            // but we're not sure we can catch all callers, so handle it but
            // assert false in debug mode.
            SkASSERT(false);
            rec->fStorageSize = 0;
            rec->fHeapStorage = sk_malloc_throw(storageRequired);
            rec->fObj = static_cast<void*>(rec->fHeapStorage);
        } else {
            // There is space in fStorage.
            rec->fStorageSize = storageRequired;
            rec->fHeapStorage = nullptr;
            rec->fObj = static_cast<void*>(fStorage.fBytes + fStorageUsed);
            fStorageUsed += storageRequired;
        }
        rec->fKillProc = DestroyT<T>;
        fNumObjects++;
        return rec->fObj;
    }

    /*
     *  Free the memory reserved last without calling the destructor.
     *  Can be used in a nested way, i.e. after reserving A and B, calling
     *  freeLast once will free B and calling it again will free A.
     */
    void freeLast() {
        SkASSERT(fNumObjects > 0);
        Rec* rec = &fRecs[fNumObjects - 1];
        sk_free(rec->fHeapStorage);
        fStorageUsed -= rec->fStorageSize;

        fNumObjects--;
    }

private:
    struct Rec {
        size_t fStorageSize;  // 0 if allocated on heap
        void*  fObj;
        void*  fHeapStorage;
        void   (*fKillProc)(void*);
    };

    // Used to call the destructor for allocated objects.
    template<typename T>
    static void DestroyT(void* ptr) {
        static_cast<T*>(ptr)->~T();
    }

    struct SK_STRUCT_ALIGN(16) Storage {
        // we add kMaxObjects * 15 to account for the worst-case slop, where each allocation wasted
        // 15 bytes (due to forcing each to be 16-byte aligned)
        char    fBytes[kTotalBytes + kMaxObjects * 15];
    };

    Storage     fStorage;
    // Number of bytes used so far.
    size_t      fStorageUsed;
    uint32_t    fNumObjects;
    Rec         fRecs[kMaxObjects];
};

#endif // SkSmallAllocator_DEFINED
