/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicodeUtils_DEFINED
#define SkUnicodeUtils_DEFINED

#include <cassert>
#include <cstddef>
#include <cstdint>

typedef int32_t SkUnichar;

// utf-8 functions /////////////////////////////////////////////////////////////

static constexpr int kMaxBytesInUTF8Sequence = 4;

static constexpr inline bool SkUTF8_ByteIsValid(uint8_t c) {
    return c < 0xF5 && (c & 0xFE) != 0xC0;
}

static constexpr inline bool SkUTF8_ByteIsContinuation(uint8_t c) { return (c & 0xC0) == 0x80; }

static constexpr inline int SkUTF8_LeadByteToCount(uint8_t c) {
    return (void)assert(SkUTF8_ByteIsValid(c) && !SkUTF8_ByteIsContinuation(c)),
           (((0xE5 << 24) >> (c >> 4 << 1)) & 3) + 1;
}

static constexpr inline int SkUTF8_CountUTF8Bytes(const char utf8[]) {
    return (void)assert(utf8),
           SkUTF8_LeadByteToCount(*(const uint8_t*)utf8);
}

/**
 *  This function is safe: invalid sequences will return -1;
 */
int SkUTF8_CountUnichars(const void* utf8, size_t byteLength);

/**
 *  This function is safe: invalid UTF8 sequences will return -1
 *  When -1 is returned, ptr is unchanged.
 *  Precondition: *ptr < end;
 */
SkUnichar SkUTF8_NextUnicharWithError(const char** ptr, const char* end);

/**
 *  this version replaces invalid utf-8 sequences with code point U+FFFD.
 */
static inline SkUnichar SkUTF8_NextUnichar(const char** ptr, const char* end) {
    SkUnichar val = SkUTF8_NextUnicharWithError(ptr, end);
    if (val < 0) {
        *ptr = end;
        return 0xFFFD;  // REPLACEMENT CHARACTER
    }
    return val;
}

SkUnichar SkUTF8_NextUnichar(const char**);

/**
 *  Return the number of bytes need to convert a unichar
 *  into a utf8 sequence. Will be 1..kMaxBytesInUTF8Sequence
 *  or 0 if uni is illegal.
 */
size_t SkUTF8_FromUnichar(SkUnichar uni, char utf8[] = nullptr);

// utf-16 functions ////////////////////////////////////////////////////////////

int SkUTF16_CountUnichars(const void* utf16, size_t byteLength);

static constexpr inline bool SkUTF16_IsHighSurrogate(uint16_t c) { return (c & 0xFC00) == 0xD800; }

static constexpr inline bool SkUTF16_IsLowSurrogate(uint16_t c) { return (c & 0xFC00) == 0xDC00; }

/**
 *  Returns the current unichar and then moves past it (*p++)
 */
SkUnichar SkUTF16_NextUnichar(const uint16_t**);

size_t SkUTF16_FromUnichar(SkUnichar uni, uint16_t utf16[] = nullptr);

size_t SkUTF16_ToUTF8(const uint16_t utf16[], int numberOf16BitValues, char utf8[] = nullptr);

// utf-32 functions ////////////////////////////////////////////////////////////

int SkUTF32_CountUnichars(const void* utf32, size_t byteLength);

///////////////////////////////////////////////////////////////////////////////

#endif  // SkUnicodeUtils_DEFINED
