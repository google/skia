/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFixedAlloc_DEFINED
#define SkFixedAlloc_DEFINED

#include "SkTFitsIn.h"
#include "SkTypes.h"
#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>

// SkFixedAlloc allocates objects out of a fixed-size buffer and destroys them when destroyed.
class SkFixedAlloc {
public:
    SkFixedAlloc(void* ptr, size_t len);
    ~SkFixedAlloc() { this->reset(); }

    // Allocates a new T in the buffer if possible.  If not, returns nullptr.
    // Assumptions:
    // * max alignment value is 32 - if alignment is greater than 32, the allocation is best effort.
    // * footer is 32 bits - 5 bits of alignment and 27 bits of deleter difference from Base.
    // * deleter difference - the difference D is -2^26 <= D < 2^26.
    template <typename T, typename... Args>
    T* make(Args&&... args) {
        auto mask = alignof(T) - 1;

        // Align fCursor for this allocation.
        char* objStart = (char*)((uintptr_t)(fCursor + mask) & ~mask);
        ptrdiff_t padding = objStart - fCursor;
        Releaser releaser = [](char* objEnd) {
            char* objStart = objEnd - (sizeof(T) + sizeof(Footer));
            ((T*)objStart)->~T();
            return objStart;
        };

        ptrdiff_t deleterDiff = (char*)releaser - (char*)Base;

        if (objStart + sizeof(T) + sizeof(Footer) > fEnd
            || padding >= 32
            || deleterDiff >= (1 << 26)
            || deleterDiff < -(1 << 26)) {
            // Ran out of space, or code not store info in the Footer.
            return nullptr;
        }

        // Advance cursor to end of the object.
        fCursor = objStart + sizeof(T);

        Footer footer = (Footer)(SkLeftShift((int64_t)deleterDiff, 5) | padding);
        memcpy(fCursor, &footer, sizeof(Footer));
        fCursor += sizeof(Footer);

        return new (objStart) T(std::forward<Args>(args)...);
    }

    // Destroys the last object allocated and frees its space in the buffer.
    void undo();

    // Destroys all objects and frees all space in the buffer.
    void reset();

private:
    using Footer  = int32_t;
    using Releaser = char*(*)(char*);

    // A function pointer to use for offsets of releasers.
    static void Base();

    char* const fStorage;
    char*       fCursor;
    char* const fEnd;
};

class SkFallbackAlloc {
public:
    explicit SkFallbackAlloc(SkFixedAlloc*);
    ~SkFallbackAlloc() { this->reset(); }

    // Allocates a new T with the SkFixedAlloc if possible.  If not, uses the heap.
    template <typename T, typename... Args>
    T* make(Args&&... args) {
        // Once we go heap we never go back to fixed.  This keeps destructor ordering sane.
        if (fHeapAllocs.empty()) {
            if (T* ptr = fFixedAlloc->make<T>(std::forward<Args>(args)...)) {
                return ptr;
            }
        }

        char* ptr = new char[sizeof(T)];
        fHeapAllocs.push_back({[](char* ptr) { ((T*)ptr)->~T(); delete [] ptr; }, ptr});
        return new (ptr) T(std::forward<Args>(args)...);
    }


    // Destroys all objects and frees all space in the SkFixedAlloc.
    void reset();

private:
    // Destroys the last object allocated and frees any space it used in the SkFixedAlloc.
    void undo();

    struct HeapAlloc {
        void (*deleter)(char*);
        char* ptr;
    };

    SkFixedAlloc*          fFixedAlloc;
    std::vector<HeapAlloc> fHeapAllocs;
};

class SkArenaAlloc {
public:
    SkArenaAlloc() { }
    SkArenaAlloc(char* block, size_t size, size_t extraSize = 0)
    : fDtorCursor{block}
    , fCursor{block}
    , fEnd{block + size}
    , fExtraSize{extraSize}
    {
        if (size < sizeof(Footer)) {
            fEnd = fCursor = fDtorCursor = nullptr;
        }

        if (fCursor != nullptr) {
            this->installFooter(EndChain, 0);
        }
    }

    ~SkArenaAlloc() {
        this->reset();
    }

    template <typename T, typename... Args>
    T* make(Args&&... args) {
        auto mask = alignof(T) - 1;

        // Align fCursor for this allocation.
    restart:
        char* objStart = (char*)((uintptr_t)(fCursor + mask) & ~mask);
        size_t overhead = 0;
        bool needsSkipFooter = false;
        if (!std::is_trivially_destructible<T>::value) {
            needsSkipFooter = fCursor != fDtorCursor;
            overhead = sizeof(Footer);
            if (needsSkipFooter) {
                overhead += sizeof(Footer) + sizeof(ptrdiff_t);
            }
        }

        if (objStart + sizeof(T) + overhead > fEnd) {
            this->ensureSpace(sizeof(T), alignof(T));
            goto restart;
        }

        SkASSERT(objStart + sizeof(T) + overhead < fEnd);

        ptrdiff_t padding = objStart - fCursor;

        // Advance cursor to end of the object.
        fCursor = objStart + sizeof(T);

        if (!std::is_trivially_destructible<T>::value) {
            if (needsSkipFooter) {
                this->installIntFooter<Skipper>(fCursor - fDtorCursor, 0);
            }
            Releaser releaser = [](char* objEnd) {
                char* objStart = objEnd - (sizeof(T) + sizeof(Footer));
                ((T*)objStart)->~T();
                return objStart;
            };

            this->installFooter(releaser, padding);
        }

        return new(objStart) T(std::forward<Args>(args)...);
    }

