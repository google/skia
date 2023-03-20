/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SWITCHCASE
#define SKSL_SWITCHCASE

#include "include/private/SkSLStatement.h"
#include "include/private/SkSLString.h"
#include "src/sksl/ir/SkSLExpression.h"

#include <cinttypes>

namespace SkSL {

/**
 * A single case of a 'switch' statement.
 */
class SwitchCase final : public Statement {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kSwitchCase;

    static std::unique_ptr<SwitchCase> Make(Position pos, SKSL_INT value,
            std::unique_ptr<Statement> statement) {
        return std::unique_ptr<SwitchCase>(new SwitchCase(pos, /*isDefault=*/false, value,
                std::move(statement)));
    }

    static std::unique_ptr<SwitchCase> MakeDefault(Position pos,
            std::unique_ptr<Statement> statement) {
        return std::unique_ptr<SwitchCase>(new SwitchCase(pos, /*isDefault=*/true, -1,
                std::move(statement)));
    }

    bool isDefault() const {
        return fDefault;
    }

    SKSL_INT value() const {
        SkASSERT(!this->isDefault());
        return fValue;
    }

    std::unique_ptr<Statement>& statement() {
        return fStatement;
    }

    const std::unique_ptr<Statement>& statement() const {
        return fStatement;
    }

    std::unique_ptr<Statement> clone() const override {
        return fDefault ? SwitchCase::MakeDefault(fPosition, this->statement()->clone())
                        : SwitchCase::Make(fPosition, this->value(), this->statement()->clone());
    }

    std::string description() const override {
        if (this->isDefault()) {
            return String::printf("default:\n%s", fStatement->description().c_str());
        } else {
            return String::printf("case %" PRId64 ":\n%s",
                                  (int64_t) this->value(),
                                  fStatement->description().c_str());
        }
    }

private:
    SwitchCase(Position pos, bool isDefault, SKSL_INT value, std::unique_ptr<Statement> statement)
        : INHERITED(pos, kIRNodeKind)
        , fDefault(isDefault)
        , fValue(std::move(value))
        , fStatement(std::move(statement)) {}

    bool fDefault;
    SKSL_INT fValue;
    std::unique_ptr<Statement> fStatement;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
