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

///////////////////////////////////////////////////////////////////////////////
#include "GrTextureProxy.h"

GrSurfaceProxyRef::GrSurfaceProxyRef() {
    fProxy = nullptr;
    fOwnRef = false;
    fPendingIO = false;
}

GrSurfaceProxyRef::GrSurfaceProxyRef(sk_sp<GrSurfaceProxy> proxy, GrIOType ioType) {
    fProxy = nullptr;
    fOwnRef = false;
    fPendingIO = false;
    this->setProxy(std::move(proxy), ioType);
}

GrSurfaceProxyRef::~GrSurfaceProxyRef() {
    this->reset();
}

void GrSurfaceProxyRef::reset() {
    if (fPendingIO) {
        SkASSERT(fProxy);
        switch (fIOType) {
            case kRead_GrIOType:
                fProxy->completedRead();
                break;
            case kWrite_GrIOType:
                fProxy->completedWrite();
                break;
            case kRW_GrIOType:
                fProxy->completedRead();
                fProxy->completedWrite();
                break;
        }
        fPendingIO = false;
    }
    if (fOwnRef) {
        SkASSERT(fProxy);
        fProxy->unref();
        fOwnRef = false;
    }

    fProxy = nullptr;
}

void GrSurfaceProxyRef::setProxy(sk_sp<GrSurfaceProxy> proxy, GrIOType ioType) {
    SkASSERT(!fPendingIO);
    SkASSERT(SkToBool(fProxy) == fOwnRef);
    SkSafeUnref(fProxy);
    if (!proxy) {
        fProxy = nullptr;
        fOwnRef = false;
    } else {
        fProxy = proxy.release();   // due to the semantics of this class we unpack from sk_sp
        fOwnRef = true;
        fIOType = ioType;
    }
}

void GrSurfaceProxyRef::markPendingIO() const {
    // This should only be called when the owning GrProgramElement gets its first
    // pendingExecution ref.
    SkASSERT(!fPendingIO);
    SkASSERT(fProxy);
    fPendingIO = true;
    switch (fIOType) {
        case kRead_GrIOType:
            fProxy->addPendingRead();
            break;
        case kWrite_GrIOType:
            fProxy->addPendingWrite();
            break;
        case kRW_GrIOType:
            fProxy->addPendingRead();
            fProxy->addPendingWrite();
            break;
    }
}

void GrSurfaceProxyRef::pendingIOComplete() const {
    // This should only be called when the owner's pending executions have ocurred but it is still
    // reffed.
    SkASSERT(fOwnRef);
    SkASSERT(fPendingIO);
    switch (fIOType) {
        case kRead_GrIOType:
            fProxy->completedRead();
            break;
        case kWrite_GrIOType:
            fProxy->completedWrite();
            break;
        case kRW_GrIOType:
            fProxy->completedRead();
            fProxy->completedWrite();
            break;

    }
    fPendingIO = false;
}

void GrSurfaceProxyRef::removeRef() const {
    // This should only be called once, when the owners last ref goes away and
    // there is a pending execution.
    SkASSERT(fOwnRef);
    SkASSERT(fPendingIO);
    SkASSERT(fProxy);
    fProxy->unref();
    fOwnRef = false;
}

