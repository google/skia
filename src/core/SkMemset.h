/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMemset_DEFINED
#define SkMemset_DEFINED

#include <cstddef>
#include <cstdint>

namespace SkOpts {
    extern void (*memset16)(uint16_t[], uint16_t, int);
    extern void (*memset32)(uint32_t[], uint32_t, int);
    extern void (*memset64)(uint64_t[], uint64_t, int);

    extern void (*rect_memset16)(uint16_t[], uint16_t, int, size_t, int);
    extern void (*rect_memset32)(uint32_t[], uint32_t, int, size_t, int);
    extern void (*rect_memset64)(uint64_t[], uint64_t, int, size_t, int);

    void Init_Memset();
}  // namespace SkOpts

#endif // SkMemset_DEFINED
