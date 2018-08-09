/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "NXTMTLUtils.h"
#include "Metal/Metal.h"

namespace backend {
    namespace metal {
        void Init(id<MTLDevice> metalDevice, dawnProcTable* procs, dawnDevice* device);
    }
}

void InitNXTMTLSystemDefaultDevice(dawnProcTable* procs, dawnDevice* device) {
    backend::metal::Init(MTLCreateSystemDefaultDevice(), procs, device);
}

void InitNXTMTLSystemDefaultDevice(dawnProcTable*, dawnDevice*);

void* CreateAutoreleasePool() {
    return [[NSAutoreleasePool alloc] init];
}

void DrainAutoreleasePool(void* pool) {
    [static_cast<NSAutoreleasePool*>(pool) drain];
}

void DestroyAutoreleasePool(void* pool) {
    [static_cast<NSAutoreleasePool*>(pool) release];
}
