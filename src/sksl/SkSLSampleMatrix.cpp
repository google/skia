/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLSampleMatrix.h"

#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarationsStatement.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLWhileStatement.h"

namespace SkSL {

SampleMatrix SampleMatrix::merge(const SampleMatrix& other) {
    if (fKind == Kind::kVariable || other.fKind == Kind::kVariable) {
        *this = SampleMatrix::MakeVariable(this->fHasPerspective || other.fHasPerspective);
        return *this;
    }
    if (other.fKind == Kind::kConstantOrUniform) {
        if (fKind == other.fKind) {
            if (fExpression == other.fExpression) {
                return *this;
            }
            *this = SampleMatrix::MakeVariable(this->fHasPerspective || other.fHasPerspective);
            return *this;
        }
        SkASSERT(fKind == Kind::kNone);
        *this = other;
        return *this;
    }
    return *this;
}

} // namespace
