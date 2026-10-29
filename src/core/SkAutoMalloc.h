/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAutoMalloc_DEFINED
#define SkAutoMalloc_DEFINED

#include "include/private/SkAlign.h"
#include "include/private/SkAssert.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkNoncopyable.h"
#include "include/private/SkTemplates.h"

#include <array>
#include <cstddef>
#include <memory>

/**
 *  Manage an allocated block of heap memory. This object is the sole manager of
 *  the lifetime of the block, so the caller must not call sk_free() or delete
 *  on the block, unless release() was called.
 */
class [[nodiscard]] SkAutoMalloc : SkNoncopyable {
public:
    explicit SkAutoMalloc(size_t size = 0)
        : fPtr(size ? sk_malloc_throw(size) : nullptr), fSize(size) {}

    /**
     *  Passed to reset to specify what happens if the requested size is smaller
     *  than the current size (and the current block was dynamically allocated).
     */
    enum OnShrink {
        /**
         *  If the requested size is smaller than the current size, and the
         *  current block is dynamically allocated, free the old block and
         *  malloc a new block of the smaller size.
         */
        kAlloc_OnShrink,

        /**
         *  If the requested size is smaller than the current size, and the
         *  current block is dynamically allocated, just return the old
         *  block.
         */
        kReuse_OnShrink
    };

    /**
     *  Reallocates the block to a new size. The ptr may or may not change.
     */
    void* reset(size_t size = 0, OnShrink shrink = kAlloc_OnShrink) {
        if (size != fSize && (size > fSize || kReuse_OnShrink != shrink)) {
            fPtr.reset(size ? sk_malloc_throw(size) : nullptr);
            fSize = size;
        }
        return fPtr.get();
    }

    /**
     *  Return the allocated block.
     */
    void* get() { return fPtr.get(); }
    const void* get() const { return fPtr.get(); }

   /** Transfer ownership of the current ptr to the caller, setting the
       internal reference to null. Note the caller is reponsible for calling
       sk_free on the returned address.
    */
    void* release() {
        fSize = 0;
        return fPtr.release();
    }

private:
    struct WrapFree {
        void operator()(void* p) { sk_free(p); }
    };
    std::unique_ptr<void, WrapFree> fPtr;
    size_t fSize;  // can be larger than the requested size (see kReuse)
};

/**
 *  Manage an allocated block of memory. If the requested size is <= kSizeRequested (or slightly
 *  more), then the allocation will come from the stack rather than the heap. This object is the
 *  sole manager of the lifetime of the block, so the caller must not call sk_free() or delete on
 *  the block.
 */
template <size_t kSizeRequested> class [[nodiscard]] SkAutoSMalloc : SkNoncopyable {
public:
    /**
     *  Creates initially empty storage. get() returns a ptr, but it is to a zero-byte allocation.
     *  Must call reset(size) to return an allocated block.
     */
    SkAutoSMalloc() = default;

    /**
     *  Allocate a block of the specified size. If size <= kSizeRequested (or slightly more), then
     *  the allocation will come from the stack, otherwise it will be dynamically allocated.
     */
    explicit SkAutoSMalloc(size_t size) { this->reset(size); }

    /**
     *  Return the allocated block. May return non-null even if the block is of zero size. Since
     *  this may be on the stack or dynamically allocated, the caller must not call sk_free() on it,
     *  but must rely on SkAutoSMalloc to manage it.
     */
    void* get() const { return fHeap ? fHeap.get() : const_cast<std::byte*>(fStorage.data()); }

    /**
     *  Return a new block of the requested size, freeing (as necessary) any previously allocated
     *  block. As with the constructor, if size <= kSizeRequested (or slightly more) then the return
     *  block may be allocated locally, rather than from the heap.
     */
    void* reset(size_t size,
                SkAutoMalloc::OnShrink shrink = SkAutoMalloc::kAlloc_OnShrink,
                bool* didChangeAlloc = nullptr) {
        size = (size < kSize) ? kSize : size;
        bool alloc = size != fSize && (SkAutoMalloc::kAlloc_OnShrink == shrink || size > fSize);
        if (didChangeAlloc) {
            *didChangeAlloc = alloc;
        }
        if (alloc) {
            SkASSERT(fHeap || size != kSize);  // otherwise we lied when setting didChangeAlloc.
            fHeap.reset();
            if (size != kSize) {
                fHeap.reset(sk_malloc_throw(size));
            }
            fSize = size;
        }
        SkASSERT(fSize >= size && fSize >= kSize);
        SkASSERT(!fHeap || fSize > kSize);
        return this->get();
    }

private:
    // Align up to 32 bits.
    static const size_t kSizeAlign4 = SkAlign4(kSizeRequested);
#if defined(SK_BUILD_FOR_GOOGLE3)
    // Stack frame size is limited for SK_BUILD_FOR_GOOGLE3. 4k is less than the actual max, but some functions
    // have multiple large stack allocations.
    static const size_t kMaxBytes = 4 * 1024;
    static const size_t kSize = kSizeRequested > kMaxBytes ? kMaxBytes : kSizeAlign4;
#else
    static const size_t kSize = kSizeAlign4;
#endif

    skia_private::UniqueVoidPtr fHeap;  // null when the block is fStorage
    size_t fSize = kSize;               // can be larger than the requested size (see kReuse)
    alignas(std::max_align_t) std::array<std::byte, kSize> fStorage;
};
// Can't guard the constructor because it's a template class.

#endif
