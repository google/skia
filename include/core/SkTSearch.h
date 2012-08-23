
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTSearch_DEFINED
#define SkTSearch_DEFINED

#include "SkTypes.h"

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

template <typename T>
int SkTSearch(const T* base, int count, const T& target, size_t elemSize)
{
    SkASSERT(count >= 0);
    if (count <= 0)
        return ~0;

    SkASSERT(base != NULL); // base may be NULL if count is zero

    int lo = 0;
    int hi = count - 1;

    while (lo < hi)
    {
        int mid = (hi + lo) >> 1;
        const T* elem = (const T*)((const char*)base + mid * elemSize);

        if (*elem < target)
            lo = mid + 1;
        else
            hi = mid;
    }

    const T* elem = (const T*)((const char*)base + hi * elemSize);
    if (*elem != target)
    {
        if (*elem < target)
            hi += 1;
        hi = ~hi;
    }
    return hi;
}

template <typename T, int (COMPARE)(const T*, const T*)>
int SkTSearch(const T* base, int count, const T& target, size_t elemSize)
{
    SkASSERT(count >= 0);
    if (count <= 0) {
        return ~0;
    }

    SkASSERT(base != NULL); // base may be NULL if count is zero

    int lo = 0;
    int hi = count - 1;

    while (lo < hi) {
        int mid = (hi + lo) >> 1;
        const T* elem = (const T*)((const char*)base + mid * elemSize);

        if (COMPARE(elem, &target) < 0)
            lo = mid + 1;
        else
            hi = mid;
    }

    const T* elem = (const T*)((const char*)base + hi * elemSize);
    int pred = COMPARE(elem, &target);
    if (pred != 0) {
        if (pred < 0)
            hi += 1;
        hi = ~hi;
    }
    return hi;
}

template <typename T>
int SkTSearch(const T* base, int count, const T& target, size_t elemSize,
              int (*compare)(const T*, const T*))
{
    SkASSERT(count >= 0);
    if (count <= 0) {
        return ~0;
    }

    SkASSERT(base != NULL); // base may be NULL if count is zero

    int lo = 0;
    int hi = count - 1;

    while (lo < hi) {
        int mid = (hi + lo) >> 1;
        const T* elem = (const T*)((const char*)base + mid * elemSize);

        if ((*compare)(elem, &target) < 0)
            lo = mid + 1;
        else
            hi = mid;
    }

    const T* elem = (const T*)((const char*)base + hi * elemSize);
    int pred = (*compare)(elem, &target);
    if (pred != 0) {
        if (pred < 0)
            hi += 1;
        hi = ~hi;
    }
    return hi;
}

template <typename T>
int SkTSearch(const T** base, int count, const T* target, size_t elemSize,
    int (*compare)(const T*, const T*))
{
    SkASSERT(count >= 0);
    if (count <= 0)
        return ~0;

    SkASSERT(base != NULL); // base may be NULL if count is zero

    int lo = 0;
    int hi = count - 1;

    while (lo < hi)
    {
        int mid = (hi + lo) >> 1;
        const T* elem = *(const T**)((const char*)base + mid * elemSize);

        if ((*compare)(elem, target) < 0)
            lo = mid + 1;
        else
            hi = mid;
    }

    const T* elem = *(const T**)((const char*)base + hi * elemSize);
    int pred = (*compare)(elem, target);
    if (pred != 0)
    {
        if (pred < 0)
            hi += 1;
        hi = ~hi;
    }
    return hi;
}

template <typename T,  int (COMPARE)(const T*, const T*)>
int SkTSearch(const T** base, int count, const T* target, size_t elemSize)
{
    SkASSERT(count >= 0);
    if (count <= 0)
        return ~0;

    SkASSERT(base != NULL); // base may be NULL if count is zero

    int lo = 0;
    int hi = count - 1;

    while (lo < hi)
    {
        int mid = (hi + lo) >> 1;
        const T* elem = *(const T**)((const char*)base + mid * elemSize);

        if (COMPARE(elem, target) < 0)
            lo = mid + 1;
        else
            hi = mid;
    }

    const T* elem = *(const T**)((const char*)base + hi * elemSize);
    int pred = COMPARE(elem, target);
    if (pred != 0)
    {
        if (pred < 0)
            hi += 1;
        hi = ~hi;
    }
    return hi;
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

