/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SRC_PARTITION_ALLOC_NOOP_RAW_PTR_EXCLUSION_H_
#define SRC_PARTITION_ALLOC_NOOP_RAW_PTR_EXCLUSION_H_

// Marks a field as excluded from the `raw_ptr<T>` usage enforcement via
// Chromium Clang plugin.
//
// Example:
//     RAW_PTR_EXCLUSION Foo* foo_;
//
// `RAW_PTR_EXCLUSION` should be avoided, as exclusions makes it significantly easier for any bug
// involving the pointer to become a security vulnerability. For additional guidance please see the
// "When to use raw_ptr<T>" section of:
// https://source.chromium.org/chromium/chromium/src/+/main:base/memory/raw_ptr.md
#define RAW_PTR_EXCLUSION

#endif  // SRC_PARTITION_ALLOC_NOOP_RAW_PTR_EXCLUSION_H_
