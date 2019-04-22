/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTRecorder_DEFINED
#define GrTRecorder_DEFINED

#include "GrTypes.h"
#include "SkArenaAlloc.h"
#include "SkTLogic.h"

/**
 * Records a list of items with a common base type, optional associated data, and
 * permanent memory addresses.
 *
 * This class allocates space for the stored items and associated data in a SkArenaAlloc.
 * There is an overhead of 3 pointers for each stored item.
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
    struct Footer {
        TBase* fTBase = nullptr;
        Footer* fPrev = nullptr;
        Footer* fNext = nullptr;
    };

    template <bool IsConst, bool IsReverse> class IterImpl;

    template <bool> struct ConstOrNotHelper;
    template <> struct ConstOrNotHelper<false> { using type = TBase; };
    template <> struct ConstOrNotHelper<true> { using type = const TBase; };
    template <bool Const> using ConstOrNot = typename ConstOrNotHelper<Const>::type;

    template <bool IsReverse> struct Move;
    template <> struct Move<false> {
        static Footer* Forward(Footer* i) { return i->fNext; }
        static Footer* Backward(Footer* i) { return i->fPrev; }
    };
    template <> struct Move<true> {
        static Footer* Forward(Footer* i) { return i->fPrev; }
        static Footer* Backward(Footer* i) { return i->fNext; }
    };

public:
    using iterator = IterImpl<false, false>;
    using reverse_iterator = IterImpl<false, true>;
    using const_iterator = IterImpl<true, false>;
    using const_reverse_iterator = IterImpl<true, true>;

    /**
     * Create a recorder.
     *
     * @param initialSizeInBytes  The amount of memory reserved by the recorder initially,
                                  and after calls to reset().
     */
    explicit GrTRecorder(size_t initialSizeInBytes) : fArena(initialSizeInBytes) {}
    GrTRecorder(const GrTRecorder&) = delete;
    GrTRecorder& operator=(const GrTRecorder&) = delete;

    ~GrTRecorder() {
        this->reset();
    }

    bool empty() { return !SkToBool(fTail); }

    /** The last item. Must not be empty. */
    TBase& back() {
        SkASSERT(!this->empty());
        return *fTail->fTBase;
    }

    /** Forward mutable iteration */
    iterator begin() { return iterator(fHead); }
    iterator end() { return iterator(nullptr); }

    /** Forward const iteration */
    const_iterator begin() const { return const_iterator(fHead); }
    const_iterator end() const { return const_iterator(nullptr); }

    /** Reverse mutable iteration */
    reverse_iterator rbegin() { return reverse_iterator(fTail); }
    reverse_iterator rend() { return reverse_iterator(nullptr); }

    /** Reverse const iteration */
    const_reverse_iterator rbegin() const { return const_reverse_iterator(fTail); }
    const_reverse_iterator rend() const { return const_reverse_iterator(nullptr); }

    /**
     * Removes and destroys the last block added to the recorder if any. Does not actually free
     * the memory used by the last item. Memory is only freed with reset() or recorder destruction.
     */
    void pop_back();

    /** Destruct all items in the list and reset to empty. Frees memory allocated from arena. */
    void reset();

    /** Emplace a new TItem (which derives from TBase) in the recorder. */
    template <typename TItem, typename... Args> TItem& emplace(Args... args) {
        return this->emplaceWithData<TItem, Args...>(0, std::forward<Args>(args)...);
    }

    /**
     * Emplace a new TItem (which derives from TBase) in the recorder with extra data space. The
     * extra data immediately follows the stored item with no extra alignment. E.g.,
     *      void* extraData = &recorder->emplaceWithData<Subclass>(dataSize, ...) + 1;
     */
    template <typename TItem, typename... Args>
    SK_WHEN((std::is_base_of<TBase, TItem>::value), TItem&)
    emplaceWithData(size_t extraDataSize, Args... args);

private:
    SkArenaAlloc fArena;
    Footer* fHead = nullptr;
    Footer* fTail = nullptr;
};

////////////////////////////////////////////////////////////////////////////////

template <typename TBase> inline void GrTRecorder<TBase>::pop_back() {
    if (fTail) {
        this->back().~TBase();
        fTail = fTail->fPrev;
        if (!fTail) {
            fHead = nullptr;
        }
    }
}

template <typename TBase>
template <typename TItem, typename... Args>
inline SK_WHEN((std::is_base_of<TBase, TItem>::value), TItem&)
GrTRecorder<TBase>::emplaceWithData(size_t extraDataSize, Args... args) {
    // Allocate TItem, extra data, and Footer at once.
    static constexpr size_t kTAlign = alignof(TItem);
    static constexpr size_t kFootAlign = alignof(Footer);
    static constexpr size_t kAllocAlign = kTAlign > kFootAlign ? kTAlign : kFootAlign;
    const size_t sizeBeforeFooter = sizeof(TItem) + extraDataSize;
    const size_t footerPadding = GrSizeAlignUp(sizeBeforeFooter, kFootAlign) - sizeBeforeFooter;
    const size_t totalSize = sizeBeforeFooter + footerPadding + sizeof(Footer);

    auto data = reinterpret_cast<char*>(fArena.makeBytesAlignedTo(totalSize, kAllocAlign));

    Footer* footer = new (data + totalSize - sizeof(Footer)) Footer();
    footer->fPrev = fTail;
    if (fTail) {
        fTail->fNext = footer;
    }
    fTail = footer;
    if (!fHead) {
        fHead = footer;
    }
    auto* item = new (data) TItem(std::forward<Args>(args)...);
    footer->fTBase = item;
    return *item;
}

/**
 * Iterates through a recorder either front-to-back or back-to-front and either const or not.
 */
template <typename TBase>
template <bool IsConst, bool IsReverse>
class GrTRecorder<TBase>::IterImpl {
private:
    using V = ConstOrNot<IsConst>;
    using M = Move<IsReverse>;

public:
    IterImpl operator++() {
        fCurr = M::Forward(fCurr);
        return *this;
    }

    IterImpl operator++(int) {
        auto old = fCurr;
        fCurr = M::Forward(fCurr);
        return {old};
    }

    IterImpl operator--() {
        fCurr = M::Backward(fCurr);
        return *this;
    }

    IterImpl operator--(int) {
        auto old = fCurr;
        fCurr = M::Backward(fCurr);
        return {old};
    }

    V& operator*() const { return *fCurr->fTBase; }
    V* operator->() const { return fCurr->fTBase; }

    bool operator==(const IterImpl& that) const { return fCurr == that.fCurr; }
    bool operator!=(const IterImpl& that) const { return !(*this == that); }

private:
    IterImpl(Footer* curr) : fCurr(curr) {}
    Footer* fCurr;
    friend class GrTRecorder<TBase>;
};

template <typename TBase> inline void GrTRecorder<TBase>::reset() {
    for (auto& i : *this) {
        i.~TBase();
    }
    fHead = fTail = nullptr;
    fArena.reset();
}

#endif
