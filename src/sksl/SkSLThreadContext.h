/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_THREADCONTEXT
#define SKSL_THREADCONTEXT

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLProgramSettings.h"

#include <cstdint>
#include <memory>
#include <string_view>

namespace SkSL {

class Compiler;
class Context;
class Pool;
class Position;
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

    static ThreadContext& Instance();

private:
    ThreadContext(SkSL::Context& context,
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
    SkSL::Context& fContext;
    std::unique_ptr<Pool> fPool;
    SkSL::ProgramConfig* fOldConfig;
    DefaultErrorReporter fDefaultErrorReporter;
    ErrorReporter& fOldErrorReporter;
    ProgramSettings fSettings;

    friend class SkSL::Compiler;
};

} // namespace SkSL

#endif
