/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_DataUtils_DEFINED
#define skgpu_DataUtils_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkSize.h"

#include <cstddef>

enum class SkTextureCompressionType;

namespace skgpu {

enum class Mipmapped : bool;

size_t NumCompressedBlocks(SkTextureCompressionType, SkISize baseDimensions);

// Returns a value that can be used to set rowBytes for a transfer function.
size_t CompressedRowBytes(SkTextureCompressionType, int w);

// Return the pixel dimensions of a compressed texture. The topmost levels
// of a compressed mipmapped texture (i.e., 1x1 or 2x2) still occupy a full
// block and thus objectively take up more pixels (e.g., 4x4 pixels for ETC1).
SkISize CompressedDimensions(SkTextureCompressionType, SkISize baseDimensions);

SkISize CompressedDimensionsInBlocks(SkTextureCompressionType, SkISize baseDimensions);

void FillInCompressedData(SkTextureCompressionType,
                          SkISize dimensions,
                          skgpu::Mipmapped,
                          char* dest,
                          const SkColor4f& color);

} // namespace skgpu

#endif // skgpu_DataUtils_DEFINED
