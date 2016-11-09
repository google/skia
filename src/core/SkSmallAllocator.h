/*
 * Copyright 2014 Google, Inc
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSmallAllocator_DEFINED
#define SkSmallAllocator_DEFINED

#include "SkTArray.h"
#include "SkTypes.h"

#include <utility>

/*
 *  Template class for allocating small objects without additional heap memory
 *  allocations.
 *
 *  kTotalBytes is the total number of bytes provided for storage for all
 *  objects created by this allocator. If an object to be created is larger
 *  than the storage (minus storage already used), it will be allocated on the
 *  heap. This class's destructor will handle calling the destructor for each
 *  object it allocated and freeing its memory.
 */
template<uint32_t kExpectedObjects, size_t kTotalBytes>
class SkSmallAllocator : SkNoncopyable {
public:
    ~SkSmallAllocator() {
        // Destruct in reverse order, in case an earlier object points to a
        // later object.
        while (fRecs.count() > 0) {
            this->deleteLast();
        }
    }

    /*
     *  Create a new object of type T. Its lifetime will be handled by this
     *  SkSmallAllocator.
     */
    template<typename T, typename... Args>
    T* createT(Args&&... args) {
        void* buf = this->reserve(sizeof(T), DefaultDestructor<T>);
        return new (buf) T(std::forward<Args>(args)...);
    }

    /*
     * Create a new object of size using initer to initialize the memory. The initer function has
     * the signature T* initer(void* storage). If initer is unable to initialize the memory it
     * should return nullptr where SkSmallAllocator will free the memory.
     */
    template <typename T, typename Initer>
    T* createWithIniterT(size_t size, Initer initer) {
        SkASSERT(size >= sizeof(T));

        void* storage = this->reserve(size, DefaultDestructor<T>);
        T* candidate = initer(storage);
        if (!candidate) {
            // Initializing didn't workout so free the memory.
            this->freeLast();
        }

        return candidate;
    }

    /*
     * Free the last object allocated and call its destructor. This can be called multiple times
     * removing objects from the pool in reverse order.
     */
    void deleteLast() {
        SkASSERT(fRecs.count() > 0);
        Rec& rec = fRecs.back();
        rec.fDestructor(rec.fObj);
        this->freeLast();
    }

private:
    using Destructor = void(*)(void*);
    struct Rec {
        size_t     fStorageSize;  // 0 if allocated on heap
        char*      fObj;
        Destructor fDestructor;
    };

    // Used to call the destructor for allocated objects.
    template<typename T>
    static void DefaultDestructor(void* ptr) {
        static_cast<T*>(ptr)->~T();
    }

    // Reserve storageRequired from fStorage if possible otherwise allocate on the heap.
    void* reserve(size_t storageRequired, Destructor destructor) {
        const size_t storageRemaining = sizeof(fStorage) - fStorageUsed;
        Rec& rec = fRecs.push_back();
        if (storageRequired > storageRemaining) {
            // Allocate on the heap. Ideally we want to avoid this situation.

            // With the gm composeshader_bitmap2, storage required is 4476
            // and storage remaining is 3392. Increasing the base storage
            // causes google 3 tests to fail.

            rec.fStorageSize = 0;
            rec.fObj = new char [storageRequired];
        } else {
            // There is space in fStorage.
            rec.fStorageSize = storageRequired;
            rec.fObj = &fStorage[fStorageUsed];
            fStorageUsed += storageRequired;
        }
        rec.fDestructor = destructor;
        return rec.fObj;
    }

    void freeLast() {
        Rec& rec = fRecs.back();
        if (0 == rec.fStorageSize) {
            delete [] rec.fObj;
        }
        fStorageUsed -= rec.fStorageSize;
        fRecs.pop_back();
    }

    size_t                                 fStorageUsed {0};  // Number of bytes used so far.
    SkSTArray<kExpectedObjects, Rec, true> fRecs;
    char                                   fStorage[kTotalBytes];
};

#endif // SkSmallAllocator_DEFINED
