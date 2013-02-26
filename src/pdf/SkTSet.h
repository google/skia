/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTSet_DEFINED
#define SkTSet_DEFINED

#include "SkTDArray.h"
#include "SkTypes.h"

/** \class SkTSet<T>

    The SkTSet template class defines a set.
    Main operations supported now are: add, merge, find and contains.

    TSet<T> is mutable.
*/

// TODO: Add remove, intersect and difference operations.
// TODO: Add bench tests.
template <typename T> class SK_API SkTSet {
public:
    SkTSet() {
        fArray = SkNEW(SkTDArray<T>);
    }

    ~SkTSet() {
        SkASSERT(fArray);
        SkDELETE(fArray);
    }

    SkTSet(const SkTSet<T>& src) {
        this->fArray = SkNEW_ARGS(SkTDArray<T>, (*src.fArray));
#ifdef SK_DEBUG
        validate();
#endif
    }

    SkTSet<T>& operator=(const SkTSet<T>& src) {
        *this->fArray = *src.fArray;
#ifdef SK_DEBUG
        validate();
#endif
        return *this;
    }

    /** Merges src elements into this, and returns the number of duplicates
     * found.
    */
    int mergeInto(const SkTSet<T>& src) {
        SkASSERT(fArray);
        int duplicates = 0;

        SkTDArray<T>* fArrayNew = new SkTDArray<T>();
        fArrayNew->setReserve(count() + src.count());
        int i = 0;
        int j = 0;

        while (i < count() && j < src.count()) {
            if ((*fArray)[i] < (*src.fArray)[j]) {
                fArrayNew->push((*fArray)[i]);
                i++;
            } else if ((*fArray)[i] > (*src.fArray)[j]) {
                fArrayNew->push((*src.fArray)[j]);
                j++;
            } else {
                duplicates++;
                j++; // Skip one of the duplicates.
            }
        }

        while (i < count()) {
            fArrayNew->push((*fArray)[i]);
            i++;
        }

        while (j < src.count()) {
            fArrayNew->push((*src.fArray)[j]);
            j++;
        }
        SkDELETE(fArray);
        fArray = fArrayNew;
        fArrayNew = NULL;

#ifdef SK_DEBUG
        validate();
#endif
        return duplicates;
    }

    /** Adds a new element into set and returns true if the element is already
     * in this set.
    */
    bool add(const T& elem) {
        SkASSERT(fArray);

        int pos = 0;
        int i = find(elem, &pos);
        if (i >= 0) {
            return false;
        }
        *fArray->insert(pos) = elem;
#ifdef SK_DEBUG
        validate();
#endif
        return true;
    }

    /** Returns true if this set is empty.
    */
    bool isEmpty() const {
        SkASSERT(fArray);
        return fArray->isEmpty();
    }

    /** Return the number of elements in the set.
     */
    int count() const {
        SkASSERT(fArray);
        return fArray->count();
    }

    /** Return the number of bytes in the set: count * sizeof(T).
     */
    size_t bytes() const {
        SkASSERT(fArray);
        return fArray->bytes();
    }

    /** Return the beginning of a set iterator.
     * Elements in the iterator will be sorted ascending.
     */
    const T*  begin() const {
        SkASSERT(fArray);
        return fArray->begin();
    }

    /** Return the end of a set iterator.
     */
    const T*  end() const {
        SkASSERT(fArray);
        return fArray->end();
    }

    const T&  operator[](int index) const {
        SkASSERT(fArray);
        return (*fArray)[index];
    }

    /** Resets the set (deletes memory and initiates an empty set).
     */
    void reset() {
        SkASSERT(fArray);
        fArray->reset();
    }

    /** Rewinds the set (preserves memory and initiates an empty set).
     */
    void rewind() {
        SkASSERT(fArray);
        fArray->rewind();
    }

    /** Reserves memory for the set.
     */
    void setReserve(size_t reserve) {
        SkASSERT(fArray);
        fArray->setReserve(reserve);
    }

    /** Returns the index where an element was found.
     * Returns -1 if the element was not found, and it fills *posToInsertSorted
     * with the index of the place where elem should be inserted to preserve the
     * internal array sorted.
     * If element was found, *posToInsertSorted is undefined.
     */
    int find(const T& elem, int* posToInsertSorted = NULL) const {
        SkASSERT(fArray);

        if (fArray->count() == 0) {
            if (posToInsertSorted) {
                *posToInsertSorted = 0;
            }
            return -1;
        }
        int iMin = 0;
        int iMax = fArray->count();

        while (iMin < iMax - 1) {
            int iMid = (iMin + iMax) / 2;
            if (elem < (*fArray)[iMid]) {
                iMax = iMid;
            } else {
                iMin = iMid;
            }
        }
        if (elem == (*fArray)[iMin]) {
            return iMin;
        }
        if (posToInsertSorted) {
            if (elem < (*fArray)[iMin]) {
                *posToInsertSorted = iMin;
            } else {
                *posToInsertSorted = iMin + 1;
            }
        }

        return -1;
    }

    /** Returns true if the array contains this element.
     */
    bool contains(const T& elem) const {
        SkASSERT(fArray);
        return (this->find(elem) >= 0);
    }

    /** Copies internal array to destination.
     */
    void copy(T* dst) const {
        SkASSERT(fArray);
        fArray->copyRange(0, fArray->count(), dst);
    }

    /** Returns a const reference to the internal vector.
     */
    const SkTDArray<T>& toArray() {
        SkASSERT(fArray);
        return *fArray;
    }

    /** Unref all elements in the set.
     */
    void unrefAll() {
        SkASSERT(fArray);
        fArray->unrefAll();
    }

    /** safeUnref all elements in the set.
     */
     void safeUnrefAll() {
        SkASSERT(fArray);
        fArray->safeUnrefAll();
    }

#ifdef SK_DEBUG
    void validate() const {
        SkASSERT(fArray);
        fArray->validate();
        SkASSERT(isSorted() && !hasDuplicates());
    }

    bool hasDuplicates() const {
        for (int i = 0; i < fArray->count() - 1; ++i) {
            if ((*fArray)[i] == (*fArray)[i + 1]) {
                return true;
            }
        }
        return false;
    }

    bool isSorted() const {
        for (int i = 0; i < fArray->count() - 1; ++i) {
            // Use only < operator
            if (!((*fArray)[i] < (*fArray)[i + 1])) {
                return false;
            }
        }
        return true;
    }
#endif

private:
    SkTDArray<T>* fArray;
};

#endif
