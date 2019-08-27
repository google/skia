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

sk_sp<GrMtlSemaphore> GrMtlSemaphore::Make(GrMtlGpu* gpu, bool isOwned) {
#if GR_METAL_SDK_VERSION >= 200
    id<MTLEvent> event = [gpu->device() newEvent];
    uint64_t value = 1; // seems like a reasonable starting point
    return sk_sp<GrMtlSemaphore>(new GrMtlSemaphore(gpu, event, value, isOwned));
#else
    return nullptr;
#endif
}

sk_sp<GrMtlSemaphore> GrMtlSemaphore::MakeWrapped(GrMtlGpu* gpu,
                                                  GrMTLHandle event,
                                                  uint64_t value,
                                                  GrWrapOwnership ownership) {
#if GR_METAL_SDK_VERSION >= 200
    id<MTLEvent> mtlEvent = (__bridge_transfer id<MTLEvent>)event;
    auto sema = sk_sp<GrMtlSemaphore>(new GrMtlSemaphore(gpu, mtlEvent, value,
                                                         kBorrow_GrWrapOwnership != ownership));
    return sema;
#else
    return nullptr;
#endif
}

#if GR_METAL_SDK_VERSION >= 200
GrMtlSemaphore::GrMtlSemaphore(GrMtlGpu* gpu, id<MTLEvent> event, uint64_t value, bool isOwned)
        : INHERITED(gpu), fEvent(event), fValue(value) {
    isOwned ? this->registerWithCache(SkBudgeted::kNo)
            : this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}
#endif

void GrMtlSemaphore::onRelease() {
    fEvent = nil;
    INHERITED::onRelease();
}

void GrMtlSemaphore::onAbandon() {
    fEvent = nil;
    INHERITED::onAbandon();
}

GrBackendSemaphore GrMtlSemaphore::backendSemaphore() const {
    GrBackendSemaphore backendSemaphore;
    // Both GrMtlSemaphore and GrBackendSemaphore will have strong ownership at this point.
    // TODO: is this the correct model? If we don't use __bridge_retained it's possible the
    // original GrMtlSemaphore could be released and then this handle will be invalid.
    // But when we create the new GrMtlSemaphore from this we'll have to use __bridge_transfer,
    // which will release this ref. This will be fine if we only use a GrBackendSemaphore once
    // to create a new GrMtlSemaphore.
#if GR_METAL_SDK_VERSION >= 200
    GrMTLHandle handle = (__bridge_retained GrMTLHandle)(fEvent);
#else
    GrMTLHandle handle = (GrMTLHandle)(fEvent);
#endif
    backendSemaphore.initMetal(handle, fValue);
    return backendSemaphore;
}
