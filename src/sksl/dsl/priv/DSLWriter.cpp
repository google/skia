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
    std::unique_ptr<SkSL::Statement> decl =
            VarDeclaration::Convert(ThreadContext::Context(),
                                    var.fPosition,
                                    var.fModifiersPos,
                                    var.fModifiers,
                                    var.fType.skslType(),
                                    var.fNamePosition,
                                    var.fName,
                                    var.fStorage,
                                    var.fInitialValue.releaseIfPossible());
    return decl ? std::move(decl)
                : SkSL::Nop::Make();
}

void DSLWriter::AddVarDeclaration(DSLStatement& existing, DSLVar& additional) {
    existing = DSLStatement(Block::MakeCompoundStatement(existing.releaseIfPossible(),
                                                         Declaration(additional)));
}

} // namespace dsl
} // namespace SkSL
