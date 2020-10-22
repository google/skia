/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SWITCHSTATEMENT
#define SKSL_SWITCHSTATEMENT

#include "src/sksl/ir/SkSLNodeArrayWrapper.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSwitchCase.h"

namespace SkSL {

class SymbolTable;

/**
 * A 'switch' statement.
 */
class SwitchStatement : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kSwitch;

    using CaseArray = NodeArrayWrapper<SwitchCase, Statement>;

    using ConstCaseArray = ConstNodeArrayWrapper<SwitchCase, Statement>;

    SwitchStatement(int offset, bool isStatic, std::unique_ptr<Expression> value,
                    std::vector<std::unique_ptr<SwitchCase>> cases,
                    const std::shared_ptr<SymbolTable> symbols)
    : INHERITED(offset, SwitchStatementData{isStatic, std::move(symbols)}) {
        fExpressionChildren.push_back(std::move(value));
        fStatementChildren.reserve_back(cases.size());
        for (std::unique_ptr<SwitchCase>& c : cases) {
            fStatementChildren.push_back(std::move(c));
        }
    }

    std::unique_ptr<Expression>& value() {
        return fExpressionChildren[0];
    }

    const std::unique_ptr<Expression>& value() const {
        return fExpressionChildren[0];
    }

    CaseArray cases() {
        return CaseArray(&fStatementChildren);
    }

    ConstCaseArray cases() const {
        return ConstCaseArray(&fStatementChildren);
    }

    bool isStatic() const {
        return this->switchStatementData().fIsStatic;
    }

    const std::shared_ptr<SymbolTable>& symbols() const {
        return this->switchStatementData().fSymbols;
    }

    std::unique_ptr<Statement> clone() const override {
        std::vector<std::unique_ptr<SwitchCase>> cloned;
        for (const std::unique_ptr<Statement>& s : fStatementChildren) {
            cloned.emplace_back((SwitchCase*) s->as<SwitchCase>().clone().release());
        }
        return std::unique_ptr<Statement>(new SwitchStatement(
                                                      fOffset,
                                                      this->isStatic(),
                                                      this->value()->clone(),
                                                      std::move(cloned),
                                                      SymbolTable::WrapIfBuiltin(this->symbols())));
    }

    String description() const override {
        String result;
        if (this->isStatic()) {
            result += "@";
        }
        result += String::printf("switch (%s) {\n", this->value()->description().c_str());
        for (const auto& c : this->cases()) {
            result += c.description();
        }
        result += "}";
        return result;
    }

private:
    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
