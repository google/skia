
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGpuObject.h"
#include "GrGpu.h"

GrGpuObject::GrGpuObject(GrGpu* gpu, bool isWrapped)
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

GrGpuObject::~GrGpuObject() {
    SkASSERT(0 == fRefCnt);
    // subclass should have released this.
    SkASSERT(this->wasDestroyed());
}

void GrGpuObject::release() {
    if (NULL != fGpu) {
        this->onRelease();
        fGpu->removeObject(this);
        fGpu = NULL;
    }
}

void GrGpuObject::abandon() {
    if (NULL != fGpu) {
        this->onAbandon();
        fGpu->removeObject(this);
        fGpu = NULL;
    }
}

const GrContext* GrGpuObject::getContext() const {
    if (NULL != fGpu) {
        return fGpu->getContext();
    } else {
        return NULL;
    }
}

GrContext* GrGpuObject::getContext() {
    if (NULL != fGpu) {
        return fGpu->getContext();
    } else {
        return NULL;
    }
}

uint32_t GrGpuObject::CreateUniqueID() {
    static int32_t gUniqueID = SK_InvalidUniqueID;
    uint32_t id;
    do {
        id = static_cast<uint32_t>(sk_atomic_inc(&gUniqueID) + 1);
    } while (id == SK_InvalidUniqueID);
    return id;
}
