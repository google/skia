/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSLWRITER
#define SKSL_DSLWRITER

#include "src/sksl/SkSLModifiersPool.h"
#include "src/sksl/dsl/Expression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLStatement.h"
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

namespace SkSL {

class Compiler;
class Context;
class IRGenerator;
class SymbolTable;
class Type;

namespace dsl {

class ErrorHandler;

class DSLWriter {
public:
    SkSL::Compiler& compiler() {
        return *fCompiler;
    }

    SkSL::IRGenerator& irGenerator();

    const SkSL::Context& context();

    const std::shared_ptr<SkSL::SymbolTable>& symbolTable();

    const SkSL::Modifiers* modifiers(SkSL::Modifiers modifiers);

    SkSL::String name(const char* name);

    std::vector<std::unique_ptr<SkSL::ProgramElement>>& programElements() {
        return fProgramElements;
    }

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
     GrGLSLFragmentProcessor* currentProcessor() {
        return fCurrentProcessor;
    }

    void setCurrentProcessor(GrGLSLFragmentProcessor* processor) {
        fCurrentProcessor = processor;
    }

    GrGLSLFragmentProcessor::EmitArgs* currentEmitArgs() {
        return fEmitArgs;
    }

    void setCurrentEmitArgs(GrGLSLFragmentProcessor::EmitArgs* emitArgs);
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

    void setErrorHandler(ErrorHandler* errorHandler) {
        fErrorHandler = errorHandler;
    }

    void reportError(const char* msg);

    void reset() {
        fNameCount = 0;
    }

    static DSLWriter& Instance();

private:
    DSLWriter(std::unique_ptr<SkSL::Compiler> compiler)
        : fCompiler(std::move(compiler)) {
        SkSL::ParsedModule module = fCompiler->moduleForProgramKind(SkSL::Program::kFragment_Kind);
        SkSL::IRGenerator& ir = this->irGenerator();
        ir.fSymbolTable = module.fSymbols;
        ir.pushSymbolTable();
    }

    static DSLWriter* CreateInstance();

    SkSL::Program::Settings fSettings;
    std::unique_ptr<SkSL::Compiler> fCompiler;
    int fNameCount = 0;
    std::vector<std::unique_ptr<SkSL::ProgramElement>> fProgramElements;
    ErrorHandler* fErrorHandler = nullptr;
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    GrGLSLFragmentProcessor* fCurrentProcessor = nullptr;
    GrGLSLFragmentProcessor::EmitArgs* fEmitArgs = nullptr;
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
};

} // namespace dsl

} // namespace SkSL

#endif
