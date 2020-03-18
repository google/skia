/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlTypes_DEFINED
#define GrMtlTypes_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/ports/SkCFObject.h"

#ifdef SK_ENABLE_MTL_API_AVAILABLE
#define SK_MTL_API_AVAILABLE API_AVAILABLE
#else
#define SK_MTL_API_AVAILABLE(...)
#endif

/**
 * Declares typedefs for Metal types used in Ganesh cpp code
 */
typedef unsigned int GrMTLPixelFormat;
typedef const void*  GrMTLHandle;

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_METAL
/**
 * Types for interacting with Metal resources created externally to Skia.
 * This is used by GrBackendObjects.
 */
struct GrMtlTextureInfo {
public:
    GrMtlTextureInfo() {}

    sk_cf_obj<const void*> fTexture;

    bool operator==(const GrMtlTextureInfo& that) const {
        return fTexture == that.fTexture;
    }
};
#endif

#endif
