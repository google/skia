/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_GLSLCODEGENERATOR
#define SKSL_GLSLCODEGENERATOR

#include <stack>
#include <tuple>
#include <unordered_map>

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
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
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
#include "src/sksl/ir/SkSLVarDeclarationsStatement.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/ir/SkSLWhileStatement.h"

namespace SkSL {

#define kLast_Capability SpvCapabilityMultiViewport

/**
 * Converts a Program into GLSL code.
 */
class GLSLCodeGenerator : public CodeGenerator {
public:
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

    GLSLCodeGenerator(const Context* context, const Program* program, ErrorReporter* errors,
                      OutputStream* out)
    : INHERITED(program, errors, out)
    , fLineEnding("\n")
    , fContext(*context)
    , fProgramKind(program->fKind) {}

    bool generateCode() override;

protected:
    enum class SwizzleOrder {
        MASK_FIRST,
        CONSTANTS_FIRST
    };

    void write(const char* s);

    void writeLine();

    void writeLine(const char* s);

    void write(const String& s);

    void write(StringFragment s);

    void writeLine(const String& s);

    virtual void writeHeader();

    virtual bool usesPrecisionModifiers() const;

    virtual String getTypeName(const Type& type);

    void writeType(const Type& type);

    void writeExtension(const String& name);

    void writeExtension(const String& name, bool require);

    void writeInterfaceBlock(const InterfaceBlock& intf);

    void writeFunctionStart(const FunctionDeclaration& f);

    void writeFunctionDeclaration(const FunctionDeclaration& f);

    virtual void writeFunction(const FunctionDefinition& f);

    void writeLayout(const Layout& layout);

    void writeModifiers(const Modifiers& modifiers, bool globalContext);

    virtual void writeInputVars();

    virtual void writeVarInitializer(const Variable& var, const Expression& value);

    const char* getTypePrecision(const Type& type);

    void writeTypePrecision(const Type& type);

    void writeVarDeclarations(const VarDeclarations& decl, bool global);

    void writeFragCoord();

    virtual void writeVariableReference(const VariableReference& ref);

    void writeExpression(const Expression& expr, Precedence parentPrecedence);

    void writeIntrinsicCall(const FunctionCall& c);

    void writeMinAbsHack(Expression& absExpr, Expression& otherExpr);

    void writeDeterminantHack(const Expression& mat);

    void writeInverseHack(const Expression& mat);

    void writeTransposeHack(const Expression& mat);

    void writeInverseSqrtHack(const Expression& x);

    virtual void writeFunctionCall(const FunctionCall& c);

    void writeConstructor(const Constructor& c, Precedence parentPrecedence);

    virtual void writeFieldAccess(const FieldAccess& f);

    void writeConstantSwizzle(const Swizzle& swizzle, const String& constants);

    void writeSwizzleMask(const Swizzle& swizzle, const String& mask);

    void writeSwizzleConstructor(const Swizzle& swizzle, const String& constants,
                                 const String& mask, SwizzleOrder order);

    void writeSwizzleConstructor(const Swizzle& swizzle, const String& constants,
                                 const String& mask, const String& reswizzle);
    virtual void writeSwizzle(const Swizzle& swizzle);

    static Precedence GetBinaryPrecedence(Token::Kind op);

    virtual void writeBinaryExpression(const BinaryExpression& b, Precedence parentPrecedence);
    void writeShortCircuitWorkaroundExpression(const BinaryExpression& b,
                                               Precedence parentPrecedence);

    void writeTernaryExpression(const TernaryExpression& t, Precedence parentPrecedence);

    virtual void writeIndexExpression(const IndexExpression& expr);

    void writePrefixExpression(const PrefixExpression& p, Precedence parentPrecedence);

    void writePostfixExpression(const PostfixExpression& p, Precedence parentPrecedence);

    void writeBoolLiteral(const BoolLiteral& b);

    virtual void writeIntLiteral(const IntLiteral& i);

    void writeFloatLiteral(const FloatLiteral& f);

    virtual void writeSetting(const Setting& s);

    void writeStatement(const Statement& s);

    void writeStatements(const std::vector<std::unique_ptr<Statement>>& statements);

    void writeBlock(const Block& b);

    virtual void writeIfStatement(const IfStatement& stmt);

    void writeForStatement(const ForStatement& f);

    void writeWhileStatement(const WhileStatement& w);

    void writeDoStatement(const DoStatement& d);

    virtual void writeSwitchStatement(const SwitchStatement& s);

    virtual void writeReturnStatement(const ReturnStatement& r);

    virtual void writeProgramElement(const ProgramElement& e);

    const char* fLineEnding;
    const Context& fContext;
    StringStream fExtensions;
    StringStream fGlobals;
    StringStream fExtraFunctions;
    String fFunctionHeader;
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
    bool fFoundExternalSamplerDecl = false;
    bool fFoundRectSamplerDecl = false;
    bool fFoundGSInvocations = false;
    bool fSetupFragPositionGlobal = false;
    bool fSetupFragPositionLocal = false;
    bool fSetupFragCoordWorkaround = false;
    // if non-empty, replace all texture / texture2D / textureProj / etc. calls with this name
    String fTextureFunctionOverride;

    // We map function names to function class so we can quickly deal with function calls that need
    // extra processing
    enum class FunctionClass {
        kAbs,
        kAtan,
        kDeterminant,
        kDFdx,
        kDFdy,
        kFwidth,
        kFMA,
        kFract,
        kInverse,
        kInverseSqrt,
        kMin,
        kPow,
        kSaturate,
        kTexture,
        kTranspose
    };
    static std::unordered_map<StringFragment, FunctionClass>* fFunctionClasses;

    typedef CodeGenerator INHERITED;
};

}

#endif
