/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_APPENDSTAGE
#define SKSL_APPENDSTAGE

#ifndef SKSL_STANDALONE

#include "src/core/SkRasterPipeline.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

struct AppendStage : public Expression {
    AppendStage(const Context& context, int offset, SkRasterPipeline::StockStage stage,
                std::vector<std::unique_ptr<Expression>> arguments)
    : INHERITED(offset, kAppendStage_Kind, *context.fVoid_Type)
    , fStage(stage)
    , fArguments(std::move(arguments)) {}

    std::unique_ptr<Expression> clone() const override {
        std::vector<std::unique_ptr<Expression>> cloned;
        for (const auto& arg : fArguments) {
            cloned.push_back(arg->clone());
        }
        return std::unique_ptr<Expression>(new AppendStage(fOffset, fStage, std::move(cloned),
                                                           &fType));
    }

    String description() const override {
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

    bool hasSideEffects() const override {
        return true;
    }

    SkRasterPipeline::StockStage fStage;

    std::vector<std::unique_ptr<Expression>> fArguments;

    typedef Expression INHERITED;

private:
    AppendStage(int offset, SkRasterPipeline::StockStage stage,
                std::vector<std::unique_ptr<Expression>> arguments, const Type* type)
    : INHERITED(offset, kAppendStage_Kind, *type)
    , fStage(stage)
    , fArguments(std::move(arguments)) {}

};

} // namespace

#endif // SKSL_STANDALONE

#endif // SKSL_APPENDSTAGE
