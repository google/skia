/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GpuTypesPriv_DEFINED
#define skgpu_GpuTypesPriv_DEFINED

#include "include/core/SkColorType.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/private/base/SkAssert.h"

#include <chrono>

namespace skgpu {

// The old libstdc++ uses the draft name "monotonic_clock" rather than "steady_clock". This might
// not actually be monotonic, depending on how libstdc++ was built. However, this is only currently
// used for idle resource purging so it shouldn't cause a correctness problem.
#if defined(__GLIBCXX__) && (__GLIBCXX__ < 20130000)
using StdSteadyClock = std::chrono::monotonic_clock;
#else
using StdSteadyClock = std::chrono::steady_clock;
#endif

// In general we try to not mix CompressionType and ColorType, but currently SkImage still requires
// an SkColorType even for CompressedTypes so we need some conversion.
static constexpr SkColorType CompressionTypeToSkColorType(SkTextureCompressionType compression) {
    switch (compression) {
        case SkTextureCompressionType::kNone:            return kUnknown_SkColorType;
        case SkTextureCompressionType::kETC2_RGB8_UNORM: return kRGB_888x_SkColorType;
        case SkTextureCompressionType::kBC1_RGB8_UNORM:  return kRGB_888x_SkColorType;
        case SkTextureCompressionType::kBC1_RGBA8_UNORM: return kRGBA_8888_SkColorType;
    }

    SkUNREACHABLE;
}

static constexpr const char* CompressionTypeToStr(SkTextureCompressionType compression) {
    switch (compression) {
        case SkTextureCompressionType::kNone:            return "kNone";
        case SkTextureCompressionType::kETC2_RGB8_UNORM: return "kETC2_RGB8_UNORM";
        case SkTextureCompressionType::kBC1_RGB8_UNORM:  return "kBC1_RGB8_UNORM";
        case SkTextureCompressionType::kBC1_RGBA8_UNORM: return "kBC1_RGBA8_UNORM";
    }
    SkUNREACHABLE;
}

} // namespace skgpu

#endif // skgpu_GpuTypesPriv_DEFINED
