/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONREFERENCE
#define SKSL_FUNCTIONREFERENCE

#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"

namespace SkSL {

/**
 * An identifier referring to a function name. This is an intermediate value: FunctionReferences are
 * always eventually replaced by FunctionCalls in valid programs.
 */
class FunctionReference final : public Expression {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kFunctionReference;

    FunctionReference(const Context& context, Position pos,
                      const FunctionDeclaration* overloadChain)
        : INHERITED(pos, kIRNodeKind, context.fTypes.fInvalid.get())
        , fOverloadChain(overloadChain) {}

    const FunctionDeclaration* overloadChain() const {
        return fOverloadChain;
    }

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::unique_ptr<Expression>(new FunctionReference(pos, this->overloadChain(),
                                                                 &this->type()));
    }

    std::string description(OperatorPrecedence) const override {
        return "<function>";
    }

private:
    FunctionReference(Position pos, const FunctionDeclaration* overloadChain, const Type* type)
            : INHERITED(pos, kIRNodeKind, type)
            , fOverloadChain(overloadChain) {}

    const FunctionDeclaration* fOverloadChain;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
