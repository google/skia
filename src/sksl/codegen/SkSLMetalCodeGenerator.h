/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_METALCODEGENERATOR
#define SKSL_METALCODEGENERATOR

#include "include/core/SkSpan.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/codegen/SkSLCodeGenerator.h"
#include "src/sksl/ir/SkSLModifierFlags.h"

#include <cstdint>
#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>

namespace SkSL {

class AnyConstructor;
class BinaryExpression;
class Block;
class ConstructorArrayCast;
class ConstructorCompound;
class ConstructorMatrixResize;
class Context;
class DoStatement;
class Expression;
class ExpressionStatement;
class Extension;
struct Field;
class FieldAccess;
class ForStatement;
class FunctionCall;
class FunctionDeclaration;
class FunctionDefinition;
class FunctionPrototype;
class IfStatement;
class IndexExpression;
class InterfaceBlock;
class Literal;
class Operator;
class OutputStream;
class Position;
class PostfixExpression;
class PrefixExpression;
class ProgramElement;
class ReturnStatement;
class Statement;
class StructDefinition;
class SwitchStatement;
class Swizzle;
class TernaryExpression;
class Type;
class VarDeclaration;
class Variable;
class VariableReference;
enum class OperatorPrecedence : uint8_t;
enum IntrinsicKind : int8_t;
struct Layout;
struct Program;

/**
 * Converts a Program into Metal code.
 */
class MetalCodeGenerator : public CodeGenerator {
public:
    MetalCodeGenerator(const Context* context, const Program* program, OutputStream* out)
    : INHERITED(context, program, out)
    , fReservedWords({"atan2", "rsqrt", "rint", "dfdx", "dfdy", "vertex", "fragment"})
    , fLineEnding("\n") {}

    bool generateCode() override;

protected:
    using Precedence = OperatorPrecedence;

    using Requirements =  int;
    static constexpr Requirements kNo_Requirements          = 0;
    static constexpr Requirements kInputs_Requirement       = 1 << 0;
    static constexpr Requirements kOutputs_Requirement      = 1 << 1;
    static constexpr Requirements kUniforms_Requirement     = 1 << 2;
    static constexpr Requirements kGlobals_Requirement      = 1 << 3;
    static constexpr Requirements kFragCoord_Requirement    = 1 << 4;
    static constexpr Requirements kSampleMaskIn_Requirement = 1 << 5;
    static constexpr Requirements kVertexID_Requirement     = 1 << 6;
    static constexpr Requirements kInstanceID_Requirement   = 1 << 7;
    static constexpr Requirements kThreadgroups_Requirement = 1 << 8;

    class GlobalStructVisitor;
    void visitGlobalStruct(GlobalStructVisitor* visitor);

    class ThreadgroupStructVisitor;
    void visitThreadgroupStruct(ThreadgroupStructVisitor* visitor);

    void write(std::string_view s);

    void writeLine(std::string_view s = std::string_view());

    void finishLine();

    void writeHeader();

    void writeSampler2DPolyfill();

    void writeUniformStruct();

    void writeInputStruct();

    void writeOutputStruct();

    void writeInterfaceBlocks();

    void writeStructDefinitions();

    void writeConstantVariables();

    void writeFields(SkSpan<const Field> fields, Position pos);

    int size(const Type* type, bool isPacked) const;

    int alignment(const Type* type, bool isPacked) const;

    void writeGlobalStruct();

    void writeGlobalInit();

    void writeThreadgroupStruct();

    void writeThreadgroupInit();

    void writePrecisionModifier();

    std::string typeName(const Type& type);

    void writeStructDefinition(const StructDefinition& s);

    void writeType(const Type& type);

    void writeExtension(const Extension& ext);

    void writeInterfaceBlock(const InterfaceBlock& intf);

    void writeFunctionRequirementParams(const FunctionDeclaration& f,
                                        const char*& separator);

    void writeFunctionRequirementArgs(const FunctionDeclaration& f, const char*& separator);

    bool writeFunctionDeclaration(const FunctionDeclaration& f);

    void writeFunction(const FunctionDefinition& f);

    void writeFunctionPrototype(const FunctionPrototype& f);

    void writeLayout(const Layout& layout);

    void writeModifiers(ModifierFlags flags);

    void writeVarInitializer(const Variable& var, const Expression& value);

    void writeName(std::string_view name);

    void writeVarDeclaration(const VarDeclaration& decl);

    void writeFragCoord();

    void writeVariableReference(const VariableReference& ref);

    void writeExpression(const Expression& expr, Precedence parentPrecedence);

    void writeMinAbsHack(Expression& absExpr, Expression& otherExpr);

    std::string getInversePolyfill(const ExpressionArray& arguments);

    std::string getBitcastIntrinsic(const Type& outType);

    std::string getTempVariable(const Type& varType);

    void writeFunctionCall(const FunctionCall& c);

    bool matrixConstructHelperIsNeeded(const ConstructorCompound& c);
    std::string getMatrixConstructHelper(const AnyConstructor& c);
    void assembleMatrixFromMatrix(const Type& sourceMatrix, int rows, int columns);
    void assembleMatrixFromExpressions(const AnyConstructor& ctor, int rows, int columns);

    void writeMatrixCompMult();

    void writeOuterProduct();

    void writeMatrixTimesEqualHelper(const Type& left, const Type& right, const Type& result);

