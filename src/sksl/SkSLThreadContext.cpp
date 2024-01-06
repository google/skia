/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLThreadContext.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLPool.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

#include <type_traits>

namespace SkSL {

ThreadContext::ThreadContext(SkSL::Context& context,
                             const SkSL::Module* module,
                             SkSL::ProgramKind kind,
                             const SkSL::ProgramSettings& settings,
                             std::string_view source,
                             bool isModule)
        : fContext(context)
        , fSettings(settings) {
    SkASSERT(!fContext.fConfig);  // recursive ThreadContext creation is disallowed

    if (!isModule) {
        if (settings.fUseMemoryPool) {
            fPool = Pool::Create();
            fPool->attachToThread();
        }
    }

    fConfig = std::make_unique<SkSL::ProgramConfig>();
    fConfig->fKind = kind;
    fConfig->fSettings = settings;
    fConfig->fIsBuiltinCode = isModule;
    fContext.fConfig = fConfig.get();
    fContext.fModule = module;
    fContext.fSymbolTable = module->fSymbols;
    this->setupSymbolTable();
    fContext.fErrors->setSource(source);
}

ThreadContext::~ThreadContext() {
    if (fContext.fSymbolTable) {
        fContext.fSymbolTable = nullptr;
    }
    fContext.fConfig = nullptr;
    fContext.fErrors->setSource(std::string_view());
    if (fPool) {
        fPool->detachFromThread();
    }
}

void ThreadContext::Start(SkSL::Context& context,
                          const SkSL::Module* module,
                          SkSL::ProgramKind kind,
                          const SkSL::ProgramSettings& settings,
                          std::string_view source) {
    ThreadContext::SetInstance(
            std::unique_ptr<ThreadContext>(new ThreadContext(context,
                                                             module,
                                                             kind,
                                                             settings,
                                                             source,
                                                             /*isModule=*/false)));
}

void ThreadContext::StartModule(SkSL::Context& context,
                                const SkSL::Module* parentModule,
                                SkSL::ProgramKind kind,
                                const SkSL::ProgramSettings& settings,
                                std::string_view source) {
    ThreadContext::SetInstance(
            std::unique_ptr<ThreadContext>(new ThreadContext(context,
                                                             parentModule,
                                                             kind,
                                                             settings,
                                                             source,
                                                             /*isModule=*/true)));
}

void ThreadContext::End() {
    ThreadContext::SetInstance(nullptr);
}

void ThreadContext::setupSymbolTable() {
    SymbolTable::Push(&fContext.fSymbolTable, fContext.fConfig->fIsBuiltinCode);

    fContext.fSymbolTable->markModuleBoundary();
}

static thread_local ThreadContext* sInstance = nullptr;

void ThreadContext::SetInstance(std::unique_ptr<ThreadContext> newInstance) {
    SkASSERT((sInstance == nullptr) != (newInstance == nullptr));
    delete sInstance;
    sInstance = newInstance.release();
}

ThreadContext& ThreadContext::Instance() {
    SkASSERTF(sInstance, "ThreadContext::Start() has not been called");
    return *sInstance;
}

} // namespace SkSL
