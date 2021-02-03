/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PIPELINESTAGECODEGENERATOR
#define SKSL_PIPELINESTAGECODEGENERATOR

#include "src/sksl/SkSLCodeGenerator.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

namespace SkSL {

class PipelineStageCodeGenerator : public CodeGenerator {
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

    PipelineStageCodeGenerator(const Context* context, const Program* program,
                               ErrorReporter* errors, OutputStream* out,
                               PipelineStageArgs* outArgs);

    bool generateCode() override;

private:
    void write(const char* s);
    void writeLine(const char* s = nullptr);
    void write(const String& s);
    void write(StringFragment s);

    void writeType(const Type& type);

    void writeFunctionDeclaration(const FunctionDeclaration& f);
    void writeFunction(const FunctionDefinition& f);

    void writeModifiers(const Modifiers& modifiers);

    void writeVarDeclaration(const VarDeclaration& var);

    static Precedence GetBinaryPrecedence(Token::Kind op);

    void writeExpression(const Expression& expr, Precedence parentPrecedence);
    void writeFunctionCall(const FunctionCall& c);
    void writeConstructor(const Constructor& c, Precedence parentPrecedence);
    void writeFieldAccess(const FieldAccess& f);
    void writeSwizzle(const Swizzle& swizzle);
    void writeBinaryExpression(const BinaryExpression& b, Precedence parentPrecedence);
    void writeTernaryExpression(const TernaryExpression& t, Precedence parentPrecedence);
    void writeIndexExpression(const IndexExpression& expr);
    void writePrefixExpression(const PrefixExpression& p, Precedence parentPrecedence);
    void writePostfixExpression(const PostfixExpression& p, Precedence parentPrecedence);
    void writeVariableReference(const VariableReference& ref);

    void writeStatement(const Statement& s);
    void writeBlock(const Block& b);
    void writeIfStatement(const IfStatement& stmt);
    void writeForStatement(const ForStatement& f);
    void writeReturnStatement(const ReturnStatement& r);

    void writeProgramElement(const ProgramElement& e);

    const Context& fContext;
    PipelineStageArgs* fArgs;
    bool fCastReturnsToHalf = false;

    using INHERITED = CodeGenerator;
};

}  // namespace SkSL

#endif

#endif
