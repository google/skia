/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUtils_DEFINED
#define SkUtils_DEFINED

#include "include/core/SkFontTypes.h"
#include "src/core/SkOpts.h"
#include "src/utils/SkUTF.h"

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

// Unlike the functions in SkUTF.h, these two functions do not take an array
// length parameter.  When possible, use SkUTF::NextUTF{8,16} instead.
SkUnichar SkUTF8_NextUnichar(const char**);
SkUnichar SkUTF16_NextUnichar(const uint16_t**);

///////////////////////////////////////////////////////////////////////////////

static inline bool SkUTF16_IsLeadingSurrogate(uint16_t c) { return ((c) & 0xFC00) == 0xD800; }

static inline bool SkUTF16_IsTrailingSurrogate (uint16_t c) { return ((c) & 0xFC00) == 0xDC00; }

///////////////////////////////////////////////////////////////////////////////

static inline int SkUTFN_CountUnichars(SkTextEncoding enc, const void* utfN, size_t bytes) {
    switch (enc) {
        case SkTextEncoding::kUTF8:  return SkUTF::CountUTF8((const char*)utfN, bytes);
        case SkTextEncoding::kUTF16: return SkUTF::CountUTF16((const uint16_t*)utfN, bytes);
        case SkTextEncoding::kUTF32: return SkUTF::CountUTF32((const int32_t*)utfN, bytes);
        default: SkDEBUGFAIL("unknown text encoding"); return -1;
    }
}

static inline SkUnichar SkUTFN_Next(SkTextEncoding enc, const void** ptr, const void* stop) {
    switch (enc) {
        case SkTextEncoding::kUTF8:
            return SkUTF::NextUTF8((const char**)ptr, (const char*)stop);
        case SkTextEncoding::kUTF16:
            return SkUTF::NextUTF16((const uint16_t**)ptr, (const uint16_t*)stop);
        case SkTextEncoding::kUTF32:
            return SkUTF::NextUTF32((const int32_t**)ptr, (const int32_t*)stop);
        default: SkDEBUGFAIL("unknown text encoding"); return -1;
    }
}

///////////////////////////////////////////////////////////////////////////////

namespace SkHexadecimalDigits {
    extern const char gUpper[16];  // 0-9A-F
    extern const char gLower[16];  // 0-9a-f
}

#endif
