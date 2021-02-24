/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CPPCODEGENERATOR
#define SKSL_CPPCODEGENERATOR

#include "src/sksl/SkSLGLSLCodeGenerator.h"
#include "src/sksl/SkSLSectionAndParameterHelper.h"

#include <set>

#if defined(SKSL_STANDALONE) || GR_TEST_UTILS

namespace SkSL {

class CPPCodeGenerator : public GLSLCodeGenerator {
public:
    CPPCodeGenerator(const Context* context, const Program* program, ErrorReporter* errors,
                     String name, OutputStream* out);

    bool generateCode() override;

private:
    using Precedence = Operator::Precedence;

    void writef(const char* s, va_list va) SK_PRINTF_LIKE(2, 0);

    void writef(const char* s, ...) SK_PRINTF_LIKE(2, 3);

    bool writeSection(const char* name, const char* prefix = "");

    void writeHeader() override;

    bool usesPrecisionModifiers() const override;

    String getTypeName(const Type& type) override;

    void writeBinaryExpression(const BinaryExpression& b, Precedence parentPrecedence) override;

    void writeIntLiteral(const IntLiteral& i) override;

    void writeSwizzle(const Swizzle& swizzle) override;

    void writeVariableReference(const VariableReference& ref) override;

    String getSamplerHandle(const Variable& var);

    void writeIfStatement(const IfStatement& s) override;

    void writeReturnStatement(const ReturnStatement& s) override;

    void writeSwitchStatement(const SwitchStatement& s) override;

    String getSampleVarName(const char* prefix, int sampleCounter);

    void writeFunctionCall(const FunctionCall& c) override;

    void writeFunction(const FunctionDefinition& f) override;

    void prepareHelperFunction(const FunctionDeclaration& decl);

    void prototypeHelperFunction(const FunctionDeclaration& decl);

    void writeSetting(const Setting& s) override;

    void writeProgramElement(const ProgramElement& p) override;

    void addUniform(const Variable& var);

    // writes a printf escape that will be filled in at runtime by the given C++ expression string
    void writeRuntimeValue(const Type& type, const Layout& layout, const String& cppCode);
    String formatRuntimeValue(const Type& type, const Layout& layout, const String& cppCode,
                              std::vector<String>* formatArgs);

    void writeVarInitializer(const Variable& var, const Expression& value) override;

    void writeInputVars() override;

    void writePrivateVars();

    void writePrivateVarValues();

    void writeCodeAppend(const String& code);

    String assembleCodeAndFormatArgPrintf(const String& code);

    bool writeEmitCode(std::vector<const Variable*>& uniforms);

    void writeSetData(std::vector<const Variable*>& uniforms);

    void writeGetKey();

    void writeOnTextureSampler();

    void writeClone();

    void writeDumpInfo();

    void writeTest();

    // If the returned C++ is included in the generated code, then the variable name stored in
    // cppVar will refer to a valid SkString that matches the Expression. Successful returns leave
    // the output buffer (and related state) unmodified.
    //
    // In the simplest cases, this will return "SkString {cppVar}(\"{e}\");", while more advanced
    // cases will properly insert format arguments.
    String convertSKSLExpressionToCPP(const Expression& e, const String& cppVar);

    // Process accumulated sksl to split it into appended code sections, properly interleaved with
    // the extra emit code blocks, based on statement/block locations and the inserted tokens
    // from newExtraEmitCodeBlock(). It is necessary to split the sksl after the program has been
    // fully walked since many elements redirect fOut to simultaneously build header sections and
    // bodies that are then concatenated; due to this it is not possible to split the sksl emission
    // on the fly.
    void flushEmittedCode();

    // Start a new extra emit code block for accumulating C++ code. This will insert a token into
    // the sksl stream to mark the fence between previous complete sksl statements and where the
    // C++ code added to the new block will be added to emitCode(). These tokens are removed by
    // flushEmittedCode() as it consumes them before passing pure sksl to writeCodeAppend().
    void newExtraEmitCodeBlock();

    // Append CPP code to the current extra emit code block.
    void addExtraEmitCodeLine(const String& toAppend);

    int getChildFPIndex(const Variable& var) const;

    String fName;
    String fFullName;
    SectionAndParameterHelper fSectionAndParameterHelper;
    std::vector<String> fExtraEmitCodeBlocks;

    std::vector<String> fFormatArgs;
    // true if the sksl declared its main() function with a float2 parameter AND referenced that
    // parameter in its body.
    bool fAccessSampleCoordsDirectly = false;

    // If true, we are writing a C++ expression instead of a GLSL expression
    bool fCPPMode = false;

    // True while compiling the main() function of the FP.
    bool fInMain = false;

    // Gives unique but predictable names to invocations of sample().
    int fSampleCounter = 0;

    // if not null, we are accumulating SkSL for emitCode into fOut, which
    // replaced the original buffer with a StringStream. The original buffer is
    // stored here for restoration.
    OutputStream* fCPPBuffer = nullptr;

    using INHERITED = GLSLCodeGenerator;
};

}  // namespace SkSL

#endif // defined(SKSL_STANDALONE) || GR_TEST_UTILS

#endif // SKSL_CPPCODEGENERATOR
