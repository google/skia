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

#ifdef SK_DEBUG
static inline bool utf8_byte_is_valid(uint8_t c) {
    return c < 0xF5 && (c & 0xFE) != 0xC0;
}
static inline bool utf8_byte_is_continuation(uint8_t c) {
    return  (c & 0xC0) == 0x80;
}
#endif

int SkUTF8_CountUnichars(const char utf8[]) {
    SkASSERT(utf8);

    int count = 0;

    for (;;) {
        uint8_t c = *(const uint8_t*)utf8;
        if (c == 0) {
            break;
        }
        SkASSERT(utf8_byte_is_valid(c) && !utf8_byte_is_continuation(c));
        utf8 += (((0xE5 << 24) >> ((unsigned)c >> 4 << 1)) & 3) + 1;
        count += 1;
    }
    return count;
}

SkUnichar SkUTF8_NextUnichar(const char** ptr) {
    SkASSERT(ptr && *ptr);

    const uint8_t*  p = (const uint8_t*)*ptr;
    int             c = *p;
    int             hic = c << 24;

    SkASSERT(utf8_byte_is_valid(c) && !utf8_byte_is_continuation(c));

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
    SkUnichar c = SkUTF::NextUTF16(srcPtr, *srcPtr + 2);
    if (c == -1) {
        SkASSERT(false);
        ++(*srcPtr);
        return 0xFFFD;  // REPLACEMENT CHARACTER.
    }
    return c;
}

const char SkHexadecimalDigits::gUpper[16] =
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
const char SkHexadecimalDigits::gLower[16] =
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
