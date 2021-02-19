/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLSwizzle.h"

namespace SkSL {

std::unique_ptr<Expression> Swizzle::MakeWith01(const Context& context,
                                                std::unique_ptr<Expression> base,
                                                ComponentArray inComponents) {
    const int offset = base->fOffset;
    const Type& baseType = base->type();

    // The IRGenerator is responsible for enforcing these invariants.
    SkASSERTF(baseType.isVector() || baseType.isScalar(),
              "cannot swizzle type '%s'", baseType.description().c_str());
    SkASSERT(inComponents.count() >= 1 && inComponents.count() <= 4);

    ComponentArray maskComponents;
    for (int8_t component : inComponents) {
        switch (component) {
            case SwizzleComponent::ZERO:
            case SwizzleComponent::ONE:
                // Skip over constant fields for now.
                break;
            case SwizzleComponent::X:
                maskComponents.push_back(SwizzleComponent::X);
                break;
            case SwizzleComponent::Y:
                if (baseType.columns() >= 2) {
                    maskComponents.push_back(SwizzleComponent::Y);
                    break;
                }
                [[fallthrough]];
            case SwizzleComponent::Z:
                if (baseType.columns() >= 3) {
                    maskComponents.push_back(SwizzleComponent::Z);
                    break;
                }
                [[fallthrough]];
            case SwizzleComponent::W:
                if (baseType.columns() >= 4) {
                    maskComponents.push_back(SwizzleComponent::W);
                    break;
                }
                [[fallthrough]];
            default:
                SkDEBUGFAILF("invalid swizzle component %d", component);
                return nullptr;
        }
    }

    // First, we need a vector expression that is the non-constant portion of the swizzle, packed:
    //   scalar.xxx  -> type3(scalar)
    //   scalar.x0x0 -> type2(scalar)
    //   vector.zyx  -> vector.zyx
    //   vector.x0y0 -> vector.xy
    std::unique_ptr<Expression> expr = Swizzle::Make(context, std::move(base), maskComponents);

    // If we have processed the entire swizzle, we're done.
    if (maskComponents.count() == inComponents.count()) {
        return expr;
    }

    // Now we create a constructor that has the correct number of elements for the final swizzle,
    // with all fields at the start. It's not finished yet; constants we need will be added below.
    //   scalar.x0x0 -> type4(type2(x), ...)
    //   vector.y111 -> type4(vector.y, ...)
    //   vector.z10x -> type4(vector.zx, ...)
    //
    // The constructor will have at most three arguments: { base expr, constant 0, constant 1 }
    ExpressionArray constructorArgs;
    constructorArgs.reserve_back(3);
    constructorArgs.push_back(std::move(expr));

    // Apply another swizzle to shuffle the constants into the correct place. Any constant values we
    // need are also tacked on to the end of the constructor.
    //   scalar.x0x0 -> type4(type2(x), 0).xyxy
    //   vector.y111 -> type4(vector.y, 1).xyyy
    //   vector.z10x -> type4(vector.zx, 1, 0).xzwy
    const Type* numberType = &baseType.componentType();
    ComponentArray swizzleComponents;
    int maskFieldIdx = 0;
    int constantFieldIdx = maskComponents.size();
    int constantZeroIdx = -1, constantOneIdx = -1;

    for (int i = 0; i < inComponents.count(); i++) {
        switch (inComponents[i]) {
            case SwizzleComponent::ZERO:
                if (constantZeroIdx == -1) {
                    // Synthesize a 'type(0)' argument at the end of the constructor.
                    ExpressionArray zeroArgs;
                    zeroArgs.push_back(std::make_unique<IntLiteral>(context, offset,/*fValue=*/0));
                    constructorArgs.push_back(Constructor::Make(context, offset, *numberType,
                                                                std::move(zeroArgs)));
                    constantZeroIdx = constantFieldIdx++;
                }
                swizzleComponents.push_back(constantZeroIdx);
                break;
            case SwizzleComponent::ONE:
                if (constantOneIdx == -1) {
                    // Synthesize a 'type(1)' argument at the end of the constructor.
                    ExpressionArray oneArgs;
                    oneArgs.push_back(std::make_unique<IntLiteral>(context, offset, /*fValue=*/1));
                    constructorArgs.push_back(Constructor::Make(context, offset, *numberType,
                                                                std::move(oneArgs)));
                    constantOneIdx = constantFieldIdx++;
                }
                swizzleComponents.push_back(constantOneIdx);
                break;
            default:
                // The non-constant fields are already in the expected order.
                swizzleComponents.push_back(maskFieldIdx++);
                break;
        }
    }

    expr = Constructor::Make(context, offset,
                             numberType->toCompound(context, constantFieldIdx, /*rows=*/1),
                             std::move(constructorArgs));

    return Swizzle::Make(context, std::move(expr), swizzleComponents);
}

std::unique_ptr<Expression> Swizzle::Make(const Context& context,
                                          std::unique_ptr<Expression> expr,
                                          ComponentArray components) {
    // The IRGenerator is responsible for enforcing these invariants.
    const Type& exprType = expr->type();
    SkASSERTF(exprType.isVector() || exprType.isScalar(),
              "cannot swizzle type '%s'", exprType.description().c_str());
    SkASSERT(components.count() >= 1 && components.count() <= 4);

    // Confirm that the component array only contains X/Y/Z/W. (Call MakeWith01 if you want support
    // for ZERO and ONE. Once initial IR generation is complete, no swizzles should have zeros or
    // ones in them.)
    SkASSERT(std::all_of(components.begin(), components.end(), [](int8_t component) {
        return component >= SwizzleComponent::X &&
               component <= SwizzleComponent::W;
    }));

    // SkSL supports splatting a scalar via `scalar.xxxx`, but not all versions of GLSL allow this.
    // Replace swizzles with equivalent constructors (`scalar.xxx` --> `half3(value)`).
    if (exprType.isScalar()) {
        int offset = expr->fOffset;

        ExpressionArray ctorArgs;
        ctorArgs.push_back(std::move(expr));
        return Constructor::Make(context, offset,
                                 exprType.toCompound(context, components.size(), /*rows=*/1),
                                 std::move(ctorArgs));
    }

    if (context.fConfig->fSettings.fOptimize) {
        // Detect identity swizzles like `color.rgba` and return the base-expression as-is.
        if (components.count() == exprType.columns()) {
            bool identity = true;
            for (int i = 0; i < components.count(); ++i) {
                if (components[i] != i) {
                    identity = false;
                    break;
                }
            }
            if (identity) {
                return expr;
            }
        }
    }

    // The swizzle could not be simplified, so apply the requested swizzle to the base expression.
    return std::make_unique<Swizzle>(context, std::move(expr), components);
}

}  // namespace SkSL
