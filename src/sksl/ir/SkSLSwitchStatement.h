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

/**
 * A 'switch' statement.
 */
struct SwitchStatement : public Statement {
    SwitchStatement(IRGenerator* irGenerator, int offset, bool isStatic,
                    IRNode::ID value, std::vector<IRNode::ID> cases,
                    const std::shared_ptr<SymbolTable> symbols)
    : INHERITED(irGenerator, offset, kSwitch_Kind)
    , fIsStatic(isStatic)
    , fValue(value)
    , fSymbols(std::move(symbols))
    , fCases(std::move(cases)) {}

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new SwitchStatement(fIRGenerator, fOffset, fIsStatic,
                                                            fValue, fCases, fSymbols));
    }

    String description() const override {
        String result;
        if (fIsStatic) {
            result += "@";
        }
        result += String::printf("switch (%s) {\n", fValue.node().description().c_str());
        for (const auto& c : fCases) {
            result += c.node().description();
        }
        result += "}";
        return result;
    }

    bool fIsStatic;
    IRNode::ID fValue;
    // it's important to keep fCases defined after (and thus destroyed before) fSymbols, because
    // destroying statements can modify reference counts in symbols
    const std::shared_ptr<SymbolTable> fSymbols;
    std::vector<IRNode::ID> fCases;

    typedef Statement INHERITED;
};

} // namespace

#endif
