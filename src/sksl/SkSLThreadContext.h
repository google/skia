/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_THREADCONTEXT
#define SKSL_THREADCONTEXT

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLProgram.h"

#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

namespace SkSL {

class Compiler;
class Pool;
class ProgramElement;
class Variable;
enum class ProgramKind : int8_t;
struct Module;

/**
 * Thread-safe class that tracks per-thread state associated with SkSL output.
 */
class ThreadContext {
public:
    ~ThreadContext();

    /**
     * Initializes our thread-local state for compiling a program.
     */
    static void Start(SkSL::Compiler* compiler,
                      SkSL::ProgramKind kind,
                      const SkSL::ProgramSettings& settings);

    /**
     * Initializes our thread-local state for compiling a module (SkSL include files).
     */
    static void StartModule(SkSL::Compiler* compiler,
                            SkSL::ProgramKind kind,
                            const SkSL::ProgramSettings& settings,
                            const SkSL::Module* parentModule);

    /**
     * Signals the end of compilation. This must be called sometime after a call to Start() and
     * before the termination of the thread.
     */
    static void End();

    /**
     * Returns the Compiler used by SkSL in the current thread.
     */
    static SkSL::Compiler& Compiler() { return *Instance().fCompiler; }

    /**
     * Returns the Context used by SkSL in the current thread.
     */
    static SkSL::Context& Context();

    /**
     * Returns the collection to which SkSL program elements in this thread should be appended.
     */
    static std::vector<std::unique_ptr<SkSL::ProgramElement>>& ProgramElements() {
        return Instance().fProgramElements;
    }

    static std::vector<const ProgramElement*>& SharedElements() {
        return Instance().fSharedElements;
    }

    /**
     * Returns the current ProgramConfig.
     */
    static bool IsModule() { return Instance().fConfig->fIsBuiltinCode; }

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
     * when any compilation errors occur.
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

private:
    ThreadContext(SkSL::Compiler* compiler,
                  SkSL::ProgramKind kind,
                  const SkSL::ProgramSettings& settings,
                  const SkSL::Module* module,
                  bool isModule);

    static void SetInstance(std::unique_ptr<ThreadContext>);

    class DefaultErrorReporter : public ErrorReporter {
        void handleError(std::string_view msg, Position pos) override;
    };

    void setupSymbolTable();

    std::unique_ptr<SkSL::ProgramConfig> fConfig;
    SkSL::Compiler* fCompiler;
    std::unique_ptr<Pool> fPool;
    SkSL::ProgramConfig* fOldConfig;
    std::vector<std::unique_ptr<SkSL::ProgramElement>> fProgramElements;
    std::vector<const SkSL::ProgramElement*> fSharedElements;
    DefaultErrorReporter fDefaultErrorReporter;
    ErrorReporter& fOldErrorReporter;
    ProgramSettings fSettings;
    RTAdjustData fRTAdjust;
    Program::Interface fInterface;

    friend class Compiler;
};

} // namespace SkSL

#endif
