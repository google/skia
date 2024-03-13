/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/mtl/GrMtlSemaphore.h"

#include "include/gpu/ganesh/mtl/GrMtlBackendSemaphore.h"
#include "src/gpu/ganesh/mtl/GrMtlGpu.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

sk_sp<GrMtlEvent> GrMtlEvent::Make(GrMtlGpu* gpu) {
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, *)) {
        id<MTLEvent> event = [gpu->device() newEvent];
        return sk_sp<GrMtlEvent>(new GrMtlEvent(event));
    } else {
        return nullptr;
    }
}

sk_sp<GrMtlEvent> GrMtlEvent::MakeWrapped(GrMTLHandle event) {
    // The GrMtlEvent will have strong ownership at this point.
    // The GrMTLHandle will subsequently only have weak ownership.
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, *)) {
        id<MTLEvent> mtlEvent = (__bridge_transfer id<MTLEvent>)event;
        return sk_sp<GrMtlEvent>(new GrMtlEvent(mtlEvent));
    } else {
        return nullptr;
    }
}

GrBackendSemaphore GrMtlSemaphore::backendSemaphore() const {
    GrBackendSemaphore backendSemaphore;
    // The GrMtlSemaphore and the GrBackendSemaphore will have strong ownership at this point.
    // Whoever uses the GrBackendSemaphore will subsquently steal this ref (see MakeWrapped, above).
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, *)) {
        GrMTLHandle handle = (__bridge_retained GrMTLHandle)(fEvent->mtlEvent());
        backendSemaphore = GrBackendSemaphores::MakeMtl(handle, fValue);
    }
    return backendSemaphore;
}

GR_NORETAIN_END
