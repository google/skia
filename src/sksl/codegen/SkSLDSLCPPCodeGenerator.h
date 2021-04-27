/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSLCPPCODEGENERATOR
#define SKSL_DSLCPPCODEGENERATOR

#include "src/sksl/SkSLSectionAndParameterHelper.h"
#include "src/sksl/codegen/SkSLGLSLCodeGenerator.h"

#include <set>

#if defined(SKSL_STANDALONE) || GR_TEST_UTILS

namespace SkSL {

class DSLCPPCodeGenerator : public GLSLCodeGenerator {
public:
    DSLCPPCodeGenerator(const Context* context, const Program* program, ErrorReporter* errors,
                        String name, OutputStream* out);

    bool generateCode() override;

private:
    using Precedence = Operator::Precedence;

    void writeAnyConstructor(const AnyConstructor& c, Precedence parentPrecedence) override;

    void writeBlock(const Block& b);

    void writeCastConstructor(const AnyConstructor& c, Precedence parentPrecedence) override;

    void writeDoStatement(const DoStatement& d);

    void writeFloatLiteral(const FloatLiteral& f) override;

    void writeForStatement(const ForStatement& f);

    void writeFunctionBody(const Block& b);

    void writeIfStatement(const IfStatement& r) override;

    void writeReturnStatement(const ReturnStatement& r) override;

    void writeStatement(const Statement& s);

    void writeSwitchStatement(const SwitchStatement& s) override;

    void writeTernaryExpression(const TernaryExpression& t, Precedence parentPrecedence) override;

    void writeVar(const Variable& var);

    void writeVarCtorExpression(const Variable& var);

    void writeVarDeclaration(const VarDeclaration& var, bool global);

    void writef(const char* s, va_list va) SK_PRINTF_LIKE(2, 0);

    void writef(const char* s, ...) SK_PRINTF_LIKE(2, 3);

    bool writeSection(const char* name, const char* prefix = "");

    void writeHeader() override;

    void writeCppInitialValue(const Variable& var);

    bool usesPrecisionModifiers() const override;

    String getTypeName(const Type& type) override;

    String getDSLType(const Type& type);

    String getDSLModifiers(const Modifiers& type);

    String getDefaultDSLValue(const Variable& var);

    void writeSwizzle(const Swizzle& swizzle) override;

    void writeVariableReference(const VariableReference& ref) override;

    void writeFunctionCall(const FunctionCall& c) override;

    void writeFunction(const FunctionDefinition& f) override;

    void prepareHelperFunction(const FunctionDeclaration& decl);

    void prototypeHelperFunction(const FunctionDeclaration& decl);

    void writeSetting(const Setting& s) override;

    void writeProgramElement(const ProgramElement& p) override;

    void addUniform(const Variable& var);

    // writes a printf escape that will be filled in at runtime by the given C++ expression string
    String formatRuntimeValue(const Type& type, const Layout& layout, const String& cppCode,
                              std::vector<String>* formatArgs);

    void writeInputVars() override;

    void writePrivateVars();

    void writePrivateVarValues();

    bool writeEmitCode(std::vector<const Variable*>& uniforms);

    void writeSetData(std::vector<const Variable*>& uniforms);

    void writeGetKey();

    void writeClone();

    void writeDumpInfo();

    void writeTest();

    int getChildFPIndex(const Variable& var) const;

    const char* getVariableCppName(const Variable& var);

    String fName;
    String fFullName;
    SectionAndParameterHelper fSectionAndParameterHelper;
    std::unordered_map<const Variable*, String> fVariableCppNames;

    // true if the sksl declared its main() function with a float2 parameter AND referenced that
    // parameter in its body.
    bool fAccessSampleCoordsDirectly = false;

    // If true, we are writing a C++ expression instead of a GLSL expression
    bool fCPPMode = false;

    // True while compiling the main() function of the FP.
    bool fInMain = false;

    using INHERITED = GLSLCodeGenerator;
};

}  // namespace SkSL

#endif // defined(SKSL_STANDALONE) || GR_TEST_UTILS

#endif // SKSL_DSLCPPCODEGENERATOR
