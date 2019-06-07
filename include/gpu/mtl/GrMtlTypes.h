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

/**
 * Declares typedefs for Metal types used in Ganesh cpp code
 */
typedef unsigned int GrMTLPixelFormat;

///////////////////////////////////////////////////////////////////////////////

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
