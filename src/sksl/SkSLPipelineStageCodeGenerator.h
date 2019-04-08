/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PIPELINESTAGECODEGENERATOR
#define SKSL_PIPELINESTAGECODEGENERATOR

#include "SkSLGLSLCodeGenerator.h"
#include "SkSLSectionAndParameterHelper.h"

#include <set>

namespace SkSL {

class PipelineStageCodeGenerator : public GLSLCodeGenerator {
public:
    PipelineStageCodeGenerator(const Context* context, const Program* program,
                               ErrorReporter* errors, OutputStream* out,
                               std::vector<Compiler::FormatArg>* outFormatArgs);

private:
    void writef(const char* s, va_list va) SKSL_PRINTF_LIKE(2, 0);

    void writef(const char* s, ...) SKSL_PRINTF_LIKE(2, 3);

    bool writeSection(const char* name, const char* prefix = "");

    void writeHeader() override;

    bool usesPrecisionModifiers() const override;

    String getTypeName(const Type& type) override;

    void writeBinaryExpression(const BinaryExpression& b, Precedence parentPrecedence) override;

    void writeFunctionCall(const FunctionCall& c) override;

    void writeIntLiteral(const IntLiteral& i) override;

    void writeVariableReference(const VariableReference& ref) override;

    void writeIfStatement(const IfStatement& s) override;

    void writeReturnStatement(const ReturnStatement& r) override;

    void writeSwitchStatement(const SwitchStatement& s) override;

    void writeFunction(const FunctionDefinition& f) override;

    void writeProgramElement(const ProgramElement& p) override;

    bool writeEmitCode(std::vector<const Variable*>& uniforms);

    String fName;
    String fFullName;
    SectionAndParameterHelper fSectionAndParameterHelper;
    String fExtraEmitCodeCode;
    std::set<int> fWrittenTransformedCoords;
    std::vector<Compiler::FormatArg>* fFormatArgs;
    const FunctionDeclaration* fCurrentFunction;

    typedef GLSLCodeGenerator INHERITED;
};

}

#endif
