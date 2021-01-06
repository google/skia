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

#include <TargetConditionals.h>

#if TARGET_OS_SIMULATOR
#define SK_API_AVAILABLE_CA_METAL_LAYER SK_API_AVAILABLE(macos(10.11), ios(13.0))
#else  // TARGET_OS_SIMULATOR
#define SK_API_AVAILABLE_CA_METAL_LAYER SK_API_AVAILABLE(macos(10.11), ios(8.0))
#endif  // TARGET_OS_SIMULATOR

#if defined(SK_BUILD_FOR_MAC)
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 110000
#define GR_METAL_SDK_VERSION 230
#elif __MAC_OS_X_VERSION_MAX_ALLOWED >= 101500
#define GR_METAL_SDK_VERSION 220
#elif __MAC_OS_X_VERSION_MAX_ALLOWED >= 101400
#define GR_METAL_SDK_VERSION 210
#else
#error Must use at least 10.14 SDK to build Metal backend for MacOS
#endif
#else
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 140000 || __TV_OS_VERSION_MAX_ALLOWED >= 140000
#define GR_METAL_SDK_VERSION 230
#elif __IPHONE_OS_VERSION_MAX_ALLOWED >= 130000 || __TV_OS_VERSION_MAX_ALLOWED >= 130000
#define GR_METAL_SDK_VERSION 220
#elif __IPHONE_OS_VERSION_MAX_ALLOWED >= 120000 || __TV_OS_VERSION_MAX_ALLOWED >= 120000
#define GR_METAL_SDK_VERSION 210
#else
#error Must use at least 12.00 SDK to build Metal backend for iOS
#endif
#endif

/**
 * Types for interacting with Metal resources created externally to Skia.
 * This is used by GrBackendObjects.
 */
struct GrMtlTextureInfo {
public:
    GrMtlTextureInfo() {}

    sk_cfp<const void*> fTexture;

    bool operator==(const GrMtlTextureInfo& that) const {
        return fTexture == that.fTexture;
    }
};

#endif

#endif
