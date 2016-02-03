
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkUtils.h"

/*  0xxxxxxx    1 total
    10xxxxxx    // never a leading byte
    110xxxxx    2 total
    1110xxxx    3 total
    11110xxx    4 total

    11 10 01 01 xx xx xx xx 0...
    0xE5XX0000
    0xE5 << 24
*/

#ifdef SK_DEBUG
    static void assert_utf8_leadingbyte(unsigned c) {
        SkASSERT(c <= 0xF7);    // otherwise leading byte is too big (more than 4 bytes)
        SkASSERT((c & 0xC0) != 0x80);   // can't begin with a middle char
    }

    int SkUTF8_LeadByteToCount(unsigned c) {
        assert_utf8_leadingbyte(c);
        return (((0xE5 << 24) >> (c >> 4 << 1)) & 3) + 1;
    }
#else
    #define assert_utf8_leadingbyte(c)
#endif

int SkUTF8_CountUnichars(const char utf8[]) {
    SkASSERT(utf8);

    int count = 0;

    for (;;) {
        int c = *(const uint8_t*)utf8;
        if (c == 0) {
            break;
        }
        utf8 += SkUTF8_LeadByteToCount(c);
        count += 1;
    }
    return count;
}

int SkUTF8_CountUnichars(const char utf8[], size_t byteLength) {
    SkASSERT(utf8 || 0 == byteLength);

    int         count = 0;
    const char* stop = utf8 + byteLength;

    while (utf8 < stop) {
        utf8 += SkUTF8_LeadByteToCount(*(const uint8_t*)utf8);
        count += 1;
    }
    return count;
}

SkUnichar SkUTF8_ToUnichar(const char utf8[]) {
    SkASSERT(utf8);

    const uint8_t*  p = (const uint8_t*)utf8;
    int             c = *p;
    int             hic = c << 24;

    assert_utf8_leadingbyte(c);

    if (hic < 0) {
        uint32_t mask = (uint32_t)~0x3F;
        hic = SkLeftShift(hic, 1);
        do {
            c = (c << 6) | (*++p & 0x3F);
            mask <<= 5;
        } while ((hic = SkLeftShift(hic, 1)) < 0);
        c &= ~mask;
    }
    return c;
}

SkUnichar SkUTF8_NextUnichar(const char** ptr) {
    SkASSERT(ptr && *ptr);

    const uint8_t*  p = (const uint8_t*)*ptr;
    int             c = *p;
    int             hic = c << 24;

    assert_utf8_leadingbyte(c);

    if (hic < 0) {
        uint32_t mask = (uint32_t)~0x3F;
        hic = SkLeftShift(hic, 1);
        do {
            c = (c << 6) | (*++p & 0x3F);
            mask <<= 5;
        } while ((hic = SkLeftShift(hic, 1)) < 0);
        c &= ~mask;
    }
    *ptr = (char*)p + 1;
    return c;
}

SkUnichar SkUTF8_PrevUnichar(const char** ptr) {
    SkASSERT(ptr && *ptr);

    const char* p = *ptr;

    if (*--p & 0x80) {
        while (*--p & 0x40) {
            ;
        }
    }

    *ptr = (char*)p;
    return SkUTF8_NextUnichar(&p);
}

size_t SkUTF8_FromUnichar(SkUnichar uni, char utf8[]) {
    if ((uint32_t)uni > 0x10FFFF) {
        SkDEBUGFAIL("bad unichar");
        return 0;
    }

    if (uni <= 127) {
        if (utf8) {
            *utf8 = (char)uni;
        }
        return 1;
    }

    char    tmp[4];
    char*   p = tmp;
    size_t  count = 1;

    SkDEBUGCODE(SkUnichar orig = uni;)

    while (uni > 0x7F >> count) {
        *p++ = (char)(0x80 | (uni & 0x3F));
        uni >>= 6;
        count += 1;
    }

    if (utf8) {
        p = tmp;
        utf8 += count;
        while (p < tmp + count - 1) {
            *--utf8 = *p++;
        }
        *--utf8 = (char)(~(0xFF >> count) | uni);
    }

    SkASSERT(utf8 == nullptr || orig == SkUTF8_ToUnichar(utf8));
    return count;
}

///////////////////////////////////////////////////////////////////////////////