    template <typename T>
    T* makeArrayDefault(size_t count) {
        auto mask = alignof(T) - 1;
        auto arraySize = count * sizeof(T);

        // Align fCursor for this allocation.
    restart:
        char* objStart = (char*)((uintptr_t)(fCursor + mask) & ~mask);
        size_t overhead = 0;
        bool needsSkipFooter = false;
        if (!std::is_trivially_destructible<T>::value) {
            needsSkipFooter = fCursor != fDtorCursor;
            overhead = sizeof(Footer);
            if (needsSkipFooter) {
                overhead += sizeof(Footer) + sizeof(ptrdiff_t);
            }
        }

        if (objStart + arraySize + overhead > fEnd) {
            this->ensureSpace(arraySize, alignof(T));
            goto restart;
        }

        SkASSERT(objStart + arraySize + overhead < fEnd);

        ptrdiff_t padding = objStart - fCursor;
        // Advance cursor to end of the object.
        fCursor = objStart + arraySize;

        if (!std::is_trivially_destructible<T>::value) {
            if (needsSkipFooter) {
                this->installIntFooter<Skipper>(fCursor - fDtorCursor, 0);
            }

            this->installIntFooter<ArrayReleaser<T>>(count, padding);
        }

        return new(objStart) T[count];
    }

    void reset() {
        char* releaser = fDtorCursor;
        while (releaser != nullptr) {
            releaser = this->callReleaser(releaser);
        }
    }

private:
    using Footer = int32_t;

    using Releaser = char*(*)(char*);
    void ensureSpace(size_t size, size_t alignment) {
        constexpr size_t headerSize = sizeof(Footer) + sizeof(ptrdiff_t);
        auto objSizeAndOverhead = size + headerSize + sizeof(Footer);
        if (alignment > alignof(std::max_align_t)) {
            objSizeAndOverhead += alignment - 1;
        }

        auto allocationSize = std::max(objSizeAndOverhead, fExtraSize);

        if (allocationSize > (1 << 15)) {
            auto mask = (1 << 12) - 1;
            allocationSize = (allocationSize + mask) & ~mask;
        }

        char* newBlock = new char[allocationSize];

        auto previousDtor = fDtorCursor;
        fCursor = newBlock;
        fDtorCursor = newBlock;
        fEnd = fCursor + allocationSize;
        this->installIntFooter<NextBlock>(previousDtor - fCursor, 0);
    }

    void installFooter(Releaser releaser, ptrdiff_t padding) {
        ptrdiff_t releaserDiff = (char *)releaser - (char *)EndChain;
        ptrdiff_t footerData = SkLeftShift((int64_t)releaserDiff, 5) | padding;
        if (padding >= 32 || !SkTFitsIn<int32_t>(footerData)) {
            // Footer data will not fit.
            SkFAIL("Constraints are busted.");
        }

        Footer footer = (Footer)(footerData);
        memcpy(fCursor, &footer, sizeof(Footer));
        fCursor += sizeof(Footer);
        fDtorCursor = fCursor;
    }

    template<typename Action>
    void installIntFooter(ptrdiff_t size, ptrdiff_t padding) {
        if (SkTFitsIn<int32_t>(size)) {
            int32_t smallSize = static_cast<int32_t>(size);
            memmove(fCursor, &smallSize, sizeof(int32_t));
            fCursor += sizeof(int32_t);
            this->installFooter(
                [](char* footerEnd) {
                    Action action;
                    char* objEnd = footerEnd - (sizeof(Footer) + sizeof(int32_t));
                    int32_t data;
                    memmove(&data, objEnd, sizeof(int32_t));
                    return action(objEnd, data);
                },
                padding);
        } else {
            memmove(fCursor, &size, sizeof(ptrdiff_t));
            fCursor += sizeof(ptrdiff_t);
            this->installFooter(
                [](char* footerEnd) {
                    Action action;
                    char* objEnd = footerEnd - (sizeof(Footer) + sizeof(ptrdiff_t));
                    ptrdiff_t data;
                    memmove(&data, objEnd, sizeof(ptrdiff_t));
                    return action(objEnd, data);
                },
                padding);
        }
    }

    char* callReleaser(char* end) {
        Footer footer;
        memcpy(&footer, end - sizeof(Footer), sizeof(Footer));

        Releaser releaser = (Releaser)((char*)EndChain + (footer >> 5));
        ptrdiff_t padding = footer & 31;

        return releaser(end) - padding;
    }

    static char* EndChain(char*) { return nullptr; }

    struct Skipper {
        char* operator()(char* objEnd, ptrdiff_t size) { return objEnd + size; }
    };

    struct NextBlock {
        char* operator()(char* objEnd, ptrdiff_t size) { delete [] objEnd; return objEnd + size; }
    };

    template<typename T>
    struct ArrayReleaser {
        char* operator()(char* objEnd, ptrdiff_t count) {
            char* objStart = objEnd - count * sizeof(T);
            T* array = (T*) objStart;
            for (int i = 0; i < count; i++) {
                array[i].~T();
            }
            return objStart;
        }
    };


    char*  fDtorCursor     {nullptr};
    char*  fCursor         {nullptr};
    char*  fEnd            {nullptr};
    size_t fExtraSize;

};

#endif//SkFixedAlloc_DEFINED
