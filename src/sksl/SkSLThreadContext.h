/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_THREADCONTEXT
#define SKSL_THREADCONTEXT

#include "include/core/SkStringView.h"
#include "include/private/SkSLModifiers.h"
#include "src/sksl/SkSLMangler.h"
#include "src/sksl/ir/SkSLProgram.h"
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/gpu/GrFragmentProcessor.h"
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include <list>
#include <stack>

namespace SkSL {

class Compiler;
class Context;
class IRGenerator;
struct ParsedModule;
class ProgramElement;
class SymbolTable;
class Type;
class Variable;

namespace dsl {

class DSLCore;
class DSLWriter;

} // namespace dsl

/**
 * Thread-safe class that tracks per-thread state associated with SkSL output.
 */
class ThreadContext {
public:
    ThreadContext(SkSL::Compiler* compiler,  SkSL::ProgramKind kind,
              const SkSL::ProgramSettings& settings, SkSL::ParsedModule module, bool isModule);

    ~ThreadContext();

    /**
     * Returns true if the DSL has been started.
     */
    static bool IsActive();

    /**
     * Returns the Compiler used by DSL operations in the current thread.
     */
    static SkSL::Compiler& Compiler() { return *Instance().fCompiler; }

    /**
     * Returns the IRGenerator used by DSL operations in the current thread.
     */
    static SkSL::IRGenerator& IRGenerator();

    /**
     * Returns the Context used by DSL operations in the current thread.
     */
    static SkSL::Context& Context();

    /**
     * Returns the Settings used by DSL operations in the current thread.
     */
    static SkSL::ProgramSettings& Settings();

    /**
     * Returns the Program::Inputs used by the current thread.
     */
    static SkSL::Program::Inputs& Inputs();

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

    static bool IsModule() { return GetProgramConfig()->fIsBuiltinCode; }

    /**
     * Returns the final pointer to a pooled Modifiers object that should be used to represent the
     * given modifiers.
     */
    static const SkSL::Modifiers* Modifiers(const SkSL::Modifiers& modifiers);

    struct RTAdjustData {
        // Points to a standalone sk_RTAdjust variable, if one exists.
        const Variable* fVar = nullptr;
        // Points to the interface block containing an sk_RTAdjust field, if one exists.
        const Variable* fInterfaceBlock = nullptr;
        // If fInterfaceBlock is non-null, contains the index of the sk_RTAdjust field within it.
        int fFieldIndex = -1;
    };

    /**
     * Returns a struct containing information about the RTAdjust variable.
     */
    static RTAdjustData& RTAdjustState();

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
#else
    static bool InFragmentProcessor() {
        return false;
    }
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

    /**
     * Returns the ErrorReporter associated with the current thread. This object will be notified
     * when any DSL errors occur.
     */
    static ErrorReporter& GetErrorReporter() {
        return *Context().fErrors;
    }

    static void SetErrorReporter(ErrorReporter* errorReporter);

    /**
     * Notifies the current ErrorReporter that an error has occurred. The default error handler
     * prints the message to stderr and aborts.
     */
    static void ReportError(skstd::string_view msg, PositionInfo info = PositionInfo::Capture());

    /**
     * Forwards any pending errors to the DSL ErrorReporter.
     */
    static void ReportErrors(PositionInfo pos);

    static ThreadContext& Instance();

    static void SetInstance(std::unique_ptr<ThreadContext> instance);

private:
    class DefaultErrorReporter : public ErrorReporter {
        void handleError(skstd::string_view msg, PositionInfo pos) override;
    };

    std::unique_ptr<SkSL::ProgramConfig> fConfig;
    std::unique_ptr<SkSL::ModifiersPool> fModifiersPool;
    SkSL::Compiler* fCompiler;
    std::unique_ptr<Pool> fPool;
    SkSL::ProgramConfig* fOldConfig;
    SkSL::ModifiersPool* fOldModifiersPool;
    std::vector<std::unique_ptr<SkSL::ProgramElement>> fProgramElements;
    std::vector<const SkSL::ProgramElement*> fSharedElements;
    DefaultErrorReporter fDefaultErrorReporter;
    ErrorReporter& fOldErrorReporter;
    ProgramSettings fSettings;
    Mangler fMangler;
    RTAdjustData fRTAdjust;
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    struct StackFrame {
        GrFragmentProcessor::ProgramImpl* fProcessor;
        GrFragmentProcessor::ProgramImpl::EmitArgs* fEmitArgs;
        SkSL::StatementArray fSavedDeclarations;
    };
    std::stack<StackFrame, std::list<StackFrame>> fStack;
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

    friend class dsl::DSLCore;
    friend class dsl::DSLWriter;
};

} // namespace SkSL

#endif
