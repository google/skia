/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_WGSLCODEGENERATOR
#define SKSL_WGSLCODEGENERATOR

#include "include/core/SkSpan.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkEnumBitMask.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLMemoryLayout.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/codegen/SkSLCodeGenerator.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace SkSL {

class AnyConstructor;
class BinaryExpression;
class Block;
class Context;
class ConstructorCompound;
class ConstructorDiagonalMatrix;
class ConstructorMatrixResize;
class DoStatement;
class Expression;
struct Field;
class FieldAccess;
class ForStatement;
class FunctionCall;
class FunctionDeclaration;
class FunctionDefinition;
class GlobalVarDeclaration;
class IfStatement;
class IndexExpression;
class InterfaceBlock;
enum IntrinsicKind : int8_t;
struct Layout;
class Literal;
class ModifiersDeclaration;
class OutputStream;
class PostfixExpression;
class PrefixExpression;
struct Program;
class ProgramElement;
class ReturnStatement;
class Statement;
class StructDefinition;
class SwitchCase;
class SwitchStatement;
class Swizzle;
class TernaryExpression;
class Type;
class VarDeclaration;
class Variable;
class VariableReference;

// Represents a function's dependencies that are not accessible in global scope. For instance,
// pipeline stage input and output parameters must be passed in as an argument.
//
// This is a bitmask enum. (It would be inside `class WGSLCodeGenerator`, but this leads to build
// errors in MSVC.)
enum class WGSLFunctionDependency : uint8_t {
    kNone = 0,
    kPipelineInputs  = 1 << 0,
    kPipelineOutputs = 1 << 1,
};
using WGSLFunctionDependencies = SkEnumBitMask<WGSLFunctionDependency>;

}  // namespace SkSL

SK_MAKE_BITMASK_OPS(SkSL::WGSLFunctionDependency)

namespace SkSL {

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
        kLastFragColor,  // input
        kFrontFacing,    // input
        kSampleIndex,    // input
        kFragDepth,      // output
        kSampleMaskIn,   // input
        kSampleMask,     // output

        // Compute stage:
        kLocalInvocationId,     // input
        kLocalInvocationIndex,  // input
        kGlobalInvocationId,    // input
        kWorkgroupId,           // input
        kNumWorkgroups,         // input
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
        using DepsMap = skia_private::THashMap<const FunctionDeclaration*,
                                               WGSLFunctionDependencies>;

        // Mappings used to synthesize function parameters according to dependencies on pipeline
        // input/output variables.
        DepsMap fDependencies;

        // These flags track extensions that will need to be enabled.
        bool fPixelLocalExtension = false;
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

    // Helpers to declare a pipeline stage IO parameter declaration.
    void writePipelineIODeclaration(const Layout& layout,
                                    const Type& type,
                                    std::string_view name,
                                    Delimiter delimiter);
    void writeUserDefinedIODecl(const Layout& layout,
                                const Type& type,
                                std::string_view name,
                                Delimiter delimiter);
    void writeBuiltinIODecl(const Type& type,
                            std::string_view name,
                            Builtin builtin,
                            Delimiter delimiter);
    void writeVariableDecl(const Layout& layout,
                           const Type& type,
                           std::string_view name,
                           Delimiter delimiter);

    // Write a function definition.
    void writeFunction(const FunctionDefinition& f);
    void writeFunctionDeclaration(const FunctionDeclaration& f,
                                  SkSpan<const bool> paramNeedsDedicatedStorage);

    // Write the program entry point.
    void writeEntryPoint(const FunctionDefinition& f);

    // Writers for supported statement types.
    void writeStatement(const Statement& s);
    void writeStatements(const StatementArray& statements);
    void writeBlock(const Block& b);
    void writeDoStatement(const DoStatement& expr);
    void writeExpressionStatement(const Expression& expr);
    void writeForStatement(const ForStatement& s);
    void writeIfStatement(const IfStatement& s);
    void writeReturnStatement(const ReturnStatement& s);
    void writeSwitchStatement(const SwitchStatement& s);
    void writeSwitchCases(SkSpan<const SwitchCase* const> cases);
    void writeEmulatedSwitchFallthroughCases(SkSpan<const SwitchCase* const> cases,
                                             std::string_view switchValue);
    void writeSwitchCaseList(SkSpan<const SwitchCase* const> cases);
    void writeVarDeclaration(const VarDeclaration& varDecl);

