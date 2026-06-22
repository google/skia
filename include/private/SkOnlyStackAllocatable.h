/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOnlyStackAllocatable_DEFINED
#define SkOnlyStackAllocatable_DEFINED

#include <stddef.h>

/**
 * SK_ONLY_STACK_ALLOCATABLE is a macro that bans heap allocation for a class or struct
 * and serves as a marker for Clang static analysis tools (like the MiraclePtr rewriter).
 *
 * In Chromium, Clang plugins enforce that:
 * 1. StackAllocated structs are only embedded inside other StackAllocated structs.
 * 2. Pointers to StackAllocated structs are not rewritten to raw_ptr.
 *
 * Place this macro inside your class/struct definition (typically at the top):
 *   SK_ONLY_STACK_ALLOCATABLE;
 */
#define SK_ONLY_STACK_ALLOCATABLE                            \
    using IsStackAllocatedTypeMarker [[maybe_unused]] = int; \
    void* operator new(size_t) = delete;                     \
    void* operator new[](size_t) = delete

#endif
