/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTSet_DEFINED
#define SkTSet_DEFINED

#include "SkTSort.h"
#include "SkTDArray.h"
#include "SkTypes.h"

/** \class SkTSet<T>

    The SkTSet template class defines a set. Elements are additionally
    guaranteed to be sorted by their insertion order.
    Main operations supported now are: add, merge, find and contains.

    TSet<T> is mutable.
*/

// TODO: Add remove, intersect and difference operations.
// TODO: Add bench tests.
template <typename T> class SkTSet {
public:
    SkTSet() {
        fSetArray = SkNEW(SkTDArray<T>);
        fOrderedArray = SkNEW(SkTDArray<T>);
    }

    ~SkTSet() {
        SkASSERT(fSetArray);
        SkDELETE(fSetArray);
        SkASSERT(fOrderedArray);
        SkDELETE(fOrderedArray);
    }

    SkTSet(const SkTSet<T>& src) {
        this->fSetArray = SkNEW_ARGS(SkTDArray<T>, (*src.fSetArray));
        this->fOrderedArray = SkNEW_ARGS(SkTDArray<T>, (*src.fOrderedArray));
#ifdef SK_DEBUG
        validate();
#endif
    }

    SkTSet<T>& operator=(const SkTSet<T>& src) {
        *this->fSetArray = *src.fSetArray;
        *this->fOrderedArray = *src.fOrderedArray;
#ifdef SK_DEBUG
        validate();
#endif
        return *this;
    }

    /** Merges src elements into this, and returns the number of duplicates
     * found. Elements from src will retain their ordering and will be ordered
     * after the elements currently in this set.
     *
     * Implementation note: this uses a 2-stage merge to obtain O(n log n) time.
     * The first stage goes through src.fOrderedArray, checking if
     * this->contains() is false before adding to this.fOrderedArray.
     * The second stage does a standard sorted list merge on the fSetArrays.
     */
    int mergeInto(const SkTSet<T>& src) {
        SkASSERT(fSetArray);
        SkASSERT(fOrderedArray);

        // Do fOrderedArray merge.
        for (int i = 0; i < src.count(); ++i) {
            if (!contains((*src.fOrderedArray)[i])) {
                fOrderedArray->push((*src.fOrderedArray)[i]);
            }
        }

        // Do fSetArray merge.
        int duplicates = 0;

        SkTDArray<T>* fArrayNew = new SkTDArray<T>();
        fArrayNew->setReserve(fOrderedArray->count());
        int i = 0;
        int j = 0;

        while (i < fSetArray->count() && j < src.count()) {
            if ((*fSetArray)[i] < (*src.fSetArray)[j]) {
                fArrayNew->push((*fSetArray)[i]);
                i++;
            } else if ((*fSetArray)[i] > (*src.fSetArray)[j]) {
                fArrayNew->push((*src.fSetArray)[j]);
                j++;
            } else {
                duplicates++;
                j++; // Skip one of the duplicates.
            }
        }

        while (i < fSetArray->count()) {
            fArrayNew->push((*fSetArray)[i]);
            i++;
        }

        while (j < src.count()) {
            fArrayNew->push((*src.fSetArray)[j]);
            j++;
        }
        SkDELETE(fSetArray);
        fSetArray = fArrayNew;
        fArrayNew = NULL;

#ifdef SK_DEBUG
        validate();
#endif
        return duplicates;
    }

    /** Adds a new element into set and returns false if the element is already
     * in this set.
    */
    bool add(const T& elem) {
        SkASSERT(fSetArray);
        SkASSERT(fOrderedArray);

        int pos = 0;
        int i = find(elem, &pos);
        if (i >= 0) {
            return false;
        }
        *fSetArray->insert(pos) = elem;
        fOrderedArray->push(elem);
#ifdef SK_DEBUG
        validate();
#endif
        return true;
    }

    /** Returns true if this set is empty.
    */
    bool isEmpty() const {
        SkASSERT(fOrderedArray);
        SkASSERT(fSetArray);
        SkASSERT(fSetArray->isEmpty() == fOrderedArray->isEmpty());
        return fOrderedArray->isEmpty();
    }

    /** Return the number of elements in the set.
     */
    int count() const {
        SkASSERT(fOrderedArray);
        SkASSERT(fSetArray);
        SkASSERT(fSetArray->count() == fOrderedArray->count());
        return fOrderedArray->count();
    }

    /** Return the number of bytes in the set: count * sizeof(T).
     */
    size_t bytes() const {
        SkASSERT(fOrderedArray);
        return fOrderedArray->bytes();
    }

    /** Return the beginning of a set iterator.
     * Elements in the iterator will be sorted ascending.
     */
    const T*  begin() const {
        SkASSERT(fOrderedArray);
        return fOrderedArray->begin();
    }

    /** Return the end of a set iterator.
     */
    const T*  end() const {
        SkASSERT(fOrderedArray);
        return fOrderedArray->end();
    }

    const T&  operator[](int index) const {
        SkASSERT(fOrderedArray);
        return (*fOrderedArray)[index];
    }

    /** Resets the set (deletes memory and initiates an empty set).
     */
    void reset() {
        SkASSERT(fSetArray);
        SkASSERT(fOrderedArray);
        fSetArray->reset();
        fOrderedArray->reset();
    }

    /** Rewinds the set (preserves memory and initiates an empty set).
     */
    void rewind() {
        SkASSERT(fSetArray);
        SkASSERT(fOrderedArray);
        fSetArray->rewind();
        fOrderedArray->rewind();
    }

    /** Reserves memory for the set.
     */
    void setReserve(size_t reserve) {
        SkASSERT(fSetArray);
        SkASSERT(fOrderedArray);
        fSetArray->setReserve(reserve);
        fOrderedArray->setReserve(reserve);
    }

    /** Returns true if the array contains this element.
     */
    bool contains(const T& elem) const {
        SkASSERT(fSetArray);
        return (this->find(elem) >= 0);
    }

    /** Copies internal array to destination.
     */
    void copy(T* dst) const {
        SkASSERT(fOrderedArray);
        fOrderedArray->copyRange(dst, 0, fOrderedArray->count());
    }

    /** Returns a const reference to the internal vector.
     */
    const SkTDArray<T>& toArray() {
        SkASSERT(fOrderedArray);
        return *fOrderedArray;
    }

    /** Unref all elements in the set.
     */
    void unrefAll() {
        SkASSERT(fSetArray);
        SkASSERT(fOrderedArray);
        fOrderedArray->unrefAll();
        // Also reset the other array, as SkTDArray::unrefAll does an
        // implcit reset
        fSetArray->reset();
    }

    /** safeUnref all elements in the set.
     */
    void safeUnrefAll() {
        SkASSERT(fSetArray);
        SkASSERT(fOrderedArray);
        fOrderedArray->safeUnrefAll();
        // Also reset the other array, as SkTDArray::safeUnrefAll does an
        // implcit reset
        fSetArray->reset();
    }

#ifdef SK_DEBUG
    void validate() const {
        SkASSERT(fSetArray);
        SkASSERT(fOrderedArray);
        fSetArray->validate();
        fOrderedArray->validate();
        SkASSERT(isSorted() && !hasDuplicates() && arraysConsistent());
    }

    bool hasDuplicates() const {
        for (int i = 0; i < fSetArray->count() - 1; ++i) {
            if ((*fSetArray)[i] == (*fSetArray)[i + 1]) {
                return true;
            }
        }
        return false;
    }

    bool isSorted() const {
        for (int i = 0; i < fSetArray->count() - 1; ++i) {
            // Use only < operator
            if (!((*fSetArray)[i] < (*fSetArray)[i + 1])) {
                return false;
            }
        }
        return true;
    }

    /** Checks if fSetArray is consistent with fOrderedArray
     */
    bool arraysConsistent() const {
        if (fSetArray->count() != fOrderedArray->count()) {
            return false;
        }
        if (fOrderedArray->count() == 0) {
            return true;
        }

        // Copy and sort fOrderedArray, then compare to fSetArray.
        // A O(n log n) algorithm is necessary as O(n^2) will choke some GMs.
        SkAutoMalloc sortedArray(fOrderedArray->bytes());
        T* sortedBase = reinterpret_cast<T*>(sortedArray.get());
        int count = fOrderedArray->count();
        fOrderedArray->copyRange(sortedBase, 0, count);

        SkTQSort<T>(sortedBase, sortedBase + count - 1);

        for (int i = 0; i < count; ++i) {
            if (sortedBase[i] != (*fSetArray)[i]) {
                return false;
            }
        }

        return true;
    }
#endif

private:
    SkTDArray<T>* fSetArray;        // Sorted by pointer address for fast
                                    // lookup.
    SkTDArray<T>* fOrderedArray;    // Sorted by insertion order for
                                    // deterministic output.

    /** Returns the index in fSetArray where an element was found.
     * Returns -1 if the element was not found, and it fills *posToInsertSorted
     * with the index of the place where elem should be inserted to preserve the
     * internal array sorted.
     * If element was found, *posToInsertSorted is undefined.
     */
    int find(const T& elem, int* posToInsertSorted = NULL) const {
        SkASSERT(fSetArray);

        if (fSetArray->count() == 0) {
            if (posToInsertSorted) {
                *posToInsertSorted = 0;
            }
            return -1;
        }
        int iMin = 0;
        int iMax = fSetArray->count();

        while (iMin < iMax - 1) {
            int iMid = (iMin + iMax) / 2;
            if (elem < (*fSetArray)[iMid]) {
                iMax = iMid;
            } else {
                iMin = iMid;
            }
        }
        if (elem == (*fSetArray)[iMin]) {
            return iMin;
        }
        if (posToInsertSorted) {
            if (elem < (*fSetArray)[iMin]) {
                *posToInsertSorted = iMin;
            } else {
                *posToInsertSorted = iMin + 1;
            }
        }

        return -1;
    }
};

#endif
