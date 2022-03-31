/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_WGSLCODEGENERATOR
#define SKSL_WGSLCODEGENERATOR

#include "include/sksl/SkSLOperator.h"
#include "src/sksl/codegen/SkSLCodeGenerator.h"

namespace SkSL {

class AnyConstructor;
class BinaryExpression;
class Block;
class ConstructorCompound;
class Expression;
class ExpressionStatement;
class FunctionDeclaration;
class FunctionDefinition;
class Literal;
class ProgramElement;
class ReturnStatement;
class Statement;
class VarDeclaration;
class Variable;

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

    WGSLCodeGenerator(const Context* context, const Program* program, OutputStream* out)
            : INHERITED(context, program, out) {}

    bool generateCode() override;

private:
    using INHERITED = CodeGenerator;
    using Precedence = Operator::Precedence;

    // Write output content while correctly handling indentation.
    void write(std::string_view s);
    void writeLine(std::string_view s = std::string_view());
    void finishLine();
    void writeName(std::string_view name);

    // Helpers to declare a pipeline stage IO parameter declaration.
    void writePipelineIODeclaration(Modifiers modifiers, const Type& type, std::string_view name);
    void writeUserDefinedVariableDecl(const Type& type, std::string_view name, int location);
    void writeBuiltinVariableDecl(const Type& type, std::string_view name, Builtin kind);

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
    void writeReturnStatement(const ReturnStatement& s);
    void writeVarDeclaration(const VarDeclaration& varDecl);

    // Writers for expressions.
    void writeExpression(const Expression& e, Precedence parentPrecedence);
    void writeBinaryExpression(const BinaryExpression& b, Precedence parentPrecedence);
    void writeLiteral(const Literal& l);

    // Constructor expressions
    void writeAnyConstructor(const AnyConstructor& c, Precedence parentPrecedence);
    void writeConstructorCompound(const ConstructorCompound& c, Precedence parentPrecedence);
    void writeConstructorCompoundVector(const ConstructorCompound& c, Precedence parentPrecedence);

    // Generic recursive ProgramElement visitor.
    void writeProgramElement(const ProgramElement& e);

    // We bundle all varying pipeline stage inputs and outputs in a struct.
    void writeStageInputStruct();
    void writeStageOutputStruct();

    SkTHashSet<std::string_view> fReservedWords;

    // Output processing state.
    int fIndentation = 0;
    bool fAtLineStart = false;
};

}  // namespace SkSL

#endif  // SKSL_WGSLCODEGENERATOR
