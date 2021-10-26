/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_METALCODEGENERATOR
#define SKSL_METALCODEGENERATOR

#include <unordered_map>
#include <unordered_set>

#include "src/sksl/SkSLOperators.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/codegen/SkSLCodeGenerator.h"

namespace SkSL {

class BinaryExpression;
class Block;
class ConstructorArrayCast;
class ConstructorCompound;
class ConstructorMatrixResize;
class DoStatement;
class Extension;
class FieldAccess;
class ForStatement;
class FunctionCall;
class FunctionDeclaration;
class FunctionDefinition;
class FunctionPrototype;
class IfStatement;
struct IndexExpression;
class InterfaceBlock;
enum IntrinsicKind : int8_t;
class Literal;
class PostfixExpression;
class PrefixExpression;
class ReturnStatement;
class Setting;
class StructDefinition;
class SwitchStatement;
struct Swizzle;
class TernaryExpression;
class VarDeclaration;
class VariableReference;

/**
 * Converts a Program into Metal code.
 */
class MetalCodeGenerator : public CodeGenerator {
public:
    inline static constexpr const char* SAMPLER_SUFFIX = "Smplr";
    inline static constexpr const char* PACKED_PREFIX = "packed_";

    MetalCodeGenerator(const Context* context, const Program* program, OutputStream* out)
    : INHERITED(context, program, out)
    , fReservedWords({"atan2", "rsqrt", "rint", "dfdx", "dfdy", "vertex", "fragment"})
    , fLineEnding("\n") {}

    bool generateCode() override;

protected:
    using Precedence = Operator::Precedence;

    typedef int Requirements;
    inline static constexpr Requirements kNo_Requirements       = 0;
    inline static constexpr Requirements kInputs_Requirement    = 1 << 0;
    inline static constexpr Requirements kOutputs_Requirement   = 1 << 1;
    inline static constexpr Requirements kUniforms_Requirement  = 1 << 2;
    inline static constexpr Requirements kGlobals_Requirement   = 1 << 3;
    inline static constexpr Requirements kFragCoord_Requirement = 1 << 4;

    static const char* OperatorName(Operator op);

    class GlobalStructVisitor;
    void visitGlobalStruct(GlobalStructVisitor* visitor);

    void write(skstd::string_view s);

    void writeLine(skstd::string_view s = skstd::string_view());

    void finishLine();

    void writeHeader();

    void writeUniformStruct();

    void writeInputStruct();

    void writeOutputStruct();

    void writeInterfaceBlocks();

    void writeStructDefinitions();

    void writeFields(const std::vector<Type::Field>& fields, int parentLine,
                     const InterfaceBlock* parentIntf = nullptr);

    int size(const Type* type, bool isPacked) const;

    int alignment(const Type* type, bool isPacked) const;

    void writeGlobalStruct();

    void writeGlobalInit();

    void writePrecisionModifier();

    String typeName(const Type& type);

    void writeStructDefinition(const StructDefinition& s);

    void writeType(const Type& type);

    void writeExtension(const Extension& ext);

    void writeInterfaceBlock(const InterfaceBlock& intf);

    void writeFunctionStart(const FunctionDeclaration& f);

    void writeFunctionRequirementParams(const FunctionDeclaration& f,
                                        const char*& separator);

    void writeFunctionRequirementArgs(const FunctionDeclaration& f, const char*& separator);

    bool writeFunctionDeclaration(const FunctionDeclaration& f);

    void writeFunction(const FunctionDefinition& f);

    void writeFunctionPrototype(const FunctionPrototype& f);

    void writeLayout(const Layout& layout);

    void writeModifiers(const Modifiers& modifiers);

    void writeVarInitializer(const Variable& var, const Expression& value);

    void writeName(skstd::string_view name);

    void writeVarDeclaration(const VarDeclaration& decl);

    void writeFragCoord();

    void writeVariableReference(const VariableReference& ref);

    void writeExpression(const Expression& expr, Precedence parentPrecedence);

    void writeMinAbsHack(Expression& absExpr, Expression& otherExpr);

