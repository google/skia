/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_METALCODEGENERATOR
#define SKSL_METALCODEGENERATOR

#include <set>
#include <stack>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "src/sksl/SkSLCodeGenerator.h"
#include "src/sksl/SkSLOperators.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLFunctionPrototype.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLInlineMarker.h"
#include "src/sksl/ir/SkSLIntLiteral.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

/**
 * Converts a Program into Metal code.
 */
class MetalCodeGenerator : public CodeGenerator {
public:
    static constexpr const char* SAMPLER_SUFFIX = "Smplr";
    static constexpr const char* PACKED_PREFIX = "packed_";

    MetalCodeGenerator(const Context* context, const Program* program, ErrorReporter* errors,
                      OutputStream* out)
    : INHERITED(program, errors, out)
    , fReservedWords({"atan2", "rsqrt", "rint", "dfdx", "dfdy", "vertex", "fragment"})
    , fLineEnding("\n")
    , fContext(*context) {
        this->setupIntrinsics();
    }

    bool generateCode() override;

protected:
    using Precedence = Operator::Precedence;

    typedef int Requirements;
    static constexpr Requirements kNo_Requirements       = 0;
    static constexpr Requirements kInputs_Requirement    = 1 << 0;
    static constexpr Requirements kOutputs_Requirement   = 1 << 1;
    static constexpr Requirements kUniforms_Requirement  = 1 << 2;
    static constexpr Requirements kGlobals_Requirement   = 1 << 3;
    static constexpr Requirements kFragCoord_Requirement = 1 << 4;

    enum IntrinsicKind {
        kAtan_IntrinsicKind,
        kBitcast_IntrinsicKind,
        kBitCount_IntrinsicKind,
        kCompareEqual_IntrinsicKind,
        kCompareGreaterThan_IntrinsicKind,
        kCompareGreaterThanEqual_IntrinsicKind,
        kCompareLessThan_IntrinsicKind,
        kCompareLessThanEqual_IntrinsicKind,
        kCompareNotEqual_IntrinsicKind,
        kDegrees_IntrinsicKind,
        kDFdx_IntrinsicKind,
        kDFdy_IntrinsicKind,
        kDistance_IntrinsicKind,
        kDot_IntrinsicKind,
        kFaceforward_IntrinsicKind,
        kFindLSB_IntrinsicKind,
        kFindMSB_IntrinsicKind,
        kInverse_IntrinsicKind,
        kInversesqrt_IntrinsicKind,
        kLength_IntrinsicKind,
        kMatrixCompMult_IntrinsicKind,
        kMod_IntrinsicKind,
        kNormalize_IntrinsicKind,
        kRadians_IntrinsicKind,
        kReflect_IntrinsicKind,
        kRefract_IntrinsicKind,
        kRoundEven_IntrinsicKind,
        kTexture_IntrinsicKind,
    };

    static const char* OperatorName(Operator op);

    class GlobalStructVisitor;
    void visitGlobalStruct(GlobalStructVisitor* visitor);

    void setupIntrinsics();

    void write(const char* s);

    void writeLine();

    void writeLine(const char* s);

    void write(const String& s);

    void writeLine(const String& s);

    void finishLine();

    void writeHeader();

    void writeUniformStruct();

    void writeInputStruct();

    void writeOutputStruct();

    void writeInterfaceBlocks();

    void writeStructDefinitions();

    void writeFields(const std::vector<Type::Field>& fields, int parentOffset,
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

    void writeModifiers(const Modifiers& modifiers, bool globalContext);

    void writeVarInitializer(const Variable& var, const Expression& value);

    void writeName(const String& name);

    void writeVarDeclaration(const VarDeclaration& decl, bool global);

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

    bool matrixConstructHelperIsNeeded(const Constructor& c);
    String getMatrixConstructHelper(const Constructor& c);
    void assembleMatrixFromMatrix(const Type& sourceMatrix, int rows, int columns);
    void assembleMatrixFromExpressions(const ExpressionArray& args, int rows, int columns);

    void writeMatrixCompMult();

    void writeMatrixTimesEqualHelper(const Type& left, const Type& right, const Type& result);

    void writeMatrixEqualityHelper(const Type& left, const Type& right);

    void writeMatrixInequalityHelper(const Type& left, const Type& right);

    void writeArgumentList(const ExpressionArray& arguments);

    void writeSimpleIntrinsic(const FunctionCall& c);

    void writeIntrinsicCall(const FunctionCall& c, IntrinsicKind kind);

    bool canCoerce(const Type& t1, const Type& t2);

    void writeConstructor(const Constructor& c, Precedence parentPrecedence);

    void writeFieldAccess(const FieldAccess& f);

    void writeSwizzle(const Swizzle& swizzle);

    void writeBinaryExpression(const BinaryExpression& b, Precedence parentPrecedence);

    void writeTernaryExpression(const TernaryExpression& t, Precedence parentPrecedence);

    void writeIndexExpression(const IndexExpression& expr);

    void writePrefixExpression(const PrefixExpression& p, Precedence parentPrecedence);

    void writePostfixExpression(const PostfixExpression& p, Precedence parentPrecedence);

    void writeBoolLiteral(const BoolLiteral& b);

    void writeIntLiteral(const IntLiteral& i);

    void writeFloatLiteral(const FloatLiteral& f);

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

    std::unordered_map<String, IntrinsicKind> fIntrinsicMap;
    std::unordered_set<String> fReservedWords;
    std::unordered_map<const Type::Field*, const InterfaceBlock*> fInterfaceBlockMap;
    std::unordered_map<const InterfaceBlock*, String> fInterfaceBlockNameMap;
    int fAnonInterfaceCount = 0;
    int fPaddingCount = 0;
    const char* fLineEnding;
    const Context& fContext;
    String fFunctionHeader;
    StringStream fExtraFunctions;
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
    String fRTHeightName;
    const FunctionDeclaration* fCurrentFunction = nullptr;
    int fSwizzleHelperCount = 0;
    bool fIgnoreVariableReferenceModifiers = false;

    using INHERITED = CodeGenerator;
};

}  // namespace SkSL

#endif
