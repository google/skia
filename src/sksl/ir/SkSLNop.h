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
struct Nop : public Statement {
    Nop()
    : INHERITED(-1, kNop_Kind) {}

    virtual bool isEmpty() const override {
        return true;
    }

#ifdef SK_DEBUG
    String description() const override {
        return String(";");
    }
#endif

    std::unique_ptr<Statement> clone() const override {
        return std::unique_ptr<Statement>(new Nop());
    }

    typedef Statement INHERITED;
};

} // namespace

#endif
