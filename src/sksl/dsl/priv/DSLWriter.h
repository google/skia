/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSLWRITER
#define SKSL_DSLWRITER

#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLStatement.h"
#include "include/sksl/DSLExpression.h"
#include "include/sksl/DSLStatement.h"
#include "src/sksl/SkSLMangler.h"
#include "src/sksl/SkSLOperators.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLProgram.h"
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include <stack>

class AutoDSLContext;

namespace SkSL {

class Compiler;
class Context;
class IRGenerator;
class ProgramElement;
class SymbolTable;
class Type;
class Variable;

namespace dsl {

class ErrorHandler;

/**
 * Thread-safe class that tracks per-thread state associated with DSL output. This class is for
 * internal use only.
 */
class DSLWriter {
public:
    DSLWriter(SkSL::Compiler* compiler);

    ~DSLWriter();

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
     * Returns the collection to which DSL program elements in this thread should be appended.
     */
    static std::vector<std::unique_ptr<SkSL::ProgramElement>>& ProgramElements() {
        return Instance().fProgramElements;
    }

    /**
     * Returns the SymbolTable of the current thread's IRGenerator.
     */
    static const std::shared_ptr<SkSL::SymbolTable>& SymbolTable();

    static void Reset();

    /**
     * Returns the final pointer to a pooled Modifiers object that should be used to represent the
     * given modifiers.
     */
    static const SkSL::Modifiers* Modifiers(SkSL::Modifiers modifiers);

    /**
     * Returns the SkSL variable corresponding to a DSLVar.
     */
    static const SkSL::Variable& Var(const DSLVar& var);

    /**
     * For use in testing only: marks the variable as having been declared, so that it can be
     * destroyed without generating errors.
     */
    static void MarkDeclared(DSLVar& var);

    /**
     * Returns the (possibly mangled) final name that should be used for an entity with the given
     * raw name.
     */
    static const char* Name(const char* name);

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

    static bool InFragmentProcessor() {
        return !Instance().fStack.empty();
    }

    /**
     * Pushes a new processor / emitArgs pair for the current thread.
     */
    static void StartFragmentProcessor(GrGLSLFragmentProcessor* processor,
                                       GrGLSLFragmentProcessor::EmitArgs* emitArgs);

    /**
     * Pops the processor / emitArgs pair associated with the current thread.
     */
    static void EndFragmentProcessor();

    static GrGLSLUniformHandler::UniformHandle VarUniformHandle(const DSLVar& var);
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

    static std::unique_ptr<SkSL::Expression> Call(const FunctionDeclaration& function,
                                                  ExpressionArray arguments);

    /**
     * Reports an error if the argument is null. Returns its argument unmodified.
     */
    static std::unique_ptr<SkSL::Expression> Check(std::unique_ptr<SkSL::Expression> expr);

    static DSLPossibleExpression Coerce(std::unique_ptr<Expression> left, const SkSL::Type& type);

    static DSLPossibleExpression Construct(const SkSL::Type& type,
                                           std::vector<DSLExpression> rawArgs);

    static std::unique_ptr<Expression> ConvertBinary(std::unique_ptr<Expression> left, Operator op,
                                                     std::unique_ptr<Expression> right);

    static std::unique_ptr<SkSL::Expression> ConvertField(std::unique_ptr<Expression> base,
                                                          const char* name);

    static std::unique_ptr<Expression> ConvertIndex(std::unique_ptr<Expression> base,
                                                    std::unique_ptr<Expression> index);

    static std::unique_ptr<Expression> ConvertPostfix(std::unique_ptr<Expression> expr,
                                                      Operator op);

    static std::unique_ptr<Expression> ConvertPrefix(Operator op, std::unique_ptr<Expression> expr);

    static DSLPossibleStatement ConvertSwitch(std::unique_ptr<Expression> value,
                                              ExpressionArray caseValues,
                                              SkTArray<SkSL::StatementArray> caseStatements);

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
    static void ReportError(const char* msg, PositionInfo* info = nullptr);

    /**
     * Returns whether name mangling is enabled. This should always be enabled outside of tests.
     */
    static bool ManglingEnabled() {
        return Instance().fMangle;
    }

    static DSLWriter& Instance();

    static void SetInstance(std::unique_ptr<DSLWriter> instance);

private:
    SkSL::ProgramConfig fConfig;
    SkSL::Compiler* fCompiler;
    std::unique_ptr<Pool> fPool;
    std::shared_ptr<SkSL::SymbolTable> fOldSymbolTable;
    SkSL::ProgramConfig* fOldConfig;
    std::vector<std::unique_ptr<SkSL::ProgramElement>> fProgramElements;
    ErrorHandler* fErrorHandler = nullptr;
    bool fMangle = true;
    bool fMarkVarsDeclared = false;
    Mangler fMangler;
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    struct StackFrame {
        GrGLSLFragmentProcessor* fProcessor;
        GrGLSLFragmentProcessor::EmitArgs* fEmitArgs;
    };
    std::stack<StackFrame> fStack;
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

    friend class DSLCore;
    friend class DSLVar;
    friend class ::AutoDSLContext;
};

} // namespace dsl

} // namespace SkSL

#endif
