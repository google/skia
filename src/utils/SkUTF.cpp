// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/private/SkTFitsIn.h"
#include "src/utils/SkUTF.h"

#include <climits>

static constexpr inline int32_t left_shift(int32_t value, int32_t shift) {
    return (int32_t) ((uint32_t) value << shift);
}

template <typename T> static constexpr bool is_align2(T x) { return 0 == (x & 1); }

template <typename T> static constexpr bool is_align4(T x) { return 0 == (x & 3); }

static constexpr inline bool utf16_is_high_surrogate(uint16_t c) { return (c & 0xFC00) == 0xD800; }

static constexpr inline bool utf16_is_low_surrogate(uint16_t c) { return (c & 0xFC00) == 0xDC00; }

/** @returns   -1  iff invalid UTF8 byte,
                0  iff UTF8 continuation byte,
                1  iff ASCII byte,
                2  iff leading byte of 2-byte sequence,
                3  iff leading byte of 3-byte sequence, and
                4  iff leading byte of 4-byte sequence.
      I.e.: if return value > 0, then gives length of sequence.
*/
static int utf8_byte_type(uint8_t c) {
    if (c < 0x80) {
        return 1;
    } else if (c < 0xC0) {
        return 0;
    } else if (c >= 0xF5 || (c & 0xFE) == 0xC0) { // "octet values c0, c1, f5 to ff never appear"
        return -1;
    } else {
        int value = (((0xe5 << 24) >> ((unsigned)c >> 4 << 1)) & 3) + 1;
        // assert(value >= 2 && value <=4);
        return value;
    }
}
static bool utf8_type_is_valid_leading_byte(int type) { return type > 0; }

static bool utf8_byte_is_continuation(uint8_t c) { return utf8_byte_type(c) == 0; }

////////////////////////////////////////////////////////////////////////////////

int SkUTF::CountUTF8(const char* utf8, size_t byteLength) {
    if (!utf8) {
        return -1;
    }
    int count = 0;
    const char* stop = utf8 + byteLength;
    while (utf8 < stop) {
        int type = utf8_byte_type(*(const uint8_t*)utf8);
        if (!utf8_type_is_valid_leading_byte(type) || utf8 + type > stop) {
            return -1;  // Sequence extends beyond end.
        }
        while(type-- > 1) {
            ++utf8;
            if (!utf8_byte_is_continuation(*(const uint8_t*)utf8)) {
                return -1;
            }
        }
        ++utf8;
        ++count;
    }
    return count;
}

int SkUTF::CountUTF16(const uint16_t* utf16, size_t byteLength) {
    if (!utf16 || !is_align2(intptr_t(utf16)) || !is_align2(byteLength)) {
        return -1;
    }
    const uint16_t* src = (const uint16_t*)utf16;
    const uint16_t* stop = src + (byteLength >> 1);
    int count = 0;
    while (src < stop) {
        unsigned c = *src++;
        if (utf16_is_low_surrogate(c)) {
            return -1;
        }
        if (utf16_is_high_surrogate(c)) {
            if (src >= stop) {
                return -1;
            }
            c = *src++;
            if (!utf16_is_low_surrogate(c)) {
                return -1;
            }
        }
        count += 1;
    }
    return count;
}

int SkUTF::CountUTF32(const int32_t* utf32, size_t byteLength) {
    if (!is_align4(intptr_t(utf32)) || !is_align4(byteLength) || !SkTFitsIn<int>(byteLength >> 2)) {
        return -1;
    }
    const uint32_t kInvalidUnicharMask = 0xFF000000;    // unichar fits in 24 bits
    const uint32_t* ptr = (const uint32_t*)utf32;
    const uint32_t* stop = ptr + (byteLength >> 2);
    while (ptr < stop) {
        if (*ptr & kInvalidUnicharMask) {
            return -1;
        }
        ptr += 1;
    }
    return (int)(byteLength >> 2);
}

template <typename T>
static SkUnichar next_fail(const T** ptr, const T* end) {
    *ptr = end;
    return -1;
}

SkUnichar SkUTF::NextUTF8(const char** ptr, const char* end) {
    if (!ptr || !end ) {
        return -1;
    }
    const uint8_t*  p = (const uint8_t*)*ptr;
    if (!p || p >= (const uint8_t*)end) {
        return next_fail(ptr, end);
    }
    int             c = *p;
    int             hic = c << 24;

    if (!utf8_type_is_valid_leading_byte(utf8_byte_type(c))) {
        return next_fail(ptr, end);
    }
    if (hic < 0) {
        uint32_t mask = (uint32_t)~0x3F;
        hic = left_shift(hic, 1);
        do {
            ++p;
            if (p >= (const uint8_t*)end) {
                return next_fail(ptr, end);
            }
            // check before reading off end of array.
            uint8_t nextByte = *p;
            if (!utf8_byte_is_continuation(nextByte)) {
                return next_fail(ptr, end);
            }
            c = (c << 6) | (nextByte & 0x3F);
            mask <<= 5;
        } while ((hic = left_shift(hic, 1)) < 0);
        c &= ~mask;
    }
    *ptr = (char*)p + 1;
    return c;
}

