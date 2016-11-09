/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypesPriv_DEFINED
#define SkTypesPriv_DEFINED

#include "SkTypes.h"

#include <memory>


struct SkFreeWrapper {
    void operator()(void* p) { sk_free(p); }
};
class SkAutoFree : public std::unique_ptr<void, SkFreeWrapper> {
public:
    SkAutoFree(void* ptr = nullptr)
        : std::unique_ptr<void, SkFreeWrapper>(ptr) {}
    static SkAutoFree Calloc(size_t size) {
        return SkAutoFree(size ? sk_calloc_throw(size) : nullptr);
    }
    static SkAutoFree Malloc(size_t size) {
        return SkAutoFree(size ? sk_malloc_throw(size) : nullptr);
    }
};

/**
 *  Manage an allocated block of heap memory. This object is the sole manager of
 *  the lifetime of the block, so the caller must not call sk_free() or delete
 *  on the block, unless release() was called.
 */
class SkAutoMalloc : public SkAutoFree {
public:
    explicit SkAutoMalloc(size_t size = 0) : SkAutoFree(SkAutoFree::Malloc(size)) {}
    void* reset(size_t size = 0) {
        *this = SkAutoMalloc(size);
        return this->get();
    }
};

/**
 *  Passed to reset to specify what happens if the requested size is smaller
 *  than the current size (and the current block was dynamically allocated).
 */
enum class SkAutoSMallocOnShrink {
    /**
     *  If the requested size is smaller than the current size, and the
     *  current block is dynamically allocated, free the old block and
     *  malloc a new block of the smaller size.
     */
    kAlloc,
    /**
     *  If the requested size is smaller than the current size, and the
     *  current block is dynamically allocated, just return the old
     *  block.
     */
    kReuse,
};

/**
 *  Manage an allocated block of memory. If the requested size is <= kSizeRequested (or slightly
 *  more), then the allocation will come from the stack rather than the heap. This object is the
 *  sole manager of the lifetime of the block, so the caller must not call sk_free() or delete on
 *  the block.
 */
template <size_t kSizeRequested> class SkAutoSMalloc : SkNoncopyable {
public:
    /**
     *  Creates initially empty storage. get() returns a ptr, but it is to a zero-byte allocation.
     *  Must call reset(size) to return an allocated block.
     */
    SkAutoSMalloc() {
        fPtr = fStorage;
        fSize = kSize;
    }

    /**
     *  Allocate a block of the specified size. If size <= kSizeRequested (or slightly more), then
     *  the allocation will come from the stack, otherwise it will be dynamically allocated.
     */
    explicit SkAutoSMalloc(size_t size) {
        fPtr = fStorage;
        fSize = kSize;
        this->reset(size);
    }

    /**
     *  Free the allocated block (if any). If the block was small enough to have been allocated on
     *  the stack, then this does nothing.
     */
    ~SkAutoSMalloc() {
        if (fPtr != (void*)fStorage) {
            sk_free(fPtr);
        }
    }

    /**
     *  Return the allocated block. May return non-null even if the block is of zero size. Since
     *  this may be on the stack or dynamically allocated, the caller must not call sk_free() on it,
     *  but must rely on SkAutoSMalloc to manage it.
     */
    void* get() const { return fPtr; }


    /**
     *  Return a new block of the requested size, freeing (as necessary) any previously allocated
     *  block. As with the constructor, if size <= kSizeRequested (or slightly more) then the return
     *  block may be allocated locally, rather than from the heap.
     */
    void* reset(size_t size,
                SkAutoSMallocOnShrink shrink = SkAutoSMallocOnShrink::kAlloc,
                bool* didChangeAlloc = NULL) {
        size = (size < kSize) ? kSize : size;
        bool alloc = size != fSize && (SkAutoSMallocOnShrink::kAlloc == shrink || size > fSize);
        if (didChangeAlloc) {
            *didChangeAlloc = alloc;
        }
        if (alloc) {
            if (fPtr != (void*)fStorage) {
                sk_free(fPtr);
            }

            if (size == kSize) {
                SkASSERT(fPtr != fStorage); // otherwise we lied when setting didChangeAlloc.
                fPtr = fStorage;
            } else {
                fPtr = sk_malloc_flags(size, SK_MALLOC_THROW | SK_MALLOC_TEMP);
            }

            fSize = size;
        }
        SkASSERT(fSize >= size && fSize >= kSize);
        SkASSERT((fPtr == fStorage) || fSize > kSize);
        return fPtr;
    }

private:
    // Align up to 32 bits.
    static const size_t kSizeAlign4 = SkAlign4(kSizeRequested);
#if defined(GOOGLE3)
    // Stack frame size is limited for GOOGLE3. 4k is less than the actual max, but some functions
    // have multiple large stack allocations.
    static const size_t kMaxBytes = 4 * 1024;
    static const size_t kSize = kSizeRequested > kMaxBytes ? kMaxBytes : kSizeAlign4;
#else
    static const size_t kSize = kSizeAlign4;
#endif

    void*       fPtr;
    size_t      fSize;  // can be larger than the requested size (see kReuse)
    uint32_t    fStorage[kSize >> 2];
};
// Can't guard the constructor because it's a template class.

#endif
