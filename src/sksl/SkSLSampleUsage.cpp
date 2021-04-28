/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLSampleUsage.h"

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
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {

SampleUsage SampleUsage::merge(const SampleUsage& other) {
    // This function is only used when processing SkSL, to determine the combined SampleUsage for
    // a child fp/shader/etc. We should never see matrix sampling here.
    SkASSERT(fKind != Kind::kUniformMatrix && other.fKind != Kind::kUniformMatrix);

    static_assert(Kind::kExplicit > Kind::kPassThrough);
    static_assert(Kind::kPassThrough > Kind::kNone);
    fKind = std::max(fKind, other.fKind);

    return *this;
}

std::string SampleUsage::constructor() const {
    // This function is only used when processing SkSL. We should never see matrix sampling here.
    SkASSERT(fKind != Kind::kUniformMatrix);

    switch (fKind) {
        case Kind::kNone:        return "SkSL::SampleUsage()";
        case Kind::kPassThrough: return "SkSL::SampleUsage::PassThrough()";
        case Kind::kExplicit:    return "SkSL::SampleUsage::Explicit()";
        default: SkUNREACHABLE;
    }
}

}  // namespace SkSL
