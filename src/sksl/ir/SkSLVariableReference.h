/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_VARIABLEREFERENCE
#define SKSL_VARIABLEREFERENCE

#include "SkSLExpression.h"

namespace SkSL {

/**
 * A reference to a variable, through which it can be read or written. In the statement:
 *
 * x = x + 1;
 *
 * there is only one Variable 'x', but two VariableReferences to it.
 */
struct VariableReference : public Expression {
    VariableReference(Position position, std::shared_ptr<Variable> variable)
    : INHERITED(position, kVariableReference_Kind, variable->fType)
    , fVariable(std::move(variable)) {}

    std::string description() const override {
        return fVariable->fName;
    }

    const std::shared_ptr<Variable> fVariable;

    typedef Expression INHERITED;
};

} // namespace

#endif
