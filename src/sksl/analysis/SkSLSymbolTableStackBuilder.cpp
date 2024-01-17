/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLStatement.h"

#include <vector>

namespace SkSL {

class SymbolTable;

namespace Analysis {

SymbolTableStackBuilder::SymbolTableStackBuilder(const Statement* stmt,
                                                 std::vector<SymbolTable*>* stack) {
    if (stmt) {
        switch (stmt->kind()) {
            case Statement::Kind::kBlock:
                if (SymbolTable* symbols = stmt->as<Block>().symbolTable()) {
                    stack->push_back(symbols);
                    fStackToPop = stack;
                }
                break;

            case Statement::Kind::kFor:
                if (SymbolTable* symbols = stmt->as<ForStatement>().symbols()) {
                    stack->push_back(symbols);
                    fStackToPop = stack;
                }
                break;

            default:
                break;
        }
    }
}

SymbolTableStackBuilder::~SymbolTableStackBuilder() {
    if (fStackToPop) {
        fStackToPop->pop_back();
    }
}

}  // namespace Analysis
}  // namespace SkSL
