/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrMtlTypesPriv.h"

#include <CoreFoundation/CoreFoundation.h>

GrMtlBackendSurfaceInfo::GrMtlBackendSurfaceInfo(const GrMtlTextureInfo& info) : fTexture(info.fTexture) {
  CFRetain(fTexture);
}

GrMtlBackendSurfaceInfo::~GrMtlBackendSurfaceInfo() {
  SkASSERT(!fTexture);
}

void GrMtlBackendSurfaceInfo::cleanup() {
  CFRelease(fTexture);
  fTexture = nullptr;
}

void GrMtlBackendSurfaceInfo::assign(const GrMtlBackendSurfaceInfo& that, bool isValid) {
  CFRetain(that.fTexture);
  if (isValid)
    CFRelease(fTexture);
  fTexture = that.fTexture;
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

