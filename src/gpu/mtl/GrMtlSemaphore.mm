/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlSemaphore.h"

#include "src/gpu/mtl/GrMtlGpu.h"

std::unique_ptr<GrMtlSemaphore> GrMtlSemaphore::Make(GrMtlGpu* gpu) {
    if (@available(macOS 10.14, iOS 12.0, *)) {
        sk_cf_obj<GrMTLHandle> event(static_cast<GrMTLHandle>([gpu->device() newEvent]));
        uint64_t value = 1; // seems like a reasonable starting point
        return std::unique_ptr<GrMtlSemaphore>(new GrMtlSemaphore(std::move(event), value));
    } else {
        return nullptr;
    }
}

std::unique_ptr<GrMtlSemaphore> GrMtlSemaphore::MakeWrapped(GrMTLHandle event,
                                                            uint64_t value) {
    // Implicitly the GrMtlSemaphore will take ownership at this point.
    // The incoming GrMTLHandle will subsequently only have weak ownership.
    // TODO: Can we manage shared ownership now?
    if (@available(macOS 10.14, iOS 12.0, *)) {
        sk_cf_obj<GrMTLHandle> mtlEvent(event);
        return std::unique_ptr<GrMtlSemaphore>(new GrMtlSemaphore(std::move(mtlEvent), value));
    } else {
        return nullptr;
    }
}

GrMtlSemaphore::GrMtlSemaphore(sk_cf_obj<GrMTLHandle> event, uint64_t value)
        : fEvent(std::move(event)), fValue(value) {
}

GrBackendSemaphore GrMtlSemaphore::backendSemaphore() const {
    GrBackendSemaphore backendSemaphore;
    // The GrMtlSemaphore and the GrBackendSemaphore will have strong ownership at this point.
    // Whoever uses the GrBackendSemaphore will subsquently steal this ref (see MakeWrapped, above).
    if (@available(macOS 10.14, iOS 12.0, *)) {
        SkASSERT(fEvent);
        GrMTLHandle handle = CFRetain(fEvent.get());
        backendSemaphore.initMetal(handle, fValue);
    }
    return backendSemaphore;
}
