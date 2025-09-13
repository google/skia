/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlUtilsPriv_DEFINED
#define skgpu_MtlUtilsPriv_DEFINED

#import <Metal/Metal.h>

#include "include/core/SkTextureCompressionType.h"
#include "src/gpu/SkSLToBackend.h"
#include "src/sksl/codegen/SkSLMetalCodeGenerator.h"

namespace SkSL {

enum class ProgramKind : int8_t;
struct ProgramInterface;
struct ProgramSettings;
struct ShaderCaps;

}  // namespace SkSL

namespace skgpu {

class ShaderErrorHandler;

inline bool SkSLToMSL(const SkSL::ShaderCaps* caps,
                      const std::string& sksl,
                      SkSL::ProgramKind programKind,
                      const SkSL::ProgramSettings& settings,
                      SkSL::NativeShader* msl,
                      SkSL::ProgramInterface* outInterface,
                      ShaderErrorHandler* errorHandler) {
    return SkSLToBackend(caps,
                         &SkSL::ToMetal,
                         "MSL",
                         sksl,
                         programKind,
                         settings,
                         msl,
                         outInterface,
                         errorHandler);
}

bool MtlFormatIsCompressed(MTLPixelFormat);

// NOTE: All of these functions are actually only used by Ganesh. When building Graphite in
// isolation, the linker should drop them.

uint32_t MtlFormatChannels(MTLPixelFormat);

size_t MtlFormatBytesPerBlock(MTLPixelFormat);

SkTextureCompressionType MtlFormatToCompressionType(MTLPixelFormat);

const char* MtlFormatToString(MTLPixelFormat);

}  // namespace skgpu

#endif // skgpu_MtlUtilsPriv_DEFINED
