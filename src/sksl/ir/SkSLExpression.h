/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_EXPRESSION
#define SKSL_EXPRESSION

#include "SkSLIRNode.h"
#include "SkSLType.h"

namespace SkSL {

/**
 * Abstract supertype of all expressions. 
 */
struct Expression : public IRNode {
    enum Kind {
        kBinary_Kind,
        kBoolLiteral_Kind,
        kConstructor_Kind,
        kIntLiteral_Kind,
        kFieldAccess_Kind,
        kFloatLiteral_Kind,
        kFunctionReference_Kind,
        kFunctionCall_Kind,
        kIndex_Kind,
        kPrefix_Kind,
        kPostfix_Kind,
        kSwizzle_Kind,
        kVariableReference_Kind,
        kTernary_Kind,
        kTypeReference_Kind,
    };

    Expression(Position position, Kind kind, const Type& type)
    : INHERITED(position)
    , fKind(kind)
    , fType(std::move(type)) {}

    virtual bool isConstant() const {
        return false;
    }

    const Kind fKind;
    const Type& fType;

    typedef IRNode INHERITED;
};

} // namespace

#endif
