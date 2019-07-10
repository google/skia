/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FIELDACCESS
#define SKSL_FIELDACCESS

#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * An expression which extracts a field from a struct, as in 'foo.bar'.
 */
struct FieldAccess : public Expression {
    enum OwnerKind {
        kDefault_OwnerKind,
        // this field access is to a field of an anonymous interface block (and thus, the field name
        // is actually in global scope, so only the field name needs to be written in GLSL)
        kAnonymousInterfaceBlock_OwnerKind
    };

    FieldAccess(IRGenerator* irGenerator, IRNode::ID base, int fieldIndex,
                OwnerKind ownerKind = kDefault_OwnerKind)
    : INHERITED(irGenerator, base.node().fOffset, kFieldAccess_Kind,
                base.expressionNode().fType.typeNode().fields()[fieldIndex].fType)
    , fBase(base)
    , fFieldIndex(fieldIndex)
    , fOwnerKind(ownerKind) {}

    bool hasSideEffects() const override {
        return fBase.expressionNode().hasSideEffects();
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new FieldAccess(fIRGenerator, fBase, fFieldIndex,
                                                        fOwnerKind));
    }

    String description() const override {
        return fBase.node().description() + "." +
               fBase.expressionNode().fType.typeNode().fields()[fFieldIndex].fName;
    }

    IRNode::ID fBase;
    const int fFieldIndex;
    const OwnerKind fOwnerKind;

    typedef Expression INHERITED;
};

} // namespace

#endif
