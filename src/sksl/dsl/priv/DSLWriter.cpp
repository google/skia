/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/priv/DSLWriter.h"

#include "include/core/SkTypes.h"
#include "include/private/SkSLDefines.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "include/sksl/DSLCore.h"
#include "include/sksl/DSLExpression.h"
#include "include/sksl/DSLModifiers.h"
#include "include/sksl/DSLStatement.h"
#include "include/sksl/DSLType.h"
#include "include/sksl/DSLVar.h"
#include "include/sksl/SkSLPosition.h"
#include "src/sksl/SkSLModifiersPool.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <utility>
#include <vector>

namespace SkSL {

namespace dsl {

SkSL::Variable* DSLWriter::Var(DSLVarBase& var) {
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
                                                                          var.fPosition,
                                                                          var.fModifiers.fPosition,
                                                                          var.fModifiers.fModifiers,
                                                                          &var.fType.skslType(),
                                                                          var.fNamePosition,
                                                                          var.fName,
                                                                          /*isArray=*/false,
                                                                          /*arraySize=*/nullptr,
                                                                          var.storage());
        SkSL::Variable* varPtr = skslvar.get();
        if (var.storage() != SkSL::VariableStorage::kParameter) {
            var.fDeclaration = VarDeclaration::Convert(ThreadContext::Context(),
                                                       std::move(skslvar),
                                                       var.fInitialValue.releaseIfPossible(),
                                                       /*addToSymbolTable=*/false);
            if (var.fDeclaration) {
                var.fVar = varPtr;
                var.fInitialized = true;
            }
        }
    }
    return var.fVar;
}

std::unique_ptr<SkSL::Variable> DSLWriter::CreateParameterVar(DSLParameter& var) {
    // This should only be called on undeclared parameter variables, but we allow the creation to go
    // ahead regardless so we don't have to worry about null pointers potentially sneaking in and
    // breaking things. DSLFunction is responsible for reporting errors for invalid parameters.
    return SkSL::Variable::Convert(ThreadContext::Context(),
                                   var.fPosition,
                                   var.fModifiers.fPosition,
                                   var.fModifiers.fModifiers,
                                   &var.fType.skslType(),
                                   var.fNamePosition,
                                   var.fName,
                                   /*isArray=*/false,
                                   /*arraySize=*/nullptr,
                                   var.storage());
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

void DSLWriter::AddVarDeclaration(DSLStatement& existing, DSLVar& additional) {
    if (existing.fStatement->is<Block>()) {
        SkSL::Block& block = existing.fStatement->as<Block>();
        SkASSERT(!block.isScope());
        block.children().push_back(Declare(additional).release());
    } else if (existing.fStatement->is<VarDeclaration>()) {
        Position pos = existing.fStatement->fPosition;
        StatementArray stmts;
        stmts.reserve_back(2);
        stmts.push_back(std::move(existing.fStatement));
        stmts.push_back(Declare(additional).release());
        existing.fStatement = SkSL::Block::Make(pos, std::move(stmts),
                                                Block::Kind::kCompoundStatement);
    } else if (existing.fStatement->isEmpty()) {
        // If the variable declaration generated an error, we can end up with a Nop statement here.
        existing.fStatement = Declare(additional).release();
    }
}

void DSLWriter::Reset() {
    SymbolTable::Pop(&ThreadContext::SymbolTable());
    SymbolTable::Push(&ThreadContext::SymbolTable());
    ThreadContext::ProgramElements().clear();
    ThreadContext::GetModifiersPool()->clear();
}

} // namespace dsl

} // namespace SkSL
