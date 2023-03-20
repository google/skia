/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLIRNode.h"
#include "include/private/SkSLStatement.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"

#include <memory>
#include <utility>
#include <vector>

namespace SkSL {

class SymbolTable;

namespace Analysis {

SymbolTableStackBuilder::SymbolTableStackBuilder(const Statement* stmt,
                                                 std::vector<std::shared_ptr<SymbolTable>>* stack) {
    if (stmt) {
        switch (stmt->kind()) {
            case Statement::Kind::kBlock:
                if (std::shared_ptr<SymbolTable> symbols = stmt->as<Block>().symbolTable()) {
                    stack->push_back(std::move(symbols));
                    fStackToPop = stack;
                }
                break;

            case Statement::Kind::kFor:
                if (std::shared_ptr<SymbolTable> symbols = stmt->as<ForStatement>().symbols()) {
                    stack->push_back(std::move(symbols));
                    fStackToPop = stack;
                }
                break;

            case Statement::Kind::kSwitch:
                if (std::shared_ptr<SymbolTable> symbols = stmt->as<SwitchStatement>().symbols()) {
                    stack->push_back(std::move(symbols));
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
