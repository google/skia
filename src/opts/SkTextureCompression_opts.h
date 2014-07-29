/*
 * Copyright 2014
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextureCompression_opts_DEFINED
#define SkTextureCompression_opts_DEFINED

#include "SkTextureCompressor.h"
#include "SkImageInfo.h"

SkTextureCompressor::CompressionProc
SkTextureCompressorGetPlatformProc(SkColorType colorType, SkTextureCompressor::Format fmt);

// Returns true if dimX and dimY are set to the block size of the supplied
// compression format according to how the platform can consume them. Returns false otherwise.
bool SkTextureCompressorGetPlatformDims(SkTextureCompressor::Format fmt, int* dimX, int* dimY);

#endif  // SkTextureCompression_opts_DEFINED
