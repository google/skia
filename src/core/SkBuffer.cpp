
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBuffer.h"

////////////////////////////////////////////////////////////////////////////////////////

void SkRBuffer::readNoSizeCheck(void* buffer, size_t size)
{
    SkASSERT((fData != 0 && fStop == 0) || fPos + size <= fStop);
    if (buffer)
        memcpy(buffer, fPos, size);
    fPos += size;
}

const void* SkRBuffer::skip(size_t size)
{
    const void* result = fPos;
    readNoSizeCheck(NULL, size);
    return result;
}

size_t SkRBuffer::skipToAlign4()
{
    size_t pos = this->pos();
    size_t n = SkAlign4(pos) - pos;
    fPos += n;
    return n;
}

void* SkWBuffer::skip(size_t size)
{
    void* result = fPos;
    writeNoSizeCheck(NULL, size);
    return fData == NULL ? NULL : result;
}

void SkWBuffer::writeNoSizeCheck(const void* buffer, size_t size)
{
    SkASSERT(fData == 0 || fStop == 0 || fPos + size <= fStop);
    if (fData && buffer)
        memcpy(fPos, buffer, size);
    fPos += size;
}

size_t SkWBuffer::padToAlign4()
{
    size_t pos = this->pos();
    size_t n = SkAlign4(pos) - pos;

    if (n && fData)
    {
        char* p = fPos;
        char* stop = p + n;
        do {
            *p++ = 0;
        } while (p < stop);
    }
    fPos += n;
    return n;
}

#if 0
#ifdef SK_DEBUG
    static void AssertBuffer32(const void* buffer)
    {
        SkASSERT(buffer);
        SkASSERT(((size_t)buffer & 3) == 0);
    }
#else
    #define AssertBuffer32(buffer)
#endif

void* sk_buffer_write_int32(void* buffer, int32_t value)
{
    AssertBuffer32(buffer);
    *(int32_t*)buffer = value;
    return (char*)buffer + sizeof(int32_t);
}

void* sk_buffer_write_int32(void* buffer, const int32_t values[], int count)
{
    AssertBuffer32(buffer);
    SkASSERT(count >= 0);

    memcpy((int32_t*)buffer, values, count * sizeof(int32_t));
    return (char*)buffer + count * sizeof(int32_t);
}

const void* sk_buffer_read_int32(const void* buffer, int32_t* value)
{
    AssertBuffer32(buffer);
    if (value)
        *value = *(const int32_t*)buffer;
    return (const char*)buffer + sizeof(int32_t);
}

const void* sk_buffer_read_int32(const void* buffer, int32_t values[], int count)
{
    AssertBuffer32(buffer);
    SkASSERT(count >= 0);

    if (values)
        memcpy(values, (const int32_t*)buffer, count * sizeof(int32_t));
    return (const char*)buffer + count * sizeof(int32_t);
}

void* sk_buffer_write_ptr(void* buffer, void* ptr)
{
    AssertBuffer32(buffer);
    *(void**)buffer = ptr;
    return (char*)buffer + sizeof(void*);
}

const void* sk_buffer_read_ptr(const void* buffer, void** ptr)
{
    AssertBuffer32(buffer);
    if (ptr)
        *ptr = *(void**)buffer;
    return (const char*)buffer + sizeof(void*);
}

#endif
