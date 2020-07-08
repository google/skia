/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DCommandSignature_DEFINED
#define GrD3DCommandSignature_DEFINED

#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/GrManagedResource.h"

class GrD3DGpu;

class GrD3DCommandSignature : public GrManagedResource {
public:
    enum class ForIndexed : bool {
        kYes = true,
        kNo = false
    };
    static sk_sp<GrD3DCommandSignature> Make(GrD3DGpu* gpu, ForIndexed indexed, unsigned int slot);

    bool isCompatible(ForIndexed indexed, unsigned int slot) const {
        return (fIndexed == indexed && fSlot == slot);
    }

    ID3D12CommandSignature* commandSignature() const { return fCommandSignature.get(); }

#ifdef SK_TRACE_MANAGED_RESOURCES
    /** Output a human-readable dump of this resource's information
    */
    void dumpInfo() const override {
        SkDebugf("GrD3DCommandSignature: %p, (%d refs)\n",
                 fCommandSignature.get(), this->getRefCnt());
    }
#endif

private:
    GrD3DCommandSignature(gr_cp<ID3D12CommandSignature> commandSignature, ForIndexed indexed,
                          unsigned int slot)
        : fCommandSignature(commandSignature)
        , fIndexed(indexed)
        , fSlot(slot) {}

    // This will be called right before this class is destroyed and there is no reason to explicitly
    // release the fCommandSignature cause the gr_cp will handle that in the dtor.
    void freeGPUData() const override {}

    gr_cp<ID3D12CommandSignature> fCommandSignature;
    ForIndexed fIndexed;
    unsigned int fSlot;
};

#endif
