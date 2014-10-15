/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTRecorder_DEFINED
#define GrTRecorder_DEFINED

#include "SkTemplates.h"
#include "SkTypes.h"

template<typename TBase, typename TAlign> class GrTRecorder;
template<typename TItem> struct GrTRecorderAllocWrapper;

/**
 * Records a list of items with a common base type, optional associated data, and
 * permanent memory addresses.
 *
 * This class preallocates its own chunks of memory for hosting objects, so new items can
 * be created without excessive calls to malloc().
 *
 * To create a new item and append it to the back of the list, use the following macros:
 *
 *     GrNEW_APPEND_TO_RECORDER(recorder, SubclassName, (args))
 *     GrNEW_APPEND_WITH_DATA_TO_RECORDER(recorder, SubclassName, (args), sizeOfData)
 *
 * Upon reset or delete, the items are destructed in the same order they were received,
 * not reverse (stack) order.
 *
 * @param TBase   Common base type of items in the list. If TBase is not a class with a
 *                virtual destructor, the client is responsible for invoking any necessary
 *                destructors.
 *
 *                For now, any subclass used in the list must have the same start address
 *                as TBase (or in other words, the types must be convertible via
 *                reinterpret_cast<>). Classes with multiple inheritance (or any subclass
 *                on an obscure compiler) may not be compatible. This is runtime asserted
 *                in debug builds.
 *
 * @param TAlign  A type whose size is the desired memory alignment for object allocations.
 *                This should be the largest known alignment requirement for all objects
 *                that may be stored in the list.
 */
template<typename TBase, typename TAlign> class GrTRecorder : SkNoncopyable {
public:
    class Iter;

    /**
     * Create a recorder.
     *
     * @param initialSizeInBytes  The amount of memory reserved by the recorder initially,
                                  and after calls to reset().
     */
    GrTRecorder(int initialSizeInBytes)
        : fHeadBlock(MemBlock::Alloc(LengthOf(initialSizeInBytes))),
          fTailBlock(fHeadBlock),
          fLastItem(NULL) {}

    ~GrTRecorder() {
        this->reset();
        MemBlock::Free(fHeadBlock);
    }

    bool empty() { return !fLastItem; }

    TBase& back() {
        SkASSERT(!this->empty());
        return *fLastItem;
    }

    /**
     * Destruct all items in the list and reset to empty.
     */
    void reset();

    /**
     * Retrieve the extra data associated with an item that was allocated using
     * GrNEW_APPEND_WITH_DATA_TO_RECORDER().
     *
     * @param item  The item whose data to retrieve. The pointer must be of the same type
     *              that was allocated initally; it can't be a pointer to a base class.
     *
     * @return The item's associated data.
     */
    template<typename TItem> static const void* GetDataForItem(const TItem* item) {
        const TAlign* ptr = reinterpret_cast<const TAlign*>(item);
        return &ptr[length_of<TItem>::kValue];
    }
    template<typename TItem> static void* GetDataForItem(TItem* item) {
        TAlign* ptr = reinterpret_cast<TAlign*>(item);
        return &ptr[length_of<TItem>::kValue];
    }

private:
    template<typename TItem> struct length_of {
        enum { kValue = (sizeof(TItem) + sizeof(TAlign) - 1) / sizeof(TAlign) };
    };
    static int LengthOf(int bytes) { return (bytes + sizeof(TAlign) - 1) / sizeof(TAlign); }

    struct Header {
        int fTotalLength;
    };
    template<typename TItem> TItem* alloc_back(int dataLength);

    struct MemBlock : SkNoncopyable {
        static MemBlock* Alloc(int length) {
            MemBlock* block = reinterpret_cast<MemBlock*>(
                sk_malloc_throw(sizeof(TAlign) * (length_of<MemBlock>::kValue + length)));
            block->fLength = length;
            block->fBack = 0;
            block->fNext = NULL;
            return block;
        }

        static void Free(MemBlock* block) {
            if (!block) {
                return;
            }
            Free(block->fNext);
            sk_free(block);
        }

        TAlign& operator [](int i) {
            return reinterpret_cast<TAlign*>(this)[length_of<MemBlock>::kValue + i];
        }

        int       fLength;
        int       fBack;
        MemBlock* fNext;
    };
    MemBlock* const fHeadBlock;
    MemBlock* fTailBlock;

    TBase*    fLastItem;

    template<typename TItem> friend struct GrTRecorderAllocWrapper;

    template <typename UBase, typename UAlign, typename UItem>
    friend void* operator new(size_t, GrTRecorder<UBase, UAlign>&,
                              const GrTRecorderAllocWrapper<UItem>&);

    friend class Iter;
};

