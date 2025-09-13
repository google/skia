/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef CompressedTexture_DEFINED
#define CompressedTexture_DEFINED

#include "include/core/SkColorType.h"
#include "include/core/SkPixmap.h"

namespace sk_gpu_test {
/**
 * BC1 compress an image that contains only either opaque black or transparent black and one
 * other color.
 *   opaque pixmaps      -> kBC1_RGB8_UNORM
 *   transparent pixmaps -> kBC1_RGBA8_UNORM
 */
void TwoColorBC1Compress(const SkPixmap& pixmap, SkColor otherColor, char* dstPixels);
}  // namespace sk_gpu_test

#endif  // CompressedTexture_DEFINED
