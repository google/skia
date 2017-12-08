/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef BinaryAsset_DEFINED
#define BinaryAsset_DEFINED

#include <cstddef>

struct BinaryAsset {
    const char* name;
    const void* data;
    size_t len;
};

#endif  // BinaryAsset_DEFINED
