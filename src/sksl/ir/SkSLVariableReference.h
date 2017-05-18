/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARIABLEREFERENCE
#define SKSL_VARIABLEREFERENCE

#include "SkSLBoolLiteral.h"
#include "SkSLConstructor.h"
#include "SkSLExpression.h"
#include "SkSLFloatLiteral.h"
#include "SkSLIRGenerator.h"
#include "SkSLIntLiteral.h"

namespace SkSL {

/**
 * A reference to a variable, through which it can be read or written. In the statement:
 *
 * x = x + 1;
 *
 * there is only one Variable 'x', but two VariableReferences to it.
 */
struct VariableReference : public Expression {
    enum RefKind {
        kRead_RefKind,
        kWrite_RefKind,
        kReadWrite_RefKind
    };

    VariableReference(Position position, const Variable& variable, RefKind refKind = kRead_RefKind)
    : INHERITED(position, kVariableReference_Kind, variable.fType)
    , fVariable(variable)
    , fRefKind(refKind) {
        if (refKind != kRead_RefKind) {
            fVariable.fWriteCount++;
        }
        if (refKind != kWrite_RefKind) {
            fVariable.fReadCount++;
        }
    }

    ~VariableReference() override {
        if (fRefKind != kWrite_RefKind) {
            fVariable.fReadCount--;
        }
    }

    RefKind refKind() {
        return fRefKind;
    }

    void setRefKind(RefKind refKind) {
        if (fRefKind != kRead_RefKind) {
            fVariable.fWriteCount--;
        }
        if (fRefKind != kWrite_RefKind) {
            fVariable.fReadCount--;
        }
        if (refKind != kRead_RefKind) {
            fVariable.fWriteCount++;
        }
        if (refKind != kWrite_RefKind) {
            fVariable.fReadCount++;
        }
        fRefKind = refKind;
    }

    bool hasSideEffects() const override {
        return false;
    }

    String description() const override {
        return fVariable.fName;
    }

    static std::unique_ptr<Expression> copy_constant(const IRGenerator& irGenerator,
                                                     const Expression* expr) {
        ASSERT(expr->isConstant());
        switch (expr->fKind) {
            case Expression::kIntLiteral_Kind:
                return std::unique_ptr<Expression>(new IntLiteral(
                                                                 irGenerator.fContext,
                                                                 Position(),
                                                                 ((IntLiteral*) expr)->fValue));
            case Expression::kFloatLiteral_Kind:
                return std::unique_ptr<Expression>(new FloatLiteral(
                                                               irGenerator.fContext,
                                                               Position(),
                                                               ((FloatLiteral*) expr)->fValue));
            case Expression::kBoolLiteral_Kind:
                return std::unique_ptr<Expression>(new BoolLiteral(irGenerator.fContext,
                                                                   Position(),
                                                                   ((BoolLiteral*) expr)->fValue));
            case Expression::kConstructor_Kind: {
                const Constructor* c = (const Constructor*) expr;
                std::vector<std::unique_ptr<Expression>> args;
                for (const auto& arg : c->fArguments) {
                    args.push_back(copy_constant(irGenerator, arg.get()));
                }
                return std::unique_ptr<Expression>(new Constructor(Position(), c->fType,
                                                                   std::move(args)));
            }
            default:
                ABORT("unsupported constant\n");
        }
    }

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override {
        if (fRefKind != kRead_RefKind) {
            return nullptr;
        }
        auto exprIter = definitions.find(&fVariable);
        if (exprIter != definitions.end() && exprIter->second &&
            (*exprIter->second)->isConstant()) {
            return copy_constant(irGenerator, exprIter->second->get());
        }
        return nullptr;
    }

    const Variable& fVariable;
    RefKind fRefKind;

private:
    typedef Expression INHERITED;
};

} // namespace

#endif
