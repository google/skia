/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/base/SkTSearch.h"

#include "include/private/base/SkMalloc.h"

#include <cstring>
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

    SkASSERT(base != nullptr);

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
