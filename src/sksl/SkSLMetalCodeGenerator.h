/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_METALCODEGENERATOR
#define SKSL_METALCODEGENERATOR

#include <stack>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#include "src/sksl/SkSLCodeGenerator.h"
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
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

#define kLast_Capability SpvCapabilityMultiViewport

/**
 * Converts a Program into Metal code.
 */
class MetalCodeGenerator : public CodeGenerator {
public:
    static constexpr const char* SAMPLER_SUFFIX = "Smplr";
    static constexpr const char* PACKED_PREFIX = "packed_";

    enum Precedence {
        kParentheses_Precedence    =  1,
        kPostfix_Precedence        =  2,
        kPrefix_Precedence         =  3,
        kMultiplicative_Precedence =  4,
        kAdditive_Precedence       =  5,
        kShift_Precedence          =  6,
        kRelational_Precedence     =  7,
        kEquality_Precedence       =  8,
        kBitwiseAnd_Precedence     =  9,
        kBitwiseXor_Precedence     = 10,
        kBitwiseOr_Precedence      = 11,
        kLogicalAnd_Precedence     = 12,
        kLogicalXor_Precedence     = 13,
        kLogicalOr_Precedence      = 14,
        kTernary_Precedence        = 15,
        kAssignment_Precedence     = 16,
        kSequence_Precedence       = 17,
        kTopLevel_Precedence       = kSequence_Precedence
    };

    MetalCodeGenerator(const Context* context, const Program* program, ErrorReporter* errors,
                      OutputStream* out)
    : INHERITED(program, errors, out)
    , fReservedWords({"atan2", "rsqrt", "dfdx", "dfdy", "vertex", "fragment"})
    , fLineEnding("\n")
    , fContext(*context) {
        this->setupIntrinsics();
    }

    bool generateCode() override;

protected:
    typedef int Requirements;
    static constexpr Requirements kNo_Requirements       = 0;
    static constexpr Requirements kInputs_Requirement    = 1 << 0;
    static constexpr Requirements kOutputs_Requirement   = 1 << 1;
    static constexpr Requirements kUniforms_Requirement  = 1 << 2;
    static constexpr Requirements kGlobals_Requirement   = 1 << 3;
    static constexpr Requirements kFragCoord_Requirement = 1 << 4;

    enum IntrinsicKind {
        kSpecial_IntrinsicKind,
        kMetal_IntrinsicKind,
    };

    enum SpecialIntrinsic {
        kBitcast_SpecialIntrinsic,
        kDegrees_SpecialIntrinsic,
        kDistance_SpecialIntrinsic,
        kDot_SpecialIntrinsic,
        kFaceforward_SpecialIntrinsic,
        kFindLSB_SpecialIntrinsic,
        kFindMSB_SpecialIntrinsic,
        kLength_SpecialIntrinsic,
        kMod_SpecialIntrinsic,
        kNormalize_SpecialIntrinsic,
        kRadians_SpecialIntrinsic,
        kTexture_SpecialIntrinsic,
    };

    enum MetalIntrinsic {
        kEqual_MetalIntrinsic,
        kNotEqual_MetalIntrinsic,
        kLessThan_MetalIntrinsic,
        kLessThanEqual_MetalIntrinsic,
        kGreaterThan_MetalIntrinsic,
        kGreaterThanEqual_MetalIntrinsic,
    };

    static const char* OperatorName(Token::Kind op);

    class GlobalStructVisitor;
    void visitGlobalStruct(GlobalStructVisitor* visitor);

    void setupIntrinsics();

    void write(const char* s);

    void writeLine();

    void writeLine(const char* s);

    void write(const String& s);

    void writeLine(const String& s);

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

    bool writeStructDefinition(const Type& type);

    void disallowArrayTypes(const Type& type, int offset);

    void writeBaseType(const Type& type);

    void writeArrayDimensions(const Type& type);

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

    void writeIntrinsicCall(const FunctionCall& c);

    void writeMinAbsHack(Expression& absExpr, Expression& otherExpr);

    String getOutParamHelper(const FunctionCall& c,
                             const ExpressionArray& arguments,
                             const SkTArray<VariableReference*>& outVars);

    String getInverseHack(const Expression& mat);

    String getBitcastIntrinsic(const Type& outType);

    String getTempVariable(const Type& varType);

    void writeFunctionCall(const FunctionCall& c);

    bool matrixConstructHelperIsNeeded(const Constructor& c);
    String getMatrixConstructHelper(const Constructor& c);
    void assembleMatrixFromMatrix(const Type& sourceMatrix, int rows, int columns);
    void assembleMatrixFromExpressions(const ExpressionArray& args, int rows, int columns);

    void writeMatrixTimesEqualHelper(const Type& left, const Type& right, const Type& result);

    void writeSpecialIntrinsic(const FunctionCall& c, SpecialIntrinsic kind);

    bool canCoerce(const Type& t1, const Type& t2);

    void writeConstructor(const Constructor& c, Precedence parentPrecedence);

    void writeFieldAccess(const FieldAccess& f);

    void writeSwizzle(const Swizzle& swizzle);

    static Precedence GetBinaryPrecedence(Token::Kind op);

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

    typedef std::pair<IntrinsicKind, int32_t> Intrinsic;
    std::unordered_map<String, Intrinsic> fIntrinsicMap;
    std::unordered_set<String> fReservedWords;
    std::unordered_map<const Type::Field*, const InterfaceBlock*> fInterfaceBlockMap;
    std::unordered_map<const InterfaceBlock*, String> fInterfaceBlockNameMap;
    int fAnonInterfaceCount = 0;
    int fPaddingCount = 0;
    const char* fLineEnding;
    const Context& fContext;
    String fFunctionHeader;
    StringStream fExtraFunctions;
    Program::Kind fProgramKind;
    int fVarCount = 0;
    int fIndentation = 0;
    bool fAtLineStart = false;
    // Keeps track of which struct types we have written. Given that we are unlikely to ever write
    // more than one or two structs per shader, a simple linear search will be faster than anything
    // fancier.
    std::vector<const Type*> fWrittenStructs;
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
