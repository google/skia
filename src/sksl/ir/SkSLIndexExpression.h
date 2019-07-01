/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INDEX
#define SKSL_INDEX

#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * Given a type, returns the type that will result from extracting an array value from it.
 */
static IRNode::ID index_type(const Context& context, IRNode::ID typeID) {
    const Type& type = typeID.typeNode();
    if (type.kind() == Type::kMatrix_Kind) {
        if (type.componentType() == context.fFloat_Type) {
            switch (type.rows()) {
                case 2: return context.fFloat2_Type;
                case 3: return context.fFloat3_Type;
                case 4: return context.fFloat4_Type;
                default: SkASSERT(false);
            }
        } else if (type.componentType() == context.fHalf_Type) {
            switch (type.rows()) {
                case 2: return context.fHalf2_Type;
                case 3: return context.fHalf3_Type;
                case 4: return context.fHalf4_Type;
                default: SkASSERT(false);
            }
        } else {
           SkASSERT(type.componentType() == context.fDouble_Type);
            switch (type.rows()) {
                case 2: return context.fDouble2_Type;
                case 3: return context.fDouble3_Type;
                case 4: return context.fDouble4_Type;
                default: SkASSERT(false);
            }
        }
    }
    return type.componentType();
}

/**
 * An expression which extracts a value from an array or matrix, as in 'm[2]'.
 */
struct IndexExpression : public Expression {
    IndexExpression(IRGenerator* irGenerator, IRNode::ID base, IRNode::ID index)
    : INHERITED(irGenerator, base.node().fOffset, kIndex_Kind,
                index_type(irGenerator->fContext, base.expressionNode().fType))
    , fBase(base)
    , fIndex(index) {
        SkASSERT(fIndex.expressionNode().fType == irGenerator->fContext.fInt_Type ||
                 fIndex.expressionNode().fType == irGenerator->fContext.fUInt_Type);
    }

    bool hasSideEffects() const override {
        return fBase.expressionNode().hasSideEffects() || fIndex.expressionNode().hasSideEffects();
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new IndexExpression(fIRGenerator, fBase, fIndex, fType));
    }

    String description() const override {
        return fBase.node().description() + "[" + fIndex.node().description() + "]";
    }

    IRNode::ID fBase;
    IRNode::ID fIndex;

    typedef Expression INHERITED;

private:
    IndexExpression(IRGenerator* irGenerator, IRNode::ID base, IRNode::ID index, IRNode::ID type)
    : INHERITED(irGenerator, base.node().fOffset, kIndex_Kind, type)
    , fBase(base)
    , fIndex(index) {}
};

} // namespace

#endif
