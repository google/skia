/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INCLUDES
#define SKSL_INCLUDES

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLEnum.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLField.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVarDeclarationsStatement.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

template<typename T>
std::vector<std::unique_ptr<T>> make_vector(int count, ...) {
    std::vector<std::unique_ptr<T>> result;
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; ++i) {
        result.emplace_back(va_arg(args, T*));
    }
    va_end(args);
    return result;
}

template<typename T>
T pop_symbols(std::shared_ptr<SkSL::SymbolTable>* symbols, T value) {
    *symbols = (*symbols)->fParent;
    return value;
}

const FunctionDeclaration* get_function(SymbolTable& symbols, const char* name,
                                        const char* signature);

SymbolTable* symbol_table_for(const FunctionDeclaration& f,
                              std::shared_ptr<SymbolTable>& parent);

Variable* simple_parameter(const char* name, const char* type,
                           std::shared_ptr<SymbolTable>& symbols);

void set_initial_value(Variable* var, Expression* expr);

} // namespace

#endif