    String getOutParamHelper(const FunctionCall& c,
                             const ExpressionArray& arguments,
                             const SkTArray<VariableReference*>& outVars);

    String getInversePolyfill(const ExpressionArray& arguments);

    String getBitcastIntrinsic(const Type& outType);

    String getTempVariable(const Type& varType);

    void writeFunctionCall(const FunctionCall& c);

    bool matrixConstructHelperIsNeeded(const ConstructorCompound& c);
    String getMatrixConstructHelper(const AnyConstructor& c);
    void assembleMatrixFromMatrix(const Type& sourceMatrix, int rows, int columns);
    void assembleMatrixFromExpressions(const AnyConstructor& ctor, int rows, int columns);

    void writeMatrixCompMult();

    void writeOuterProduct();

    void writeMatrixTimesEqualHelper(const Type& left, const Type& right, const Type& result);

    void writeMatrixDivisionHelpers(const Type& type);

    void writeMatrixEqualityHelpers(const Type& left, const Type& right);

    String getVectorFromMat2x2ConstructorHelper(const Type& matrixType);

    void writeArrayEqualityHelpers(const Type& type);

    void writeStructEqualityHelpers(const Type& type);

    void writeEqualityHelpers(const Type& leftType, const Type& rightType);

    void writeArgumentList(const ExpressionArray& arguments);

    void writeSimpleIntrinsic(const FunctionCall& c);

    bool writeIntrinsicCall(const FunctionCall& c, IntrinsicKind kind);

    bool canCoerce(const Type& t1, const Type& t2);

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

    void writeBinaryExpression(const BinaryExpression& b, Precedence parentPrecedence);

    void writeTernaryExpression(const TernaryExpression& t, Precedence parentPrecedence);

    void writeIndexExpression(const IndexExpression& expr);

    void writePrefixExpression(const PrefixExpression& p, Precedence parentPrecedence);

    void writePostfixExpression(const PostfixExpression& p, Precedence parentPrecedence);

    void writeLiteral(const Literal& f);

    void writeSetting(const Setting& s);

    void writeStatement(const Statement& s);

    void writeStatements(const StatementArray& statements);

    void writeBlock(const Block& b);

    void writeIfStatement(const IfStatement& stmt);

    void writeForStatement(const ForStatement& f);

    void writeDoStatement(const DoStatement& d);

    void writeSwitchStatement(const SwitchStatement& s);

    void writeReturnStatementFromMain();

    void writeReturnStatement(const ReturnStatement& r);

    void writeProgramElement(const ProgramElement& e);

    Requirements requirements(const FunctionDeclaration& f);

    Requirements requirements(const Expression* e);

    Requirements requirements(const Statement* s);

    int getUniformBinding(const Modifiers& m);

    int getUniformSet(const Modifiers& m);

    std::unordered_set<skstd::string_view> fReservedWords;
    std::unordered_map<const Type::Field*, const InterfaceBlock*> fInterfaceBlockMap;
    std::unordered_map<const InterfaceBlock*, skstd::string_view> fInterfaceBlockNameMap;
    int fAnonInterfaceCount = 0;
    int fPaddingCount = 0;
    const char* fLineEnding;
    String fFunctionHeader;
    StringStream fExtraFunctions;
    StringStream fExtraFunctionPrototypes;
    int fVarCount = 0;
    int fIndentation = 0;
    bool fAtLineStart = false;
    std::set<String> fWrittenIntrinsics;
    // true if we have run into usages of dFdx / dFdy
    bool fFoundDerivatives = false;
    std::unordered_map<const FunctionDeclaration*, Requirements> fRequirements;
    bool fSetupFragPositionGlobal = false;
    bool fSetupFragPositionLocal = false;
    std::unordered_set<String> fHelpers;
    int fUniformBuffer = -1;
    String fRTFlipName;
    const FunctionDeclaration* fCurrentFunction = nullptr;
    int fSwizzleHelperCount = 0;
    bool fIgnoreVariableReferenceModifiers = false;

    using INHERITED = CodeGenerator;
};

}  // namespace SkSL

#endif
