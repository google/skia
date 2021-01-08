/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLExpression.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLIntLiteral.h"

#include "math.h"

namespace SkSL {

namespace dsl {

DSLExpression::DSLExpression() {}

DSLExpression::DSLExpression(std::unique_ptr<SkSL::Expression> expression)
    : fExpression(DSLWriter::Check(std::move(expression))) {}

DSLExpression::DSLExpression(float value)
    : fExpression(std::make_unique<SkSL::FloatLiteral>(DSLWriter::Context(),
                                                       /*offset=*/-1,
                                                       value)) {
    if (!isfinite(value)) {
        if (isinf(value)) {
            DSLWriter::ReportError("error: floating point value is infinite\n");
        } else if (isnan(value)) {
            DSLWriter::ReportError("error: floating point value is NaN\n");
        }
    }
}

DSLExpression::DSLExpression(int value)
    : fExpression(std::make_unique<SkSL::IntLiteral>(DSLWriter::Context(),
                                                     /*offset=*/-1,
                                                     value)) {}

DSLExpression::DSLExpression(bool value)
    : fExpression(std::make_unique<SkSL::BoolLiteral>(DSLWriter::Context(),
                                                     /*offset=*/-1,
                                                     value)) {}

DSLExpression::~DSLExpression() {
    SkASSERTF(fExpression == nullptr,
              "Expression destroyed without being incorporated into output tree");
}

std::unique_ptr<SkSL::Expression> DSLExpression::release() {
    return std::move(fExpression);
}

} // namespace dsl

} // namespace SkSL
