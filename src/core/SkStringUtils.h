/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStringUtils_DEFINED
#define SkStringUtils_DEFINED

#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTArray.h"

#include <cstddef>
#include <cstdint>

enum SkScalarAsStringType {
    kDec_SkScalarAsStringType,
    kHex_SkScalarAsStringType,
};

void SkAppendScalar(SkString*, SkScalar, SkScalarAsStringType);

static inline void SkAppendScalarDec(SkString* str, SkScalar value) {
    SkAppendScalar(str, value, kDec_SkScalarAsStringType);
}

static inline void SkAppendScalarHex(SkString* str, SkScalar value) {
    SkAppendScalar(str, value, kHex_SkScalarAsStringType);
}

/** Indents every non-empty line of the string by tabCnt tabs */
SkString SkTabString(const SkString& string, int tabCnt);

SkString SkStringFromUTF16(const uint16_t* src, size_t count);

#if defined(SK_BUILD_FOR_WIN)
    #define SK_strcasecmp   _stricmp
#else
    #define SK_strcasecmp   strcasecmp
#endif

enum SkStrSplitMode {
    // Strictly return all results. If the input is ",," and the separator is ',' this will return
    // an array of three empty strings.
    kStrict_SkStrSplitMode,

    // Only nonempty results will be added to the results. Multiple separators will be
    // coalesced. Separators at the beginning and end of the input will be ignored.  If the input is
    // ",," and the separator is ',', this will return an empty vector.
    kCoalesce_SkStrSplitMode
};

// Split str on any characters in delimiters into out.  (strtok with a non-destructive API.)
void SkStrSplit(const char* str,
                const char* delimiters,
                SkStrSplitMode splitMode,
                skia_private::TArray<SkString>* out);

inline void SkStrSplit(
        const char* str, const char* delimiters, skia_private::TArray<SkString>* out) {
    SkStrSplit(str, delimiters, kCoalesce_SkStrSplitMode, out);
}

#endif
