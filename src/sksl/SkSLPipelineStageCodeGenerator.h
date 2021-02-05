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
struct Program;
class VarDeclaration;

namespace PipelineStage {
    using DeclareUniformFn = std::function<String(const VarDeclaration*)>;
    using DefineFunctionFn = std::function<String(const FunctionDeclaration*, String)>;
    using SampleChildFn  = std::function<String(int /*index*/, String /*coords -or- matrix*/)>;

    void ConvertProgram(const Program& program,
                        const char* sampleCoords,
                        DeclareUniformFn declareUniform,
                        DefineFunctionFn defineFunction,
                        SampleChildFn sampleChild,
                        SampleChildFn sampleChildWithMatrix);
}  // namespace PipelineStage

}  // namespace SkSL

#endif

#endif
