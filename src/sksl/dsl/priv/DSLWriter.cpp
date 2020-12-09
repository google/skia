/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/priv/DSLWriter.h"

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/gpu/mock/GrMockCaps.h"
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/DSL_core.h"

#if !SKSL_USE_THREAD_LOCAL
#include <pthread.h>
#endif // !SKSL_USE_THREAD_LOCAL

namespace SkSL {

namespace dsl {

DSLWriter::DSLWriter(std::unique_ptr<SkSL::Compiler> compiler)
    : fCompiler(std::move(compiler)) {
    SkSL::ParsedModule module = fCompiler->moduleForProgramKind(SkSL::Program::kFragment_Kind);
    SkSL::IRGenerator& ir = *fCompiler->fIRGenerator;
    ir.fSymbolTable = module.fSymbols;
    ir.pushSymbolTable();
}

SkSL::IRGenerator& DSLWriter::IRGenerator() {
    return *Compiler().fIRGenerator;
}

const SkSL::Context& DSLWriter::Context() {
    return IRGenerator().fContext;
}

const std::shared_ptr<SkSL::SymbolTable>& DSLWriter::SymbolTable() {
    return IRGenerator().fSymbolTable;
}

const SkSL::Modifiers* DSLWriter::Modifiers(SkSL::Modifiers modifiers) {
    return IRGenerator().fModifiers->addToPool(modifiers);
}

SkSL::StringFragment DSLWriter::Name(const char* name) {
    if (ManglingEnabled()) {
        const SkSL::String* s = SymbolTable()->takeOwnershipOfString(std::make_unique<SkSL::String>(
                                                         name +
                                                         SkSL::String("_") +
                                                         SkSL::to_string(++Instance().fNameCount)));
        return SkSL::StringFragment(s->c_str(), s->length());
    }
    return SkSL::StringFragment(name);
}

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
void DSLWriter::Push(GrGLSLFragmentProcessor* processor,
                     GrGLSLFragmentProcessor::EmitArgs* emitArgs) {
    Instance().fStack.push({processor, emitArgs});
    IRGenerator().pushSymbolTable();
}

void DSLWriter::Pop() {
    DSLWriter& instance = Instance();
    SkASSERT(!instance.fStack.empty());
    instance.fStack.pop();
    IRGenerator().popSymbolTable();
}
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

void DSLWriter::ReportError(const char* msg) {
    if (Instance().fErrorHandler) {
        Instance().fErrorHandler->handleError(msg);
    } else {
        SK_ABORT("%sNo SkSL DSL error handler configured, treating this as a fatal error\n", msg);
    }
}

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

DSLWriter* DSLWriter::CreateInstance() {
    static SkSL::Program::Settings settings;
    static GrMockCaps caps((GrContextOptions()), GrMockOptions());
    auto compiler = std::make_unique<SkSL::Compiler>(caps.shaderCaps());
    compiler->fInliner.reset(compiler->fIRGenerator->fModifiers.get(), &settings);
    compiler->fIRGenerator->fKind = SkSL::Program::kFragment_Kind;
    compiler->fIRGenerator->fFile = std::make_unique<SkSL::ASTFile>();
    compiler->fIRGenerator->fSettings = &settings;
    return new DSLWriter(std::move(compiler));
}

#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

#if !SK_SUPPORT_GPU || defined(SKSL_STANDALONE)

DSLWriter& DSLWriter::Instance() {
    SkUNREACHABLE;
}

#elif SKSL_USE_THREAD_LOCAL

DSLWriter& DSLWriter::Instance() {
    thread_local static DSLWriter* instance = nullptr;
    if (!instance) {
        instance = CreateInstance();
    }
    return *instance;
}

#else

static pthread_key_t get_pthread_key() {
    static pthread_key_t sKey = []{
        pthread_key_t key;
        int result = pthread_key_create(&key, /*destructor=*/nullptr);
        if (result != 0) {
            SK_ABORT("pthread_key_create failure: %d", result);
        }
        return key;
    }();
    return sKey;
}

DSLWriter& DSLWriter::Instance() {
    DSLWriter* instance = static_cast<DSLWriter*>(pthread_getspecific(get_pthread_key()));
    if (!instance) {
        instance = CreateInstance();
        pthread_setspecific(get_pthread_key(), instance);
    }
    return *instance;
}

#endif

} // namespace dsl

} // namespace SkSL
