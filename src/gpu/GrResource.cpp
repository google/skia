
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrResource.h"
#include "GrGpu.h"

GrResource::GrResource(GrGpu* gpu, bool isWrapped) {
    fGpu              = gpu;
    fCacheEntry       = NULL;
    fDeferredRefCount = 0;
    if (isWrapped) {
        fFlags = kWrapped_FlagBit;
    } else {
        fFlags = 0;
    }
    fGpu->insertResource(this);
}

GrResource::~GrResource() {
    // subclass should have released this.
    SkASSERT(0 == fDeferredRefCount);
    SkASSERT(!this->isValid());
}

void GrResource::release() {
    if (NULL != fGpu) {
        this->onRelease();
        fGpu->removeResource(this);
        fGpu = NULL;
    }
}

void GrResource::abandon() {
    if (NULL != fGpu) {
        this->onAbandon();
        fGpu->removeResource(this);
        fGpu = NULL;
    }
}

const GrContext* GrResource::getContext() const {
    if (NULL != fGpu) {
        return fGpu->getContext();
    } else {
        return NULL;
    }
}

GrContext* GrResource::getContext() {
    if (NULL != fGpu) {
        return fGpu->getContext();
    } else {
        return NULL;
    }
}
