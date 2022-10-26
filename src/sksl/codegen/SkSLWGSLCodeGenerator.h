/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_WGSLCODEGENERATOR
#define SKSL_WGSLCODEGENERATOR

#include "include/private/SkSLDefines.h"
#include "include/private/SkTHash.h"
#include "src/sksl/codegen/SkSLCodeGenerator.h"

#include <cstdint>
#include <string_view>
#include <type_traits>
#include <utility>

namespace sknonstd {
template <typename T> struct is_bitmask_enum;
}  // namespace sknonstd

namespace SkSL {

class AnyConstructor;
class BinaryExpression;
class Block;
class Context;
class ConstructorCompound;
class Expression;
class ExpressionStatement;
class FieldAccess;
class FunctionDeclaration;
class FunctionDefinition;
class Literal;
class OutputStream;
class ProgramElement;
class ReturnStatement;
class Statement;
class Type;
class VarDeclaration;
class VariableReference;
enum class OperatorPrecedence : uint8_t;
struct Modifiers;
struct Program;
struct Swizzle;

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

    // Represents a function's dependencies that are not accessible in global scope. For instance,
    // pipeline stage input and output parameters must be passed in as an argument.
    //
    // This is a bitmask enum.
    enum class FunctionDependencies : uint8_t {
        kNone = 0,
        kPipelineInputs = 1,
        kPipelineOutputs = 2,
    };

    // Variable declarations can be terminated by:
    //   - comma (","), e.g. in struct member declarations or function parameters
    //   - semicolon (";"), e.g. in function scope variables
    // A "none" option is provided to skip the delimiter when not needed, e.g. at the end of a list
    // of declarations.
    enum class Delimiter {
        kComma,
        kSemicolon,
        kNone,
    };

    struct ProgramRequirements {
        using DepsMap = SkTHashMap<const FunctionDeclaration*, FunctionDependencies>;

        ProgramRequirements() = default;
        ProgramRequirements(DepsMap dependencies, bool mainNeedsCoordsArgument)
                : dependencies(std::move(dependencies))
                , mainNeedsCoordsArgument(mainNeedsCoordsArgument) {}

        // Mappings used to synthesize function parameters according to dependencies on pipeline
        // input/output variables.
        DepsMap dependencies;

        // True, if the main function takes a coordinate parameter. This is used to ensure that
        // sk_FragCoord is declared as part of pipeline inputs.
        bool mainNeedsCoordsArgument;
    };

    WGSLCodeGenerator(const Context* context, const Program* program, OutputStream* out)
            : INHERITED(context, program, out) {}

    bool generateCode() override;

private:
    using INHERITED = CodeGenerator;
    using Precedence = OperatorPrecedence;

    // Called by generateCode() as the first step.
    void preprocessProgram();

    // Write output content while correctly handling indentation.
    void write(std::string_view s);
    void writeLine(std::string_view s = std::string_view());
    void finishLine();
    void writeName(std::string_view name);

    // Helpers to declare a pipeline stage IO parameter declaration.
    void writePipelineIODeclaration(Modifiers modifiers,
                                    const Type& type,
                                    std::string_view name,
                                    Delimiter delimiter);
    void writeUserDefinedIODecl(const Type& type,
                                std::string_view name,
                                int location,
                                Delimiter delimiter);
    void writeBuiltinIODecl(const Type& type,
                            std::string_view name,
                            Builtin builtin,
                            Delimiter delimiter);

    // Write a function definition.
    void writeFunction(const FunctionDefinition& f);
    void writeFunctionDeclaration(const FunctionDeclaration& f);

    // Write the program entry point.
    void writeEntryPoint(const FunctionDefinition& f);

    // Writers for supported statement types.
    void writeStatement(const Statement& s);
    void writeStatements(const StatementArray& statements);
    void writeBlock(const Block& b);
    void writeExpressionStatement(const ExpressionStatement& s);
    void writeReturnStatement(const ReturnStatement& s);
    void writeVarDeclaration(const VarDeclaration& varDecl);

    // Writers for expressions.
    void writeExpression(const Expression& e, Precedence parentPrecedence);
    void writeBinaryExpression(const BinaryExpression& b, Precedence parentPrecedence);
    void writeFieldAccess(const FieldAccess& f);
    void writeLiteral(const Literal& l);
    void writeSwizzle(const Swizzle& swizzle);
    void writeVariableReference(const VariableReference& r);

    // Constructor expressions
    void writeAnyConstructor(const AnyConstructor& c, Precedence parentPrecedence);
    void writeConstructorCompound(const ConstructorCompound& c, Precedence parentPrecedence);
    void writeConstructorCompoundVector(const ConstructorCompound& c, Precedence parentPrecedence);

    // Generic recursive ProgramElement visitor.
    void writeProgramElement(const ProgramElement& e);

    // We bundle all varying pipeline stage inputs and outputs in a struct.
    void writeStageInputStruct();
    void writeStageOutputStruct();

    // Stores the disallowed identifier names.
    // TODO(skia:13092): populate this
    SkTHashSet<std::string_view> fReservedWords;
    ProgramRequirements fRequirements;
    int fPipelineInputCount = 0;

    // Output processing state.
    int fIndentation = 0;
    bool fAtLineStart = false;
};

}  // namespace SkSL

namespace sknonstd {
template <>
struct is_bitmask_enum<SkSL::WGSLCodeGenerator::FunctionDependencies> : std::true_type {};
}  // namespace sknonstd

#endif  // SKSL_WGSLCODEGENERATOR
