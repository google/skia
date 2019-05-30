/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlTypes_DEFINED
#define GrMtlTypes_DEFINED

#include "include/gpu/GrTypes.h"

/**
 * Declares typedefs for Metal types used in Ganesh cpp code
 */
typedef unsigned int GrMTLPixelFormat;

///////////////////////////////////////////////////////////////////////////////

/**
 * Wrapper class for managing lifetime of CoreFoundation objects. It will call
 * CFRetain and CFRelease appropriately on creation, assignment, and deletion.
 */
class GrCFResource {
public:
    GrCFResource() : fCFObject(nullptr) {}
    explicit GrCFResource(const void* resource);
    GrCFResource(const GrCFResource& that);
    ~GrCFResource();

    GrCFResource& operator=(const void* resource) {
        this->assign(resource);
        return *this;
    }
    GrCFResource& operator=(const GrCFResource& that) {
        this->assign(that.fCFObject);
        return *this;
    }

    const void* get() const { return fCFObject; }

    bool operator==(const GrCFResource& that) const {
        return this->fCFObject == that.fCFObject;
    }
    bool operator!=(const GrCFResource& that) const {
        return this->fCFObject != that.fCFObject;
    }

private:
    void assign(const void*);

    const void* fCFObject;
};

/**
 * Types for interacting with Metal resources created externally to Skia.
 * This is used by GrBackendObjects.
 */
struct GrMtlTextureInfo {
public:
    GrMtlTextureInfo() {}

    GrCFResource fTexture;

    bool operator==(const GrMtlTextureInfo& that) const {
        return fTexture == that.fTexture;
    }
};

#endif
