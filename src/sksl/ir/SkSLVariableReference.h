/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARIABLEREFERENCE
#define SKSL_VARIABLEREFERENCE

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"

#include <cstdint>
#include <memory>
#include <string>

namespace SkSL {

class Variable;
enum class OperatorPrecedence : uint8_t;

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

    inline static constexpr Kind kIRNodeKind = Kind::kVariableReference;

    VariableReference(Position pos, const Variable* variable, RefKind refKind);

    // Creates a VariableReference. There isn't much in the way of error-checking or optimization
    // opportunities here.
    static std::unique_ptr<Expression> Make(Position pos,
                                            const Variable* variable,
                                            RefKind refKind = RefKind::kRead) {
        SkASSERT(variable);
        return std::make_unique<VariableReference>(pos, variable, refKind);
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

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::make_unique<VariableReference>(pos, this->variable(), this->refKind());
    }

    std::string description(OperatorPrecedence) const override;

private:
    const Variable* fVariable;
    VariableRefKind fRefKind;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
