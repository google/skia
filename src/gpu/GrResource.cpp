
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrResource.h"
#include "GrGpu.h"

GrResource::GrResource(GrGpu* gpu) {
    fGpu        = gpu;
    fNext       = NULL;
    fPrevious   = NULL;
    fGpu->insertResource(this);
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
