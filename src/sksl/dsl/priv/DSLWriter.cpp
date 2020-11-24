/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/priv/DSLWriter.h"

#if SK_SUPPORT_GPU
#include "src/gpu/mock/GrMockCaps.h"
#endif // SK_SUPPORT_GPU
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/DSL.h"

namespace SkSL {

namespace dsl {

DSLWriter::~DSLWriter() {
    SkASSERT(fVars.count() == 0);
}

SkSL::IRGenerator& DSLWriter::irGenerator() {
    return *fCompiler.fIRGenerator;
}

const SkSL::Context& DSLWriter::context() {
    return this->irGenerator().fContext;
}

const std::shared_ptr<SkSL::SymbolTable>& DSLWriter::symbolTable() {
    SkASSERT(fCompiler.fIRGenerator->fSymbolTable);
    return fCompiler.fIRGenerator->fSymbolTable;
}

const SkSL::Modifiers* DSLWriter::modifiers(SkSL::Modifiers modifiers) {
    return fCompiler.fIRGenerator->fModifiers->addToPool(modifiers);
}

SkSL::String DSLWriter::name(const char* name) {
    return name + SkSL::String("_") + SkSL::to_string(++fNameCount);
}

void DSLWriter::setCurrentEmitArgs(GrGLSLFragmentProcessor::EmitArgs* emitArgs) {
    fEmitArgs = emitArgs;
    if (emitArgs) {
        sk_SampleCoord.fName = emitArgs->fSampleCoord;
        sk_InColor.fName = emitArgs->fInputColor;
        sk_OutColor.fName = emitArgs->fOutputColor;
    } else {
        sk_SampleCoord.fName = "<uninitialized>";
        sk_InColor.fName = "<uninitialized>";
        sk_OutColor.fName = "<uninitialized>";

    }
}

void DSLWriter::reportError(const char* msg) {
    if (fErrorHandler) {
        fErrorHandler->handle(msg);
    } else {
        SK_ABORT("%sNo SkSL DSL error handler configured, treating this as a fatal error\n", msg);
    }
}

DSLWriter& DSLWriter::Instance() {
#if SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)
    static SkSL::Program::Settings settings;
    static GrMockCaps caps((GrContextOptions()), GrMockOptions());
    thread_local static DSLWriter* instance = nullptr;
    thread_local static SkSL::Compiler* compiler = nullptr;
    if (!instance) {
        compiler = new SkSL::Compiler(caps.shaderCaps());
        compiler->fInliner.reset(compiler->fIRGenerator->fModifiers.get(), &settings);
        compiler->fIRGenerator->fKind = SkSL::Program::kFragment_Kind;
        compiler->fIRGenerator->fFile = std::make_unique<SkSL::ASTFile>();
        compiler->fIRGenerator->fSettings = &settings;
        instance = new DSLWriter(compiler);
    }
    return *instance;
#else
    SkUNREACHABLE;
#endif // SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)
}

std::unique_ptr<SkSL::Statement> Statement(std::unique_ptr<SkSL::Statement> stmt) {
    return stmt;
}

std::unique_ptr<SkSL::Statement> Statement(Expression expr) {
    return std::make_unique<SkSL::ExpressionStatement>(expr.release());
}

} // namespace dsl

} // namespace SkSL
