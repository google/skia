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

/**
 * Thread-safe class that tracks per-thread state associated with DSL output. This class is for
 * internal use only.
 */
class DSLWriter {
public:
    /**
     * Returns the Compiler used by DSL operations in the current thread.
     */
    static SkSL::Compiler& Compiler() {
        return *Instance().fCompiler;
    }

    /**
     * Returns the IRGenerator used by DSL operations in the current thread.
     */
    static SkSL::IRGenerator& IRGenerator();

    /**
     * Returns the Context used by DSL operations in the current thread.
     */
    static const SkSL::Context& Context();

    /**
     * Returns the SymbolTable of the current thread's IRGenerator.
     */
    static const std::shared_ptr<SkSL::SymbolTable>& SymbolTable();

    /**
     * Returns the Compiler used by DSL operations in the current thread.
     */
    static const SkSL::Modifiers* Modifiers(SkSL::Modifiers modifiers);

    /**
     * Returns the (possibly mangled) final name that should be used for an entity with the given
     * raw name.
     */
    static SkSL::StringFragment Name(const char* name);

    /**
     * Returns the collection to which DSL program elements in this thread should be appended.
     */
    static std::vector<std::unique_ptr<SkSL::ProgramElement>>& ProgramElements() {
        return Instance().fProgramElements;
    }

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    /**
     * Returns the fragment processor for which DSL output is being generated for the current
     * thread.
     */
    static GrGLSLFragmentProcessor* CurrentProcessor() {
        SkASSERTF(!Instance().fStack.empty(), "This feature requires a FragmentProcessor");
        return Instance().fStack.top().fProcessor;
    }

    /**
     * Returns the EmitArgs for fragment processor output in the current thread.
     */
    static GrGLSLFragmentProcessor::EmitArgs* CurrentEmitArgs() {
        SkASSERTF(!Instance().fStack.empty(), "This feature requires a FragmentProcessor");
        return Instance().fStack.top().fEmitArgs;
    }

    /**
     * Pushes a new processor / emitArgs pair for the current thread.
     */
    static void Push(GrGLSLFragmentProcessor* processor,
                     GrGLSLFragmentProcessor::EmitArgs* emitArgs);

    /**
     * Pops the processor / emitArgs pair associated with the current thread.
     */
    static void Pop();
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

    /**
     * Sets the ErrorHandler associated with the current thread. This object will be notified when
     * any DSL errors occur. With a null ErrorHandler (the default), any errors will be dumped to
     * stderr and a fatal exception will be generated.
     */
    static void SetErrorHandler(ErrorHandler* errorHandler) {
        Instance().fErrorHandler = errorHandler;
    }

    /**
     * Notifies the current ErrorHandler that a DSL error has occurred. With a null ErrorHandler
     * (the default), any errors will be dumped to stderr and a fatal exception will be generated.
     */
    static void ReportError(const char* msg);

    /**
     * Readies the DSLWriter to begin outputting a new top-level FragmentProcessor.
     */
    static void Reset() {
        Instance().fNameCount = 0;
    }

    /**
     * Returns whether name mangling is enabled.
     */
    static bool ManglingEnabled() {
        return Instance().fMangle;
    }

    /**
     * Enables or disables name mangling. Mangling should always be enabling except for tests which
     * need to guarantee consistent output.
     */
    static void SetManglingEnabled(bool mangle) {
        Instance().fMangle = mangle;
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
