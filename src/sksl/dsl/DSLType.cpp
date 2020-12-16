/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLType.h"

#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLConstructor.h"

namespace SkSL {

namespace dsl {

const SkSL::Type& DSLType::skslType() const {
    if (fSkSLType) {
        return *fSkSLType;
    }
    const SkSL::Context& context = DSLWriter::Context();
    switch (fTypeConstant) {
        case kBool:
            return *context.fBool_Type;
        case kBool2:
            return *context.fBool2_Type;
        case kBool3:
            return *context.fBool3_Type;
        case kBool4:
            return *context.fBool4_Type;
        case kHalf:
            return *context.fHalf_Type;
        case kHalf2:
            return *context.fHalf2_Type;
        case kHalf3:
            return *context.fHalf3_Type;
        case kHalf4:
            return *context.fHalf4_Type;
        case kHalf2x2:
            return *context.fHalf2x2_Type;
        case kHalf3x2:
            return *context.fHalf3x2_Type;
        case kHalf4x2:
            return *context.fHalf4x2_Type;
        case kHalf2x3:
            return *context.fHalf2x3_Type;
        case kHalf3x3:
            return *context.fHalf3x3_Type;
        case kHalf4x3:
            return *context.fHalf4x3_Type;
        case kHalf2x4:
            return *context.fHalf2x4_Type;
        case kHalf3x4:
            return *context.fHalf3x4_Type;
        case kHalf4x4:
            return *context.fHalf4x4_Type;
        case kFloat:
            return *context.fFloat_Type;
        case kFloat2:
            return *context.fFloat2_Type;
        case kFloat3:
            return *context.fFloat3_Type;
        case kFloat4:
            return *context.fFloat4_Type;
        case kFloat2x2:
            return *context.fFloat2x2_Type;
        case kFloat3x2:
            return *context.fFloat3x2_Type;
        case kFloat4x2:
            return *context.fFloat4x2_Type;
        case kFloat2x3:
            return *context.fFloat2x3_Type;
        case kFloat3x3:
            return *context.fFloat3x3_Type;
        case kFloat4x3:
            return *context.fFloat4x3_Type;
        case kFloat2x4:
            return *context.fFloat2x4_Type;
        case kFloat3x4:
            return *context.fFloat3x4_Type;
        case kFloat4x4:
            return *context.fFloat4x4_Type;
        case kInt:
            return *context.fInt_Type;
        case kInt2:
            return *context.fInt2_Type;
        case kInt3:
            return *context.fInt3_Type;
        case kInt4:
            return *context.fInt4_Type;
        case kShort:
            return *context.fShort_Type;
        case kShort2:
            return *context.fShort2_Type;
        case kShort3:
            return *context.fShort3_Type;
        case kShort4:
            return *context.fShort4_Type;
        case kVoid:
            return *context.fVoid_Type;
        default:
            SkUNREACHABLE;
    }
}

DSLExpression dsl_construct(const SkSL::Type& type, std::vector<DSLExpression> rawArgs) {
    SkSL::ExpressionArray args;
    for (DSLExpression& arg : rawArgs) {
        args.push_back(arg.release());
    }
    return DSLExpression(DSLWriter::IRGenerator().call(
                                         /*offset=*/-1,
                                         std::make_unique<SkSL::TypeReference>(DSLWriter::Context(),
                                                                               /*offset=*/-1,
                                                                               &type),
                                         std::move(args)));
}

DSLType Array(const DSLType& base, int count) {
    SkSL::String name = base.skslType().name() + "[" + SkSL::to_string(count) + "]";
    return DSLType(DSLWriter::SymbolTable()->takeOwnershipOfSymbol(
                                          SkSL::Type::MakeArrayType(name, base.skslType(), count)));
}

} // namespace dsl

} // namespace SkSL
