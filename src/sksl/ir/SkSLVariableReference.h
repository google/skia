/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARIABLEREFERENCE
#define SKSL_VARIABLEREFERENCE

#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

class IRGenerator;
class Variable;

enum class VariableRefKind : int8_t {
    kRead,
    kWrite,
    kReadWrite,
    // taking the address of a variable - we consider this a read & write but don't complain if
    // the variable was not previously assigned
    kPointer
};

/**
 * A reference to a variable, through which it can be read or written. In the statement:
 *
 * x = x + 1;
 *
 * there is only one Variable 'x', but two VariableReferences to it.
 */
class VariableReference final : public Expression {
public:
    using RefKind = VariableRefKind;

    static constexpr Kind kExpressionKind = Kind::kVariableReference;

    VariableReference(int line, const Variable* variable, RefKind refKind);

    // Creates a VariableReference. There isn't much in the way of error-checking or optimization
    // opportunities here.
    static std::unique_ptr<Expression> Make(int line,
                                            const Variable* variable,
                                            RefKind refKind = RefKind::kRead) {
        SkASSERT(variable);
        return std::make_unique<VariableReference>(line, variable, refKind);
    }

    VariableReference(const VariableReference&) = delete;
    VariableReference& operator=(const VariableReference&) = delete;

    const Variable* variable() const {
        return fVariable;
    }

    RefKind refKind() const {
        return fRefKind;
    }

    void setRefKind(RefKind refKind);
    void setVariable(const Variable* variable);

    bool hasProperty(Property property) const override;

    bool isConstantOrUniform() const override;

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<VariableReference>(fLine, this->variable(), this->refKind());
    }

    String description() const override;

private:
    const Variable* fVariable;
    VariableRefKind fRefKind;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
