/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkUnicodeUtils.h"

/*  0xxxxxxx    1 total
    10xxxxxx    // never a leading byte
    110xxxxx    2 total
    1110xxxx    3 total
    11110xxx    4 total

    11 10 01 01 xx xx xx xx 0...
    0xE5XX0000
    0xE5 << 24
*/

static constexpr inline uint16_t toU16(int32_t v) {
    // IS THIS WELL-DEFINED?
    return (void)assert((uint32_t)v <= UINT16_MAX), (uint16_t)v;
}

static constexpr inline int32_t left_shift(int32_t value, int32_t shift) {
    return (int32_t) ((uint32_t) value << shift);
}

template <typename T> static constexpr bool is_align2(T x) { return 0 == (x & 1); }

template <typename T> static constexpr bool is_align4(T x) { return 0 == (x & 3); }

/**
 * @returns -1  iff invalid UTF8 byte,
 *           0  iff UTF8 continuation byte,
 *           1  iff ASCII byte,
 *           2  iff leading byte of 2-byte sequence,
 *           3  iff leading byte of 3-byte sequence, and
 *           4  iff leading byte of 4-byte sequence.
 *
 * I.e.: if return value > 0, then gives length of sequence.
*/
static int utf8_byte_type(uint8_t c) {
    if (c < 0x80) {
        return 1;
    } else if (c < 0xC0) {
        return 0;
    } else if (c < 0xF5 && (c & 0xFE) != 0xC0) { // "octet values C0, C1, F5 to FF never appear"
        return (((0xE5 << 24) >> ((unsigned)c >> 4 << 1)) & 3) + 1;
    } else {
        return -1;
    }
}

static bool utf8_type_is_valid_leading_byte(int type) { return type > 0; }

static constexpr inline bool utf8_byte_is_leading_byte(uint8_t c) {
    return SkUTF8_ByteIsValid(c) && !SkUTF8_ByteIsContinuation(c);
}

