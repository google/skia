/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnGraphiteUtilsPriv_DEFINED
#define skgpu_graphite_DawnGraphiteUtilsPriv_DEFINED

#include "include/core/SkImageInfo.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/dawn/DawnGraphiteTypesPriv.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/ir/SkSLProgram.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

namespace SkSL {
class Compiler;
struct ProgramSettings;
}

namespace skgpu {
class ShaderErrorHandler;
}

namespace skgpu::graphite {
class DawnSharedContext;

bool DawnFormatIsDepthOrStencil(wgpu::TextureFormat);
bool DawnFormatIsDepth(wgpu::TextureFormat);
bool DawnFormatIsStencil(wgpu::TextureFormat);

wgpu::TextureFormat DawnDepthStencilFlagsToFormat(SkEnumBitMask<DepthStencilFlags>);

bool DawnCompileWGSLShaderModule(const DawnSharedContext* sharedContext,
                                 const char* label,
                                 const std::string& wgsl,
                                 wgpu::ShaderModule* module,
                                 ShaderErrorHandler*);

#if !defined(__EMSCRIPTEN__)
namespace ycbcrUtils {

bool DawnDescriptorIsValid(const wgpu::YCbCrVkDescriptor&);

bool DawnDescriptorUsesExternalFormat(const wgpu::YCbCrVkDescriptor&);

static constexpr int kUsesExternalFormatBits  = 1;
static constexpr int kYcbcrModelBits          = 3;
static constexpr int kYcbcrRangeBits          = 1;
static constexpr int kXChromaOffsetBits       = 1;
static constexpr int kYChromaOffsetBits       = 1;
// wgpu::FilterMode contains Undefined/Nearest/Linear entries (Linear is 2).
static constexpr int kChromaFilterBits        = 2;
static constexpr int kForceExplicitReconBits  = 1;
static constexpr int kComponentBits           = 3;

static constexpr int kUsesExternalFormatShift = 0;
static constexpr int kYcbcrModelShift         = kUsesExternalFormatShift + kUsesExternalFormatBits;
static constexpr int kYcbcrRangeShift         = kYcbcrModelShift         + kYcbcrModelBits;
static constexpr int kXChromaOffsetShift      = kYcbcrRangeShift         + kYcbcrRangeBits;
static constexpr int kYChromaOffsetShift      = kXChromaOffsetShift      + kXChromaOffsetBits;
static constexpr int kChromaFilterShift       = kYChromaOffsetShift      + kYChromaOffsetBits;
static constexpr int kForceExplicitReconShift = kChromaFilterShift       + kChromaFilterBits;
static constexpr int kComponentRShift         = kForceExplicitReconShift + kForceExplicitReconBits;
static constexpr int kComponentGShift         = kComponentRShift         + kComponentBits;
static constexpr int kComponentBShift         = kComponentGShift         + kComponentBits;
static constexpr int kComponentAShift         = kComponentBShift         + kComponentBits;

static constexpr uint32_t kUseExternalFormatMask =
        ((1 << kUsesExternalFormatBits) - 1) << kUsesExternalFormatShift;
static constexpr uint32_t kYcbcrModelMask =
        ((1 << kYcbcrModelBits) - 1) << kYcbcrModelShift;
static constexpr uint32_t kYcbcrRangeMask =
        ((1 << kYcbcrRangeBits) - 1) << kYcbcrRangeShift;
static constexpr uint32_t kXChromaOffsetMask =
        ((1 << kXChromaOffsetBits) - 1) << kXChromaOffsetShift;
static constexpr uint32_t kYChromaOffsetMask =
        ((1 << kYChromaOffsetBits) - 1) << kYChromaOffsetShift;
static constexpr uint32_t kChromaFilterMask =
        ((1 << kChromaFilterBits) - 1) << kChromaFilterShift;
static constexpr uint32_t kForceExplicitReconMask =
        ((1 << kForceExplicitReconBits) - 1) << kForceExplicitReconShift;
static constexpr uint32_t kComponentRMask = ((1 << kComponentBits) - 1) << kComponentRShift;
static constexpr uint32_t kComponentBMask = ((1 << kComponentBits) - 1) << kComponentBShift;
static constexpr uint32_t kComponentGMask = ((1 << kComponentBits) - 1) << kComponentGShift;
static constexpr uint32_t kComponentAMask = ((1 << kComponentBits) - 1) << kComponentAShift;
} // namespace ycbcrUtils
#endif // !defined(__EMSCRIPTEN__)

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnGraphiteUtilsPriv_DEFINED
