/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlTypesPriv_DEFINED
#define GrMtlTypesPriv_DEFINED

#include "include/gpu/mtl/GrMtlTypes.h"

// This struct is to used to store the the actual information about the Metal backend texture on
// GrBackendTexture and GrBackendRenderTarget. This differs from the public GrMtlTextureInfo in
// that it maintains a strong reference to the MTLTexture object.
struct GrMtlBackendSurfaceInfo {
    GrMtlBackendSurfaceInfo() {}
    GrMtlBackendSurfaceInfo(const GrMtlBackendSurfaceInfo&) = delete;
    GrMtlBackendSurfaceInfo& operator=(const GrMtlBackendSurfaceInfo&) = delete;
    explicit GrMtlBackendSurfaceInfo(const GrMtlTextureInfo& info);
    ~GrMtlBackendSurfaceInfo();

    void cleanup();

    // Assigns the passed in GrMtlBackendSurfaceInfo to this object. if isValid is true we will also
    // attempt to unref the old fTexture.
    void assign(const GrMtlBackendSurfaceInfo&, bool isValid);

    GrMtlTextureInfo snapTextureInfo() const;

#if GR_TEST_UTILS
    bool operator==(const GrMtlBackendSurfaceInfo& that) const;
#endif

private:
    // Strong reference to the MTLTexture, maintained by CFRetain and CFRelease.
    const void* fTexture = nullptr;
};

#endif
