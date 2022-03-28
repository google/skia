/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_WGSLCODEGENERATOR
#define SKSL_WGSLCODEGENERATOR

#include "src/sksl/codegen/SkSLCodeGenerator.h"

namespace SkSL {

class FunctionDeclaration;
class FunctionDefinition;
class ProgramElement;

/**
 * Convert a Program into WGSL code.
 */
class WGSLCodeGenerator : public CodeGenerator {
public:
    // See https://www.w3.org/TR/WGSL/#builtin-values
    enum class Builtin {
        // Vertex stage:
        kVertexIndex,    // input
        kInstanceIndex,  // input
        kPosition,       // output, fragment stage input

        // Fragment stage:
        kFrontFacing,  // input
        kSampleIndex,  // input
        kFragDepth,    // output
        kSampleMask,   // input, output

        // Compute stage:
        kLocalInvocationId,     // input
        kLocalInvocationIndex,  // input
        kGlobalInvocationId,    // input
        kWorkgroupId,           // input
        kNumWorkgroups,         // input
    };

    WGSLCodeGenerator(const Context* context, const Program* program, OutputStream* out)
            : INHERITED(context, program, out) {}

    bool generateCode() override;

private:
    using INHERITED = CodeGenerator;

    // Write output content while correctly handling indentation.
    void write(std::string_view s);
    void writeLine(std::string_view s = std::string_view());
    void finishLine();
    void writeName(std::string_view name);

    // Helpers to declare a pipeline stage IO parameter declaration.
    void writePipelineIODeclaration(Modifiers modifiers, const Type& type, std::string_view name);
    void writeUserDefinedVariableDecl(const Type& type, std::string_view name, int location);
    void writeBuiltinVariableDecl(const Type& type, std::string_view name, Builtin kind);

    // Generic recursive ProgramElement visitor.
    void writeProgramElement(const ProgramElement& e);

    // We bundle all varying pipeline stage inputs and outputs in a struct.
    void writeStageInputStruct();
    void writeStageOutputStruct();

    SkTHashSet<std::string_view> fReservedWords;

    // Output processing state.
    int fIndentation = 0;
    bool fAtLineStart = false;
};

}  // namespace SkSL

#endif  // SKSL_WGSLCODEGENERATOR
