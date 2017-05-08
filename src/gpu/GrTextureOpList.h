/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTexureOpList_DEFINED
#define GrTexureOpList_DEFINED

#include "GrGpuResource.h"
#include "GrOpList.h"
#include "GrSurfaceProxy.h"

#include "SkTArray.h"

class GrAuditTrail;
class GrGpu;
class GrOp;
class GrTextureProxy;
struct SkIPoint;
struct SkIRect;

class GrTextureOpList final : public GrOpList {
public:
    GrTextureOpList(GrTextureProxy*, GrGpu*, GrAuditTrail*);

    ~GrTextureOpList() override;

    /**
     * Empties the draw buffer of any queued ops.
     */
    void reset() override;

    void abandonGpuResources() override {}
    void freeGpuResources() override {}

    /**
     * Together these two functions flush all queued ops to GrGpuCommandBuffer. The return value
     * of executeOps() indicates whether any commands were actually issued to the GPU.
     */
    void prepareOps(GrOpFlushState* flushState) override;
    bool executeOps(GrOpFlushState* flushState) override;

    /**
     * Copies a pixel rectangle from one surface to another. This call may finalize
     * reserved vertex/index data (as though a draw call was made). The src pixels
     * copied are specified by srcRect. They are copied to a rect of the same
     * size in dst with top left at dstPoint. If the src rect is clipped by the
     * src bounds then  pixel values in the dst rect corresponding to area clipped
     * by the src rect are not overwritten. This method is not guaranteed to succeed
     * depending on the type of surface, configs, etc, and the backend-specific
     * limitations.
     */
    bool copySurface(GrResourceProvider* resourceProvider,
                     GrSurfaceProxy* dst,
                     GrSurfaceProxy* src,
                     const SkIRect& srcRect,
                     const SkIPoint& dstPoint);

    GrTextureOpList* asTextureOpList() override { return this; }

    SkDEBUGCODE(void dump() const override;)

private:
    // MDB TODO: The unique IDs are only needed for the audit trail. There should only be one
    // on the opList itself.
    void recordOp(std::unique_ptr<GrOp>,
                  GrGpuResource::UniqueID resourceUniqueID,
                  GrSurfaceProxy::UniqueID proxyUniqueID);

    SkSTArray<2, std::unique_ptr<GrOp>, true> fRecordedOps;
    GrGpu*                          fGpu;

    typedef GrOpList INHERITED;
};

#endif
