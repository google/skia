/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLConstructorArray.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLTypeReference.h"

namespace SkSL {

static bool index_out_of_range(const Context& context, SKSL_INT index, const Expression& base) {
    if (index >= 0 && index < base.type().columns()) {
        return false;
    }

    context.fErrors->error(base.fLine, "index " + to_string(index) + " out of range for '" +
                                       base.type().displayName() + "'");
    return true;
}

const Type& IndexExpression::IndexType(const Context& context, const Type& type) {
    if (type.isMatrix()) {
        if (type.componentType() == *context.fTypes.fFloat) {
            switch (type.rows()) {
                case 2: return *context.fTypes.fFloat2;
                case 3: return *context.fTypes.fFloat3;
                case 4: return *context.fTypes.fFloat4;
                default: SkASSERT(false);
            }
        } else if (type.componentType() == *context.fTypes.fHalf) {
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
                                                     SymbolTable& symbolTable,
                                                     std::unique_ptr<Expression> base,
                                                     std::unique_ptr<Expression> index) {
    // Convert an array type reference: `int[10]`.
    if (base->is<TypeReference>()) {
        const Type& baseType = base->as<TypeReference>().value();
        SKSL_INT arraySize = baseType.convertArraySize(context, std::move(index));
        if (!arraySize) {
            return nullptr;
        }
        return TypeReference::Convert(context, base->fLine,
                                      symbolTable.addArrayDimension(&baseType, arraySize));
    }
    // Convert an index expression with an expression inside of it: `arr[a * 3]`.
    const Type& baseType = base->type();
    if (!baseType.isArray() && !baseType.isMatrix() && !baseType.isVector()) {
        context.fErrors->error(base->fLine,
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
        if (index_out_of_range(context, indexValue, *base)) {
            return nullptr;
        }
    }
    return IndexExpression::Make(context, std::move(base), std::move(index));
}

std::unique_ptr<Expression> IndexExpression::Make(const Context& context,
                                                  std::unique_ptr<Expression> base,
                                                  std::unique_ptr<Expression> index) {
    const Type& baseType = base->type();
    SkASSERT(baseType.isArray() || baseType.isMatrix() || baseType.isVector());
    SkASSERT(index->type().isInteger());

    const Expression* indexExpr = ConstantFolder::GetConstantValueForVariable(*index);
    if (indexExpr->isIntLiteral()) {
        SKSL_INT indexValue = indexExpr->as<Literal>().intValue();
        if (!index_out_of_range(context, indexValue, *base)) {
            if (baseType.isVector()) {
                // Constant array indexes on vectors can be converted to swizzles: `v[2]` --> `v.z`.
                // Swizzling is harmless and can unlock further simplifications for some base types.
                return Swizzle::Make(context, std::move(base), ComponentArray{(int8_t)indexValue});
            }

            if (baseType.isArray() && !base->hasSideEffects()) {
                // Indexing an constant array constructor with a constant index can just pluck out
                // the requested value from the array.
                const Expression* baseExpr = ConstantFolder::GetConstantValueForVariable(*base);
                if (baseExpr->is<ConstructorArray>()) {
                    const ConstructorArray& arrayCtor = baseExpr->as<ConstructorArray>();
                    const ExpressionArray& arguments = arrayCtor.arguments();
                    SkASSERT(arguments.count() == baseType.columns());

                    return arguments[indexValue]->clone();
                }
            }

            if (baseType.isMatrix() && !base->hasSideEffects()) {
                // Matrices can be constructed with vectors that don't line up on column boundaries,
                // so extracting out the values from the constructor can be tricky. Fortunately, we
                // can reconstruct an equivalent vector using `getConstantValue`. If we
                // can't extract the data using `getConstantValue`, it wasn't constant and
                // we're not obligated to simplify anything.
                const Expression* baseExpr = ConstantFolder::GetConstantValueForVariable(*base);
                int vecWidth = baseType.rows();
                const Type& scalarType = baseType.componentType();
                const Type& vecType = scalarType.toCompound(context, vecWidth, /*rows=*/1);
                indexValue *= vecWidth;

                ExpressionArray ctorArgs;
                ctorArgs.reserve_back(vecWidth);
                for (int slot = 0; slot < vecWidth; ++slot) {
                    skstd::optional<double> slotVal = baseExpr->getConstantValue(indexValue + slot);
                    if (slotVal.has_value()) {
                        ctorArgs.push_back(Literal::Make(baseExpr->fLine, *slotVal, &scalarType));
                    } else {
                        ctorArgs.reset();
                        break;
                    }
                }

                if (!ctorArgs.empty()) {
                    int line = ctorArgs.front()->fLine;
                    return ConstructorCompound::Make(context, line, vecType, std::move(ctorArgs));
                }
            }
        }
    }

    return std::make_unique<IndexExpression>(context, std::move(base), std::move(index));
}

}  // namespace SkSL
