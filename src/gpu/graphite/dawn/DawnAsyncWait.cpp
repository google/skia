/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnAsyncWait.h"

#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/gpu/graphite/dawn/DawnUtilsPriv.h"

namespace skgpu::graphite {

DawnAsyncWait::DawnAsyncWait(const DawnSharedContext* sharedContext)
        : fSharedContext(sharedContext)
        , fSignaled(false) {}

bool DawnAsyncWait::yieldAndCheck() const {
    if (fSharedContext->hasTick()) {
        if (fSignaled.load(std::memory_order_acquire)) {
            return true;
        }

        fSharedContext->tick();
    }
    return fSignaled.load(std::memory_order_acquire);
}

bool DawnAsyncWait::mayBusyWait() const { return fSharedContext->caps()->allowCpuSync(); }

void DawnAsyncWait::busyWait() const {
    SkASSERT(fSharedContext->hasTick());
    while (!this->yieldAndCheck()) {}
}

}  // namespace skgpu::graphite
