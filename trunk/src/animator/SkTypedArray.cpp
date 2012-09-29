
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkTypedArray.h"

SkTypedArray::SkTypedArray() : fType(SkType_Unknown) {
}

SkTypedArray::SkTypedArray(SkDisplayTypes type) : fType(type) {
}

bool SkTypedArray::getIndex(int index, SkOperand* operand) {
    if (index >= count()) {
        SkASSERT(0);
        return false;
    }
    *operand = begin()[index];
    return true;
}


#if SK_SMALLER_ARRAY_TEMPLATE_EXPERIMENT == 1
SkDS32Array::SkDS32Array()
{
    fReserve = fCount = 0;
    fArray = NULL;
#ifdef SK_DEBUG
    fData = NULL;
#endif
}

SkDS32Array::SkDS32Array(const SkDS32Array& src)
{
    fReserve = fCount = 0;
    fArray = NULL;
#ifdef SK_DEBUG
    fData = NULL;
#endif
    SkDS32Array tmp(src.fArray, src.fCount);
    this->swap(tmp);
}

SkDS32Array::SkDS32Array(const int32_t src[], U16CPU count)
{
    SkASSERT(src || count == 0);

    fReserve = fCount = 0;
    fArray = NULL;
#ifdef SK_DEBUG
    fData = NULL;
#endif
    if (count)
    {
        fArray = (int32_t*)sk_malloc_throw(count * sizeof(int32_t));
#ifdef SK_DEBUG
        fData = (int32_t (*)[kDebugArraySize]) fArray;
#endif
        memcpy(fArray, src, sizeof(int32_t) * count);
        fReserve = fCount = SkToU16(count);
    }
}

SkDS32Array& SkDS32Array::operator=(const SkDS32Array& src)
{
    if (this != &src)
    {
        if (src.fCount > fReserve)
        {
            SkDS32Array tmp(src.fArray, src.fCount);
            this->swap(tmp);
        }
        else
        {
            memcpy(fArray, src.fArray, sizeof(int32_t) * src.fCount);
            fCount = src.fCount;
        }
    }
    return *this;
}

int operator==(const SkDS32Array& a, const SkDS32Array& b)
{
    return a.fCount == b.fCount &&
            (a.fCount == 0 || !memcmp(a.fArray, b.fArray, a.fCount * sizeof(int32_t)));
}

void SkDS32Array::swap(SkDS32Array& other)
{
    SkTSwap(fArray, other.fArray);
#ifdef SK_DEBUG
    SkTSwap(fData, other.fData);
#endif
    SkTSwap(fReserve, other.fReserve);
    SkTSwap(fCount, other.fCount);
}

int32_t* SkDS32Array::append(U16CPU count, const int32_t* src)
{
    unsigned oldCount = fCount;
    if (count)
    {
        SkASSERT(src == NULL || fArray == NULL ||
                src + count <= fArray || fArray + count <= src);

        this->growBy(count);
        if (src)
            memcpy(fArray + oldCount, src, sizeof(int32_t) * count);
    }
    return fArray + oldCount;
}

int SkDS32Array::find(const int32_t& elem) const
{
    const int32_t* iter = fArray;
    const int32_t* stop = fArray + fCount;

    for (; iter < stop; iter++)
    {
        if (*iter == elem)
            return (int) (iter - fArray);
    }
    return -1;
}

void SkDS32Array::growBy(U16CPU extra)
{
    SkASSERT(extra);
    SkASSERT(fCount + extra <= 0xFFFF);

    if (fCount + extra > fReserve)
    {
        size_t size = fCount + extra + 4;
        size += size >> 2;
        int32_t* array = (int32_t*)sk_malloc_throw(size * sizeof(int32_t));
        memcpy(array, fArray, fCount * sizeof(int32_t));

        sk_free(fArray);
        fArray = array;
#ifdef SK_DEBUG
        fData = (int32_t (*)[kDebugArraySize]) fArray;
#endif
        fReserve = SkToU16((U16CPU)size);
    }
    fCount = SkToU16(fCount + extra);
}

int32_t* SkDS32Array::insert(U16CPU index, U16CPU count, const int32_t* src)
{
    SkASSERT(count);
    int oldCount = fCount;
    this->growBy(count);
    int32_t* dst = fArray + index;
    memmove(dst + count, dst, sizeof(int32_t) * (oldCount - index));
    if (src)
        memcpy(dst, src, sizeof(int32_t) * count);
    return dst;
}


    int SkDS32Array::rfind(const int32_t& elem) const
    {
        const int32_t* iter = fArray + fCount;
        const int32_t* stop = fArray;

        while (iter > stop)
        {
            if (*--iter == elem)
                return (int) (iter - stop);
        }
        return -1;
    }

#endif
