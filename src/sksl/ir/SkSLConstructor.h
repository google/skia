/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR
#define SKSL_CONSTRUCTOR

#include "SkSLExpression.h"
#include "SkSLFloatLiteral.h"
#include "SkSLIntLiteral.h"
#include "SkSLIRGenerator.h"

namespace SkSL {

/**
 * Represents the construction of a compound type, such as "vec2(x, y)".
 *
 * Vector constructors will always consist of either exactly 1 scalar, or a collection of vectors
 * and scalars totalling exactly the right number of scalar components.
 *
 * Matrix constructors will always consist of either exactly 1 scalar, exactly 1 matrix, or a
 * collection of vectors and scalars totalling exactly the right number of scalar components.
 */
struct Constructor : public Expression {
    Constructor(Position position, const Type& type,
                std::vector<std::unique_ptr<Expression>> arguments)
    : INHERITED(position, kConstructor_Kind, type)
    , fArguments(std::move(arguments)) {}

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override {
        if (fArguments.size() == 1 && fArguments[0]->fKind == Expression::kIntLiteral_Kind) {
            if (fType == *irGenerator.fContext.fFloat_Type) {
                // promote float(1) to 1.0
                int64_t intValue = ((IntLiteral&) *fArguments[0]).fValue;
                return std::unique_ptr<Expression>(new FloatLiteral(irGenerator.fContext,
                                                                    fPosition,
                                                                    intValue));
            } else if (fType == *irGenerator.fContext.fUInt_Type) {
                // promote uint(1) to 1u
                int64_t intValue = ((IntLiteral&) *fArguments[0]).fValue;
                return std::unique_ptr<Expression>(new IntLiteral(irGenerator.fContext,
                                                                  fPosition,
                                                                  intValue,
                                                                  &fType));
            }
        }
        return nullptr;
    }

    bool hasSideEffects() const override {
        for (const auto& arg : fArguments) {
            if (arg->hasSideEffects()) {
                return true;
            }
        }
        return false;
    }

    String description() const override {
        String result = fType.description() + "(";
        String separator;
        for (size_t i = 0; i < fArguments.size(); i++) {
            result += separator;
            result += fArguments[i]->description();
            separator = ", ";
        }
        result += ")";
        return result;
    }

    bool isConstant() const override {
        for (size_t i = 0; i < fArguments.size(); i++) {
            if (!fArguments[i]->isConstant()) {
                return false;
            }
        }
        return true;
    }

    const Expression& getVecComponent(int index) const {
        ASSERT(fType.kind() == Type::kVector_Kind);
        if (fArguments.size() == 1 && fArguments[0]->fType.kind() == Type::kScalar_Kind) {
            return *fArguments[0];
        }
        int current = 0;
        for (const auto& arg : fArguments) {
            ASSERT(current <= index);
            if (arg->fType.kind() == Type::kScalar_Kind) {
                if (index == current) {
                    return *arg;
                }
                current++;
            } else {
                ASSERT(arg->fType.kind() == Type::kVector_Kind);
                ASSERT(arg->fKind == Expression::kConstructor_Kind);
                if (current + arg->fType.columns() > index) {
                    return ((const Constructor&) *arg).getVecComponent(index - current);
                }
                current += arg->fType.columns();
            }
        }
        ABORT("failed to find vector component %d in %s\n", index, description().c_str());
    }

    double getFVecComponent(int index) const {
        const Expression& c = this->getVecComponent(index);
        ASSERT(c.fKind == Expression::kFloatLiteral_Kind);
        return ((FloatLiteral&) c).fValue;
    }

    int64_t getIVecComponent(int index) const {
        const Expression& c = this->getVecComponent(index);
        ASSERT(c.fKind == Expression::kIntLiteral_Kind);
        return ((IntLiteral&) c).fValue;
    }

    std::vector<std::unique_ptr<Expression>> fArguments;

    typedef Expression INHERITED;
};

} // namespace

#endif