int SkUTF8_CountUnichars(const char utf8[]) {
    assert(utf8);

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

// SAFE: returns -1 if invalid UTF-8
int SkUTF8_CountUnichars(const void* text, size_t byteLength) {
    assert(text);
    const char* utf8 = static_cast<const char*>(text);
    if (byteLength == 0) {
        return 0;
    }

    int         count = 0;
    const char* stop = utf8 + byteLength;

    while (utf8 < stop) {
        int type = utf8_byte_type(*(const uint8_t*)utf8);
        assert(type >= -1 && type <= 4);
        if (!utf8_type_is_valid_leading_byte(type) || utf8 + type > stop) {
            // Sequence extends beyond end.
            return -1;
        }
        while(type-- > 1) {
            ++utf8;
            if (!SkUTF8_ByteIsContinuation(*(const uint8_t*)utf8)) {
                return -1;
            }
        }
        ++utf8;
        ++count;
    }
    return count;
}

SkUnichar SkUTF8_ToUnichar(const char utf8[]) {
    assert(utf8);

    const uint8_t*  p = (const uint8_t*)utf8;
    int             c = *p;
    int             hic = c << 24;

    assert(utf8_byte_is_leading_byte(c));

    if (hic < 0) {
        uint32_t mask = (uint32_t)~0x3F;
        hic = left_shift(hic, 1);
        do {
            c = (c << 6) | (*++p & 0x3F);
            mask <<= 5;
        } while ((hic = left_shift(hic, 1)) < 0);
        c &= ~mask;
    }
    return c;
}

// SAFE: returns -1 on invalid UTF-8 sequence.
SkUnichar SkUTF8_NextUnicharWithError(const char** ptr, const char* end) {
    assert(ptr && *ptr);
    assert(*ptr < end);
    const uint8_t*  p = (const uint8_t*)*ptr;
    int             c = *p;
    int             hic = c << 24;

    if (!utf8_byte_is_leading_byte(c)) {
        return -1;
    }
    if (hic < 0) {
        uint32_t mask = (uint32_t)~0x3F;
        hic = left_shift(hic, 1);
        do {
            ++p;
            if (p >= (const uint8_t*)end) {
                return -1;
            }
            // check before reading off end of array.
            uint8_t nextByte = *p;
            if (!SkUTF8_ByteIsContinuation(nextByte)) {
                return -1;
            }
            c = (c << 6) | (nextByte & 0x3F);
            mask <<= 5;
        } while ((hic = left_shift(hic, 1)) < 0);
        c &= ~mask;
    }
    *ptr = (char*)p + 1;
    return c;
}

SkUnichar SkUTF8_NextUnichar(const char** ptr) {
    assert(ptr && *ptr);

    const uint8_t*  p = (const uint8_t*)*ptr;
    int             c = *p;
    int             hic = c << 24;

    assert(utf8_byte_is_leading_byte(c));

    if (hic < 0) {
        uint32_t mask = (uint32_t)~0x3F;
        hic = left_shift(hic, 1);
        do {
            c = (c << 6) | (*++p & 0x3F);
            mask <<= 5;
        } while ((hic = left_shift(hic, 1)) < 0);
        c &= ~mask;
    }
    *ptr = (char*)p + 1;
    return c;
}

SkUnichar SkUTF8_PrevUnichar(const char** ptr) {
    assert(ptr && *ptr);

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
        assert(false); // "bad unichar";
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

    SkUnichar orig = uni;
    (void)orig;

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

    assert(utf8 == nullptr || orig == SkUTF8_ToUnichar(utf8));
    return count;
}

///////////////////////////////////////////////////////////////////////////////

int SkUTF16_CountUnichars(const uint16_t src[]) {
    assert(src);

    int count = 0;
    unsigned c;
    while ((c = *src++) != 0) {
        assert(!SkUTF16_IsLowSurrogate(c));
        if (SkUTF16_IsHighSurrogate(c)) {
            c = *src++;
            assert(SkUTF16_IsLowSurrogate(c));
        }
        count += 1;
    }
    return count;
}

// returns -1 on error
int SkUTF16_CountUnichars(const void* text, size_t byteLength) {
    assert(text);
    if (byteLength == 0) {
        return 0;
    }
    if (!is_align2(intptr_t(text)) || !is_align2(byteLength)) {
        return -1;
    }

    const uint16_t* src = static_cast<const uint16_t*>(text);
    const uint16_t* stop = src + (byteLength >> 1);
    int count = 0;
    while (src < stop) {
        unsigned c = *src++;
        assert(!SkUTF16_IsLowSurrogate(c));
        if (SkUTF16_IsHighSurrogate(c)) {
            if (src >= stop) {
                return -1;
            }
            c = *src++;
            if (!SkUTF16_IsLowSurrogate(c)) {
                return -1;
            }
        }
        count += 1;
    }
    return count;
}

SkUnichar SkUTF16_NextUnichar(const uint16_t** srcPtr) {
    assert(srcPtr && *srcPtr);

    const uint16_t* src = *srcPtr;
    SkUnichar       c = *src++;

    assert(!SkUTF16_IsLowSurrogate(c));
    if (SkUTF16_IsHighSurrogate(c)) {
        unsigned c2 = *src++;
        assert(SkUTF16_IsLowSurrogate(c2));

        // c = ((c & 0x3FF) << 10) + (c2 & 0x3FF) + 0x10000
        // c = (((c & 0x3FF) + 64) << 10) + (c2 & 0x3FF)
        c = (c << 10) + c2 + (0x10000 - (0xD800 << 10) - 0xDC00);
    }
    *srcPtr = src;
    return c;
}

SkUnichar SkUTF16_PrevUnichar(const uint16_t** srcPtr) {
    assert(srcPtr && *srcPtr);

    const uint16_t* src = *srcPtr;
    SkUnichar       c = *--src;

    assert(!SkUTF16_IsHighSurrogate(c));
    if (SkUTF16_IsLowSurrogate(c)) {
        unsigned c2 = *--src;
        assert(SkUTF16_IsHighSurrogate(c2));
        c = (c2 << 10) + c + (0x10000 - (0xD800 << 10) - 0xDC00);
    }
    *srcPtr = src;
    return c;
}

size_t SkUTF16_FromUnichar(SkUnichar uni, uint16_t dst[]) {
    assert((unsigned)uni <= 0x10FFFF);

    int extra = (uni > 0xFFFF);

    if (dst) {
        if (extra) {
            // dst[0] = toU16(0xD800 | ((uni - 0x10000) >> 10));
            // dst[0] = toU16(0xD800 | ((uni >> 10) - 64));
            dst[0] = toU16((0xD800 - 64) + (uni >> 10));
            dst[1] = toU16(0xDC00 | (uni & 0x3FF));

            assert(SkUTF16_IsHighSurrogate(dst[0]));
            assert(SkUTF16_IsLowSurrogate(dst[1]));
        } else {
            dst[0] = toU16(uni);
            assert(!SkUTF16_IsHighSurrogate(dst[0]));
            assert(!SkUTF16_IsLowSurrogate(dst[0]));
        }
    }
    return 1 + extra;
}

size_t SkUTF16_ToUTF8(const uint16_t utf16[], int numberOf16BitValues,
                      char utf8[]) {
    assert(numberOf16BitValues >= 0);
    if (numberOf16BitValues <= 0) {
        return 0;
    }

    assert(utf16 != nullptr);

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
int SkUTF32_CountUnichars(const void* text, size_t byteLength) {
    if (byteLength == 0) {
        return 0;
    }
    if (!is_align4(intptr_t(text)) || !is_align4(byteLength)) {
        return -1;
    }
    const uint32_t kInvalidUnicharMask = 0xFF000000;    // unichar fits in 24 bits
    const uint32_t* ptr = static_cast<const uint32_t*>(text);
    const uint32_t* stop = ptr + (byteLength >> 2);
    while (ptr < stop) {
        if (*ptr & kInvalidUnicharMask) {
            return -1;
        }
        ptr += 1;
    }
    size_t count = byteLength >> 2;
    assert(count <= (size_t)INT32_MAX);
    return count;
}
