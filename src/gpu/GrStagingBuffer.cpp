/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrGpu.h"
#include "src/gpu/GrStagingBuffer.h"

#include "src/core/SkMathPriv.h"

void GrStagingBuffer::markAvailable(void* data) {
    fData = data;
    fOffset = 0;
    fGpu->moveStagingBufferFromBusyToAvailable(this);
}

void GrStagingBuffer::unmap() {
    this->onUnmap();
}

GrStagingBuffer::Slice GrStagingBuffer::allocate(size_t size) {
    size_t offset = fOffset;
    fOffset += size;
    char* data = static_cast<char*>(fData) + offset;
    return Slice(this, offset, data);
}
