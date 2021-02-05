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

    /*
     * Processes 'program' for use in a GrFragmentProcessor, or other context that wants SkSL-like
     * code as input. To support fragment processor usage, there are callbacks that allow elements
     * to be declared programmatically and to rename those elements (mangling to avoid collisions).
     * 
     * - Any reference to the main coords builtin variable will be replaced with 'sampleCoords'.
     * - Each uniform variable declaration triggers a call to 'declareUniform', which should emit
     *   the declaration, and return the (possibly different) name to use for the variable.
     * - Each function definition triggers a call to 'defineFunction', which should emit the
     *   definition, and return the (possibly different) name to use for calls to that function.
     * - Each invocation of sample() triggers a call to 'sampleChild' or 'sampleChildWithMatrix',
     *   which should return the full text of the call expression.
     */
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
