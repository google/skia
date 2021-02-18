/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLSwizzle.h"

namespace SkSL {

std::unique_ptr<Expression> Swizzle::Make(const Context& context,
                                          std::unique_ptr<Expression> base,
                                          ComponentArray inComponents) {
    const int offset = base->fOffset;
    const Type& baseType = base->type();

    // The IRGenerator is responsible for enforcing these invariants.
    SkASSERTF(baseType.isVector() || baseType.isScalar(),
              "cannot swizzle tyoe '%s'", baseType.description().c_str());
    SkASSERT(inComponents.count() > 0 && inComponents.count() <= 4);

    ComponentArray maskComponents;
    for (int component : inComponents) {
        switch (component) {
            case SwizzleComponent::ZERO:
            case SwizzleComponent::ONE:
                // Skip over constant fields for now.
                break;
            case SwizzleComponent::X:
                maskComponents.push_back(0);
                break;
            case SwizzleComponent::Y:
                if (baseType.columns() >= 2) {
                    maskComponents.push_back(1);
                    break;
                }
                [[fallthrough]];
            case SwizzleComponent::Z:
                if (baseType.columns() >= 3) {
                    maskComponents.push_back(2);
                    break;
                }
                [[fallthrough]];
            case SwizzleComponent::W:
                if (baseType.columns() >= 4) {
                    maskComponents.push_back(3);
                    break;
                }
                [[fallthrough]];
            default:
                SkDEBUGFAILF("invalid swizzle component %d", component);
                return nullptr;
        }
    }

    if (maskComponents.empty()) {
        context.fErrors.error(offset, "swizzle must refer to base expression");
        return nullptr;
    }

    // First, we need a vector expression that is the non-constant portion of the swizzle, packed:
    //   scalar.xxx  -> type3(scalar)
    //   scalar.x0x0 -> type2(scalar)
    //   vector.zyx  -> vector.zyx
    //   vector.x0y0 -> vector.xy
    std::unique_ptr<Expression> expr;
    if (baseType.isScalar()) {
        ExpressionArray scalarConstructorArgs;
        scalarConstructorArgs.push_back(std::move(base));
        expr = Constructor::Make(context, offset,
                                 baseType.toCompound(context, maskComponents.size(), 1),
                                 std::move(scalarConstructorArgs));
    } else {
        expr = std::make_unique<Swizzle>(context, std::move(base), maskComponents);
    }

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
    // We could create simpler IR in some cases by reordering here, if all fields are packed
    // contiguously. The benefits are minor, so skip the optimization to keep the algorithm simple.
    // The constructor will have at most three arguments: { base value, constant 0, constant 1 }
    ExpressionArray constructorArgs;
    constructorArgs.reserve_back(3);
    constructorArgs.push_back(std::move(expr));

    // Apply another swizzle to shuffle the constants into the correct place. Any constant values we
    // need are also tacked on to the end of the constructor.
    //   scalar.x0x0 -> type4(type2(x), 0).xyxy
    //   vector.y111 -> type4(vector.y, 1).xyyy
    //   vector.z10x -> type4(vector.zx, 1, 0).xzwy
    const Type* numberType = baseType.isNumber() ? &baseType : &baseType.componentType();
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

    // For some of our most common use cases ('.xyz0', '.xyz1'), we will now have an identity
    // swizzle; in those cases we can just return the constructor without the swizzle attached.
    for (int i = 0; i < swizzleComponents.count(); ++i) {
        if (i != (int)swizzleComponents[i]) {
            // The swizzle has an effect, so apply it.
            return std::make_unique<Swizzle>(context, std::move(expr), swizzleComponents);
        }
    }

    // The swizzle was a no-op; return the constructor expression directly.
    return expr;
}

}  // namespace SkSL
