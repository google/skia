/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLSPIRVtoHLSL.h"

#if defined(SK_DIRECT3D)

#include "third_party/externals/spirv-cross/spirv_hlsl.hpp"

/*
 * This translation unit serves as a bridge between Skia/SkSL and SPIRV-Cross.
 * Each library is built with a separate copy of spirv.h (or spirv.hpp), so we
 * avoid conflicts by never including both in the same cpp.
 */

namespace SkSL {

bool SPIRVtoHLSL(const String& spirv, String* hlsl) {
    spirv_cross::CompilerHLSL hlslCompiler((const uint32_t*)spirv.c_str(),
                                           spirv.size() / sizeof(uint32_t));
    hlsl->assign(hlslCompiler.compile());
    return true;
}

}

#else

namespace SkSL { bool SPIRVtoHLSL(const String&, String*) { return false; } }

#endif
