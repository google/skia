/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/SharedContext.h"

namespace skgpu::graphite {

void* Buffer::map() {
    SkASSERT(this->isUnmappable() || !this->sharedContext()->caps()->bufferMapsAreAsync());
    SkASSERT(this->isProtected() == Protected::kNo);
    if (!this->isMapped()) {
        this->onMap();
    }
    return fMapPtr;
}

void Buffer::asyncMap(GpuFinishedProc proc, GpuFinishedContext ctx) {
    SkASSERT(this->sharedContext()->caps()->bufferMapsAreAsync());
    SkASSERT(this->isProtected() == Protected::kNo);
    this->onAsyncMap(proc, ctx);
}

void Buffer::unmap() {
    SkASSERT(this->isUnmappable());
    this->onUnmap();
    fMapPtr = nullptr;
}

bool Buffer::isUnmappable() const { return isMapped(); }

void Buffer::onAsyncMap(skgpu::graphite::GpuFinishedProc, skgpu::graphite::GpuFinishedContext) {
    SkASSERT(!this->sharedContext()->caps()->bufferMapsAreAsync());
    SK_ABORT("Async buffer mapping not supported");
}

} // namespace skgpu::graphite

