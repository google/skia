/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlGraphiteTypesPriv_DEFINED
#define skgpu_graphite_MtlGraphiteTypesPriv_DEFINED

#include "include/core/SkString.h"
#include "include/ports/SkCFObject.h"

class SkStream;
class SkWStream;

///////////////////////////////////////////////////////////////////////////////

#include <TargetConditionals.h>

// We're using the MSL version as shorthand for the Metal SDK version here
#if defined(SK_BUILD_FOR_MAC)
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 130000
#define SKGPU_GRAPHITE_METAL_SDK_VERSION 300
#elif __MAC_OS_X_VERSION_MAX_ALLOWED >= 120000
#define SKGPU_GRAPHITE_METAL_SDK_VERSION 240
#elif __MAC_OS_X_VERSION_MAX_ALLOWED >= 110000
#define SKGPU_GRAPHITE_METAL_SDK_VERSION 230
#else
#error Must use at least 11.00 SDK to build Metal backend for MacOS
#endif
#else
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 160000 || __TV_OS_VERSION_MAX_ALLOWED >= 160000
#define SKGPU_GRAPHITE_METAL_SDK_VERSION 300
#elif __IPHONE_OS_VERSION_MAX_ALLOWED >= 150000 || __TV_OS_VERSION_MAX_ALLOWED >= 150000
#define SKGPU_GRAPHITE_METAL_SDK_VERSION 240
#elif __IPHONE_OS_VERSION_MAX_ALLOWED >= 140000 || __TV_OS_VERSION_MAX_ALLOWED >= 140000
#define SKGPU_GRAPHITE_METAL_SDK_VERSION 230
#else
#error Must use at least 14.00 SDK to build Metal backend for iOS
#endif
#endif

#import <Metal/Metal.h>

namespace skgpu {
class ShaderErrorHandler;
}

namespace skgpu::graphite {

class MtlSharedContext;
enum class TextureFormat : uint8_t;

sk_cfp<id<MTLLibrary>> MtlCompileShaderLibrary(const MtlSharedContext* sharedContext,
                                               std::string_view label,
                                               std::string_view msl,
                                               ShaderErrorHandler* errorHandler);

TextureFormat MTLPixelFormatToTextureFormat(MTLPixelFormat);
MTLPixelFormat TextureFormatToMTLPixelFormat(TextureFormat);

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_MtlGraphiteTypesPriv_DEFINED
