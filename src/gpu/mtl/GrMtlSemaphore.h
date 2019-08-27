/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlSemaphore_DEFINED
#define GrMtlSemaphore_DEFINED

#include "include/gpu/GrBackendSemaphore.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#include <Metal/Metal.h>

class GrMtlGpu;

class GrMtlSemaphore : public GrSemaphore {
public:
    static sk_sp<GrMtlSemaphore> Make(GrMtlGpu* gpu, bool isOwned);

    static sk_sp<GrMtlSemaphore> MakeWrapped(GrMtlGpu* gpu,
                                             GrMTLHandle event,
                                             uint64_t value,
                                             GrWrapOwnership ownership);

#if GR_METAL_SDK_VERSION >= 200
    id<MTLEvent> event() const { return fEvent; }
#endif
    uint64_t value() const { return fValue; }

    GrBackendSemaphore backendSemaphore() const override;

private:
    GrMtlSemaphore(GrMtlGpu* gpu, id<MTLEvent> event, uint64_t value, bool isOwned);

    void onRelease() override;
    void onAbandon() override;

#if GR_METAL_SDK_VERSION >= 200
    id<MTLEvent> fEvent;
#endif
    uint64_t     fValue;

    typedef GrSemaphore INHERITED;
};

#endif
