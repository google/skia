// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef utf8_tools_DEFINED
#define utf8_tools_DEFINED

#include <cstddef>

static inline bool is_utf8_continuation(char v) {
    return ((unsigned char)v & 0b11000000) ==
                               0b10000000;
}

static inline const char* next_utf8(const char* p, const char* end) {
    if (p < end) {
        do {
            ++p;
        } while (p < end && is_utf8_continuation(*p));
    }
    return p;
}

static inline const char* align_utf8(const char* p, const char* begin) {
    while (p > begin && is_utf8_continuation(*p)) {
        --p;
    }
    return p;
}

static inline const char* prev_utf8(const char* p, const char* begin) {
    return p > begin ? align_utf8(p - 1, begin) : begin;
}

// Makes use of ICU, if available.
bool is_utf8_whitespace(const char* p, const char* end);

// The concept of a "word" is more complex than these functions handle.
// TODO(halcanary): look at SkParagraph's getWordBoundary() or markLineBreaks()
static inline const char* next_utf8_word(const char* p, const char* end) {
    do {
        p = next_utf8(p, end);
    } while (p < end && !is_utf8_whitespace(p, end));
    return p;
}

static inline const char* prev_utf8_word(const char* p, const char* begin, const char* end) {
    do {
        p = prev_utf8(p, begin);
    } while (p > begin && !is_utf8_whitespace(p, end));
    return p;
}

#endif  // utf8_tools_DEFINED