    // Synthesizes an LValue for an expression.
    class LValue;
    class PointerLValue;
    class SwizzleLValue;
    class VectorComponentLValue;
    std::unique_ptr<LValue> makeLValue(const Expression& e);

    std::string variableReferenceNameForLValue(const VariableReference& r);
    std::string variablePrefix(const Variable& v);

    bool binaryOpNeedsComponentwiseMatrixPolyfill(const Type& left, const Type& right, Operator op);

    // Writers for expressions. These return the final expression text as a string, and emit any
    // necessary setup code directly into the program as necessary. The returned expression may be
    // a `let`-alias that cannot be assigned-into; use `makeLValue` for an assignable expression.
    std::string assembleExpression(const Expression& e, Precedence parentPrecedence);
    std::string assembleBinaryExpression(const BinaryExpression& b, Precedence parentPrecedence);
    std::string assembleBinaryExpression(const Expression& left,
                                         Operator op,
                                         const Expression& right,
                                         const Type& resultType,
                                         Precedence parentPrecedence);
    std::string assembleFieldAccess(const FieldAccess& f);
    std::string assembleFunctionCall(const FunctionCall& call, Precedence parentPrecedence);
    std::string assembleIndexExpression(const IndexExpression& i);
    std::string assembleLiteral(const Literal& l);
    std::string assemblePostfixExpression(const PostfixExpression& p, Precedence parentPrecedence);
    std::string assemblePrefixExpression(const PrefixExpression& p, Precedence parentPrecedence);
    std::string assembleSwizzle(const Swizzle& swizzle);
    std::string assembleTernaryExpression(const TernaryExpression& t, Precedence parentPrecedence);
    std::string assembleVariableReference(const VariableReference& r);
    std::string assembleName(std::string_view name);

    std::string assembleIncrementExpr(const Type& type);

    // Intrinsic helper functions.
    std::string assembleIntrinsicCall(const FunctionCall& call,
                                      IntrinsicKind kind,
                                      Precedence parentPrecedence);
    std::string assembleSimpleIntrinsic(std::string_view intrinsicName, const FunctionCall& call);
    std::string assembleUnaryOpIntrinsic(Operator op,
                                         const FunctionCall& call,
                                         Precedence parentPrecedence);
    std::string assembleBinaryOpIntrinsic(Operator op,
                                          const FunctionCall& call,
                                          Precedence parentPrecedence);
    std::string assembleVectorizedIntrinsic(std::string_view intrinsicName,
                                            const FunctionCall& call);
    std::string assemblePartialSampleCall(std::string_view functionName,
                                          const Expression& sampler,
                                          const Expression& coords);
    std::string assembleInversePolyfill(const FunctionCall& call);
    std::string assembleComponentwiseMatrixBinary(const Type& leftType,
                                                  const Type& rightType,
                                                  const std::string& left,
                                                  const std::string& right,
                                                  Operator op);

    // Constructor expressions
    std::string assembleAnyConstructor(const AnyConstructor& c);
    std::string assembleConstructorCompound(const ConstructorCompound& c);
    std::string assembleConstructorCompoundVector(const ConstructorCompound& c);
    std::string assembleConstructorCompoundMatrix(const ConstructorCompound& c);
    std::string assembleConstructorDiagonalMatrix(const ConstructorDiagonalMatrix& c);
    std::string assembleConstructorMatrixResize(const ConstructorMatrixResize& ctor);

    // Synthesized helper functions for comparison operators that are not supported by WGSL.
    std::string assembleEqualityExpression(const Type& left,
                                           const std::string& leftName,
                                           const Type& right,
                                           const std::string& rightName,
                                           Operator op,
                                           Precedence parentPrecedence);
    std::string assembleEqualityExpression(const Expression& left,
                                           const Expression& right,
                                           Operator op,
                                           Precedence parentPrecedence);

