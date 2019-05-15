
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTSearch_DEFINED
#define SkTSearch_DEFINED

#include "include/core/SkTypes.h"

/**
 *  All of the SkTSearch variants want to return the index (0...N-1) of the
 *  found element, or the bit-not of where to insert the element.
 *
 *  At a simple level, if the return value is negative, it was not found.
 *
 *  For clients that want to insert the new element if it was not found, use
 *  the following logic:
 *
 *  int index = SkTSearch(...);
 *  if (index >= 0) {
 *      // found at index
 *  } else {
 *      index = ~index; // now we are positive
 *      // insert at index
 *  }
 */


// The most general form of SkTSearch takes an array of T and a key of type K. A functor, less, is
// used to perform comparisons. It has two function operators:
//      bool operator() (const T& t, const K& k)
//      bool operator() (const K& t, const T& k)
template <typename T, typename K, typename LESS>
int SkTSearch(const T base[], int count, const K& key, size_t elemSize, LESS& less)
{
    SkASSERT(count >= 0);
    if (count <= 0) {
        return ~0;
    }

    SkASSERT(base != nullptr); // base may be nullptr if count is zero

    int lo = 0;
    int hi = count - 1;

    while (lo < hi) {
        int mid = lo + ((hi - lo) >> 1);
        const T* elem = (const T*)((const char*)base + mid * elemSize);

        if (less(*elem, key))
            lo = mid + 1;
        else
            hi = mid;
    }

    const T* elem = (const T*)((const char*)base + hi * elemSize);
    if (less(*elem, key)) {
        hi += 1;
        hi = ~hi;
    } else if (less(key, *elem)) {
        hi = ~hi;
    }
    return hi;
}

// Adapts a less-than function to a functor.
template <typename T, bool (LESS)(const T&, const T&)> struct SkTLessFunctionToFunctorAdaptor {
    bool operator()(const T& a, const T& b) { return LESS(a, b); }
};

// Specialization for case when T==K and the caller wants to use a function rather than functor.
template <typename T, bool (LESS)(const T&, const T&)>
int SkTSearch(const T base[], int count, const T& target, size_t elemSize) {
    static SkTLessFunctionToFunctorAdaptor<T, LESS> functor;
    return SkTSearch(base, count, target, elemSize, functor);
}

// Adapts operator < to a functor.
template <typename T> struct SkTLessFunctor {
    bool operator()(const T& a, const T& b) { return a < b; }
};

// Specialization for T==K, compare using op <.
template <typename T>
int SkTSearch(const T base[], int count, const T& target, size_t elemSize) {
    static SkTLessFunctor<T> functor;
    return SkTSearch(base, count, target, elemSize, functor);
}

// Similar to SkLessFunctionToFunctorAdaptor but makes the functor interface take T* rather than T.
template <typename T, bool (LESS)(const T&, const T&)> struct SkTLessFunctionToPtrFunctorAdaptor {
    bool operator() (const T* t, const T* k) { return LESS(*t, *k); }
};

// Specialization for case where domain is an array of T* and the key value is a T*, and you want
// to compare the T objects, not the pointers.
template <typename T, bool (LESS)(const T&, const T&)>
int SkTSearch(T* base[], int count, T* target, size_t elemSize) {
    static SkTLessFunctionToPtrFunctorAdaptor<T, LESS> functor;
    return SkTSearch(base, count, target, elemSize, functor);
}

int SkStrSearch(const char*const* base, int count, const char target[],
                size_t target_len, size_t elemSize);
int SkStrSearch(const char*const* base, int count, const char target[],
                size_t elemSize);

/** Like SkStrSearch, but treats target as if it were all lower-case. Assumes that
    base points to a table of lower-case strings.
*/
int SkStrLCSearch(const char*const* base, int count, const char target[],
                  size_t target_len, size_t elemSize);
int SkStrLCSearch(const char*const* base, int count, const char target[],
                  size_t elemSize);

/** Helper class to convert a string to lower-case, but only modifying the ascii
    characters. This makes the routine very fast and never changes the string
    length, but it is not suitable for linguistic purposes. Normally this is
    used for buiding and searching string tables.
*/
class SkAutoAsciiToLC {
public:
    SkAutoAsciiToLC(const char str[], size_t len = (size_t)-1);
    ~SkAutoAsciiToLC();

    const char* lc() const { return fLC; }
    size_t      length() const { return fLength; }

private:
    char*   fLC;    // points to either the heap or fStorage
    size_t  fLength;
    enum {
        STORAGE = 64
    };
    char    fStorage[STORAGE+1];
};

// Helper when calling qsort with a compare proc that has typed its arguments
#define SkCastForQSort(compare) reinterpret_cast<int (*)(const void*, const void*)>(compare)

#endif
