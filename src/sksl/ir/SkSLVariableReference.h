/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARIABLEREFERENCE
#define SKSL_VARIABLEREFERENCE

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {

class IRGenerator;

/**
 * A reference to a variable, through which it can be read or written. In the statement:
 *
 * x = x + 1;
 *
 * there is only one Variable 'x', but two VariableReferences to it.
 */
struct VariableReference : public Expression {
    static constexpr Kind kExpressionKind = kVariableReference_Kind;

    enum RefKind {
        kRead_RefKind,
        kWrite_RefKind,
        kReadWrite_RefKind,
        // taking the address of a variable - we consider this a read & write but don't complain if
        // the variable was not previously assigned
        kPointer_RefKind
    };

    VariableReference(int offset, const Variable& variable, RefKind refKind = kRead_RefKind);

    ~VariableReference() override;

    VariableReference(const VariableReference&) = delete;
    VariableReference& operator=(const VariableReference&) = delete;

    RefKind refKind() const {
        return fRefKind;
    }

    void setRefKind(RefKind refKind);

    bool hasProperty(Property property) const override {
        switch (property) {
            case Property::kSideEffects:      return false;
            case Property::kContainsRTAdjust: return fVariable.fName == "sk_RTAdjust";
            default:
                SkASSERT(false);
                return false;
        }
    }

    bool isConstantOrUniform() const override {
        return (fVariable.fModifiers.fFlags & Modifiers::kUniform_Flag) != 0;
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new VariableReference(fOffset, fVariable, fRefKind));
    }

    String description() const override {
        return fVariable.fName;
    }

    static std::unique_ptr<Expression> copy_constant(const IRGenerator& irGenerator,
                                                     const Expression* expr);

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override;

    const Variable& fVariable;
    RefKind fRefKind;

private:
    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