int SkUTF16_CountUnichars(const uint16_t src[]) {
    SkASSERT(src);

    int count = 0;
    unsigned c;
    while ((c = *src++) != 0) {
        SkASSERT(!SkUTF16_IsLowSurrogate(c));
        if (SkUTF16_IsHighSurrogate(c)) {
            c = *src++;
            SkASSERT(SkUTF16_IsLowSurrogate(c));
        }
        count += 1;
    }
    return count;
}

int SkUTF16_CountUnichars(const uint16_t src[], int numberOf16BitValues) {
    SkASSERT(src);

    const uint16_t* stop = src + numberOf16BitValues;
    int count = 0;
    while (src < stop) {
        unsigned c = *src++;
        SkASSERT(!SkUTF16_IsLowSurrogate(c));
        if (SkUTF16_IsHighSurrogate(c)) {
            SkASSERT(src < stop);
            c = *src++;
            SkASSERT(SkUTF16_IsLowSurrogate(c));
        }
        count += 1;
    }
    return count;
}

SkUnichar SkUTF16_NextUnichar(const uint16_t** srcPtr) {
    SkASSERT(srcPtr && *srcPtr);

    const uint16_t* src = *srcPtr;
    SkUnichar       c = *src++;

    SkASSERT(!SkUTF16_IsLowSurrogate(c));
    if (SkUTF16_IsHighSurrogate(c)) {
        unsigned c2 = *src++;
        SkASSERT(SkUTF16_IsLowSurrogate(c2));

        // c = ((c & 0x3FF) << 10) + (c2 & 0x3FF) + 0x10000
        // c = (((c & 0x3FF) + 64) << 10) + (c2 & 0x3FF)
        c = (c << 10) + c2 + (0x10000 - (0xD800 << 10) - 0xDC00);
    }
    *srcPtr = src;
    return c;
}

SkUnichar SkUTF16_PrevUnichar(const uint16_t** srcPtr) {
    SkASSERT(srcPtr && *srcPtr);

    const uint16_t* src = *srcPtr;
    SkUnichar       c = *--src;

    SkASSERT(!SkUTF16_IsHighSurrogate(c));
    if (SkUTF16_IsLowSurrogate(c)) {
        unsigned c2 = *--src;
        SkASSERT(SkUTF16_IsHighSurrogate(c2));
        c = (c2 << 10) + c + (0x10000 - (0xD800 << 10) - 0xDC00);
    }
    *srcPtr = src;
    return c;
}

size_t SkUTF16_FromUnichar(SkUnichar uni, uint16_t dst[]) {
    SkASSERT((unsigned)uni <= 0x10FFFF);

    int extra = (uni > 0xFFFF);

    if (dst) {
        if (extra) {
            // dst[0] = SkToU16(0xD800 | ((uni - 0x10000) >> 10));
            // dst[0] = SkToU16(0xD800 | ((uni >> 10) - 64));
            dst[0] = SkToU16((0xD800 - 64) + (uni >> 10));
            dst[1] = SkToU16(0xDC00 | (uni & 0x3FF));

            SkASSERT(SkUTF16_IsHighSurrogate(dst[0]));
            SkASSERT(SkUTF16_IsLowSurrogate(dst[1]));
        } else {
            dst[0] = SkToU16(uni);
            SkASSERT(!SkUTF16_IsHighSurrogate(dst[0]));
            SkASSERT(!SkUTF16_IsLowSurrogate(dst[0]));
        }
    }
    return 1 + extra;
}

size_t SkUTF16_ToUTF8(const uint16_t utf16[], int numberOf16BitValues,
                      char utf8[]) {
    SkASSERT(numberOf16BitValues >= 0);
    if (numberOf16BitValues <= 0) {
        return 0;
    }

    SkASSERT(utf16 != nullptr);

    const uint16_t* stop = utf16 + numberOf16BitValues;
    size_t          size = 0;

    if (utf8 == nullptr) {    // just count
        while (utf16 < stop) {
            size += SkUTF8_FromUnichar(SkUTF16_NextUnichar(&utf16), nullptr);
        }
    } else {
        char* start = utf8;
        while (utf16 < stop) {
            utf8 += SkUTF8_FromUnichar(SkUTF16_NextUnichar(&utf16), utf8);
        }
        size = utf8 - start;
    }
    return size;
}
