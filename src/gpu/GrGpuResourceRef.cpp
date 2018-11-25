/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGpuResourceRef.h"

GrGpuResourceRef::GrGpuResourceRef() {
    fResource = nullptr;
    fOwnRef = false;
    fPendingIO = false;
}

GrGpuResourceRef::GrGpuResourceRef(GrGpuResource* resource, GrIOType ioType) {
    fResource = nullptr;
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
            case kRead_GrIOType:
                fResource->completedRead();
                break;
            case kWrite_GrIOType:
                fResource->completedWrite();
                break;
            case kRW_GrIOType:
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
        fResource = nullptr;
    }
}

void GrGpuResourceRef::setResource(GrGpuResource* resource, GrIOType ioType) {
    SkASSERT(!fPendingIO);
    SkASSERT(SkToBool(fResource) == fOwnRef);
    SkSafeUnref(fResource);
    if (nullptr == resource) {
        fResource = nullptr;
        fOwnRef = false;
    } else {
        fResource = resource;
        fOwnRef = true;
        fIOType = ioType;
    }
}

void GrGpuResourceRef::markPendingIO() const {
    if (!fResource) {
        return;
    }

    // This should only be called when the owning GrProgramElement gets its first
    // pendingExecution ref.
    SkASSERT(!fPendingIO);
    fPendingIO = true;
    switch (fIOType) {
        case kRead_GrIOType:
            fResource->addPendingRead();
            break;
        case kWrite_GrIOType:
            fResource->addPendingWrite();
            break;
        case kRW_GrIOType:
            fResource->addPendingRead();
            fResource->addPendingWrite();
            break;
    }
}

void GrGpuResourceRef::pendingIOComplete() const {
    if (!fResource) {
        return;
    }

    // This should only be called when the owner's pending executions have ocurred but it is still
    // reffed.
    SkASSERT(fOwnRef);
    SkASSERT(fPendingIO);
    switch (fIOType) {
        case kRead_GrIOType:
            fResource->completedRead();
            break;
        case kWrite_GrIOType:
            fResource->completedWrite();
            break;
        case kRW_GrIOType:
            fResource->completedRead();
            fResource->completedWrite();
            break;

    }
    fPendingIO = false;
}

void GrGpuResourceRef::removeRef() const {
    if (!fResource) {
        return;
    }

    // This should only be called once, when the owners last ref goes away and
    // there is a pending execution.
    SkASSERT(fOwnRef);
    SkASSERT(fPendingIO);
    fResource->unref();
    fOwnRef = false;
}
