/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLIncludes.h"

#include "src/sksl/ir/SkSLUnresolvedFunction.h"

namespace SkSL {

const FunctionDeclaration* get_function(SymbolTable& symbols, const char* signature) {
    const char* start = strstr(signature, " ");
    SkASSERT(start);
    ++start;
    const char* end = strstr(start, "(");
    SkASSERT(end);
    const Symbol* result = symbols[StringFragment(start, end - start)];
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

SymbolTable* symbol_table_for(const char* signature, std::shared_ptr<SymbolTable>& parent) {
    const FunctionDeclaration* f = get_function(*parent, signature);
    SkASSERT(f);
    SymbolTable* result = new SymbolTable(parent, &parent->fErrorReporter);
    for (const Variable* p : f->fParameters) {
        result->addWithoutOwnership(p->fName, p);
    }
    return result;
}

Variable* simple_parameter(const char* name, const char* type,
                           std::shared_ptr<SymbolTable>& symbols) {
    return new Variable(-1, Modifiers(), name, *(Type*) (*symbols)[type],
                        (Variable::Storage) 3, nullptr);
}

void set_initial_value(Variable* var, Expression* expr) {
    var->fInitialValue = expr;
    ++var->fWriteCount;
}

void create_simple_function(std::shared_ptr<SymbolTable>& symbols, Modifiers modifiers,
                            const char* name, const char* returnType, int parameterCount, ...) {
    std::vector<const Variable*> parameters;
    va_list args;
    va_start(args, parameterCount);
    for (int i = 0; i < parameterCount; ++i) {
        const char* parameterName = va_arg(args, const char*);
        const char* parameterType = va_arg(args, const char*);
        parameters.push_back(simple_parameter(parameterName, parameterType, symbols));
    }
    va_end(args);
    symbols->add(name, std::unique_ptr<Symbol>(new FunctionDeclaration(
                                                                    -1,
                                                                    modifiers,
                                                                    name,
                                                                    std::move(parameters),
                                                                    *(Type*) (*symbols)[returnType],
                                                                    1)));
}

} // namespace
