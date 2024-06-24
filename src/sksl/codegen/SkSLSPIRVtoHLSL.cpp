/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLSPIRVtoHLSL.h"

#include <spirv_hlsl.hpp>

/*
 * This translation unit serves as a bridge between Skia/SkSL and SPIRV-Cross.
 * Each library is built with a separate copy of spirv.h (or spirv.hpp), so we
 * avoid conflicts by never including both in the same cpp.
 */

namespace SkSL {

void SPIRVtoHLSL(const std::string& spirv, std::string* hlsl) {
    spirv_cross::CompilerHLSL hlslCompiler((const uint32_t*)spirv.c_str(),
                                           spirv.size() / sizeof(uint32_t));

    spirv_cross::CompilerGLSL::Options optionsGLSL;
    // Force all uninitialized variables to be 0, otherwise they will fail to compile
    // by FXC.
    optionsGLSL.force_zero_initialized_variables = true;

    spirv_cross::CompilerHLSL::Options optionsHLSL;
    optionsHLSL.shader_model = 51;
    // PointCoord and PointSize are not supported in HLSL
    optionsHLSL.point_coord_compat = true;
    optionsHLSL.point_size_compat = true;

    hlslCompiler.set_common_options(optionsGLSL);
    hlslCompiler.set_hlsl_options(optionsHLSL);
    hlsl->assign(hlslCompiler.compile());
}

}  // namespace SkSL
