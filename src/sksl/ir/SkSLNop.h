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
    inline static constexpr Kind kIRNodeKind = Kind::kNop;

    Nop()
    : INHERITED(Position(), kIRNodeKind) {}

    static std::unique_ptr<Statement> Make() {
        return std::make_unique<Nop>();
    }

    bool isEmpty() const override {
        return true;
    }

    std::string description() const override {
        return ";";
    }

private:
    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
