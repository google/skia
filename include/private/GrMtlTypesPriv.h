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
    GrMtlBackendSurfaceInfo() : fTexture(nullptr) {}
    explicit GrMtlBackendSurfaceInfo(const GrMtlTextureInfo& info);
    GrMtlBackendSurfaceInfo(const GrMtlBackendSurfaceInfo& that) = delete;
    GrMtlBackendSurfaceInfo& operator=(const GrMtlBackendSurfaceInfo&) = delete;
    ~GrMtlBackendSurfaceInfo();

    void assign(const GrMtlBackendSurfaceInfo& that);
    void cleanup();
    GrMtlTextureInfo snapTextureInfo() const;

#if GR_TEST_UTILS
    bool operator==(const GrMtlBackendSurfaceInfo& that) const;
#endif

private:
    // Strong reference to the MTLTexture, maintained by CFRetain and CFRelease.
    const void* fTexture;
};

#endif
