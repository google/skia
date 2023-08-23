/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_GLSLCODEGENERATOR
#define SKSL_GLSLCODEGENERATOR

#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/codegen/SkSLCodeGenerator.h"
#include "src/sksl/ir/SkSLModifierFlags.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace SkSL {

class AnyConstructor;
class BinaryExpression;
class Block;
class ConstructorCompound;
class ConstructorDiagonalMatrix;
class DoStatement;
class Expression;
class ExpressionStatement;
class FieldAccess;
class ForStatement;
class FunctionCall;
class FunctionDeclaration;
class FunctionDefinition;
class FunctionPrototype;
class GlobalVarDeclaration;
class IfStatement;
class IndexExpression;
class InterfaceBlock;
class Literal;
class OutputStream;
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
struct Layout;
struct Program;
struct ShaderCaps;

/**
 * Converts a Program into GLSL code.
 */
class GLSLCodeGenerator final : public CodeGenerator {
public:
    GLSLCodeGenerator(const Context* context, const Program* program, OutputStream* out)
            : INHERITED(context, program, out) {}

    bool generateCode() override;

protected:
    using Precedence = OperatorPrecedence;

    void write(std::string_view s);

    void writeLine(std::string_view s = std::string_view());

    void finishLine();

    void writeHeader();

    bool usesPrecisionModifiers() const;

    void writeIdentifier(std::string_view identifier);

    std::string getTypeName(const Type& type);

    void writeStructDefinition(const StructDefinition& s);

    void writeType(const Type& type);

    void writeExtension(std::string_view name, bool require = true);

    void writeInterfaceBlock(const InterfaceBlock& intf);

    void writeFunctionDeclaration(const FunctionDeclaration& f);

    void writeFunctionPrototype(const FunctionPrototype& f);

    void writeFunction(const FunctionDefinition& f);

    void writeLayout(const Layout& layout);

    void writeModifiers(const Layout& layout, ModifierFlags flags, bool globalContext);

    void writeInputVars();

    void writeVarInitializer(const Variable& var, const Expression& value);

    const char* getTypePrecision(const Type& type);

    void writeTypePrecision(const Type& type);

    void writeGlobalVarDeclaration(const GlobalVarDeclaration& e);

    void writeVarDeclaration(const VarDeclaration& var, bool global);

    void writeFragCoord();

    void writeVariableReference(const VariableReference& ref);

    void writeExpression(const Expression& expr, Precedence parentPrecedence);

    void writeIntrinsicCall(const FunctionCall& c);

    void writeMinAbsHack(Expression& absExpr, Expression& otherExpr);

    void writeDeterminantHack(const Expression& mat);

    void writeInverseHack(const Expression& mat);

    void writeTransposeHack(const Expression& mat);

    void writeInverseSqrtHack(const Expression& x);

    void writeMatrixComparisonWorkaround(const BinaryExpression& x);

    void writeFunctionCall(const FunctionCall& c);

    void writeConstructorCompound(const ConstructorCompound& c, Precedence parentPrecedence);

    void writeConstructorDiagonalMatrix(const ConstructorDiagonalMatrix& c,
                                        Precedence parentPrecedence);

    void writeAnyConstructor(const AnyConstructor& c, Precedence parentPrecedence);

    void writeCastConstructor(const AnyConstructor& c, Precedence parentPrecedence);

    void writeFieldAccess(const FieldAccess& f);

    void writeSwizzle(const Swizzle& swizzle);

    void writeBinaryExpression(const BinaryExpression& b, Precedence parentPrecedence);

    void writeShortCircuitWorkaroundExpression(const BinaryExpression& b,
                                               Precedence parentPrecedence);

    void writeTernaryExpression(const TernaryExpression& t, Precedence parentPrecedence);

    void writeIndexExpression(const IndexExpression& expr);

    void writePrefixExpression(const PrefixExpression& p, Precedence parentPrecedence);

    void writePostfixExpression(const PostfixExpression& p, Precedence parentPrecedence);

    void writeLiteral(const Literal& l);

    void writeStatement(const Statement& s);

    void writeBlock(const Block& b);

    void writeIfStatement(const IfStatement& stmt);

    void writeForStatement(const ForStatement& f);

    void writeDoStatement(const DoStatement& d);

    void writeExpressionStatement(const ExpressionStatement& s);

    void writeSwitchStatement(const SwitchStatement& s);

    void writeReturnStatement(const ReturnStatement& r);

    void writeProgramElement(const ProgramElement& e);

    const ShaderCaps& caps() const { return *fContext.fCaps; }

    bool shouldRewriteVoidTypedFunctions(const FunctionDeclaration* func) const;

    StringStream fExtensions;
    StringStream fGlobals;
    StringStream fExtraFunctions;
    std::string fFunctionHeader;
    int fVarCount = 0;
    int fIndentation = 0;
    bool fAtLineStart = false;
    const FunctionDeclaration* fCurrentFunction = nullptr;

    // true if we have run into usages of dFdx / dFdy
    bool fFoundDerivatives = false;
    bool fFoundExternalSamplerDecl = false;
    bool fFoundRectSamplerDecl = false;
    bool fSetupClockwise = false;
    bool fSetupFragPosition = false;
    bool fSetupFragCoordWorkaround = false;

    // Workaround/polyfill flags
    bool fWrittenAbsEmulation = false;
    bool fWrittenDeterminant2 = false, fWrittenDeterminant3 = false, fWrittenDeterminant4 = false;
    bool fWrittenInverse2 = false, fWrittenInverse3 = false, fWrittenInverse4 = false;
    bool fWrittenTranspose[3][3] = {};

    using INHERITED = CodeGenerator;
};

}  // namespace SkSL

#endif
