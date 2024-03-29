/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlCppUtil_DEFINED
#define GrMtlCppUtil_DEFINED

#include "include/gpu/ganesh/mtl/GrMtlTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"

// Utilities that can be used from cpp files (rather than .mm).

GrMTLPixelFormat GrGetMTLPixelFormatFromMtlTextureInfo(const GrMtlTextureInfo&);

/**
 * Gets the sample count of a texture held by GrMtlTextureInfo or zero if the texture is nil.
 */
int GrMtlTextureInfoSampleCount(const GrMtlTextureInfo&);

#if defined(SK_DEBUG) || defined(GR_TEST_UTILS)
bool GrMtlFormatIsBGRA8(GrMTLPixelFormat mtlFormat);
#endif

#endif
