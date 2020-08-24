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
    static std::unique_ptr<GrMtlSemaphore> Make(GrMtlGpu* gpu);

    static std::unique_ptr<GrMtlSemaphore> MakeWrapped(GrMTLHandle event, uint64_t value);

    ~GrMtlSemaphore() override {}

    id<MTLEvent> event() const SK_API_AVAILABLE(macos(10.14), ios(12.0)) {
        return static_cast<id<MTLEvent>>(fEvent.get());
    }
    uint64_t value() const { return fValue; }

    GrBackendSemaphore backendSemaphore() const override;

private:
    GrMtlSemaphore(sk_cf_obj<GrMTLHandle> event,
                   uint64_t value);

    void setIsOwned() override {}

    sk_cf_obj<GrMTLHandle> fEvent;
    uint64_t fValue;

    typedef GrSemaphore INHERITED;
} SK_API_AVAILABLE(macos(10.14), ios(12.0));

#endif
