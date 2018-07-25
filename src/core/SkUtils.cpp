/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkUtils.h"

#include "SkTo.h"

/*  0xxxxxxx    1 total
    10xxxxxx    // never a leading byte
    110xxxxx    2 total
    1110xxxx    3 total
    11110xxx    4 total

    11 10 01 01 xx xx xx xx 0...
    0xE5XX0000
    0xE5 << 24
*/

static bool utf8_byte_is_valid(uint8_t c) {
    return c < 0xF5 && (c & 0xFE) != 0xC0;
}
static bool utf8_byte_is_continuation(uint8_t c) {
    return  (c & 0xC0) == 0x80;
}
static bool utf8_byte_is_leading_byte(uint8_t c) {
    return utf8_byte_is_valid(c) && !utf8_byte_is_continuation(c);
}

#ifdef SK_DEBUG
    static void assert_utf8_leadingbyte(unsigned c) {
        SkASSERT(utf8_byte_is_leading_byte(SkToU8(c)));
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

SkUnichar SkUTF16_NextUnichar(const uint16_t** srcPtr) {
    SkUnichar c = SkUTF16_NextUnichar(srcPtr, *srcPtr + 2);
    if (c == -1) {
        SkASSERT(false);
        ++(*srcPtr);
        return 0xFFFD;  // REPLACEMENT CHARACTER.
    }
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

// returns -1 on error
int SkUTFN_CountUnichars(
    SkTypeface::Encoding encoding, const void* utfN, size_t byteLength) {
    SkASSERT(utfN != nullptr);
    switch (encoding) {
        case SkTypeface::kUTF8_Encoding:
            return SkUTF8_CountUnichars(utfN, byteLength);
        case SkTypeface::kUTF16_Encoding:
            return SkUTF16_CountUnichars(utfN, byteLength);
        case SkTypeface::kUTF32_Encoding:
            return SkUTF32_CountUnichars(utfN, byteLength);
        default:
            SkDEBUGFAIL("unknown text encoding");
    }

    return -1;
}

const char SkHexadecimalDigits::gUpper[16] =
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
const char SkHexadecimalDigits::gLower[16] =
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
