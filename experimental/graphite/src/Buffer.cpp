/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Buffer.h"

namespace skgpu {

void* Buffer::map() {
    if (!this->isMapped()) {
        this->onMap();
    }
    return fMapPtr;
}

void Buffer::unmap() {
    SkASSERT(this->isMapped());
    this->onUnmap();
    fMapPtr = nullptr;
}

} // namespace skgpu

