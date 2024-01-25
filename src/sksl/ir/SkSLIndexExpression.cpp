/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLIndexExpression.h"

#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/ir/SkSLConstructorArray.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLSymbolTable.h"  // IWYU pragma: keep
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLTypeReference.h"

#include <cstdint>
#include <optional>

namespace SkSL {

static bool index_out_of_range(const Context& context, Position pos, SKSL_INT index,
        const Expression& base) {
    if (index >= 0) {
        if (base.type().columns() == Type::kUnsizedArray) {
            return false;
        } else if (index < base.type().columns()) {
            return false;
        }
    }
    context.fErrors->error(pos, "index " + std::to_string(index) + " out of range for '" +
                                base.type().displayName() + "'");
    return true;
}

const Type& IndexExpression::IndexType(const Context& context, const Type& type) {
    if (type.isMatrix()) {
        if (type.componentType().matches(*context.fTypes.fFloat)) {
            switch (type.rows()) {
                case 2: return *context.fTypes.fFloat2;
                case 3: return *context.fTypes.fFloat3;
                case 4: return *context.fTypes.fFloat4;
                default: SkASSERT(false);
            }
        } else if (type.componentType().matches(*context.fTypes.fHalf)) {
            switch (type.rows()) {
                case 2: return *context.fTypes.fHalf2;
                case 3: return *context.fTypes.fHalf3;
                case 4: return *context.fTypes.fHalf4;
                default: SkASSERT(false);
            }
        }
    }
    return type.componentType();
}

std::unique_ptr<Expression> IndexExpression::Convert(const Context& context,
                                                     Position pos,
                                                     std::unique_ptr<Expression> base,
                                                     std::unique_ptr<Expression> index) {
    // Convert an array type reference: `int[10]`.
    if (base->is<TypeReference>()) {
        const Type& baseType = base->as<TypeReference>().value();
        SKSL_INT arraySize = baseType.convertArraySize(context, pos, std::move(index));
        if (!arraySize) {
            return nullptr;
        }
        return TypeReference::Convert(
                context,
                pos,
                context.fSymbolTable->addArrayDimension(context, &baseType, arraySize));
    }
    // Convert an index expression with an expression inside of it: `arr[a * 3]`.
    const Type& baseType = base->type();
    if (!baseType.isArray() && !baseType.isMatrix() && !baseType.isVector()) {
        context.fErrors->error(base->fPosition,
                               "expected array, but found '" + baseType.displayName() + "'");
        return nullptr;
    }
    if (!index->type().isInteger()) {
        index = context.fTypes.fInt->coerceExpression(std::move(index), context);
        if (!index) {
            return nullptr;
        }
    }
    // Perform compile-time bounds checking on constant-expression indices.
    const Expression* indexExpr = ConstantFolder::GetConstantValueForVariable(*index);
    if (indexExpr->isIntLiteral()) {
        SKSL_INT indexValue = indexExpr->as<Literal>().intValue();
        if (index_out_of_range(context, index->fPosition, indexValue, *base)) {
            return nullptr;
        }
    }
    return IndexExpression::Make(context, pos, std::move(base), std::move(index));
}

std::unique_ptr<Expression> IndexExpression::Make(const Context& context,
                                                  Position pos,
                                                  std::unique_ptr<Expression> base,
                                                  std::unique_ptr<Expression> index) {
    const Type& baseType = base->type();
    SkASSERT(baseType.isArray() || baseType.isMatrix() || baseType.isVector());
    SkASSERT(index->type().isInteger());

    const Expression* indexExpr = ConstantFolder::GetConstantValueForVariable(*index);
    if (indexExpr->isIntLiteral()) {
        SKSL_INT indexValue = indexExpr->as<Literal>().intValue();
        if (!index_out_of_range(context, index->fPosition, indexValue, *base)) {
            if (baseType.isVector()) {
                // Constant array indexes on vectors can be converted to swizzles: `v[2]` --> `v.z`.
                // Swizzling is harmless and can unlock further simplifications for some base types.
                return Swizzle::Make(context, pos, std::move(base),
                        ComponentArray{(int8_t)indexValue});
            }

            if (baseType.isArray() && !Analysis::HasSideEffects(*base)) {
                // Indexing an constant array constructor with a constant index can just pluck out
                // the requested value from the array.
                const Expression* baseExpr = ConstantFolder::GetConstantValueForVariable(*base);
                if (baseExpr->is<ConstructorArray>()) {
                    const ConstructorArray& arrayCtor = baseExpr->as<ConstructorArray>();
                    const ExpressionArray& arguments = arrayCtor.arguments();
                    SkASSERT(arguments.size() == baseType.columns());

                    return arguments[indexValue]->clone(pos);
                }
            }

            if (baseType.isMatrix() && !Analysis::HasSideEffects(*base)) {
                // Matrices can be constructed with vectors that don't line up on column boundaries,
                // so extracting out the values from the constructor can be tricky. Fortunately, we
                // can reconstruct an equivalent vector using `getConstantValue`. If we
                // can't extract the data using `getConstantValue`, it wasn't constant and
                // we're not obligated to simplify anything.
                const Expression* baseExpr = ConstantFolder::GetConstantValueForVariable(*base);
                int vecWidth = baseType.rows();
                const Type& vecType = baseType.columnType(context);
                indexValue *= vecWidth;

                double ctorArgs[4];
                bool allConstant = true;
                for (int slot = 0; slot < vecWidth; ++slot) {
                    std::optional<double> slotVal = baseExpr->getConstantValue(indexValue + slot);
                    if (slotVal.has_value()) {
                        ctorArgs[slot] = *slotVal;
                    } else {
                        allConstant = false;
                        break;
                    }
                }

                if (allConstant) {
                    return ConstructorCompound::MakeFromConstants(context, pos, vecType, ctorArgs);
                }
            }
        }
    }

    return std::make_unique<IndexExpression>(context, pos, std::move(base), std::move(index));
}

std::string IndexExpression::description(OperatorPrecedence) const {
    return this->base()->description(OperatorPrecedence::kPostfix) + "[" +
           this->index()->description(OperatorPrecedence::kExpression) + "]";
}

}  // namespace SkSL
