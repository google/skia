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
    // When inside writeEmitCode(), certain SkSL elements need to control
    // when fragBuilder->codeAppendf is added to the function block. This
    // takes all completed statements in the SkSL buffer, and their corresponding
    // format args, and writes them into the emitCode()'s statement block
    // using writeCodeAppend().
    //
    // This control is necessary for handling special functions in SkSL, like
    // process(), which need to intermix the current FP's SkSL with that of
    // an emitted child.
    //
    //  :forceAll - If false, only the completed statements (terminated by ;),
    //     will be flushed and the sksl buffer will be set to any partial
    //     statements that remain. If true, everything is flushed, regardless.
    void flushEmittedCode(bool forceAll = false);

    void writef(const char* s, va_list va) SKSL_PRINTF_LIKE(2, 0);

    void writef(const char* s, ...) SKSL_PRINTF_LIKE(2, 3);

    bool writeSection(const char* name, const char* prefix = "");

    void writeHeader() override;

    bool usesPrecisionModifiers() const override;

    String getTypeName(const Type& type) override;

    void writeBinaryExpression(const BinaryExpression& b, Precedence parentPrecedence) override;

    void writeIndexExpression(const IndexExpression& i) override;

    void writeIntLiteral(const IntLiteral& i) override;

    void writeSwizzle(const Swizzle& swizzle) override;

    void writeVariableReference(const VariableReference& ref) override;

    String getSamplerHandle(const Variable& var);

    void writeIfStatement(const IfStatement& s) override;

    void writeReturnStatement(const ReturnStatement& s) override;

    void writeSwitchStatement(const SwitchStatement& s) override;

    void writeFunctionCall(const FunctionCall& c) override;

    void writeFunction(const FunctionDefinition& f) override;

    void writeSetting(const Setting& s) override;

    void writeProgramElement(const ProgramElement& p) override;

    void addUniform(const Variable& var);

    // writes a printf escape that will be filled in at runtime by the given C++ expression string
    void writeRuntimeValue(const Type& type, const Layout& layout, const String& cppCode);

    void writeVarInitializer(const Variable& var, const Expression& value) override;

    void writeInputVars() override;

    void writePrivateVars();

    void writePrivateVarValues();

    void writeCodeAppend(const String& code);

    bool writeEmitCode(std::vector<const Variable*>& uniforms);

    void writeSetData(std::vector<const Variable*>& uniforms);

    void writeGetKey();

    void writeOnTextureSampler();

    void writeClone();

    void writeTest();

    // If the returned C++ is included in the generated code, then the variable
    // name stored in cppVar will refer to a valid SkString that matches the
    // Expression. Successful returns leave the output buffer (and related state)
    // unmodified.
    //
    // In the simplest cases, this will return "SkString {cppVar}(\"{e}\");",
    // while more advanced cases will properly insert format arguments.
    String convertSKSLExpressionToCPP(const Expression& e, const String& cppVar);

    String fName;
    String fFullName;
    SectionAndParameterHelper fSectionAndParameterHelper;
    String fExtraEmitCodeCode;
    std::vector<String> fFormatArgs;
    std::set<int> fWrittenTransformedCoords;
    // if true, we are writing a C++ expression instead of a GLSL expression
    bool fCPPMode = false;
    bool fInMain = false;

    // if not null, we are accumulating SkSL for emitCode into fOut, which
    // replaced the original buffer with a StringStream. The original buffer is
    // stored here for restoration.
    OutputStream* fCPPBuffer = nullptr;

    typedef GLSLCodeGenerator INHERITED;
};

}

#endif