    void writeMatrixDivisionHelpers(const Type& type);

    void writeMatrixEqualityHelpers(const Type& left, const Type& right);

    std::string getVectorFromMat2x2ConstructorHelper(const Type& matrixType);

    void writeArrayEqualityHelpers(const Type& type);

    void writeStructEqualityHelpers(const Type& type);

    void writeEqualityHelpers(const Type& leftType, const Type& rightType);

    void writeArgumentList(const ExpressionArray& arguments);

    void writeSimpleIntrinsic(const FunctionCall& c);

    bool writeIntrinsicCall(const FunctionCall& c, IntrinsicKind kind);

    void writeConstructorCompound(const ConstructorCompound& c, Precedence parentPrecedence);

    void writeConstructorCompoundVector(const ConstructorCompound& c, Precedence parentPrecedence);

    void writeConstructorCompoundMatrix(const ConstructorCompound& c, Precedence parentPrecedence);

    void writeConstructorMatrixResize(const ConstructorMatrixResize& c,
                                      Precedence parentPrecedence);

    void writeAnyConstructor(const AnyConstructor& c,
                             const char* leftBracket,
                             const char* rightBracket,
                             Precedence parentPrecedence);

    void writeCastConstructor(const AnyConstructor& c,
                              const char* leftBracket,
                              const char* rightBracket,
                              Precedence parentPrecedence);

    void writeConstructorArrayCast(const ConstructorArrayCast& c, Precedence parentPrecedence);

    void writeFieldAccess(const FieldAccess& f);

    void writeSwizzle(const Swizzle& swizzle);

    // Splats a scalar expression across a matrix of arbitrary size.
    void writeNumberAsMatrix(const Expression& expr, const Type& matrixType);

    void writeBinaryExpressionElement(const Expression& expr,
                                      Operator op,
                                      const Expression& other,
                                      Precedence precedence);

    void writeBinaryExpression(const BinaryExpression& b, Precedence parentPrecedence);

    void writeTernaryExpression(const TernaryExpression& t, Precedence parentPrecedence);

    void writeIndexExpression(const IndexExpression& expr);

    void writeIndexInnerExpression(const Expression& expr);

    void writePrefixExpression(const PrefixExpression& p, Precedence parentPrecedence);

    void writePostfixExpression(const PostfixExpression& p, Precedence parentPrecedence);

    void writeLiteral(const Literal& f);

    void writeStatement(const Statement& s);

    void writeStatements(const StatementArray& statements);

    void writeBlock(const Block& b);

    void writeIfStatement(const IfStatement& stmt);

    void writeForStatement(const ForStatement& f);

    void writeDoStatement(const DoStatement& d);

    void writeExpressionStatement(const ExpressionStatement& s);

    void writeSwitchStatement(const SwitchStatement& s);

    void writeReturnStatementFromMain();

    void writeReturnStatement(const ReturnStatement& r);

    void writeProgramElement(const ProgramElement& e);

    Requirements requirements(const FunctionDeclaration& f);

    Requirements requirements(const Statement* s);

    // For compute shader main functions, writes and initializes the _in and _out structs (the
    // instances, not the types themselves)
    void writeComputeMainInputs();

    int getUniformBinding(const Layout& layout);

    int getUniformSet(const Layout& layout);

    void writeWithIndexSubstitution(const std::function<void()>& fn);

    skia_private::THashSet<std::string_view> fReservedWords;
    skia_private::THashMap<const Type*, std::string> fInterfaceBlockNameMap;
    int fAnonInterfaceCount = 0;
    int fPaddingCount = 0;
    const char* fLineEnding;
    std::string fFunctionHeader;
    StringStream fExtraFunctions;
    StringStream fExtraFunctionPrototypes;
    int fVarCount = 0;
    int fIndentation = 0;
    bool fAtLineStart = false;
    // true if we have run into usages of dFdx / dFdy
    bool fFoundDerivatives = false;
    skia_private::THashMap<const FunctionDeclaration*, Requirements> fRequirements;
    skia_private::THashSet<std::string> fHelpers;
    int fUniformBuffer = -1;
    std::string fRTFlipName;
    const FunctionDeclaration* fCurrentFunction = nullptr;
    int fSwizzleHelperCount = 0;
    static constexpr char kTextureSuffix[] = "_Tex";
    static constexpr char kSamplerSuffix[] = "_Smplr";

    // If we might use an index expression more than once, we need to capture the result in a
    // temporary variable to avoid double-evaluation. This should generally only occur when emitting
    // a function call, since we need to polyfill GLSL-style out-parameter support. (skia:14130)
    // The map holds <index-expression, temp-variable name>.
    using IndexSubstitutionMap = skia_private::THashMap<const Expression*, std::string>;

    // When fIndexSubstitution is null (usually), index-substitution does not need to be performed.
    struct IndexSubstitutionData {
        IndexSubstitutionMap fMap;
        StringStream fMainStream;
        StringStream fPrefixStream;
        bool fCreateSubstitutes = true;
    };
    std::unique_ptr<IndexSubstitutionData> fIndexSubstitutionData;

    // Workaround/polyfill flags
    bool fWrittenInverse2 = false, fWrittenInverse3 = false, fWrittenInverse4 = false;
    bool fWrittenMatrixCompMult = false;
    bool fWrittenOuterProduct = false;

    using INHERITED = CodeGenerator;
};

}  // namespace SkSL

#endif
