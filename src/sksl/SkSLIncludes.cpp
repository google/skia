/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLIncludes.h"

#include "src/sksl/ir/SkSLUnresolvedFunction.h"

namespace SkSL {

const FunctionDeclaration* get_function(SymbolTable& symbols, const char* name,
                                        const char* signature) {
    const Symbol* result = symbols[name];
    SkASSERT(result);
    switch (result->fKind) {
        case Symbol::kFunctionDeclaration_Kind:
            return (const FunctionDeclaration*) result;
        case Symbol::kUnresolvedFunction_Kind: {
            const UnresolvedFunction* u = (const UnresolvedFunction*) result;
            for (const FunctionDeclaration* f : u->fFunctions) {
                if (f->declaration() == signature) {
                    return f;
                }
            }
            SkASSERT(false);
            return nullptr;
        }
        default:
            SkASSERT(false);
            return nullptr;
    }
}

SymbolTable* symbol_table_for(const FunctionDeclaration& f, std::shared_ptr<SymbolTable>& parent) {
    SymbolTable* result = new SymbolTable(parent, &parent->fErrorReporter);
    for (const Variable* p : f.fParameters) {
        result->addWithoutOwnership(p->fName, p);
    }
    return result;
}

Variable* simple_parameter(const char* name, const char* type,
                           std::shared_ptr<SymbolTable>& symbols) {
    return new Variable(-1, Modifiers(), name, *(Type*) symbols->get(type),
                        (Variable::Storage) 3, nullptr);
}

void set_initial_value(Variable* var, Expression* expr) {
    var->fInitialValue = expr;
    ++var->fWriteCount;
}

} // namespace
