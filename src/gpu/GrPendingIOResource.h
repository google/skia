/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPendingIOResource_DEFINED
#define GrPendingIOResource_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrGpuResource.h"
#include "include/private/GrSurfaceProxy.h"
#include "include/private/SkNoncopyable.h"

class GrProxyPendingIO : SkNoncopyable {
public:
    GrProxyPendingIO() = default;
    GrProxyPendingIO(GrSurfaceProxy* resource) { this->reset(resource); }
    ~GrProxyPendingIO() { this->reset(nullptr); }

    void reset(GrSurfaceProxy* resource = nullptr) {
        if (resource == fResource) {
            return;
        }

        if (fResource) {
            fResource->unref();
        }

        fResource = resource;
        if (fResource) {
            fResource->ref();
        }
    }

    explicit operator bool() const { return SkToBool(fResource); }

    GrSurfaceProxy* get() const { return fResource; }
    GrSurfaceProxy* operator->() const { return fResource; }

private:
    bool operator==(const GrProxyPendingIO& other) const = delete;

    GrSurfaceProxy* fResource = nullptr;
};

/**
 * Helper for owning a pending read, write, read-write on a GrGpuResource. It never owns a regular
 * ref.
 */
template <typename T, GrIOType IO_TYPE>
class GrPendingIOResource : SkNoncopyable {
public:
    GrPendingIOResource() = default;
    GrPendingIOResource(T* resource) { this->reset(resource); }
    GrPendingIOResource(sk_sp<T> resource) { *this = std::move(resource); }
    GrPendingIOResource(const GrPendingIOResource& that) : GrPendingIOResource(that.get()) {}
    ~GrPendingIOResource() { this->release(); }

    GrPendingIOResource& operator=(sk_sp<T> resource) {
        this->reset(resource.get());
        return *this;
    }

    void reset(T* resource = nullptr) {
        if (resource) {
            switch (IO_TYPE) {
                case kRead_GrIOType:
                    resource->addPendingRead();
                    break;
                case kWrite_GrIOType:
                    resource->addPendingWrite();
                    break;
                case kRW_GrIOType:
                    resource->addPendingRead();
                    resource->addPendingWrite();
                    break;
            }
        }
        this->release();
        fResource = resource;
    }

    explicit operator bool() const { return SkToBool(fResource); }

    bool operator==(const GrPendingIOResource& other) const { return fResource == other.fResource; }

    T* get() const { return fResource; }
    T* operator*() const { return *fResource; }
    T* operator->() const { return fResource; }

private:
    void release() {
        if (fResource) {
            switch (IO_TYPE) {
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

    T* fResource = nullptr;
};

#endif
