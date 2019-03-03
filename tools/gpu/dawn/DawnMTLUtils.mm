/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DawnMTLUtils.h"
#include "Metal/Metal.h"
#include "dawn_native/DawnNative.h"
#include "dawn_native/MetalBackend.h"

dawnDevice CreateDawnMTLSystemDefaultDevice() {
    auto instance = std::make_unique<dawn_native::Instance>();
    instance->DiscoverDefaultAdapters();

    dawnDevice backendDevice = nullptr;

    std::vector<dawn_native::Adapter> adapters = instance->GetAdapters();
    for (dawn_native::Adapter adapter : adapters) {
        if (adapter.GetBackendType() == dawn_native::BackendType::Metal) {
            backendDevice = adapter.CreateDevice();
            break;
        }
    }
    return backendDevice;
}

void* CreateAutoreleasePool() {
    return [[NSAutoreleasePool alloc] init];
}

void DrainAutoreleasePool(void* pool) {
    [static_cast<NSAutoreleasePool*>(pool) drain];
}

void DestroyAutoreleasePool(void* pool) {
    [static_cast<NSAutoreleasePool*>(pool) release];
}
