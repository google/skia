/*
 * Copyright 2014
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextureCompression_opts.h"

SkTextureCompressor::CompressionProc
SkTextureCompressorGetPlatformProc(SkColorType colorType, SkTextureCompressor::Format fmt) {
    return NULL;
}

bool SkTextureCompressorGetPlatformDims(SkTextureCompressor::Format fmt, int* dimX, int* dimY) {
    return false;
}
