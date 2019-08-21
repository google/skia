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

    /**
     * Copies a pixel rectangle from a proxy into this opLists's target. This call may finalize
     * reserved vertex/index data (as though a draw call was made). The src pixels copied are
     * specified by srcRect. They are copied to a rect of the same size in this opList's target with
     * top left at dstPoint. If the src rect is clipped by the src bounds then  pixel values in the
     * dst rect corresponding to area clipped by the src rect are not overwritten. This method is
     * not guaranteed to succeed depending on the type of surface, configs, etc, and the
     * backend-specific limitations.
     */
    virtual bool copySurface(GrRecordingContext*,
                             GrSurfaceProxy* src,
                             const SkIRect& srcRect,
                             const SkIPoint& dstPoint) = 0;

    void endFlush() override;

protected:
    // This is a backpointer to the GrOpMemoryPool that holds the memory for this opLists' ops.
    // In the DDL case, these back pointers keep the DDL's GrOpMemoryPool alive as long as its
    // constituent opLists survive.
    sk_sp<GrOpMemoryPool> fOpMemoryPool;
    GrAuditTrail*         fAuditTrail;
};

#endif
