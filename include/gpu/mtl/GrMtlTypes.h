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

#ifdef __APPLE__

#include <TargetConditionals.h>

#if TARGET_OS_SIMULATOR
#define SK_API_AVAILABLE_CA_METAL_LAYER SK_API_AVAILABLE(macos(10.11), ios(13.0))
#else  // TARGET_OS_SIMULATOR
#define SK_API_AVAILABLE_CA_METAL_LAYER SK_API_AVAILABLE(macos(10.11), ios(8.0))
#endif  // TARGET_OS_SIMULATOR

/**
 * Types for interacting with Metal resources created externally to Skia.
 * This is used by GrBackendObjects.
 */
struct GrMtlTextureInfo {
public:
    GrMtlTextureInfo() {}

    sk_cfp<GrMTLHandle> fTexture;

    bool operator==(const GrMtlTextureInfo& that) const {
        return fTexture == that.fTexture;
    }
};

#include "include/core/SkRefCnt.h"
#ifdef __OBJC__
#import <Metal/Metal.h>
#endif

// interface classes for the GPU memory allocator
class GrMtlAlloc : public SkRefCnt {
public:
    ~GrMtlAlloc() override = default;
};

class GrMtlMemoryAllocator : public SkRefCnt {
public:
#ifdef __OBJC__
    virtual id<MTLBuffer> createBuffer(NSUInteger length, MTLResourceOptions options,
                                       sk_sp<GrMtlAlloc>* allocation) = 0;
    virtual id<MTLTexture> createTexture(MTLTextureDescriptor* texDesc,
                                         sk_sp<GrMtlAlloc>* allocation) = 0;
#endif
};

#endif

#endif
