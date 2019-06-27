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
    AppendStage(IRGenerator* irGenerator, int offset, SkRasterPipeline::StockStage stage,
                std::vector<std::unique_ptr<Expression>> arguments)
    : INHERITED(irGenerator, offset, kAppendStage_Kind, irGenerator->fContext.fVoid_Type)
    , fStage(stage)
    , fArguments(std::move(arguments)) {}

    IRNode::ID clone() const override {
/*        std::vector<std::unique_ptr<Expression>> cloned;
        for (const auto& arg : fArguments) {
            cloned.push_back(arg->clone());
        }
        return std::unique_ptr<Expression>(new AppendStage(fOffset, fStage, std::move(cloned),
                                                           &fType));*/
        abort();
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
    AppendStage(IRGenerator* irGenerator, int offset, SkRasterPipeline::StockStage stage,
                std::vector<IRNode::ID> arguments, const Type* type)
    : INHERITED(irGenerator, offset, kAppendStage_Kind, *type)
    , fStage(stage)
    , fArguments(std::move(arguments)) {}

};

} // namespace

#endif // SKSL_STANDALONE

#endif // SKSL_APPENDSTAGE
