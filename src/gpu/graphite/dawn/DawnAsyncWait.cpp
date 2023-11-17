/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnAsyncWait.h"

#include "src/gpu/dawn/DawnUtilsPriv.h"

namespace skgpu::graphite {

DawnAsyncWait::DawnAsyncWait(const wgpu::Device& device) : fDevice(device), fSignaled(false) {}

bool DawnAsyncWait::yieldAndCheck() const {
    if (fSignaled.load(std::memory_order_acquire)) {
        return true;
    }

    DawnTickDevice(fDevice);

    return fSignaled.load(std::memory_order_acquire);
}

void DawnAsyncWait::busyWait() const {
    while (!this->yieldAndCheck()) {}
}

}  // namespace skgpu::graphite
