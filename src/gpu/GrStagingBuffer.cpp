/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrGpu.h"
#include "src/gpu/GrStagingBuffer.h"

#include "src/core/SkMathPriv.h"

GrStagingBuffer::Slice GrStagingBuffer::allocate(size_t size) {
    SkASSERT(fMapPtr);
    size_t offset = fOffset;
    fOffset += size;
    char* mapPtr = static_cast<char*>(fMapPtr) + offset;
    return Slice(this, offset, mapPtr);
}
