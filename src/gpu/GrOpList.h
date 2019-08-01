/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOpList_DEFINED
#define GrOpList_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkColorData.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/GrRenderTask.h"
#include "src/gpu/GrTextureProxy.h"

class GrAuditTrail;
class GrOpMemoryPool;
class GrGpuBuffer;

class GrOpList : public GrRenderTask {
public:
    GrOpList(sk_sp<GrOpMemoryPool>, sk_sp<GrSurfaceProxy>, GrAuditTrail*);
    ~GrOpList() override;

    virtual bool copySurface(GrRecordingContext*,
                             GrSurfaceProxy* dst,
                             GrSurfaceProxy* src,
                             const SkIRect& srcRect,
                             const SkIPoint& dstPoint) = 0;

    virtual void transferFrom(GrRecordingContext*,
                              const SkIRect& srcRect,
                              GrColorType surfaceColorType,
                              GrColorType dstColorType,
                              sk_sp<GrGpuBuffer> dst,
                              size_t dstOffset) = 0;

    void endFlush() override;

    /*
     * Dump out the GrOpList dependency DAG
     */
    SkDEBUGCODE(void dump(bool printDependencies) const override;)

protected:
    // This is a backpointer to the GrOpMemoryPool that holds the memory for this opLists' ops.
    // In the DDL case, these back pointers keep the DDL's GrOpMemoryPool alive as long as its
    // constituent opLists survive.
    sk_sp<GrOpMemoryPool> fOpMemoryPool;
    GrAuditTrail*         fAuditTrail;

    GrLoadOp              fColorLoadOp    = GrLoadOp::kLoad;
    SkPMColor4f           fLoadClearColor = SK_PMColor4fTRANSPARENT;
    GrLoadOp              fStencilLoadOp  = GrLoadOp::kLoad;
};

#endif
