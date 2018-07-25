/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUtils_DEFINED
#define SkUtils_DEFINED

#include "SkOpts.h"
#include "SkTypeface.h"
#include "../utils/SkUTF.h"

/** Similar to memset(), but it assigns a 16, 32, or 64-bit value into the buffer.
    @param buffer   The memory to have value copied into it
    @param value    The value to be copied into buffer
    @param count    The number of times value should be copied into the buffer.
*/
static inline void sk_memset16(uint16_t buffer[], uint16_t value, int count) {
    SkOpts::memset16(buffer, value, count);
}
static inline void sk_memset32(uint32_t buffer[], uint32_t value, int count) {
    SkOpts::memset32(buffer, value, count);
}
static inline void sk_memset64(uint64_t buffer[], uint64_t value, int count) {
    SkOpts::memset64(buffer, value, count);
}

///////////////////////////////////////////////////////////////////////////////

int  SkUTF8_CountUnichars(const char utf8[]);

/** this version replaces invalid utf-8 sequences with code point U+FFFD. */
static inline SkUnichar SkUTF8_Next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    if (val < 0) {
        return 0xFFFD;  // REPLACEMENT CHARACTER
    }
    return val;
}

SkUnichar SkUTF8_NextUnichar(const char**);

static inline bool SkUTF16_IsHighSurrogate(uint16_t c) { return ((c) & 0xFC00) == 0xD800; }

static inline bool SkUTF16_IsLowSurrogate (uint16_t c) { return ((c) & 0xFC00) == 0xDC00; }

int SkUTF16_CountUnichars(const uint16_t utf16[]);

// returns the current unichar and then moves past it (*p++)
SkUnichar SkUTF16_NextUnichar(const uint16_t**);

///////////////////////////////////////////////////////////////////////////////

static inline int SkUTFN_CountUnichars(SkTypeface::Encoding enc, const void* utfN, size_t bytes) {
    switch (enc) {
        case SkTypeface::kUTF8_Encoding:  return SkUTF::CountUTF8((const char*)utfN, bytes);
        case SkTypeface::kUTF16_Encoding: return SkUTF::CountUTF16((const uint16_t*)utfN, bytes);
        case SkTypeface::kUTF32_Encoding: return SkUTF::CountUTF32((const int32_t*)utfN, bytes);
        default: SkDEBUGFAIL("unknown text encoding"); return -1;
    }
}

///////////////////////////////////////////////////////////////////////////////

namespace SkHexadecimalDigits {
    extern const char gUpper[16];  // 0-9A-F
    extern const char gLower[16];  // 0-9a-f
}

#endif
