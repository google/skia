/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOrderedSet_DEFINED
#define GrOrderedSet_DEFINED

#include "GrRedBlackTree.h"

template <typename T, typename C = GrLess<T> >
class GrOrderedSet : SkNoncopyable {
public:
    /**
     * Creates an empty set
     */
    GrOrderedSet() : fComp() {}
    ~GrOrderedSet() {}

    class Iter;

    /**
     * @return true if there are no items in the set, false otherwise.
     */
    bool empty() const { return fRBTree.empty(); }

    /**
     * @return the number of items in the set.
     */
    int count() const { return fRBTree.count(); }

    /**
     * Removes all items in the set
     */
    void reset() { fRBTree.reset(); }

    /**
     * Adds an element to set if it does not already exists in the set.
     * @param t  the item to add
     * @return an iterator to added element or matching element already in set
     */
    Iter insert(const T& t);

    /**
     * Removes the item indicated by an iterator. The iterator will not be valid
     * afterwards.
     * @param iter      iterator of item to remove. Must be valid (not end()).
     */
    void remove(const Iter& iter);

    /**
     * @return  an iterator to the first item in sorted order, or end() if empty
     */
    Iter begin();

    /**
     * Gets the last valid iterator. This is always valid, even on an empty.
     * However, it can never be dereferenced. Useful as a loop terminator.
     * @return  an iterator that is just beyond the last item in sorted order.
     */
    Iter end();

    /**
     * @return  an iterator that to the last item in sorted order, or end() if
     * empty.
     */
    Iter last();

    /**
     * Finds an occurrence of an item.
     * @param t     the item to find.
     * @return an iterator to a set element equal to t or end() if none exists.
     */
    Iter find(const T& t);

private:
    GrRedBlackTree<T, C> fRBTree;

    const C fComp;
};

template <typename T, typename C>
class GrOrderedSet<T,C>::Iter {
public:
    Iter() {}
    Iter(const Iter& i) { fTreeIter = i.fTreeIter; }
    Iter& operator =(const Iter& i) {
        fTreeIter = i.fTreeIter;
        return *this;
    }
    const T& operator *() const { return *fTreeIter; }
    bool operator ==(const Iter& i) const {
        return fTreeIter == i.fTreeIter;
    }
    bool operator !=(const Iter& i) const { return !(*this == i); }
    Iter& operator ++() {
        ++fTreeIter;
        return *this;
    }
    Iter& operator --() {
        --fTreeIter;
        return *this;
    }
    const typename GrRedBlackTree<T,C>::Iter& getTreeIter() const {
        return fTreeIter;
    }

private:
    friend class GrOrderedSet;
    explicit Iter(typename GrRedBlackTree<T, C>::Iter iter) {
        fTreeIter = iter;
    }
    typename GrRedBlackTree<T,C>::Iter fTreeIter;
};

template <typename T, typename C>
typename GrOrderedSet<T,C>::Iter GrOrderedSet<T,C>::begin() {
    return Iter(fRBTree.begin());
}

template <typename T, typename C>
typename GrOrderedSet<T,C>::Iter GrOrderedSet<T,C>::end() {
    return Iter(fRBTree.end());
}

template <typename T, typename C>
typename GrOrderedSet<T,C>::Iter GrOrderedSet<T,C>::last() {
    return Iter(fRBTree.last());
}

template <typename T, typename C>
typename GrOrderedSet<T,C>::Iter GrOrderedSet<T,C>::find(const T& t) {
    return Iter(fRBTree.find(t));
}

template <typename T, typename C>
typename GrOrderedSet<T,C>::Iter GrOrderedSet<T,C>::insert(const T& t) {
    if (fRBTree.find(t) == fRBTree.end()) {
        return Iter(fRBTree.insert(t));
    } else {
        return Iter(fRBTree.find(t));
    }
}

template <typename T, typename C>
void GrOrderedSet<T,C>::remove(const typename GrOrderedSet<T,C>::Iter& iter) {
    if (this->end() != iter) {
        fRBTree.remove(iter.getTreeIter());
    }
}

#endif
