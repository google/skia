/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SWITCHCASE
#define SKSL_SWITCHCASE

#include "include/private/base/SkAssert.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLStatement.h"

#include <memory>
#include <string>
#include <utility>

namespace SkSL {

/**
 * A single case of a 'switch' statement.
 */
class SwitchCase final : public Statement {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kSwitchCase;

    static std::unique_ptr<SwitchCase> Make(Position pos,
                                            SKSL_INT value,
                                            std::unique_ptr<Statement> statement);

    static std::unique_ptr<SwitchCase> MakeDefault(Position pos,
                                                   std::unique_ptr<Statement> statement);

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

    std::unique_ptr<Statement> clone() const override;

    std::string description() const override;

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
