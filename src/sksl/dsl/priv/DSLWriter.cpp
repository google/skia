/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/priv/DSLWriter.h"

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/DSLExpression.h"
#include "src/sksl/dsl/DSLStatement.h"
#include "src/sksl/dsl/DSLType.h"
#include "src/sksl/dsl/DSLVar.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <utility>

namespace SkSL {
namespace dsl {

SkSL::Variable* DSLWriter::Var(DSLVarBase& var) {
    SkASSERT(var.fStorage != SkSL::VariableStorage::kParameter);  // use CreateParameterVar instead

    // We haven't attempted to create a var yet; fVar and fDeclaration ought to be null.
    SkASSERT(!var.fVar);
    SkASSERT(!var.fDeclaration);

    std::unique_ptr<SkSL::Variable> skslvar = SkSL::Variable::Convert(ThreadContext::Context(),
                                                                      var.fPosition,
                                                                      var.fModifiersPos,
                                                                      var.fModifiers,
                                                                      &var.fType.skslType(),
                                                                      var.fNamePosition,
                                                                      var.fName,
                                                                      var.fStorage);
    var.fDeclaration = VarDeclaration::Convert(ThreadContext::Context(),
                                               std::move(skslvar),
                                               var.fInitialValue.releaseIfPossible());
    if (var.fDeclaration) {
        var.fVar = var.fDeclaration->as<VarDeclaration>().var();
    }
    return var.fVar;
}

std::unique_ptr<SkSL::Variable> DSLWriter::CreateParameterVar(DSLParameter& var) {
    // This should only be called on undeclared parameter variables, but we allow the creation to go
    // ahead regardless so we don't have to worry about null pointers potentially sneaking in and
    // breaking things. DSLFunction is responsible for reporting errors for invalid parameters.
    return SkSL::Variable::Convert(ThreadContext::Context(),
                                   var.fPosition,
                                   var.fModifiersPos,
                                   var.fModifiers,
                                   &var.fType.skslType(),
                                   var.fNamePosition,
                                   var.fName,
                                   var.fStorage);
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
    existing = DSLStatement(Block::MakeCompoundStatement(existing.releaseIfPossible(),
                                                         Declaration(additional)));
}

} // namespace dsl
} // namespace SkSL
