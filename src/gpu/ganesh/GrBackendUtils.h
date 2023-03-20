/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendUtils_DEFINED
#define GrBackendUtils_DEFINED

class GrBackendFormat;
enum class SkTextureCompressionType;

#include <cstddef>

SkTextureCompressionType GrBackendFormatToCompressionType(const GrBackendFormat& format);

// Returns the number of bytes per texel block for the given format. All non compressed formats
// are treated as having a block size of 1x1, so this is equivalent to bytesPerPixel.
size_t GrBackendFormatBytesPerBlock(const GrBackendFormat& format);

// Returns the number of bytes per pixel for the given format. All compressed formats will return 0.
size_t GrBackendFormatBytesPerPixel(const GrBackendFormat& format);

int GrBackendFormatStencilBits(const GrBackendFormat& format);

#endif
