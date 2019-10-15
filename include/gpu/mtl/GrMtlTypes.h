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
// For setting up a render target derived from a CAMetalLayer
struct GrMtlLayerInfo {
public:
    GrMtlLayerInfo() {}

    sk_cf_obj<const void*> fLayer;    // The CAMetalLayer we're grabbing the drawable from
    GrMTLHandle*           fDrawable; // Pointer to return the drawable value to the client

    bool operator==(const GrMtlLayerInfo& that) const {
        return fLayer == that.fLayer && fDrawable == that.fDrawable;
    }
};
#endif

#endif
