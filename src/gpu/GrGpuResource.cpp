
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGpuResource.h"
#include "GrGpu.h"

GrGpuResource::GrGpuResource(GrGpu* gpu, bool isWrapped)
    : fRefCnt(1)
    , fCacheEntry(NULL)
    , fUniqueID(CreateUniqueID()) {
    fGpu              = gpu;
    if (isWrapped) {
        fFlags = kWrapped_FlagBit;
    } else {
        fFlags = 0;
    }
    fGpu->insertObject(this);
}

GrGpuResource::~GrGpuResource() {
    SkASSERT(0 == fRefCnt);
    // subclass should have released this.
    SkASSERT(this->wasDestroyed());
}

void GrGpuResource::release() {
    if (NULL != fGpu) {
        this->onRelease();
        fGpu->removeObject(this);
        fGpu = NULL;
    }
}

void GrGpuResource::abandon() {
    if (NULL != fGpu) {
        this->onAbandon();
        fGpu->removeObject(this);
        fGpu = NULL;
    }
}

const GrContext* GrGpuResource::getContext() const {
    if (NULL != fGpu) {
        return fGpu->getContext();
    } else {
        return NULL;
    }
}

GrContext* GrGpuResource::getContext() {
    if (NULL != fGpu) {
        return fGpu->getContext();
    } else {
        return NULL;
    }
}

uint32_t GrGpuResource::CreateUniqueID() {
    static int32_t gUniqueID = SK_InvalidUniqueID;
    uint32_t id;
    do {
        id = static_cast<uint32_t>(sk_atomic_inc(&gUniqueID) + 1);
    } while (id == SK_InvalidUniqueID);
    return id;
}
