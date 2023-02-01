/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_RASTERPIPELINECODEGENERATOR
#define SKSL_RASTERPIPELINECODEGENERATOR

#include "include/core/SkTypes.h"
#include <memory>

namespace SkSL {

class FunctionDefinition;
struct Program;
class SkRPDebugTrace;
namespace RP { class Program; }

// Convert 'function' to Raster Pipeline stages, for use by blends, shaders, and color filters.
// The arguments to the function are passed in registers:
//   -- coordinates in src.rg for shaders
//   -- color in src.rgba for color filters
//   -- src/dst in src.rgba and dst.rgba for blenders
std::unique_ptr<RP::Program> MakeRasterPipelineProgram(const Program& program,
                                                       const FunctionDefinition& function,
                                                       SkRPDebugTrace* debugTrace = nullptr);

}  // namespace SkSL

#endif
