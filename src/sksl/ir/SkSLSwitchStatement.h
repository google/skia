/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SWITCHSTATEMENT
#define SKSL_SWITCHSTATEMENT

#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSwitchCase.h"

namespace SkSL {

class SymbolTable;

/**
 * A 'switch' statement.
 */
class SwitchStatement final : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kSwitch;

    SwitchStatement(int offset, bool isStatic, std::unique_ptr<Expression> value,
                    std::vector<std::unique_ptr<SwitchCase>> cases,
                    const std::shared_ptr<SymbolTable> symbols)
        : INHERITED(offset, kStatementKind)
        , fIsStatic(isStatic)
        , fValue(std::move(value))
        , fCases(std::move(cases))
        , fSymbols(std::move(symbols)) {}

    std::unique_ptr<Expression>& value() {
        return fValue;
    }

    const std::unique_ptr<Expression>& value() const {
        return fValue;
    }

    std::vector<std::unique_ptr<SwitchCase>>& cases() {
        return fCases;
    }

    const std::vector<std::unique_ptr<SwitchCase>>& cases() const {
        return fCases;
    }

    bool isStatic() const {
        return fIsStatic;
    }

    const std::shared_ptr<SymbolTable>& symbols() const {
        return fSymbols;
    }

    std::unique_ptr<Statement> clone() const override {
        std::vector<std::unique_ptr<SwitchCase>> cloned;
        for (const std::unique_ptr<SwitchCase>& sc : this->cases()) {
            cloned.emplace_back(&sc->clone().release()->as<SwitchCase>());
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
            result += c->description();
        }
        result += "}";
        return result;
    }

private:
    bool fIsStatic;
    std::unique_ptr<Expression> fValue;
    std::vector<std::unique_ptr<SwitchCase>> fCases;
    std::shared_ptr<SymbolTable> fSymbols;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
