
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkTSearch.h"
#include <ctype.h>

static inline const char* index_into_base(const char*const* base, int index,
                                          size_t elemSize)
{
    return *(const char*const*)((const char*)base + index * elemSize);
}

int SkStrSearch(const char*const* base, int count, const char target[],
                size_t target_len, size_t elemSize)
{
    if (count <= 0)
        return ~0;

    SkASSERT(base != NULL);

    int lo = 0;
    int hi = count - 1;

    while (lo < hi)
    {
        int mid = (hi + lo) >> 1;
        const char* elem = index_into_base(base, mid, elemSize);

        int cmp = strncmp(elem, target, target_len);
        if (cmp < 0)
            lo = mid + 1;
        else if (cmp > 0 || strlen(elem) > target_len)
            hi = mid;
        else
            return mid;
    }

    const char* elem = index_into_base(base, hi, elemSize);
    int cmp = strncmp(elem, target, target_len);
    if (cmp || strlen(elem) > target_len)
    {
        if (cmp < 0)
            hi += 1;
        hi = ~hi;
    }
    return hi;
}

int SkStrSearch(const char*const* base, int count, const char target[],
                size_t elemSize)
{
    return SkStrSearch(base, count, target, strlen(target), elemSize);
}

int SkStrLCSearch(const char*const* base, int count, const char target[],
                  size_t len, size_t elemSize)
{
    SkASSERT(target);

    SkAutoAsciiToLC tolc(target, len);

    return SkStrSearch(base, count, tolc.lc(), len, elemSize);
}

int SkStrLCSearch(const char*const* base, int count, const char target[],
                  size_t elemSize)
{
    return SkStrLCSearch(base, count, target, strlen(target), elemSize);
}

//////////////////////////////////////////////////////////////////////////////

SkAutoAsciiToLC::SkAutoAsciiToLC(const char str[], size_t len)
{
    // see if we need to compute the length
    if ((long)len < 0) {
        len = strlen(str);
    }
    fLength = len;

    // assign lc to our preallocated storage if len is small enough, or allocate
    // it on the heap
    char*   lc;
    if (len <= STORAGE) {
        lc = fStorage;
    } else {
        lc = (char*)sk_malloc_throw(len + 1);
    }
    fLC = lc;
    
    // convert any asii to lower-case. we let non-ascii (utf8) chars pass
    // through unchanged
    for (int i = (int)(len - 1); i >= 0; --i) {
        int c = str[i];
        if ((c & 0x80) == 0) {   // is just ascii
            c = tolower(c);
        }
        lc[i] = c;
    }
    lc[len] = 0;
}

SkAutoAsciiToLC::~SkAutoAsciiToLC()
{
    if (fLC != fStorage) {
        sk_free(fLC);
    }
}

//////////////////////////////////////////////////////////////////////////////

#define SK_QSortTempSize    16

static inline void sk_qsort_swap(char a[], char b[], size_t elemSize)
{
    char    tmp[SK_QSortTempSize];

    while (elemSize > 0)
    {
        size_t size = elemSize;
        if (size > SK_QSortTempSize)
            size = SK_QSortTempSize;
        elemSize -= size;

        memcpy(tmp, a, size);
        memcpy(a, b, size);
        memcpy(b, tmp, size);
        a += size;
        b += size;
    }
}

static void SkQSort_Partition(char* first, char* last, size_t elemSize, SkQSortCompareProc compare)
{
    char*   left = first;
    char*   rite = last;
    char*   pivot = left;

    while (left <= rite)
    {
        while (left < last && compare(left, pivot) < 0)
            left += elemSize;
        while (first < rite && compare(rite, pivot) > 0)
            rite -= elemSize;
        if (left <= rite)
        {
            if (left < rite)
            {
                SkASSERT(compare(left, rite) >= 0);
                sk_qsort_swap(left, rite, elemSize);
            }
            left += elemSize;
            rite -= elemSize;
        }
    }
    if (first < rite)
        SkQSort_Partition(first, rite, elemSize, compare);
    if (left < last)
        SkQSort_Partition(left, last, elemSize, compare);
}

void SkQSort(void* base, size_t count, size_t elemSize, SkQSortCompareProc compare)
{
    SkASSERT(base);
    SkASSERT(compare);
    SkASSERT(elemSize > 0);

    if (count <= 1)
        return;

    SkQSort_Partition((char*)base, (char*)base + (count - 1) * elemSize, elemSize, compare);
}

