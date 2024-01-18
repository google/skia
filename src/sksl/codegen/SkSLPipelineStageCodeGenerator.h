/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PIPELINESTAGECODEGENERATOR
#define SKSL_PIPELINESTAGECODEGENERATOR

#include "include/core/SkTypes.h"

#include <string>

namespace SkSL {

struct Program;
class VarDeclaration;

namespace PipelineStage {
    class Callbacks {
    public:
        virtual ~Callbacks() = default;

        virtual std::string getMainName() { return "main"; }
        virtual std::string getMangledName(const char* name) { return name; }
        virtual void   defineFunction(const char* declaration, const char* body, bool isMain) = 0;
        virtual void   declareFunction(const char* declaration) = 0;
        virtual void   defineStruct(const char* definition) = 0;
        virtual void   declareGlobal(const char* declaration) = 0;

        virtual std::string declareUniform(const VarDeclaration*) = 0;
        virtual std::string sampleShader(int index, std::string coords) = 0;
        virtual std::string sampleColorFilter(int index, std::string color) = 0;
        virtual std::string sampleBlender(int index, std::string src, std::string dst) = 0;

        virtual std::string toLinearSrgb(std::string color) = 0;
        virtual std::string fromLinearSrgb(std::string color) = 0;
    };

    /*
     * Processes 'program' for use in a GrFragmentProcessor, or other context that wants SkSL-like
     * code as input. To support fragment processor usage, there are callbacks that allow elements
     * to be declared programmatically and to rename those elements (mangling to avoid collisions).
     *
     * - Any reference to the main coords builtin variable will be replaced with 'sampleCoords'.
     * - Any reference to the input color builtin variable will be replaced with 'inputColor'.
     * - Any reference to the dest color builtin variable will be replaced with 'destColor'.
     *   Dest-color is used in blend programs.
     * - Each uniform variable declaration triggers a call to 'declareUniform', which should emit
     *   the declaration, and return the (possibly different) name to use for the variable.
     * - Each function definition triggers a call to 'defineFunction', which should emit the
     *   definition, and return the (possibly different) name to use for calls to that function.
     * - Each invocation of sample() triggers a call to 'sampleChild', which should return the full
     *   text of the call expression.
     */
    void ConvertProgram(const Program& program,
                        const char* sampleCoords,
                        const char* inputColor,
                        const char* destColor,
                        Callbacks* callbacks);
}  // namespace PipelineStage

}  // namespace SkSL

#endif
