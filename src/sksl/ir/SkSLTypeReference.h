/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TYPEREFERENCE
#define SKSL_TYPEREFERENCE

#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * Represents an identifier referring to a type. This is an intermediate value: TypeReferences are
 * always eventually replaced by Constructors in valid programs.
 */
struct TypeReference : public Expression {
    TypeReference(IRGenerator* irGenerator, int offset, IRNode::ID value)
    : INHERITED(irGenerator, offset, kTypeReference_Kind, irGenerator->fContext.fInvalid_Type)
    , fValue(value) {}

    bool hasSideEffects() const override {
        return false;
    }

    String description() const override {
        return String(fValue.typeNode().fName);
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new TypeReference(fIRGenerator, fOffset, fValue, fType));
    }

    IRNode::ID fValue;

    typedef Expression INHERITED;

private:
    TypeReference(IRGenerator* irGenerator, int offset, IRNode::ID value, IRNode::ID type)
    : INHERITED(irGenerator, offset, kTypeReference_Kind, type)
    , fValue(value) {}
};

} // namespace

#endif
