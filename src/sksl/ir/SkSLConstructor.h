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

    virtual std::unique_ptr<Expression> constantPropagate(
                                                        const IRGenerator& irGenerator,
                                                        const DefinitionMap& definitions) override {
        if (fArguments.size() == 1 && fArguments[0]->fKind == Expression::kIntLiteral_Kind &&
            // promote float(1) to 1.0
            fType == *irGenerator.fContext.fFloat_Type) {
            int64_t intValue = ((IntLiteral&) *fArguments[0]).fValue;
            return std::unique_ptr<Expression>(new FloatLiteral(irGenerator.fContext,
                                                                fPosition,
                                                                intValue));
        }
        return nullptr;
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

    std::vector<std::unique_ptr<Expression>> fArguments;

    typedef Expression INHERITED;
};

} // namespace

#endif
