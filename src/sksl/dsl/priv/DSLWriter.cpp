/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/priv/DSLWriter.h"

#include "include/sksl/DSLCore.h"
#include "include/sksl/DSLStatement.h"
#include "include/sksl/DSLSymbols.h"
#include "include/sksl/DSLVar.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {

namespace dsl {

bool DSLWriter::ManglingEnabled() {
    return ThreadContext::Instance().fSettings.fDSLMangling;
}

skstd::string_view DSLWriter::Name(skstd::string_view name) {
    if (ManglingEnabled()) {
        const String* s = ThreadContext::SymbolTable()->takeOwnershipOfString(
                ThreadContext::Instance().fMangler.uniqueName(name,
                    ThreadContext::SymbolTable().get()));
        return s->c_str();
    }
    return name;
}

const SkSL::Variable* DSLWriter::Var(DSLVarBase& var) {
    // fInitialized is true if we have attempted to create a var, whether or not we actually
    // succeeded. If it's true, we don't want to try again, to avoid reporting the same error
    // multiple times.
    if (!var.fInitialized) {
        // We haven't even attempted to create a var yet, so fVar ought to be null
        SkASSERT(!var.fVar);
        var.fInitialized = true;
        if (var.storage() != SkSL::VariableStorage::kParameter) {
            const SkSL::Type* baseType = &var.fType.skslType();
            if (baseType->isArray()) {
                baseType = &baseType->componentType();
            }
        }
        std::unique_ptr<SkSL::Variable> skslvar = SkSL::Variable::Convert(ThreadContext::Context(),
                var.fPosition.line(), var.fModifiers.fModifiers, &var.fType.skslType(), var.fName,
                /*isArray=*/false, /*arraySize=*/nullptr, var.storage());
        SkSL::Variable* varPtr = skslvar.get();
        if (var.storage() != SkSL::VariableStorage::kParameter) {
            // We can't call VarDeclaration::Convert directly here, because the IRGenerator has
            // special treatment for sk_FragColor that we want to preserve in DSL. We also do not
            // want the variable added to the symbol table for several reasons - DSLParser handles
            // the symbol table itself, parameters don't go into the symbol table until after the
            // FunctionDeclaration is created which makes this the wrong spot for them, and outside
            // of DSLParser we don't even need DSL variables to show up in the symbol table in the
            // first place.
            var.fDeclaration = VarDeclaration::Convert(ThreadContext::Context(), std::move(skslvar),
                    var.fInitialValue.releaseIfPossible(), /*addToSymbolTable=*/false);
            if (var.fDeclaration) {
                var.fVar = varPtr;
                var.fInitialized = true;
            }
        }
        ThreadContext::ReportErrors(var.fPosition);
    }
    return var.fVar;
}

std::unique_ptr<SkSL::Variable> DSLWriter::CreateParameterVar(DSLParameter& var) {
    // This should only be called on undeclared parameter variables, but we allow the creation to go
    // ahead regardless so we don't have to worry about null pointers potentially sneaking in and
    // breaking things. DSLFunction is responsible for reporting errors for invalid parameters.
    return SkSL::Variable::Convert(ThreadContext::Context(), var.fPosition.line(),
            var.fModifiers.fModifiers, &var.fType.skslType(), var.fName, /*isArray=*/false,
            /*arraySize=*/nullptr, var.storage());
}

std::unique_ptr<SkSL::Statement> DSLWriter::Declaration(DSLVarBase& var) {
    Var(var);
    if (!var.fDeclaration) {
        // We should have already reported an error before ending up here, just clean up the
        // initial value so it doesn't assert and return a nop.
        var.fInitialValue.releaseIfPossible();
        return SkSL::Nop::Make();
    }
    return std::move(var.fDeclaration);
}

void DSLWriter::MarkDeclared(DSLVarBase& var) {
    SkASSERT(!var.fDeclared);
    var.fDeclared = true;
}

bool DSLWriter::MarkVarsDeclared() {
    return ThreadContext::Instance().fSettings.fDSLMarkVarsDeclared;
}

void DSLWriter::AddVarDeclaration(DSLStatement& existing, DSLVar& additional) {
    if (existing.fStatement->is<Block>()) {
        SkSL::Block& block = existing.fStatement->as<Block>();
        SkASSERT(!block.isScope());
        block.children().push_back(Declare(additional).release());
    } else if (existing.fStatement->is<VarDeclaration>()) {
        StatementArray stmts;
        stmts.reserve_back(2);
        stmts.push_back(std::move(existing.fStatement));
        stmts.push_back(Declare(additional).release());
        existing.fStatement = SkSL::Block::MakeUnscoped(/*line=*/-1, std::move(stmts));
    } else if (existing.fStatement->isEmpty()) {
        // If the variable declaration generated an error, we can end up with a Nop statement here.
        existing.fStatement = Declare(additional).release();
    }
}

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
GrGLSLUniformHandler::UniformHandle DSLWriter::VarUniformHandle(const DSLGlobalVar& var) {
    return GrGLSLUniformHandler::UniformHandle(var.fUniformHandle);
}
#endif

void DSLWriter::Reset() {
    dsl::PopSymbolTable();
    dsl::PushSymbolTable();
    ThreadContext::ProgramElements().clear();
    ThreadContext::GetModifiersPool()->clear();
}

} // namespace dsl

} // namespace SkSL
