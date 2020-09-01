/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLExpression.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"

namespace SkSL {

bool Expression::hasProperty(Property property) const {
    switch (this->kind()) {
        case Kind::kBinary:
            if (property == Property::kSideEffects &&
                Compiler::IsAssignment(this->getBinaryData().fOperator)) {
                return true;
            }
            return this->expressionChild(Index::kBinaryLeft).hasProperty(property) ||
                   this->expressionChild(Index::kBinaryRight).hasProperty(property);
        default:
            SkASSERT(false);
            return false;
    }
}

bool Expression::isConstantOrUniform() const {
    switch (this->kind()) {
        case Kind::kBinary:
            return this->expressionChild(Index::kBinaryLeft).isConstantOrUniform() &&
                   this->expressionChild(Index::kBinaryRight).isConstantOrUniform();
        default:
            SkASSERT(!this->isCompileTimeConstant() || !this->hasSideEffects());
            return this->isCompileTimeConstant();
    }
}

std::unique_ptr<Expression> Expression::constantPropagate(const IRGenerator& irGenerator,
                                                          const DefinitionMap& definitions) {
    switch (this->kind()) {
        case Kind::kBinary:
            return irGenerator.constantFold(this->expressionChild(Index::kBinaryLeft),
                                            this->getBinaryData().fOperator,
                                            this->expressionChild(Index::kBinaryRight));
        default:
            return nullptr;
    }
}

std::unique_ptr<Expression> Expression::clone() const {
    switch (this->kind()) {
        case Kind::kBinary:
            return Expression::MakeBinary(fOffset,
                                          this->expressionChild(Index::kBinaryLeft).clone(),
                                          this->getBinaryData().fOperator,
                                          this->expressionChild(Index::kBinaryRight).clone(),
                                          fType);
        default:
            SkASSERT(false);
            return nullptr;
    }
}

String Expression::description() const {
    switch (this->kind()) {
        case Kind::kBinary:
            return "(" + expressionChild(Index::kBinaryLeft).description() + " " +
                   Compiler::OperatorName(this->getBinaryData().fOperator) + " " +
                   expressionChild(Index::kBinaryRight).description() + ")";
        default:
            SkASSERT(false);
            return "<unimplemented>";
    }
}

}  // namespace SkSL
