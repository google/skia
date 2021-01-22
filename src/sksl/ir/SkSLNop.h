/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_NOP
#define SKSL_NOP

#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

namespace SkSL {

/**
 * A no-op statement that does nothing.
 */
class Nop final : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kNop;

    Nop()
    : INHERITED(-1, kStatementKind) {}

    bool isEmpty() const override {
        return true;
    }

    String description() const override {
        return String(";");
    }

    std::unique_ptr<Statement> clone() const override {
        return std::unique_ptr<Statement>(new Nop());
    }

private:
    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