    // Writes a scratch variable into the program and returns its name (e.g. `_skTemp123`).
    std::string writeScratchVar(const Type& type, const std::string& value = "");

    // Writes a scratch let-variable into the program, gives it the value of `expr`, and returns its
    // name (e.g. `_skTemp123`).
    std::string writeScratchLet(const std::string& expr);
    std::string writeScratchLet(const Expression& expr, Precedence parentPrecedence);

    // Converts `expr` into a string and returns a scratch let-variable associated with the
    // expression. Compile-time constants and plain variable references will return the expression
    // directly and omit the let-variable.
    std::string writeNontrivialScratchLet(const Expression& expr, Precedence parentPrecedence);

    // Generic recursive ProgramElement visitor.
    void writeProgramElement(const ProgramElement& e);
    void writeGlobalVarDeclaration(const GlobalVarDeclaration& d);
    void writeStructDefinition(const StructDefinition& s);
    void writeModifiersDeclaration(const ModifiersDeclaration&);

    // Writes the WGSL struct fields for SkSL structs and interface blocks. Enforces WGSL address
    // space layout constraints
    // (https://www.w3.org/TR/WGSL/#address-space-layout-constraints) if a `layout` is
    // provided. A struct that does not need to be host-shareable does not require a `layout`.
    void writeFields(SkSpan<const Field> fields, const MemoryLayout* memoryLayout = nullptr);

    // We bundle uniforms, and all varying pipeline stage inputs and outputs, into separate structs.
    bool needsStageInputStruct() const;
    void writeStageInputStruct();
    bool needsStageOutputStruct() const;
    void writeStageOutputStruct();
    void writeUniformsAndBuffers();
    void prepareUniformPolyfillsForInterfaceBlock(const InterfaceBlock* interfaceBlock,
                                                  std::string_view instanceName,
                                                  MemoryLayout::Standard nativeLayout);
    void writeEnables();
    void writeUniformPolyfills();

    void writeTextureOrSampler(const Variable& var,
                               int bindingLocation,
                               std::string_view suffix,
                               std::string_view wgslType);

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
    std::string functionDependencyArgs(const FunctionDeclaration&);
    bool writeFunctionDependencyParams(const FunctionDeclaration&);

    // Code in the header appears before the main body of code.
    StringStream fHeader;

    // We assign unique names to anonymous interface blocks based on the type.
    skia_private::THashMap<const Type*, std::string> fInterfaceBlockNameMap;

    // Stores the functions which use stage inputs/outputs as well as required WGSL extensions.
    ProgramRequirements fRequirements;
    skia_private::TArray<const Variable*> fPipelineInputs;
    skia_private::TArray<const Variable*> fPipelineOutputs;

    // These fields track whether we have written the polyfill for `inverse()` for a given matrix
    // type.
    bool fWrittenInverse2 = false;
    bool fWrittenInverse3 = false;
    bool fWrittenInverse4 = false;

    // These fields control uniform polyfill support in cases where WGSL and std140 disagree.
    // In std140 layout, matrices need to be represented as arrays of @size(16)-aligned vectors, and
    // array elements are wrapped in a struct containing a single @size(16)-aligned element. Arrays
    // of matrices combine both wrappers. These wrapper structs are unpacked into natively-typed
    // globals at the shader entrypoint.
    struct FieldPolyfillInfo {
        const InterfaceBlock* fInterfaceBlock;
        std::string fReplacementName;
        bool fIsArray = false;
        bool fIsMatrix = false;
        bool fWasAccessed = false;
    };
    using FieldPolyfillMap = skia_private::THashMap<const Field*, FieldPolyfillInfo>;
    FieldPolyfillMap fFieldPolyfillMap;

    // Output processing state.
    int fIndentation = 0;
    bool fAtLineStart = false;
    bool fHasUnconditionalReturn = false;
    bool fAtFunctionScope = false;
    int fConditionalScopeDepth = 0;
    int fLocalSizeX = 1;
    int fLocalSizeY = 1;
    int fLocalSizeZ = 1;

    int fScratchCount = 0;
};

}  // namespace SkSL

#endif  // SKSL_WGSLCODEGENERATOR
