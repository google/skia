/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlSemaphore.h"

#include "src/gpu/mtl/GrMtlGpu.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

std::unique_ptr<GrMtlSemaphore> GrMtlSemaphore::Make(GrMtlGpu* gpu) {
    if (@available(macOS 10.14, iOS 12.0, *)) {
        id<MTLEvent> event = [gpu->device() newEvent];
        uint64_t value = 1; // seems like a reasonable starting point
        return std::unique_ptr<GrMtlSemaphore>(new GrMtlSemaphore(event, value));
    } else {
        return nullptr;
    }
}

std::unique_ptr<GrMtlSemaphore> GrMtlSemaphore::MakeWrapped(GrMTLHandle event,
                                                            uint64_t value) {
    // The GrMtlSemaphore will have strong ownership at this point.
    // The GrMTLHandle will subsequently only have weak ownership.
    if (@available(macOS 10.14, iOS 12.0, *)) {
        id<MTLEvent> mtlEvent = (__bridge_transfer id<MTLEvent>)event;
        return std::unique_ptr<GrMtlSemaphore>(new GrMtlSemaphore(mtlEvent, value));
    } else {
        return nullptr;
    }
}

GrMtlSemaphore::GrMtlSemaphore(id<MTLEvent> event, uint64_t value)
        : fEvent(event), fValue(value) {
}

GrBackendSemaphore GrMtlSemaphore::backendSemaphore() const {
    GrBackendSemaphore backendSemaphore;
    // The GrMtlSemaphore and the GrBackendSemaphore will have strong ownership at this point.
    // Whoever uses the GrBackendSemaphore will subsquently steal this ref (see MakeWrapped, above).
    if (@available(macOS 10.14, iOS 12.0, *)) {
        GrMTLHandle handle = (__bridge_retained GrMTLHandle)(fEvent);
        backendSemaphore.initMetal(handle, fValue);
    }
    return backendSemaphore;
}
