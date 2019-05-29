/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrMtlTypesPriv.h"

#include <CoreFoundation/CoreFoundation.h>

GrMtlBackendSurfaceInfo::GrMtlBackendSurfaceInfo(const GrMtlTextureInfo& info) : fTexture(info.fTexture) {
    if (fTexture) {
        CFRetain(fTexture);
    }
}

GrMtlBackendSurfaceInfo::~GrMtlBackendSurfaceInfo() {
    cleanup();
}

void GrMtlBackendSurfaceInfo::assign(const GrMtlBackendSurfaceInfo& that) {
    if (that.fTexture) {
        CFRetain(that.fTexture);
    }
    if (fTexture) {
        CFRelease(fTexture);
    }
    fTexture = that.fTexture;
}

void GrMtlBackendSurfaceInfo::cleanup() {
    if (fTexture) {
        CFRelease(fTexture);
        fTexture = nullptr;
    }
}

GrMtlTextureInfo GrMtlBackendSurfaceInfo::snapTextureInfo() const {
    GrMtlTextureInfo textureInfo;
    textureInfo.fTexture = fTexture;
    return textureInfo;
}

#if GR_TEST_UTILS
bool GrMtlBackendSurfaceInfo::operator==(const GrMtlBackendSurfaceInfo& that) const {
    return fTexture == that.fTexture;
}
#endif

