/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSLWRITER
#define SKSL_DSLWRITER

#include "src/sksl/SkSLModifiersPool.h"
#include "src/sksl/dsl/DSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLStatement.h"
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

#include <stack>

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
        return fStack.top().fProcessor;
    }

    GrGLSLFragmentProcessor::EmitArgs* currentEmitArgs() {
        return fStack.top().fEmitArgs;
    }

    void push(GrGLSLFragmentProcessor* processor, GrGLSLFragmentProcessor::EmitArgs* emitArgs);

    void pop();
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

    void setErrorHandler(ErrorHandler* errorHandler) {
        fErrorHandler = errorHandler;
    }

    void reportError(const char* msg);

    void reset() {
        fNameCount = 0;
    }

    void setMangle(bool mangle) {
        fMangle = mangle;
    }

    static DSLWriter& Instance();

private:
    DSLWriter(std::unique_ptr<SkSL::Compiler> compiler);

    static DSLWriter* CreateInstance();

    SkSL::Program::Settings fSettings;
    std::unique_ptr<SkSL::Compiler> fCompiler;
    int fNameCount = 0;
    std::vector<std::unique_ptr<SkSL::ProgramElement>> fProgramElements;
    ErrorHandler* fErrorHandler = nullptr;
    bool fMangle = true;
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    struct StackFrame {
        GrGLSLFragmentProcessor* fProcessor;
        GrGLSLFragmentProcessor::EmitArgs* fEmitArgs;
    };
    std::stack<StackFrame> fStack;
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
};

} // namespace dsl

} // namespace SkSL

#endif
