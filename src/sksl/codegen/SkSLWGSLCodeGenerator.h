/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_WGSLCODEGENERATOR
#define SKSL_WGSLCODEGENERATOR

#include "include/private/SkSLDefines.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/codegen/SkSLCodeGenerator.h"
#include "src/sksl/ir/SkSLType.h"

#include <cstdint>
#include <initializer_list>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

template <typename T> class SkSpan;

namespace sknonstd {
template <typename T> struct is_bitmask_enum;
}  // namespace sknonstd

namespace SkSL {

class AnyConstructor;
class BinaryExpression;
class Block;
class Context;
class ConstructorCompound;
class ConstructorDiagonalMatrix;
class Expression;
class ExpressionStatement;
class FieldAccess;
class FunctionCall;
class FunctionDeclaration;
class FunctionDefinition;
class GlobalVarDeclaration;
class IfStatement;
class Literal;
class MemoryLayout;
class OutputStream;
class Position;
class ProgramElement;
class ReturnStatement;
class Statement;
class StructDefinition;
class TernaryExpression;
class VarDeclaration;
class Variable;
class VariableReference;
enum class OperatorPrecedence : uint8_t;
struct IndexExpression;
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
            : INHERITED(context, program, out)
            , fReservedWords({"array",
                              "FSIn",
                              "FSOut",
                              "_globalUniforms",
                              "_GlobalUniforms",
                              "_return",
                              "_stageIn",
                              "_stageOut",
                              "VSIn",
                              "VSOut"}) {}

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
    void writeVariableDecl(const Type& type, std::string_view name, Delimiter delimiter);

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
    void writeIfStatement(const IfStatement& s);
    void writeReturnStatement(const ReturnStatement& s);
    void writeVarDeclaration(const VarDeclaration& varDecl);

    // Writers for expressions.
    void writeExpression(const Expression& e, Precedence parentPrecedence);
    void writeBinaryExpression(const BinaryExpression& b, Precedence parentPrecedence);
    void writeFieldAccess(const FieldAccess& f);
    void writeFunctionCall(const FunctionCall&);
    void writeIndexExpression(const IndexExpression& i);
    void writeLiteral(const Literal& l);
    void writeSwizzle(const Swizzle& swizzle);
    void writeTernaryExpression(const TernaryExpression& t, Precedence parentPrecedence);
    void writeVariableReference(const VariableReference& r);

    // Constructor expressions
    void writeAnyConstructor(const AnyConstructor& c, Precedence parentPrecedence);
    void writeConstructorCompound(const ConstructorCompound& c, Precedence parentPrecedence);
    void writeConstructorCompoundVector(const ConstructorCompound& c, Precedence parentPrecedence);
    void writeConstructorDiagonalMatrix(const ConstructorDiagonalMatrix& c,
                                        Precedence parentPrecedence);

    // Synthesized helper functions for comparison operators that are not supported by WGSL.
    void writeMatrixEquality(const Expression& left, const Expression& right);

    // Generic recursive ProgramElement visitor.
    void writeProgramElement(const ProgramElement& e);
    void writeGlobalVarDeclaration(const GlobalVarDeclaration& d);
    void writeStructDefinition(const StructDefinition& s);

    // Writes the WGSL struct fields for SkSL structs and interface blocks. Enforces WGSL address
    // space layout constraints
    // (https://www.w3.org/TR/WGSL/#address-space-layout-constraints) if a `layout` is
    // provided. A struct that does not need to be host-shareable does not require a `layout`.
    void writeFields(SkSpan<const Type::Field> fields,
                     Position parentPos,
                     const MemoryLayout* layout = nullptr);

    // We bundle all varying pipeline stage inputs and outputs in a struct.
    void writeStageInputStruct();
    void writeStageOutputStruct();

    // Writes all top-level non-opaque global uniform declarations (i.e. not part of an interface
    // block) into a single uniform block binding.
    //
    // In complete fragment/vertex/compute programs, uniforms will be declared only as interface
    // blocks and global opaque types (like textures and samplers) which we expect to be declared
    // with a unique binding and descriptor set index. However, test files that are declared as RTE
    // programs may contain OpenGL-style global uniform declarations with no clear binding index to
    // use for the containing synthesized block.
    //
    // Since we are handling these variables only to generate gold files from RTEs and never run
    // them, we always declare them at the default bind group and binding index.
    void writeNonBlockUniformsForTests();

    // For a given function declaration, writes out any implicitly required pipeline stage arguments
    // based on the function's pre-determined dependencies. These are expected to be written out as
    // the first parameters for a function that requires them. Returns true if any arguments were
    // written.
    bool writeFunctionDependencyArgs(const FunctionDeclaration&);
    bool writeFunctionDependencyParams(const FunctionDeclaration&);

    // Generate an out-parameter helper function for the given call and return its name.
    std::string writeOutParamHelper(const FunctionCall&,
                                    const ExpressionArray& args,
                                    const SkTArray<VariableReference*>& outVars);

    // Stores the disallowed identifier names.
    SkTHashSet<std::string_view> fReservedWords;
    ProgramRequirements fRequirements;
    int fPipelineInputCount = 0;
    bool fDeclaredUniformsStruct = false;

    // Out-parameters to functions are declared as pointers. While we process the arguments to a
    // out-parameter helper function, we need to temporarily track that they are re-declared as
    // pointer-parameters in the helper, so that expression-tree processing can know to correctly
    // dereference them when the variable is referenced. The contents of this set are expected to
    // be uniquely scoped for each out-param helper and will be cleared every time a new out-param
    // helper function has been emitted.
    SkTHashSet<const Variable*> fOutParamArgVars;

    // Output processing state.
    int fIndentation = 0;
    bool fAtLineStart = false;

    int fSwizzleHelperCount = 0;
    StringStream fExtraFunctions;      // all internally synthesized helpers are written here
    SkTHashSet<std::string> fHelpers;  // all synthesized helper functions, by name
};

}  // namespace SkSL

namespace sknonstd {
template <>
struct is_bitmask_enum<SkSL::WGSLCodeGenerator::FunctionDependencies> : std::true_type {};
}  // namespace sknonstd

#endif  // SKSL_WGSLCODEGENERATOR