////////////////////////////////////////////////////////////////////////////////

template<typename TBase, typename TAlign>
template<typename TItem>
TItem* GrTRecorder<TBase, TAlign>::alloc_back(int dataLength) {
    const int totalLength = length_of<Header>::kValue + length_of<TItem>::kValue + dataLength;

    if (fTailBlock->fBack + totalLength > fTailBlock->fLength) {
        SkASSERT(!fTailBlock->fNext);
        fTailBlock->fNext = MemBlock::Alloc(SkTMax(2 * fTailBlock->fLength, totalLength));
        fTailBlock = fTailBlock->fNext;
    }

    Header* header = reinterpret_cast<Header*>(&(*fTailBlock)[fTailBlock->fBack]);
    TItem* rawPtr = reinterpret_cast<TItem*>(
                        &(*fTailBlock)[fTailBlock->fBack + length_of<Header>::kValue]);

    header->fTotalLength = totalLength;
    fLastItem = rawPtr;
    fTailBlock->fBack += totalLength;

    // FIXME: We currently require that the base and subclass share the same start address.
    // This is not required by the C++ spec, and is likely to not be true in the case of
    // multiple inheritance or a base class that doesn't have virtual methods (when the
    // subclass does). It would be ideal to find a more robust solution that comes at no
    // extra cost to performance or code generality.
    SkDEBUGCODE(void* baseAddr = fLastItem;
                void* subclassAddr = rawPtr);
    SkASSERT(baseAddr == subclassAddr);

    return rawPtr;
}

template<typename TBase, typename TAlign>
class GrTRecorder<TBase, TAlign>::Iter {
public:
    Iter(GrTRecorder& recorder) : fBlock(recorder.fHeadBlock), fPosition(0), fItem(NULL) {}

    bool next() {
        if (fPosition >= fBlock->fBack) {
            SkASSERT(fPosition == fBlock->fBack);
            if (!fBlock->fNext) {
                return false;
            }
            SkASSERT(0 != fBlock->fNext->fBack);
            fBlock = fBlock->fNext;
            fPosition = 0;
        }

        Header* header = reinterpret_cast<Header*>(&(*fBlock)[fPosition]);
        fItem = reinterpret_cast<TBase*>(&(*fBlock)[fPosition + length_of<Header>::kValue]);
        fPosition += header->fTotalLength;
        return true;
    }

    TBase* get() const {
        SkASSERT(fItem);
        return fItem;
    }

    TBase* operator->() const { return this->get(); }

private:
    MemBlock* fBlock;
    int       fPosition;
    TBase*    fItem;
};

template<typename TBase, typename TAlign>
void GrTRecorder<TBase, TAlign>::reset() {
    Iter iter(*this);
    while (iter.next()) {
        iter->~TBase();
    }
    fHeadBlock->fBack = 0;
    MemBlock::Free(fHeadBlock->fNext);
    fHeadBlock->fNext = NULL;
    fTailBlock = fHeadBlock;
    fLastItem = NULL;
}

////////////////////////////////////////////////////////////////////////////////

template<typename TItem> struct GrTRecorderAllocWrapper {
    GrTRecorderAllocWrapper() : fDataLength(0) {}

    template <typename TBase, typename TAlign>
    GrTRecorderAllocWrapper(const GrTRecorder<TBase, TAlign>&, int sizeOfData)
        : fDataLength(GrTRecorder<TBase, TAlign>::LengthOf(sizeOfData)) {}

    const int fDataLength;
};

template <typename TBase, typename TAlign, typename TItem>
void* operator new(size_t size, GrTRecorder<TBase, TAlign>& recorder,
                   const GrTRecorderAllocWrapper<TItem>& wrapper) {
    SkASSERT(size == sizeof(TItem));
    return recorder.template alloc_back<TItem>(wrapper.fDataLength);
}

template <typename TBase, typename TAlign, typename TItem>
void operator delete(void*, GrTRecorder<TBase, TAlign>&, const GrTRecorderAllocWrapper<TItem>&) {
    // We only provide an operator delete to work around compiler warnings that can come
    // up for an unmatched operator new when compiling with exceptions.
    SK_CRASH();
}

#define GrNEW_APPEND_TO_RECORDER(recorder, type_name, args) \
    (new (recorder, GrTRecorderAllocWrapper<type_name>()) type_name args)

#define GrNEW_APPEND_WITH_DATA_TO_RECORDER(recorder, type_name, args, size_of_data) \
    (new (recorder, GrTRecorderAllocWrapper<type_name>(recorder, size_of_data)) type_name args)

#endif
