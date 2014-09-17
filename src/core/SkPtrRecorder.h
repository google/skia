
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPtrSet_DEFINED
#define SkPtrSet_DEFINED

#include "SkRefCnt.h"
#include "SkFlattenable.h"
#include "SkTDArray.h"

/**
 *  Maintains a set of ptrs, assigning each a unique ID [1...N]. Duplicate ptrs
 *  return the same ID (since its a set). Subclasses can override inPtr()
 *  and decPtr(). incPtr() is called each time a unique ptr is added ot the
 *  set. decPtr() is called on each ptr when the set is destroyed or reset.
 */
class SkPtrSet : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkPtrSet)

    /**
     *  Search for the specified ptr in the set. If it is found, return its
     *  32bit ID [1..N], or if not found, return 0. Always returns 0 for NULL.
     */
    uint32_t find(void*) const;

    /**
     *  Add the specified ptr to the set, returning a unique 32bit ID for it
     *  [1...N]. Duplicate ptrs will return the same ID.
     *
     *  If the ptr is NULL, it is not added, and 0 is returned.
     */
    uint32_t add(void*);

    /**
     *  Return the number of (non-null) ptrs in the set.
     */
    int count() const { return fList.count(); }

    /**
     *  Copy the ptrs in the set into the specified array (allocated by the
     *  caller). The ptrs are assgined to the array based on their corresponding
     *  ID. e.g. array[ptr.ID - 1] = ptr.
     *
     *  incPtr() and decPtr() are not called during this operation.
     */
    void copyToArray(void* array[]) const;

    /**
     *  Call decPtr() on each ptr in the set, and the reset the size of the set
     *  to 0.
     */
    void reset();

    /**
     * Set iterator.
     */
    class Iter {
    public:
        Iter(const SkPtrSet& set)
            : fSet(set)
            , fIndex(0) {}

        /**
         * Return the next ptr in the set or null if the end was reached.
         */
        void* next() {
            return fIndex < fSet.fList.count() ? fSet.fList[fIndex++].fPtr : NULL;
        }

    private:
        const SkPtrSet& fSet;
        int             fIndex;
    };

protected:
    virtual void incPtr(void*) {}
    virtual void decPtr(void*) {}

private:
    struct Pair {
        void*       fPtr;   // never NULL
        uint32_t    fIndex; // 1...N
    };

    // we store the ptrs in sorted-order (using Cmp) so that we can efficiently
    // detect duplicates when add() is called. Hence we need to store the
    // ptr and its ID/fIndex explicitly, since the ptr's position in the array
    // is not related to its "index".
    SkTDArray<Pair>  fList;

    static bool Less(const Pair& a, const Pair& b);

    typedef SkRefCnt INHERITED;
};

/**
 *  Templated wrapper for SkPtrSet, just meant to automate typecasting
 *  parameters to and from void* (which the base class expects).
 */
template <typename T> class SkTPtrSet : public SkPtrSet {
public:
    uint32_t find(T ptr) {
        return this->INHERITED::find((void*)ptr);
    }
    uint32_t add(T ptr) {
        return this->INHERITED::add((void*)ptr);
    }

    void copyToArray(T* array) const {
        this->INHERITED::copyToArray((void**)array);
    }

private:
    typedef SkPtrSet INHERITED;
};

/**
 *  Subclass of SkTPtrSet specialed to call ref() and unref() when the
 *  base class's incPtr() and decPtr() are called. This makes it a valid owner
 *  of each ptr, which is released when the set is reset or destroyed.
 */
class SkRefCntSet : public SkTPtrSet<SkRefCnt*> {
public:
    virtual ~SkRefCntSet();

protected:
    // overrides
    virtual void incPtr(void*);
    virtual void decPtr(void*);
};

class SkFactorySet : public SkTPtrSet<SkFlattenable::Factory> {};

/**
 * Similar to SkFactorySet, but only allows Factorys that have registered names.
 * Also has a function to return the next added Factory's name.
 */
class SkNamedFactorySet : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkNamedFactorySet)

    SkNamedFactorySet();

    /**
     * Find the specified Factory in the set. If it is not already in the set,
     * and has registered its name, add it to the set, and return its index.
     * If the Factory has no registered name, return 0.
     */
    uint32_t find(SkFlattenable::Factory);

    /**
     * If new Factorys have been added to the set, return the name of the first
     * Factory added after the Factory name returned by the last call to this
     * function.
     */
    const char* getNextAddedFactoryName();
private:
    int                    fNextAddedFactory;
    SkFactorySet           fFactorySet;
    SkTDArray<const char*> fNames;

    typedef SkRefCnt INHERITED;
};

#endif
