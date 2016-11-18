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


// max_align_t is needed to calculate the alignment for createWithIniterT when the T used is an
// abstract type. The complication with max_align_t is that it is defined differently for
// different builds.
namespace {
#if defined(SK_BUILD_FOR_WIN32) || defined(SK_BUILD_FOR_MAC)
    // Use std::max_align_t for compiles that follow the standard.
    #include <cstddef>
    using SystemAlignment = std::max_align_t;
#else
    // Ubuntu compiles don't have std::max_align_t defined, but MSVC does not define max_align_t.
    #include <stddef.h>
    using SystemAlignment = max_align_t;
#endif
}

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
    template <typename Initer>
    auto createWithIniter(size_t size, Initer initer) -> decltype(initer(nullptr)) {
        using ObjType = typename std::remove_pointer<decltype(initer(nullptr))>::type;
        SkASSERT(size >= sizeof(ObjType));

        void* storage = this->reserve(size, DefaultDestructor<ObjType>);
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
    using Destructor = void(*)(void*);
    struct Rec {
        char*      fObj;
        Destructor fDestructor;
    };

    // Used to call the destructor for allocated objects.
    template<typename T>
    static void DefaultDestructor(void* ptr) {
        static_cast<T*>(ptr)->~T();
    }

    static constexpr size_t kAlignment = alignof(SystemAlignment);

    static constexpr size_t AlignSize(size_t size) {
        return (size + kAlignment - 1) & ~(kAlignment - 1);
    }

    // Reserve storageRequired from fStorage if possible otherwise allocate on the heap.
    void* reserve(size_t storageRequired, Destructor destructor) {
        // Make sure that all allocations stay aligned by rounding the storageRequired up to the
        // aligned value.
        char* objectStart = fStorageEnd;
        char* objectEnd = objectStart + AlignSize(storageRequired);
        Rec& rec = fRecs.push_back();
        if (objectEnd > &fStorage[kTotalBytes]) {
            // Allocate on the heap. Ideally we want to avoid this situation.
            rec.fObj = new char [storageRequired];
        } else {
            // There is space in fStorage.
            rec.fObj = objectStart;
            fStorageEnd = objectEnd;
        }
        rec.fDestructor = destructor;
        return rec.fObj;
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
    // Since char have an alignment of 1, it should be forced onto an alignment the compiler
    // expects which is the alignment of std::max_align_t.
    alignas (kAlignment) char              fStorage[kTotalBytes];
};

#endif // SkSmallAllocator_DEFINED
