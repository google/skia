/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PIPELINESTAGECODEGENERATOR
#define SKSL_PIPELINESTAGECODEGENERATOR

#include "src/sksl/SkSLGLSLCodeGenerator.h"

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

namespace SkSL {

class PipelineStageCodeGenerator : public GLSLCodeGenerator {
public:
    PipelineStageCodeGenerator(const Context* context, const Program* program,
                               ErrorReporter* errors, OutputStream* out,
                               PipelineStageArgs* outArgs);

private:
    void writeHeader() override;

    bool usesPrecisionModifiers() const override;

    String getTypeName(const Type& type) override;

    void writeBinaryExpression(const BinaryExpression& b, Precedence parentPrecedence) override;

    void writeFunctionCall(const FunctionCall& c) override;

    void writeIntLiteral(const IntLiteral& i) override;

    void writeVariableReference(const VariableReference& ref) override;

    void writeIfStatement(const IfStatement& s) override;

    void writeSwitchStatement(const SwitchStatement& s) override;

    void writeFunction(const FunctionDefinition& f) override;

    void writeProgramElement(const ProgramElement& p) override;

    PipelineStageArgs* fArgs;

    typedef GLSLCodeGenerator INHERITED;
};

}

#endif

#endif
