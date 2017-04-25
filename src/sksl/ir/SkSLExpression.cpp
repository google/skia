/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLExpression.h"

#include "SkSLConstructor.h"
#include "SkSLBoolLiteral.h"
#include "SkSLFloatLiteral.h"
#include "SkSLIntLiteral.h"

namespace SkSL {

bool Expression::CompareConstants(const Context& context, const Expression& e1,
                                  const Expression& e2) {
    ASSERT(e1.isConstant() && e2.isConstant());
    ASSERT(e1.fType == e2.fType);
    ASSERT(e1.fKind == e2.fKind);
    switch (e1.fKind) {
        case Expression::kConstructor_Kind: {
            Constructor& c1 = (Constructor&) e1;
            Constructor& c2 = (Constructor&) e2;
            if (c1.fType.kind() == Type::kVector_Kind) {
                for (int i = 0; i < c1.fType.columns(); i++) {
                    if (!CompareConstants(context, c1.getVecComponent(i), c2.getVecComponent(i))) {
                        return false;
                    }
                }
                return true;
            }
            // shouldn't be possible to have a constant constructor that isn't a vector or matrix;
            // a constant scalar constructor should have been collapsed down to the appropriate
            // literal
            ASSERT(c1.fType.kind() == Type::kMatrix_Kind);
            const FloatLiteral fzero(context, Position(), 0);
            const IntLiteral izero(context, Position(), 0);
            const Expression* zero;
            if (c1.fType.componentType() == *context.fFloat_Type) {
                zero = &fzero;
            } else {
                ASSERT(c1.fType.componentType() == *context.fInt_Type);
                zero = &izero;
            }
            for (int col = 0; col < c1.fType.columns(); col++) {
                for (int row = 0; row < c1.fType.rows(); row++) {
                    const Expression* component1 = c1.getMatComponent(col, row);
                    const Expression* component2 = c2.getMatComponent(col, row);
                    if (!CompareConstants(context,
                                          component1 ? *component1 : *zero,
                                          component2 ? *component2 : *zero)) {
                        return false;
                    }
                }
            }
            return true;
        }
        case Expression::kBoolLiteral_Kind: {
            BoolLiteral& b1 = (BoolLiteral&) e1;
            BoolLiteral& b2 = (BoolLiteral&) e2;
            return b1.fValue == b2.fValue;
        }
        case Expression::kIntLiteral_Kind: {
            IntLiteral& i1 = (IntLiteral&) e1;
            IntLiteral& i2 = (IntLiteral&) e2;
            return i1.fValue == i2.fValue;
        }
        case Expression::kFloatLiteral_Kind: {
            FloatLiteral& f1 = (FloatLiteral&) e1;
            FloatLiteral& f2 = (FloatLiteral&) e2;
            return f1.fValue == f2.fValue;
        }
        default:
            ABORT("unsupported constant comparison");
    }
}

}
