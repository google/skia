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
    if (other.fExplicitCoords) { fExplicitCoords = true; }
    if (other.fPassThrough)    { fPassThrough    = true; }
    if (other.fHasPerspective) { fHasPerspective = true; }

    if (other.fKind == Kind::kVariable) {
        fKind = Kind::kVariable;
        fExpression.clear();
    } else if (other.fKind == Kind::kUniform) {
        if (fKind == Kind::kUniform) {
            if (fExpression != other.fExpression) {
                fKind = Kind::kVariable;
                fExpression.clear();
            } else {
                // Identical uniform expressions, so leave things as-is
            }
        } else if (fKind == Kind::kNone) {
            fKind = Kind::kUniform;
            fExpression = other.fExpression;
        } else {
            // We were already variable, so leave things as-is
            SkASSERT(fKind == Kind::kVariable);
        }
    } else {
        // other had no matrix information, so we're done
    }

    return *this;
}

std::string SampleUsage::constructor(std::string perspectiveExpression) const {
    SkASSERT(this->hasMatrix() || perspectiveExpression.empty());
    if (perspectiveExpression.empty()) {
        perspectiveExpression = fHasPerspective ? "true" : "false";
    }

    // Check for special cases where we can use our factories:
    if (!this->hasMatrix()) {
        if (fExplicitCoords && !fPassThrough) {
            return "SkSL::SampleUsage::Explicit()";
        } else if (fPassThrough && !fExplicitCoords) {
            return "SkSL::SampleUsage::PassThrough()";
        }
    }
    if (!fExplicitCoords && !fPassThrough) {
        if (fKind == Kind::kVariable) {
            return "SkSL::SampleUsage::VariableMatrix(" + perspectiveExpression + ")";
        } else if (fKind == Kind::kUniform) {
            return "SkSL::SampleUsage::UniformMatrix(\"" + fExpression + "\", " +
                   perspectiveExpression + ")";
        }
    }

    // For more complex scenarios (mixed sampling), fall back to our universal constructor
    std::string result = "SkSL::SampleUsage(SkSL::SampleUsage::Kind::";
    switch (fKind) {
        case Kind::kNone:     result += "kNone";     break;
        case Kind::kUniform:  result += "kUniform";  break;
        case Kind::kVariable: result += "kVariable"; break;
    }
    result += ", \"";
    result += fExpression;
    result += "\", ";
    result += perspectiveExpression;
    result += ", ";
    result += fExplicitCoords ? "true, " : "false, ";
    result += fPassThrough    ? "true)"  : "false)";

    return result;
}

}  // namespace SkSL
