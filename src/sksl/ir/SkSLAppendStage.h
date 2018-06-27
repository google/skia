/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_APPENDSTAGE
#define SKSL_APPENDSTAGE

#ifndef SKSL_STANDALONE

#include "SkRasterPipeline.h"
#include "SkSLContext.h"
#include "SkSLExpression.h"

namespace SkSL {

struct AppendStage : public Expression {
    AppendStage(const Context& context, int offset, SkRasterPipeline::StockStage stage,
                std::vector<std::unique_ptr<Expression>> arguments)
    : INHERITED(offset, kAppendStage_Kind, *context.fVoid_Type)
    , fStage(stage)
    , fArguments(std::move(arguments)) {}

    String description() const {
        String result = "append(";
        const char* separator = "";
        for (const auto& a : fArguments) {
            result += separator;
            result += a->description();
            separator = ", ";
        }
        result += ")";
        return result;
    }

    bool hasSideEffects() const {
        return true;
    }

    SkRasterPipeline::StockStage fStage;

    std::vector<std::unique_ptr<Expression>> fArguments;

    typedef Expression INHERITED;
};

} // namespace

#endif // SKSL_STANDALONE

#endif // SKSL_APPENDSTAGE
