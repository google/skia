// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SimpleString.h"

#include "SkArenaAlloc.h"
#include "SkScopeExit.h"

#include <cstdarg>

SimpleString AllocateString(SkArenaAlloc* arena, const char* src, size_t len) {
    if (!arena) { return SimpleString(); }
    size_t size = len + 1;
    char* ptr = (char*)arena->makeBytesAlignedTo(size, alignof(char));
    if (src) {
        memcpy(ptr, src, len);
        ptr[len] = '\0';
    } else {
        bzero(ptr, size);
    }
    return SimpleString{len, ptr};
}

SimpleString SimpleStringPrintf(SkArenaAlloc* arena, const char* format, ...) {
    if (!arena) { return SimpleString(); }
    va_list args;
    va_start(args, format);
    SK_AT_SCOPE_EXIT(va_end(args));
    va_list argsCopy;
    va_copy(argsCopy, args);
    SK_AT_SCOPE_EXIT(va_end(argsCopy));
    char buffer[1024];
    int length = vsnprintf(buffer, sizeof(buffer), format, args);
    if (length < 0) {
        return StaticString(""); // vsnprintf reported error.
    } else if (length >= (int)sizeof(buffer)) {
        char* ptr = (char*)arena->makeBytesAlignedTo(length + 1, alignof(char));
        (void)vsnprintf(ptr, length + 1, format, argsCopy);
        return SimpleString{(size_t)length, ptr};
    } else {
        return AllocateString(arena, buffer, (size_t)length);
    }
}

