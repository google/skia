/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkConvertPixels_DEFINED
#define SkConvertPixels_DEFINED

#include <cstddef>

struct SkImageInfo;

[[nodiscard]] bool SkConvertPixels(
        const SkImageInfo& dstInfo,       void* dstPixels, size_t dstRowBytes,
        const SkImageInfo& srcInfo, const void* srcPixels, size_t srcRowBytes);

#endif
