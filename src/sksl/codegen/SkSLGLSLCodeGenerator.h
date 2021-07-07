/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_GLSLCODEGENERATOR
#define SKSL_GLSLCODEGENERATOR

#include <set>
#include <stack>
#include <tuple>
#include <unordered_map>

#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "src/sksl/SkSLOperators.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/codegen/SkSLCodeGenerator.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorScalarCast.h"
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
 * Converts a Program into GLSL code.
 */
class GLSLCodeGenerator : public CodeGenerator {
public:
    GLSLCodeGenerator(const Context* context, const Program* program, ErrorReporter* errors,
                      OutputStream* out)
    : INHERITED(program, errors, out)
    , fLineEnding("\n")
    , fContext(*context) {}

    bool generateCode() override;

protected:
    using Precedence = Operator::Precedence;

    void write(skstd::string_view s);

    void writeLine(skstd::string_view s = skstd::string_view());

    void finishLine();

    virtual void writeHeader();

    virtual bool usesPrecisionModifiers() const;

    virtual String getTypeName(const Type& type);

    void writeStructDefinition(const StructDefinition& s);

    void writeType(const Type& type);

    void writeExtension(skstd::string_view name, bool require = true);

    void writeInterfaceBlock(const InterfaceBlock& intf);

    void writeFunctionStart(const FunctionDeclaration& f);

    void writeFunctionDeclaration(const FunctionDeclaration& f);

    void writeFunctionPrototype(const FunctionPrototype& f);

    virtual void writeFunction(const FunctionDefinition& f);

    void writeLayout(const Layout& layout);

    void writeModifiers(const Modifiers& modifiers, bool globalContext);

    virtual void writeInputVars();

    virtual void writeVarInitializer(const Variable& var, const Expression& value);

    const char* getTypePrecision(const Type& type);

    void writeTypePrecision(const Type& type);

    void writeVarDeclaration(const VarDeclaration& var, bool global);

    void writeFragCoord();

    virtual void writeVariableReference(const VariableReference& ref);

    void writeExpression(const Expression& expr, Precedence parentPrecedence);

    void writeIntrinsicCall(const FunctionCall& c);

    void writeMinAbsHack(Expression& absExpr, Expression& otherExpr);

    void writeDeterminantHack(const Expression& mat);

    void writeInverseHack(const Expression& mat);

    void writeTransposeHack(const Expression& mat);

    void writeInverseSqrtHack(const Expression& x);

    void writeMatrixComparisonWorkaround(const BinaryExpression& x);

    virtual void writeFunctionCall(const FunctionCall& c);

    void writeConstructorDiagonalMatrix(const ConstructorDiagonalMatrix& c,
                                        Precedence parentPrecedence);

    virtual void writeAnyConstructor(const AnyConstructor& c, Precedence parentPrecedence);

    virtual void writeCastConstructor(const AnyConstructor& c, Precedence parentPrecedence);

    virtual void writeFieldAccess(const FieldAccess& f);

    virtual void writeSwizzle(const Swizzle& swizzle);

    virtual void writeBinaryExpression(const BinaryExpression& b, Precedence parentPrecedence);

    void writeShortCircuitWorkaroundExpression(const BinaryExpression& b,
                                               Precedence parentPrecedence);

    virtual void writeTernaryExpression(const TernaryExpression& t, Precedence parentPrecedence);

    virtual void writeIndexExpression(const IndexExpression& expr);

    void writePrefixExpression(const PrefixExpression& p, Precedence parentPrecedence);

    void writePostfixExpression(const PostfixExpression& p, Precedence parentPrecedence);

    void writeBoolLiteral(const BoolLiteral& b);

    virtual void writeIntLiteral(const IntLiteral& i);

    virtual void writeFloatLiteral(const FloatLiteral& f);

    virtual void writeSetting(const Setting& s);

    void writeStatement(const Statement& s);

    void writeBlock(const Block& b);

    virtual void writeIfStatement(const IfStatement& stmt);

    void writeForStatement(const ForStatement& f);

    void writeDoStatement(const DoStatement& d);

    virtual void writeSwitchStatement(const SwitchStatement& s);

    virtual void writeReturnStatement(const ReturnStatement& r);

    virtual void writeProgramElement(const ProgramElement& e);

    const ShaderCapsClass& caps() const { return fContext.fCaps; }

    const char* fLineEnding;
    const Context& fContext;
    StringStream fExtensions;
    StringStream fGlobals;
    StringStream fExtraFunctions;
    String fFunctionHeader;
    int fVarCount = 0;
    int fIndentation = 0;
    bool fAtLineStart = false;
    std::set<String> fWrittenIntrinsics;
    // true if we have run into usages of dFdx / dFdy
    bool fFoundDerivatives = false;
    bool fFoundExternalSamplerDecl = false;
    bool fFoundRectSamplerDecl = false;
    bool fFoundGSInvocations = false;
    bool fSetupClockwise = false;
    bool fSetupFragPosition = false;
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
    static std::unordered_map<skstd::string_view, FunctionClass>* fFunctionClasses;

    using INHERITED = CodeGenerator;
};

}  // namespace SkSL

#endif
