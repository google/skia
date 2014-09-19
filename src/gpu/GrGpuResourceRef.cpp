/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGpuResourceRef.h"

GrGpuResourceRef::GrGpuResourceRef() {
    fResource = NULL;
    fOwnRef = false;
    fPendingIO = false;
}

GrGpuResourceRef::GrGpuResourceRef(GrGpuResource* resource, GrIORef::IOType ioType) {
    fResource = NULL;
    fOwnRef = false;
    fPendingIO = false;
    this->setResource(resource, ioType);
}

GrGpuResourceRef::~GrGpuResourceRef() {
    if (fOwnRef) {
        SkASSERT(fResource);
        fResource->unref();
    }
    if (fPendingIO) {
        switch (fIOType) {
            case GrIORef::kRead_IOType:
                fResource->completedRead();
                break;
            case GrIORef::kWrite_IOType:
                fResource->completedWrite();
                break;
            case GrIORef::kRW_IOType:
                fResource->completedRead();
                fResource->completedWrite();
                break;
        }
    }
}

void GrGpuResourceRef::reset() {
    SkASSERT(!fPendingIO);
    SkASSERT(SkToBool(fResource) == fOwnRef);
    if (fOwnRef) {
        fResource->unref();
        fOwnRef = false;
        fResource = NULL;
    }
}

void GrGpuResourceRef::setResource(GrGpuResource* resource, GrIORef::IOType ioType) {
    SkASSERT(!fPendingIO);
    SkASSERT(SkToBool(fResource) == fOwnRef);
    SkSafeUnref(fResource);
    if (NULL == resource) {
        fResource = NULL;
        fOwnRef = false;
    } else {
        fResource = resource;
        fOwnRef = true;
        fIOType = ioType;
    }
}

void GrGpuResourceRef::markPendingIO() const {
    // This should only be called when the owning GrProgramElement gets its first
    // pendingExecution ref.
    SkASSERT(!fPendingIO);
    SkASSERT(fResource);
    fPendingIO = true;
    switch (fIOType) {
        case GrIORef::kRead_IOType:
            fResource->addPendingRead();
            break;
        case GrIORef::kWrite_IOType:
            fResource->addPendingWrite();
            break;
        case GrIORef::kRW_IOType:
            fResource->addPendingRead();
            fResource->addPendingWrite();
            break;
    }
}

void GrGpuResourceRef::pendingIOComplete() const {
    // This should only be called when the owner's pending executions have ocurred but it is still
    // reffed.
    SkASSERT(fOwnRef);
    SkASSERT(fPendingIO);
    switch (fIOType) {
        case GrIORef::kRead_IOType:
            fResource->completedRead();
            break;
        case GrIORef::kWrite_IOType:
            fResource->completedWrite();
            break;
        case GrIORef::kRW_IOType:
            fResource->completedRead();
            fResource->completedWrite();
            break;

    }
    fPendingIO = false;
}

void GrGpuResourceRef::removeRef() const {
    // This should only be called once, when the owners last ref goes away and
    // there is a pending execution.
    SkASSERT(fOwnRef);
    SkASSERT(fPendingIO);
    SkASSERT(fResource);
    fResource->unref();
    fOwnRef = false;
}
