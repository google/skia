/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlUtilsPriv_DEFINED
#define skgpu_MtlUtilsPriv_DEFINED

#import <Metal/Metal.h>

#include "src/sksl/ir/SkSLProgram.h"

namespace SkSL {
    class Compiler;
    enum class ProgramKind : int8_t;
    struct ProgramSettings;
}

namespace skgpu {
class ShaderErrorHandler;

bool MtlFormatIsDepthOrStencil(MTLPixelFormat);
bool MtlFormatIsDepth(MTLPixelFormat);
bool MtlFormatIsStencil(MTLPixelFormat);
bool MtlFormatIsCompressed(MTLPixelFormat);

uint32_t MtlFormatChannels(MTLPixelFormat);

size_t MtlFormatBytesPerBlock(MTLPixelFormat);

#if defined(SK_DEBUG) || defined(GR_TEST_UTILS)
const char* MtlFormatToString(MTLPixelFormat);
#endif

#ifdef SK_BUILD_FOR_IOS
bool MtlIsAppInBackground();
#endif
} // namespace skgpu

#endif // skgpu_MtlUtilsPriv_DEFINED
