/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_NOP
#define SKSL_NOP

#include "include/private/SkSLStatement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

namespace SkSL {

/**
 * A no-op statement that does nothing.
 */
class Nop final : public Statement {
public:
    inline static constexpr Kind kStatementKind = Kind::kNop;

    Nop()
    : INHERITED(/*line=*/-1, kStatementKind) {}

    static std::unique_ptr<Statement> Make() {
        return std::make_unique<Nop>();
    }

    bool isEmpty() const override {
        return true;
    }

    String description() const override {
        return String(";");
    }

    std::unique_ptr<Statement> clone() const override {
        return std::make_unique<Nop>();
    }

private:
    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
