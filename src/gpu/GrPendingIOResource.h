/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPendingIOResource_DEFINED
#define GrPendingIOResource_DEFINED

#include "GrGpuResource.h"
#include "SkNoncopyable.h"
#include "SkRefCnt.h"

/**
 * Helper for owning a pending read, write, read-write on a GrGpuResource. It never owns a regular
 * ref.
 */
template <typename T, GrIOType IO_TYPE>
class GrPendingIOResource : SkNoncopyable {
public:
    GrPendingIOResource(T* resource = nullptr) : fResource(nullptr) { this->reset(resource); }

    GrPendingIOResource(const GrPendingIOResource& that) : GrPendingIOResource(that.get()) {}

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

    ~GrPendingIOResource() { this->release(); }

    explicit operator bool() const { return SkToBool(fResource); }

    bool operator==(const GrPendingIOResource& other) const { return fResource == other.fResource; }

    T* get() const { return fResource; }

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

    T* fResource;
};

#endif
