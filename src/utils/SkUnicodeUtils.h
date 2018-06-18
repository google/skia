// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef SkUnicodeUtils_DEFINED
#define SkUnicodeUtils_DEFINED

//#include "SkTypes.h"

#include <cstddef>
#include <cstdint>

typedef int32_t SkUnichar;

///////////////////////////////////////////////////////////////////////////////

static constexpr int kMaxBytesInUTF8Sequence = 4;

static constexpr inline bool SkUTF8_ByteIsValid(uint8_t c) {
    return c < 0xF5 && (c & 0xFE) != 0xC0;
}

static constexpr inline bool SkUTF8_ByteIsContinuation(uint8_t c) { return (c & 0xC0) == 0x80; }

static constexpr inline int SkUTF8_LeadByteToCount(uint8_t c) {
    return (SkUTF8_ByteIsValid(c) && !SkUTF8_ByteIsContinuation(c))
           ? (((0xE5 << 24) >> (c >> 4 << 1)) & 3) + 1
           : -1;
}

static inline int SkUTF8_CountUTF8Bytes(const char utf8[]) {
    return SkUTF8_LeadByteToCount(*(const uint8_t*)utf8);
}

int SkUTF8_CountUnichars(const char utf8[]);

/** These functions are safe: invalid sequences will return -1; */
int SkUTF8_CountUnichars(const void* utf8, size_t byteLength);
int SkUTF16_CountUnichars(const void* utf16, size_t byteLength);
int SkUTF32_CountUnichars(const void* utf32, size_t byteLength);

/** This function is safe: invalid UTF8 sequences will return -1
 *  When -1 is returned, ptr is unchanged.
 *  Precondition: *ptr < end;
 */
SkUnichar SkUTF8_NextUnicharWithError(const char** ptr, const char* end);

/** this version replaces invalid utf-8 sequences with code point U+FFFD. */
static inline SkUnichar SkUTF8_NextUnichar(const char** ptr, const char* end) {
    SkUnichar val = SkUTF8_NextUnicharWithError(ptr, end);
    if (val < 0) {
        *ptr = end;
        return 0xFFFD;  // REPLACEMENT CHARACTER
    }
    return val;
}

SkUnichar   SkUTF8_ToUnichar(const char utf8[]);
SkUnichar   SkUTF8_NextUnichar(const char**);
SkUnichar   SkUTF8_PrevUnichar(const char**);

/** Return the number of bytes need to convert a unichar
    into a utf8 sequence. Will be 1..kMaxBytesInUTF8Sequence,
    or 0 if uni is illegal.
*/
size_t      SkUTF8_FromUnichar(SkUnichar uni, char utf8[] = nullptr);

///////////////////////////////////////////////////////////////////////////////

static constexpr inline bool SkUTF16_IsHighSurrogate(uint16_t c) { return (c & 0xFC00) == 0xD800; }
static constexpr inline bool SkUTF16_IsLowSurrogate(uint16_t c) { return (c & 0xFC00) == 0xDC00; }

int SkUTF16_CountUnichars(const uint16_t utf16[]);
// returns the current unichar and then moves past it (*p++)
SkUnichar SkUTF16_NextUnichar(const uint16_t**);
SkUnichar SkUTF16_NextUnichar(const uint16_t** srcPtr, const uint16_t* end);

// this guy backs up to the previus unichar value, and returns it (*--p)
SkUnichar SkUTF16_PrevUnichar(const uint16_t**);
size_t SkUTF16_FromUnichar(SkUnichar uni, uint16_t utf16[] = nullptr);

size_t SkUTF16_ToUTF8(const uint16_t utf16[], int numberOf16BitValues,
                      char utf8[] = nullptr);

static inline bool SkUnichar_IsVariationSelector(SkUnichar uni) {
    // The 'true' ranges are:
    //      0x180B  <= uni <=  0x180D
    //      0xFE00  <= uni <=  0xFE0F
    //      0xE0100 <= uni <= 0xE01EF
    if (uni < 0x180B || uni > 0xE01EF) {
        return false;
    }
    if ((uni > 0x180D && uni < 0xFE00) || (uni > 0xFE0F && uni < 0xE0100)) {
        return false;
    }
    return true;
}

#endif  // SkUnicodeUtils_DEFINED
