/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CPPCODEGENERATOR
#define SKSL_CPPCODEGENERATOR

#include "SkSLGLSLCodeGenerator.h"
#include "SkSLSectionAndParameterHelper.h"

#include <set>

namespace SkSL {

class CPPCodeGenerator : public GLSLCodeGenerator {
public:
    CPPCodeGenerator(const Context* context, const Program* program, ErrorReporter* errors,
                     String name, OutputStream* out);

    bool generateCode() override;

private:
    void writef(const char* s, va_list va) SKSL_PRINTF_LIKE(2, 0);

    void writef(const char* s, ...) SKSL_PRINTF_LIKE(2, 3);

    void writeSection(const char* name, const char* prefix = "");

    void writeHeader() override;

    void writePrecisionModifier() override;

    void writeBinaryExpression(const BinaryExpression& b, Precedence parentPrecedence) override;

    void writeIndexExpression(const IndexExpression& i) override;

    void writeVariableReference(const VariableReference& ref) override;

    String getSamplerHandle(const Variable& var);

    void writeIfStatement(const IfStatement& s) override;

    void writeSwitchStatement(const SwitchStatement& s) override;

    void writeFunctionCall(const FunctionCall& c) override;

    void writeFunction(const FunctionDefinition& f) override;

    void writeSetting(const Setting& s) override;

    void writeProgramElement(const ProgramElement& p) override;

    void addUniform(const Variable& var);

    // writes a printf escape that will be filled in at runtime by the given C++ expression string
    void writeRuntimeValue(const Type& type, const String& cppCode);

    void writeVarInitializer(const Variable& var, const Expression& value) override;

    void writePrivateVars();

    void writePrivateVarValues();

    bool writeEmitCode(std::vector<const Variable*>& uniforms);

    void writeSetData(std::vector<const Variable*>& uniforms);

    void writeGetKey();

    void writeTest();

    String fName;
    String fFullName;
    SectionAndParameterHelper fSectionAndParameterHelper;
    String fExtraEmitCodeCode;
    std::vector<String> fFormatArgs;
    std::set<int> fWrittenTransformedCoords;
    bool fNeedColorSpaceHelper = false;

    typedef GLSLCodeGenerator INHERITED;
};

}

#endif
