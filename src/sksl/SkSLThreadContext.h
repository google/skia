/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_THREADCONTEXT
#define SKSL_THREADCONTEXT

#include "include/core/SkTypes.h"
#include "include/sksl/SkSLErrorReporter.h"
#include "include/sksl/SkSLPosition.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLProgram.h"

#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

namespace SkSL {

class Compiler;
class ModifiersPool;
class Pool;
class ProgramElement;
class SymbolTable;
class Variable;
enum class ProgramKind : int8_t;
struct Modifiers;
struct Module;

namespace dsl {

class DSLCore;

} // namespace dsl

/**
 * Thread-safe class that tracks per-thread state associated with SkSL output.
 */
class ThreadContext {
public:
    ThreadContext(SkSL::Compiler* compiler,
                  SkSL::ProgramKind kind,
                  const SkSL::ProgramSettings& settings,
                  const SkSL::Module* module,
                  bool isModule);
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
     * Returns the Context used by DSL operations in the current thread.
     */
    static SkSL::Context& Context();

    /**
     * Returns the Settings used by DSL operations in the current thread.
     */
    static const SkSL::ProgramSettings& Settings();

    /**
     * Returns the Program::Inputs used by the current thread.
     */
    static SkSL::Program::Inputs& Inputs() { return Instance().fInputs; }

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
     * Returns the current SymbolTable.
     */
    static std::shared_ptr<SkSL::SymbolTable>& SymbolTable();

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
    static const std::unique_ptr<ProgramConfig>& GetProgramConfig() { return Instance().fConfig; }

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
    static void ReportError(std::string_view msg, Position pos = Position{});

    static ThreadContext& Instance();

    static void SetInstance(std::unique_ptr<ThreadContext> instance);

private:
    class DefaultErrorReporter : public ErrorReporter {
        void handleError(std::string_view msg, Position pos) override;
    };

    void setupSymbolTable();

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
    RTAdjustData fRTAdjust;
    Program::Inputs fInputs;

    friend class dsl::DSLCore;
};

} // namespace SkSL

#endif
