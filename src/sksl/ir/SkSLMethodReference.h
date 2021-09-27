/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_METHODREFERENCE
#define SKSL_METHODREFERENCE

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
    static constexpr Kind kExpressionKind = Kind::kMethodReference;

    MethodReference(const Context& context,
                    int line,
                    std::unique_ptr<Expression> self,
                    std::vector<const FunctionDeclaration*> functions)
            : INHERITED(line, kExpressionKind, context.fTypes.fInvalid.get())
            , fSelf(std::move(self))
            , fFunctions(std::move(functions)) {}

    std::unique_ptr<Expression>& self() { return fSelf; }
    const std::unique_ptr<Expression>& self() const { return fSelf; }

    const std::vector<const FunctionDeclaration*>& functions() const { return fFunctions; }

    bool hasProperty(Property property) const override { return false; }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new MethodReference(
                fLine, this->self()->clone(), this->functions(), &this->type()));
    }

    String description() const override {
        return String("<method>");
    }

private:
    MethodReference(int line,
                    std::unique_ptr<Expression> self,
                    std::vector<const FunctionDeclaration*> functions,
                    const Type* type)
            : INHERITED(line, kExpressionKind, type)
            , fSelf(std::move(self))
            , fFunctions(std::move(functions)) {}

    std::unique_ptr<Expression> fSelf;
    std::vector<const FunctionDeclaration*> fFunctions;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
