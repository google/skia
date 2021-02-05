/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PIPELINESTAGECODEGENERATOR
#define SKSL_PIPELINESTAGECODEGENERATOR

#include "src/sksl/SkSLString.h"

#include <functional>

// TODO: This can now be used in SKSL_STANDALONE, with shim code for all of the callbacks.
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

namespace SkSL {

class FunctionDeclaration;
class FunctionDefinition;
struct Program;
class Variable;

namespace PipelineStage {
    using UniformNameFn  = std::function<String(const Variable*)>;
    using FunctionNameFn = std::function<String(const FunctionDeclaration*)>;
    using SampleChildFn  = std::function<String(int /*index*/, String /*coords -or- matrix*/)>;

    String ConvertFunction(const Program& program,
                           const FunctionDefinition& function,
                           const char* sampleCoords,
                           UniformNameFn uniformName,
                           FunctionNameFn functionName,
                           SampleChildFn sampleChild,
                           SampleChildFn sampleChildWithMatrix);
}  // namespace PipelineStage

}  // namespace SkSL

#endif

#endif