SkUnichar SkUTF::NextUTF16(const uint16_t** ptr, const uint16_t* end) {
    if (!ptr || !end ) {
        return -1;
    }
    const uint16_t* src = *ptr;
    if (!src || src + 1 > end || !is_align2(intptr_t(src))) {
        return next_fail(ptr, end);
    }
    uint16_t c = *src++;
    SkUnichar result = c;
    if (utf16_is_low_surrogate(c)) {
        return next_fail(ptr, end);  // srcPtr should never point at low surrogate.
    }
    if (utf16_is_high_surrogate(c)) {
        if (src + 1 > end) {
            return next_fail(ptr, end);  // Truncated string.
        }
        uint16_t low = *src++;
        if (!utf16_is_low_surrogate(low)) {
            return next_fail(ptr, end);
        }
        /*
        [paraphrased from wikipedia]
        Take the high surrogate and subtract 0xD800, then multiply by 0x400.
        Take the low surrogate and subtract 0xDC00.  Add these two results
        together, and finally add 0x10000 to get the final decoded codepoint.

        unicode = (high - 0xD800) * 0x400 + low - 0xDC00 + 0x10000
        unicode = (high * 0x400) - (0xD800 * 0x400) + low - 0xDC00 + 0x10000
        unicode = (high << 10) - (0xD800 << 10) + low - 0xDC00 + 0x10000
        unicode = (high << 10) + low - ((0xD800 << 10) + 0xDC00 - 0x10000)
        */
        result = (result << 10) + (SkUnichar)low - ((0xD800 << 10) + 0xDC00 - 0x10000);
    }
    *ptr = src;
    return result;
}

SkUnichar SkUTF::NextUTF32(const int32_t** ptr, const int32_t* end) {
    if (!ptr || !end ) {
        return -1;
    }
    const int32_t* s = *ptr;
    if (!s || s + 1 > end || !is_align4(intptr_t(s))) {
        return next_fail(ptr, end);
    }
    int32_t value = *s;
    const uint32_t kInvalidUnicharMask = 0xFF000000;    // unichar fits in 24 bits
    if (value & kInvalidUnicharMask) {
        return next_fail(ptr, end);
    }
    *ptr = s + 1;
    return value;
}

size_t SkUTF::ToUTF8(SkUnichar uni, char utf8[SkUTF::kMaxBytesInUTF8Sequence]) {
    if ((uint32_t)uni > 0x10FFFF) {
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
    return count;
}

size_t SkUTF::ToUTF16(SkUnichar uni, uint16_t utf16[2]) {
    if ((uint32_t)uni > 0x10FFFF) {
        return 0;
    }
    int extra = (uni > 0xFFFF);
    if (utf16) {
        if (extra) {
            utf16[0] = (uint16_t)((0xD800 - 64) + (uni >> 10));
            utf16[1] = (uint16_t)(0xDC00 | (uni & 0x3FF));
        } else {
            utf16[0] = (uint16_t)uni;
        }
    }
    return 1 + extra;
}

int SkUTF::UTF8ToUTF16(uint16_t dst[], int dstCapacity, const char src[], size_t srcByteLength) {
    if (!dst) {
        dstCapacity = 0;
    }

    int dstLength = 0;
    uint16_t* endDst = dst + dstCapacity;
    const char* endSrc = src + srcByteLength;
    while (src < endSrc) {
        SkUnichar uni = NextUTF8(&src, endSrc);
        if (uni < 0) {
            return -1;
        }

        uint16_t utf16[2];
        size_t count = ToUTF16(uni, utf16);
        if (count == 0) {
            return -1;
        }
        dstLength += count;

        if (dst) {
            uint16_t* elems = utf16;
            while (dst < endDst && count > 0) {
                *dst++ = *elems++;
                count -= 1;
            }
        }
    }
    return dstLength;
}

int SkUTF::UTF16ToUTF8(char dst[], int dstCapacity, const uint16_t src[], size_t srcLength) {
    if (!dst) {
        dstCapacity = 0;
    }

    int dstLength = 0;
    const char* endDst = dst + dstCapacity;
    const uint16_t* endSrc = src + srcLength;
    while (src < endSrc) {
        SkUnichar uni = NextUTF16(&src, endSrc);
        if (uni < 0) {
            return -1;
        }

        char utf8[SkUTF::kMaxBytesInUTF8Sequence];
        size_t count = ToUTF8(uni, utf8);
        if (count == 0) {
            return -1;
        }
        dstLength += count;

        if (dst) {
            const char* elems = utf8;
            while (dst < endDst && count > 0) {
                *dst++ = *elems++;
                count -= 1;
            }
        }
    }
    return dstLength;
}
