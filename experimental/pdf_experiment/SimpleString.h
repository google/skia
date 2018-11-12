// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SimpleString_DEFINED
#define SimpleString_DEFINED

#include <cstring>

class SkArenaAlloc;

struct SimpleString {
    size_t fLength = 0;
    const char* fStart = nullptr;
};

static inline SimpleString StaticString(const char* s) {
    return SimpleString{strlen(s), s};
}

SimpleString AllocateString(SkArenaAlloc* arena, const char* src, size_t len);

SimpleString SimpleStringPrintf(SkArenaAlloc* arena, const char* format, ...);

#endif  // SimpleString_DEFINED
