/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSLWRITER
#define SKSL_DSLWRITER

#include "include/core/SkSpan.h"
#include "include/core/SkStringView.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLStatement.h"
#include "include/sksl/DSLExpression.h"
#include "include/sksl/DSLStatement.h"
#include "src/sksl/SkSLMangler.h"
#include "src/sksl/SkSLOperators.h"
#include "src/sksl/SkSLParsedModule.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLProgram.h"
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/gpu/GrFragmentProcessor.h"
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include <list>
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

class DSLGlobalVar;
class DSLParameter;
class DSLVar;
class ErrorHandler;

/**
 * Thread-safe class that tracks per-thread state associated with DSL output. This class is for
 * internal use only.
 */
class DSLWriter {
public:
    DSLWriter(SkSL::Compiler* compiler, SkSL::ProgramKind kind,
              const SkSL::ProgramSettings& settings, SkSL::ParsedModule module, bool isModule);

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
     * Returns the Settings used by DSL operations in the current thread.
     */
    static SkSL::ProgramSettings& Settings();

    /**
     * Returns the collection to which DSL program elements in this thread should be appended.
     */
    static std::vector<std::unique_ptr<SkSL::ProgramElement>>& ProgramElements() {
        return Instance().fProgramElements;
    }

    static std::vector<const ProgramElement*>& SharedElements() {
        return Instance().fSharedElements;
    }

    /**
     * Returns the SymbolTable of the current thread's IRGenerator.
     */
    static const std::shared_ptr<SkSL::SymbolTable>& SymbolTable();

    /**
     * Returns the current memory pool.
     */
    static std::unique_ptr<Pool>& MemoryPool() { return Instance().fPool; }

    /**
     * Returns the current modifiers pool.
     */
    static std::unique_ptr<ModifiersPool>& GetModifiersPool() { return Instance().fModifiersPool; }

    /**
     * Returns the current ProgramConfig.
     */
    static std::unique_ptr<ProgramConfig>& GetProgramConfig() { return Instance().fConfig; }

    static bool IsModule() { return Instance().fIsModule; }

    static void Reset();

    /**
     * Returns the final pointer to a pooled Modifiers object that should be used to represent the
     * given modifiers.
     */
    static const SkSL::Modifiers* Modifiers(const SkSL::Modifiers& modifiers);

    /**
     * Returns the SkSL variable corresponding to a DSL var.
     */
    static const SkSL::Variable* Var(DSLVarBase& var);

    /**
     * Creates an SkSL variable corresponding to a DSLParameter.
     */
    static std::unique_ptr<SkSL::Variable> CreateParameterVar(DSLParameter& var);


    /**
     * Returns the SkSL declaration corresponding to a DSLVar.
     */
    static std::unique_ptr<SkSL::Statement> Declaration(DSLVarBase& var);

    /**
     * For use in testing only: marks the variable as having been declared, so that it can be
     * destroyed without generating errors.
     */
    static void MarkDeclared(DSLVarBase& var);

    /**
     * Returns the (possibly mangled) final name that should be used for an entity with the given
     * raw name.
     */
    static skstd::string_view Name(skstd::string_view name);

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    /**
     * Returns the fragment processor for which DSL output is being generated for the current
     * thread.
     */
    static GrFragmentProcessor::ProgramImpl* CurrentProcessor() {
        SkASSERTF(!Instance().fStack.empty(), "This feature requires a FragmentProcessor");
        return Instance().fStack.top().fProcessor;
    }

    /**
     * Returns the EmitArgs for fragment processor output in the current thread.
     */
    static GrFragmentProcessor::ProgramImpl::EmitArgs* CurrentEmitArgs() {
        SkASSERTF(!Instance().fStack.empty(), "This feature requires a FragmentProcessor");
        return Instance().fStack.top().fEmitArgs;
    }

    static bool InFragmentProcessor() {
        return !Instance().fStack.empty();
    }

    /**
     * Pushes a new processor / emitArgs pair for the current thread.
     */
    static void StartFragmentProcessor(GrFragmentProcessor::ProgramImpl* processor,
                                       GrFragmentProcessor::ProgramImpl::EmitArgs* emitArgs);

    /**
     * Pops the processor / emitArgs pair associated with the current thread.
     */
    static void EndFragmentProcessor();

    static GrGLSLUniformHandler::UniformHandle VarUniformHandle(const DSLGlobalVar& var);
#else
    static bool InFragmentProcessor() {
        return false;
    }
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

    static std::unique_ptr<SkSL::Expression> Call(const FunctionDeclaration& function,
                                                  ExpressionArray arguments);

