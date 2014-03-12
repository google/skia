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

// Used by SkSmallAllocator to call the destructor for objects it has
// allocated.
template<typename T> void destroyT(void* ptr) {
   static_cast<T*>(ptr)->~T();
}

/*
 *  Template class for allocating small objects without additional heap memory
 *  allocations. kMaxObjects is a hard limit on the number of objects that can
 *  be allocated using this class. After that, attempts to create more objects
 *  with this class will assert and return NULL.
 *  kTotalBytes is the total number of bytes provided for storage for all
 *  objects created by this allocator. If an object to be created is larger
 *  than the storage (minus storage already used), it will be allocated on the
 *  heap. This class's destructor will handle calling the destructor for each
 *  object it allocated and freeing its memory.
 */
template<uint32_t kMaxObjects, size_t kTotalBytes>
class SkSmallAllocator : public SkNoncopyable {
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
            // point to NULL.
            sk_free(rec->fHeapStorage);
        }
    }

    /*
     *  Create a new object of type T. Its lifetime will be handled by this
     *  SkSmallAllocator.
     *  Each version behaves the same but takes a different number of
     *  arguments.
     *  Note: If kMaxObjects have been created by this SkSmallAllocator, NULL
     *  will be returned.
     */
    template<typename T>
    T* createT() {
        void* buf = this->reserveT<T>();
        if (NULL == buf) {
            return NULL;
        }
        SkNEW_PLACEMENT(buf, T);
        return static_cast<T*>(buf);
    }

    template<typename T, typename A1> T* createT(const A1& a1) {
        void* buf = this->reserveT<T>();
        if (NULL == buf) {
            return NULL;
        }
        SkNEW_PLACEMENT_ARGS(buf, T, (a1));
        return static_cast<T*>(buf);
    }

    template<typename T, typename A1, typename A2>
    T* createT(const A1& a1, const A2& a2) {
        void* buf = this->reserveT<T>();
        if (NULL == buf) {
            return NULL;
        }
        SkNEW_PLACEMENT_ARGS(buf, T, (a1, a2));
        return static_cast<T*>(buf);
    }

    template<typename T, typename A1, typename A2, typename A3>
    T* createT(const A1& a1, const A2& a2, const A3& a3) {
        void* buf = this->reserveT<T>();
        if (NULL == buf) {
            return NULL;
        }
        SkNEW_PLACEMENT_ARGS(buf, T, (a1, a2, a3));
        return static_cast<T*>(buf);
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
            return NULL;
        }
        const size_t storageRemaining = SkAlign4(kTotalBytes) - fStorageUsed;
        storageRequired = SkAlign4(storageRequired);
        Rec* rec = &fRecs[fNumObjects];
        if (storageRequired > storageRemaining) {
            // Allocate on the heap. Ideally we want to avoid this situation,
            // but we're not sure we can catch all callers, so handle it but
            // assert false in debug mode.
            SkASSERT(false);
            rec->fHeapStorage = sk_malloc_throw(storageRequired);
            rec->fObj = static_cast<void*>(rec->fHeapStorage);
        } else {
            // There is space in fStorage.
            rec->fHeapStorage = NULL;
            SkASSERT(SkIsAlign4(fStorageUsed));
            rec->fObj = static_cast<void*>(fStorage + (fStorageUsed / 4));
            fStorageUsed += storageRequired;
        }
        rec->fKillProc = destroyT<T>;
        fNumObjects++;
        return rec->fObj;
    }

private:
    struct Rec {
        void* fObj;
        void* fHeapStorage;
        void  (*fKillProc)(void*);
    };

    // Number of bytes used so far.
    size_t              fStorageUsed;
    // Pad the storage size to be 4-byte aligned.
    uint32_t            fStorage[SkAlign4(kTotalBytes) >> 2];
    uint32_t            fNumObjects;
    Rec                 fRecs[kMaxObjects];
};

#endif // SkSmallAllocator_DEFINED
