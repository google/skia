/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_NOP
#define SKSL_NOP

#include "SkSLStatement.h"
#include "SkSLSymbolTable.h"

namespace SkSL {

/**
 * A no-op statement that does nothing.
 */
struct Nop : public Statement {
    Nop()
    : INHERITED(Position(), kNop_Kind) {}

    virtual bool isEmpty() const override {
        return true;
    }

    String description() const override {
        return String(";");
    }

    typedef Statement INHERITED;
};

} // namespace

#endif
