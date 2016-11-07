/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSurfaceProxy.h"

#include "GrGpuResourcePriv.h"
#include "GrOpList.h"
#include "GrTextureProvider.h"

GrSurfaceProxy::GrSurfaceProxy(sk_sp<GrSurface> surface, SkBackingFit fit)
    : INHERITED(std::move(surface))
    , fDesc(fTarget->desc())
    , fFit(fit)
    , fBudgeted(fTarget->resourcePriv().isBudgeted())
    , fUniqueID(fTarget->uniqueID())
    , fGpuMemorySize(kInvalidGpuMemorySize)
    , fLastOpList(nullptr) {
}

GrSurfaceProxy::~GrSurfaceProxy() {
    if (fLastOpList) {
        fLastOpList->clearTarget();
    }
    SkSafeUnref(fLastOpList);
}

GrSurface* GrSurfaceProxy::instantiate(GrTextureProvider* texProvider) {
    if (fTarget) {
        return fTarget;
    }

    if (SkBackingFit::kApprox == fFit) {
        fTarget = texProvider->createApproxTexture(fDesc);
    } else {
        fTarget = texProvider->createTexture(fDesc, fBudgeted);
    }
    if (!fTarget) {
        return nullptr;
    }

#ifdef SK_DEBUG
    if (kInvalidGpuMemorySize != this->getRawGpuMemorySize_debugOnly()) {
        SkASSERT(fTarget->gpuMemorySize() <= this->getRawGpuMemorySize_debugOnly());    
    }
#endif

    return fTarget;
}

void GrSurfaceProxy::setLastOpList(GrOpList* opList) {
    if (fLastOpList) {
        // The non-MDB world never closes so we can't check this condition
#ifdef ENABLE_MDB
        SkASSERT(fLastOpList->isClosed());
#endif
        fLastOpList->clearTarget();
    }

    SkRefCnt_SafeAssign(fLastOpList, opList);
}
