/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR
#define SKSL_CONSTRUCTOR

#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLIntLiteral.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"

namespace SkSL {

/**
 * Represents the construction of a compound type, such as "float2(x, y)".
 *
 * Vector constructors will always consist of either exactly 1 scalar, or a collection of vectors
 * and scalars totalling exactly the right number of scalar components.
 *
 * Matrix constructors will always consist of either exactly 1 scalar, exactly 1 matrix, or a
 * collection of vectors and scalars totalling exactly the right number of scalar components.
 */
struct Constructor : public Expression {
    Constructor(IRGenerator* irGenerator, int offset, IRNode::ID type,
                std::vector<IRNode::ID> arguments)
    : INHERITED(irGenerator, offset, kConstructor_Kind, type)
    , fArguments(std::move(arguments)) {
        if (type.node().description() == "fragmentProcessor?") {
            abort();
        }
    }

    IRNode::ID constantPropagate(const DefinitionMap& definitions) override {
        if (fArguments.size() == 1 &&
            fArguments[0].expressionNode().fKind == Expression::kIntLiteral_Kind) {
            const Type& type = fType.typeNode();
            if (type.isFloat()) {
                // promote float(1) to 1.0
                int64_t intValue = ((IntLiteral&) fArguments[0].node()).fValue;
                return fIRGenerator->createNode(new FloatLiteral(fIRGenerator, fOffset, intValue));
            } else if (type.isInteger()) {
                // promote uint(1) to 1u
                int64_t intValue = ((IntLiteral&) fArguments[0].node()).fValue;
                return fIRGenerator->createNode(new IntLiteral(fIRGenerator, fOffset, intValue,
                                                               fType));
            }
        }
        return IRNode::ID();
    }

    bool hasSideEffects() const override {
        for (const auto& arg : fArguments) {
            if (arg.expressionNode().hasSideEffects()) {
                return true;
            }
        }
        return false;
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new Constructor(fIRGenerator, fOffset, fType, fArguments));
    }

    String description() const override {
        String result = fType.node().description() + "(";
        String separator;
        for (auto arg : fArguments) {
            result += separator;
            result += arg.node().description();
            separator = ", ";
        }
        result += ")";
        return result;
    }

    bool isConstant() const override {
        for (auto arg : fArguments) {
            if (!arg.expressionNode().isConstant()) {
                return false;
            }
        }
        return true;
    }

    bool compareConstant(const Expression& other) const override {
        SkASSERT(other.fKind == Expression::kConstructor_Kind && other.fType == fType);
        Constructor& c = (Constructor&) other;
        const Type& type = c.fType.typeNode();
        if (type.kind() == Type::kVector_Kind) {
            bool isFloat = type.columns() > 1 ? type.componentType().typeNode().isFloat()
                                              : type.isFloat();
            for (int i = 0; i < type.columns(); i++) {
                if (isFloat) {
                    if (this->getFVecComponent(i) != c.getFVecComponent(i)) {
                        return false;
                    }
                } else if (this->getIVecComponent(i) != c.getIVecComponent(i)) {
                    return false;
                }
            }
            return true;
        }
        // shouldn't be possible to have a constant constructor that isn't a vector or matrix;
        // a constant scalar constructor should have been collapsed down to the appropriate
        // literal
        SkASSERT(type.kind() == Type::kMatrix_Kind);
        for (int col = 0; col < type.columns(); col++) {
            for (int row = 0; row < type.rows(); row++) {
                if (this->getMatComponent(col, row) != c.getMatComponent(col, row)) {
                    return false;
                }
            }
        }
        return true;
    }

    template<typename type>
    type getVecComponent(int index) const {
        SkASSERT(fType.typeNode().kind() == Type::kVector_Kind);
        if (fArguments.size() == 1 &&
            fArguments[0].expressionNode().fType.typeNode().kind() == Type::kScalar_Kind) {
            if (std::is_floating_point<type>::value) {
                return fArguments[0].expressionNode().getConstantFloat();
            } else {
                return fArguments[0].expressionNode().getConstantInt();
            }
        }
        int current = 0;
        for (const auto& argID : fArguments) {
            Expression& arg = argID.expressionNode();
            const Type& argType = arg.fType.typeNode();
            SkASSERT(current <= index);
            if (argType.kind() == Type::kScalar_Kind) {
                if (index == current) {
                    if (std::is_floating_point<type>::value) {
                        return arg.getConstantFloat();
                    } else {
                        return arg.getConstantInt();
                    }
                }
                current++;
            } else if (arg.fKind == kConstructor_Kind) {
                if (current + argType.columns() > index) {
                    return ((const Constructor&) arg).getVecComponent<type>(index - current);
                }
                current += argType.columns();
            } else {
                if (current + argType.columns() > index) {
                    SkASSERT(arg.fKind == kPrefix_Kind);
                    const PrefixExpression& p = (PrefixExpression&) arg;
                    const Constructor& c = (const Constructor&) p.fOperand.node();
                    return -c.getVecComponent<type>(index - current);
                }
                current += argType.columns();
            }
        }
        ABORT("failed to find vector component %d in %s\n", index, description().c_str());
    }

    SKSL_FLOAT getFVecComponent(int n) const override {
        return this->getVecComponent<SKSL_FLOAT>(n);
    }

    /**
     * For a literal vector expression, return the integer value of the n'th vector component. It is
     * an error to call this method on an expression which is not a literal vector.
     */
    SKSL_INT getIVecComponent(int n) const override {
        return this->getVecComponent<SKSL_INT>(n);
    }

    SKSL_FLOAT getMatComponent(int col, int row) const override {
        SkASSERT(this->isConstant());
        SkASSERT(fType.typeNode().kind() == Type::kMatrix_Kind);
        SkASSERT(col < fType.typeNode().columns() && row < fType.typeNode().rows());
        if (fArguments.size() == 1) {
            Expression& arg = fArguments[0].expressionNode();
            const Type& argType = arg.fType.typeNode();
            if (argType.kind() == Type::kScalar_Kind) {
                // single scalar argument, so matrix is of the form:
                // x 0 0
                // 0 x 0
                // 0 0 x
                // return x if col == row
                return col == row ? arg.getConstantFloat() : 0.0;
            }
            if (argType.kind() == Type::kMatrix_Kind) {
                SkASSERT(arg.fKind == Expression::kConstructor_Kind);
                // single matrix argument. make sure we're within the argument's bounds.
                if (col < argType.columns() && row < argType.rows()) {
                    // within bounds, defer to argument
                    return ((Constructor&) arg).getMatComponent(col, row);
                }
                // out of bounds
                return 0.0;
            }
        }
        int currentIndex = 0;
        int targetIndex = col * fType.typeNode().rows() + row;
        for (const auto& argID : fArguments) {
            Expression& arg = argID.expressionNode();
            const Type& argType = arg.fType.typeNode();
            SkASSERT(targetIndex >= currentIndex);
            SkASSERT(argType.rows() == 1);
            if (currentIndex + argType.columns() > targetIndex) {
                if (argType.columns() == 1) {
                    return arg.getConstantFloat();
                } else {
                    return arg.getFVecComponent(targetIndex - currentIndex);
                }
            }
            currentIndex += arg.fType.typeNode().columns();
        }
        ABORT("can't happen, matrix component out of bounds");
    }

    std::vector<IRNode::ID> fArguments;

    typedef Expression INHERITED;
};

} // namespace

#endif
