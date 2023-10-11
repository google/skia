/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_METHODREFERENCE
#define SKSL_METHODREFERENCE

#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

class FunctionDeclaration;

/**
 * An identifier referring to a method name, along with an instance for the call.
 * This is an intermediate value: MethodReferences are always eventually replaced by FunctionCalls
 * in valid programs.
 *
 * Method calls are only supported on effect-child types, and they all resolve to intrinsics
 * prefixed with '$', and taking the 'self' object as the last parameter. For example:
 *
 *   uniform shader child;
 *   ...
 *   child.eval(xy)  -->  $eval(xy, child)
 */
class MethodReference final : public Expression {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kMethodReference;

    MethodReference(const Context& context,
                    Position pos,
                    std::unique_ptr<Expression> self,
                    const FunctionDeclaration* overloadChain)
            : INHERITED(pos, kIRNodeKind, context.fTypes.fInvalid.get())
            , fSelf(std::move(self))
            , fOverloadChain(overloadChain) {}

    std::unique_ptr<Expression>& self() { return fSelf; }
    const std::unique_ptr<Expression>& self() const { return fSelf; }

    const FunctionDeclaration* overloadChain() const { return fOverloadChain; }

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::unique_ptr<Expression>(new MethodReference(
                pos, this->self()->clone(), this->overloadChain(), &this->type()));
    }

    std::string description(OperatorPrecedence) const override {
        return "<method>";
    }

private:
    MethodReference(Position pos,
                    std::unique_ptr<Expression> self,
                    const FunctionDeclaration* overloadChain,
                    const Type* type)
            : INHERITED(pos, kIRNodeKind, type)
            , fSelf(std::move(self))
            , fOverloadChain(overloadChain) {}

    std::unique_ptr<Expression> fSelf;
    const FunctionDeclaration* fOverloadChain;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
