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

#include <functional>
#include <type_traits>
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
        void* buf = this->reserve(sizeof(T), alignof(T), DefaultDestructor<T>);
        return new (buf) T(std::forward<Args>(args)...);
    }

    /*
     * Create a new object of size using initer to initialize the memory. The initer function has
     * the signature T* initer(void* storage). If initer is unable to initialize the memory it
     * should return nullptr where SkSmallAllocator will free the memory.
     */
    template <typename Initer>
    auto createWithIniter(size_t size, Initer initer) -> decltype(initer(nullptr)) {
        using ObjType = typename std::remove_pointer<decltype(initer(nullptr))>::type;
        SkASSERT(size >= sizeof(ObjType));

        // Just use 16 because ObjType is sometimes abstract.
        void* storage = this->reserve(size, 16, Default16Destructor<ObjType>);
        auto candidate = initer(storage);
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
    using Destructor = void(*)(char*);
    struct Rec {
        char*      fObj;
        Destructor fDestructor;
    };

    static char* AlignPtr(char* ptr, uint32_t alignment) {
        uintptr_t mask = alignment - 1;
        return (char*)(((uintptr_t)ptr + mask) & ~mask);
    }

    // Used to call the destructor for allocated objects.
    template<typename T>
    static void DefaultDestructor(char* ptr) {
        ((T*)(AlignPtr(ptr, alignof(T))))->~T();
    }

    // Used to call the destructor for allocated objects.
    template<typename T>
    static void Default16Destructor(char* ptr) {
        ((T*)(AlignPtr(ptr, 16)))->~T();
    }

    // Reserve storageRequired from fStorage if possible otherwise allocate on the heap.
    char* reserve(size_t storageRequired, uint32_t alignment, Destructor destructor) {
        // Make sure that all allocations stay aligned by rounding the storageRequired up to the
        // aligned value.
        char* objectStart = AlignPtr(fStorageEnd, alignment);
        char* objectEnd = objectStart + storageRequired;
        Rec& rec = fRecs.push_back();
        if (objectEnd > &fStorage[kTotalBytes]) {
            // Allocate on the heap. Ideally we want to avoid this situation.
            rec.fObj = new char [storageRequired + (alignment - 1)];
            objectStart = AlignPtr(rec.fObj, alignment);
        } else {
            // There is space in fStorage.
            rec.fObj = objectStart;
            fStorageEnd = objectEnd;
        }
        rec.fDestructor = destructor;
        return objectStart;
    }

    void freeLast() {
        Rec& rec = fRecs.back();
        if (std::less<char*>()(rec.fObj, fStorage)
            || !std::less<char*>()(rec.fObj, &fStorage[kTotalBytes])) {
            delete [] rec.fObj;
        } else {
            fStorageEnd = rec.fObj;
        }
        fRecs.pop_back();
    }

    SkSTArray<kExpectedObjects, Rec, true> fRecs;
    char*                                  fStorageEnd {fStorage};
    char                                   fStorage[kTotalBytes];
};

#endif // SkSmallAllocator_DEFINED