    /**
     * Invokes expr(arguments), where expr is a function or type reference.
     */
    static std::unique_ptr<SkSL::Expression> Call(std::unique_ptr<SkSL::Expression> expr,
                                                  ExpressionArray arguments);

    static DSLPossibleExpression Coerce(std::unique_ptr<Expression> expr, const SkSL::Type& type);

    static DSLPossibleExpression Construct(const SkSL::Type& type, SkSpan<DSLExpression> rawArgs);

    static std::unique_ptr<Expression> ConvertBinary(std::unique_ptr<Expression> left, Operator op,
                                                     std::unique_ptr<Expression> right);

    static std::unique_ptr<SkSL::Expression> ConvertField(std::unique_ptr<Expression> base,
                                                          skstd::string_view name);

    static std::unique_ptr<Expression> ConvertIndex(std::unique_ptr<Expression> base,
                                                    std::unique_ptr<Expression> index);

    static std::unique_ptr<Expression> ConvertPostfix(std::unique_ptr<Expression> expr,
                                                      Operator op);

    static std::unique_ptr<Expression> ConvertPrefix(Operator op, std::unique_ptr<Expression> expr);

    static DSLPossibleStatement ConvertSwitch(std::unique_ptr<Expression> value,
                                              ExpressionArray caseValues,
                                              SkTArray<SkSL::StatementArray> caseStatements,
                                              bool isStatic);

    /**
     * Returns the ErrorHandler associated with the current thread. This object will be notified
     * when any DSL errors occur. With a null ErrorHandler (the default), any errors will be dumped
     * to stderr and a fatal exception will be generated.
     */
    static ErrorHandler* GetErrorHandler() {
        return Instance().fErrorHandler;
    }

    static void SetErrorHandler(ErrorHandler* errorHandler) {
        Instance().fErrorHandler = errorHandler;
    }

    /**
     * Notifies the current ErrorHandler that a DSL error has occurred. With a null ErrorHandler
     * (the default), any errors will be dumped to stderr and a fatal exception will be generated.
     */
    static void ReportError(const char* msg, PositionInfo info = PositionInfo());

    /**
     * Returns whether name mangling is enabled. Mangling is important for the DSL because its
     * variables normally all go into the same symbol table; for instance if you were to translate
     * this legal (albeit silly) GLSL code:
     *     int x;
     *     {
     *         int x;
     *     }
     *
     * into DSL, you'd end up with:
     *     DSLVar x1(kInt_Type, "x");
     *     DSLVar x2(kInt_Type, "x");
     *     Declare(x1);
     *     Block(Declare(x2));
     *
     * with x1 and x2 ending up in the same symbol table. This is fine as long as their effective
     * names are different, so mangling prevents this situation from causing problems.
     */
    static bool ManglingEnabled() {
        return Instance().fSettings.fDSLMangling;
    }

    /**
     * Returns whether DSLVars should automatically be marked declared upon creation. This is used
     * to simplify testing.
     */
    static bool MarkVarsDeclared() {
        return Instance().fSettings.fDSLMarkVarsDeclared;
    }

    /**
     * Forwards any pending Compiler errors to the DSL ErrorHandler.
     */
    static void ReportErrors(PositionInfo pos = PositionInfo());

    static DSLWriter& Instance();

    static void SetInstance(std::unique_ptr<DSLWriter> instance);

private:
    std::unique_ptr<SkSL::ProgramConfig> fConfig;
    std::unique_ptr<SkSL::ModifiersPool> fModifiersPool;
    SkSL::Compiler* fCompiler;
    std::unique_ptr<Pool> fPool;
    SkSL::ProgramConfig* fOldConfig;
    SkSL::ModifiersPool* fOldModifiersPool;
    std::vector<std::unique_ptr<SkSL::ProgramElement>> fProgramElements;
    std::vector<const SkSL::ProgramElement*> fSharedElements;
    ErrorHandler* fErrorHandler = nullptr;
    ProgramSettings fSettings;
    Mangler fMangler;
    bool fIsModule;
    bool fEncounteredErrors = false;
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    struct StackFrame {
        GrFragmentProcessor::ProgramImpl* fProcessor;
        GrFragmentProcessor::ProgramImpl::EmitArgs* fEmitArgs;
        SkSL::StatementArray fSavedDeclarations;
    };
    std::stack<StackFrame, std::list<StackFrame>> fStack;
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

    friend class DSLCore;
    friend class DSLVar;
};

} // namespace dsl

} // namespace SkSL

#endif
