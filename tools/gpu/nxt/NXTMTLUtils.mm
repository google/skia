/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "NXTMTLUtils.h"
#include "Metal/Metal.h"
#include "dawn_native/DawnNative.h"
#include "dawn_native/MetalBackend.h"

dawnDevice CreateNXTMTLSystemDefaultDevice() {
    return dawn_native::metal::CreateDevice(MTLCreateSystemDefaultDevice());
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
