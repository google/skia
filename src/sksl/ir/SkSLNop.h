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
    Nop(IRGenerator* irGenerator)
    : INHERITED(irGenerator, -1, kNop_Kind) {}

    virtual bool isEmpty() const override {
        return true;
    }

    String description() const override {
        return String(";");
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new Nop(fIRGenerator));
    }

    typedef Statement INHERITED;
};

} // namespace

#endif
