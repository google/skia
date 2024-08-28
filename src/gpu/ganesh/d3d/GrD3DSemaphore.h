/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlSemaphore_DEFINED
#define GrMtlSemaphore_DEFINED

#include "include/gpu/ganesh/GrBackendSemaphore.h"
#include "include/gpu/ganesh/d3d/GrD3DTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrSemaphore.h"

class GrD3DGpu;

class GrD3DSemaphore : public GrSemaphore {
public:
    static std::unique_ptr<GrD3DSemaphore> Make(GrD3DGpu* gpu);

    static std::unique_ptr<GrD3DSemaphore> MakeWrapped(const GrD3DFenceInfo&);

    ~GrD3DSemaphore() override {}

    ID3D12Fence* fence() const { return fFenceInfo.fFence.get(); }
    uint64_t value() const { return fFenceInfo.fValue; }

    GrBackendSemaphore backendSemaphore() const override;

private:
    GrD3DSemaphore(const GrD3DFenceInfo&);

    void setIsOwned() override {}

    GrD3DFenceInfo fFenceInfo;

    using INHERITED = GrSemaphore;
};

#endif
