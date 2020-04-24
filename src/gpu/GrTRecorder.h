/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTRecorder_DEFINED
#define GrTRecorder_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/private/SkTLogic.h"
#include "src/core/SkArenaAlloc.h"

/**
 * Records a list of items with a common base type, optional associated data, and
 * permanent memory addresses. It supports forward iteration.
 *
 * This class allocates space for the stored items and associated data in a SkArenaAlloc.
 * There is an overhead of 1 pointer for each stored item.
 *
 * Upon reset or delete, the items are destructed in the same order they were received,
 * not reverse (stack) order.
 *
 * @param TBase   Common base type of items in the list. It is assumed that the items are
 *                trivially destructable or that TBase has a virtual destructor as ~TBase()
 *                is called to destroy the items.
 */
template <typename TBase> class GrTRecorder {
private:
    template <bool IsConst> class IterImpl;

public:
    using iterator = IterImpl<false>;
    using const_iterator = IterImpl<true>;

    /**
     * Create a recorder.
     *
     * @param initialSizeInBytes  The amount of memory reserved by the recorder initially,
                                  and after calls to reset().
     */
    explicit GrTRecorder(size_t initialSizeInBytes) : fArena(initialSizeInBytes) {}
    GrTRecorder(const GrTRecorder&) = delete;
    GrTRecorder& operator=(const GrTRecorder&) = delete;

    ~GrTRecorder() { this->reset(); }

    bool empty() { return !SkToBool(fTail); }

    /** The last item. Must not be empty. */
    TBase& back() {
        SkASSERT(!this->empty());
        return *fTail->get();
    }

    /** Forward mutable iteration */
    iterator begin() { return iterator(fHead); }
    iterator end() { return iterator(nullptr); }

    /** Forward const iteration */
    const_iterator begin() const { return const_iterator(fHead); }
    const_iterator end() const { return const_iterator(nullptr); }

    /** Destruct all items in the list and reset to empty. Frees memory allocated from arena. */
    void reset();

    /**
     * Emplace a new TItem (which derives from TBase) in the recorder. This requires equivalence
     * between reinterpret_cast<TBase*> and static_cast<TBase*> when operating on TItem*.
     * Multiple inheritance may make this not true. It is runtime asserted.
     */
    template <typename TItem, typename... Args> TItem& emplace(Args&&... args) {
        return this->emplaceWithData<TItem, Args...>(0, std::forward<Args>(args)...);
    }

    /**
     * Emplace a new TItem (which derives from TBase) in the recorder with extra data space. The
     * extra data immediately follows the stored item with no extra alignment. E.g.,
     *      void* extraData = &recorder->emplaceWithData<Subclass>(dataSize, ...) + 1;
     *
     * This requires equivalence between reinterpret_cast<TBase*> and static_cast<TBase*> when
     * operating on TItem*. Multiple inheritance may make this not true. It is runtime asserted.
     */
    template <typename TItem, typename... Args>
    SK_WHEN((std::is_base_of<TBase, TItem>::value), TItem&)
    emplaceWithData(size_t extraDataSize, Args... args);

private:
    struct Header {
        Header* fNext = nullptr;
        // We always store the T immediately after the header (and ensure proper alignment). See
        // emplaceWithData() implementation.
        TBase* get() const { return reinterpret_cast<TBase*>(const_cast<Header*>(this) + 1); }
    };

    SkArenaAlloc fArena;
    Header* fHead = nullptr;
    Header* fTail = nullptr;
};

////////////////////////////////////////////////////////////////////////////////

template <typename TBase>
template <typename TItem, typename... Args>
inline SK_WHEN((std::is_base_of<TBase, TItem>::value), TItem&)
GrTRecorder<TBase>::emplaceWithData(size_t extraDataSize, Args... args) {
    static constexpr size_t kTAlign = alignof(TItem);
    static constexpr size_t kHeaderAlign = alignof(Header);
    static constexpr size_t kAllocAlign = kTAlign > kHeaderAlign ? kTAlign : kHeaderAlign;
    static constexpr size_t kTItemOffset = GrSizeAlignUp(sizeof(Header), kAllocAlign);
    // We're assuming if we back up from kItemOffset by sizeof(Header) we will still be aligned.
    GR_STATIC_ASSERT(sizeof(Header) % alignof(Header) == 0);
    const size_t totalSize = kTItemOffset + sizeof(TItem) + extraDataSize;
    auto alloc = reinterpret_cast<char*>(fArena.makeBytesAlignedTo(totalSize, kAllocAlign));
    Header* header = new (alloc + kTItemOffset - sizeof(Header)) Header();
    if (fTail) {
        fTail->fNext = header;
    }
    fTail = header;
    if (!fHead) {
        fHead = header;
    }
    auto* item = new (alloc + kTItemOffset) TItem(std::forward<Args>(args)...);
    // We require that we can reinterpret_cast between TBase* and TItem*. Could not figure out how
    // to statically assert this. See proposal for std::is_initial_base_of here:
    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0466r0.pdf
    SkASSERT(reinterpret_cast<uintptr_t>(item) ==
             reinterpret_cast<uintptr_t>(static_cast<TBase*>(item)));
    return *item;
}

template <typename TBase> inline void GrTRecorder<TBase>::reset() {
    for (auto& i : *this) {
        i.~TBase();
    }
    GR_STATIC_ASSERT(std::is_trivially_destructible<Header>::value);
    fHead = fTail = nullptr;
    fArena.reset();
}

/**
 * Iterates through a recorder front-to-back, const or not.
 */
template <typename TBase> template <bool IsConst> class GrTRecorder<TBase>::IterImpl {
private:
    using T = typename std::conditional<IsConst, const TBase, TBase>::type;

public:
    IterImpl() = default;

    IterImpl operator++() {
        fCurr = fCurr->fNext;
        return *this;
    }

    IterImpl operator++(int) {
        auto old = fCurr;
        fCurr = fCurr->fNext;
        return {old};
    }

    T& operator*() const { return *fCurr->get(); }
    T* operator->() const { return fCurr->get(); }

    bool operator==(const IterImpl& that) const { return fCurr == that.fCurr; }
    bool operator!=(const IterImpl& that) const { return !(*this == that); }

private:
    IterImpl(Header* curr) : fCurr(curr) {}
    Header* fCurr = nullptr;

    friend class GrTRecorder<TBase>; // To construct from Header.
};

#endif
