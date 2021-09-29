/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlTypes_DEFINED
#define skgpu_MtlTypes_DEFINED

#include "include/ports/SkCFObject.h"

///////////////////////////////////////////////////////////////////////////////

#ifdef __APPLE__

#include <CoreFoundation/CoreFoundation.h>
#include <TargetConditionals.h>

#if TARGET_OS_SIMULATOR
#define SK_API_AVAILABLE_CA_METAL_LAYER SK_API_AVAILABLE(macos(10.11), ios(13.0))
#else  // TARGET_OS_SIMULATOR
#define SK_API_AVAILABLE_CA_METAL_LAYER SK_API_AVAILABLE(macos(10.11), ios(8.0))
#endif  // TARGET_OS_SIMULATOR

#endif // __APPLE__


namespace skgpu::mtl {

/**
 * Declares typedefs for Metal types used in Graphite cpp code
 */
using PixelFormat = unsigned int;
using TextureUsage = unsigned int;
using StorageMode = unsigned int;

struct TextureInfo {
    uint32_t fSampleCount = 1;
    uint32_t fLevelCount = 0;

    // Since we aren't in an Obj-C header we can't directly use Mtl types here. Each of these can
    // cast to their mapped Mtl types list below.
    PixelFormat fFormat = 0;       // MTLPixelFormat fFormat = MTLPixelFormatInvalid;
    TextureUsage fUsage = 0;       // MTLTextureUsage fUsage = MTLTextureUsageUnknown;
    StorageMode fStorageMode = 0;  // MTLStorageMode fStorageMode = MTLStorageModeShared;
};

} // namespace skgpu::mtl

#endif // skgpu_MtlTypes_DEFINED
