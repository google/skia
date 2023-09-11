/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLSwizzle.h"

#include "include/core/SkSpan.h"
#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLConstructorCompoundCast.h"
#include "src/sksl/ir/SkSLConstructorScalarCast.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"
#include "src/sksl/ir/SkSLLiteral.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>

using namespace skia_private;

namespace SkSL {

static bool validate_swizzle_domain(const ComponentArray& fields) {
    enum SwizzleDomain {
        kCoordinate,
        kColor,
        kUV,
        kRectangle,
    };

    std::optional<SwizzleDomain> domain;

    for (int8_t field : fields) {
        SwizzleDomain fieldDomain;
        switch (field) {
            case SwizzleComponent::X:
            case SwizzleComponent::Y:
            case SwizzleComponent::Z:
            case SwizzleComponent::W:
                fieldDomain = kCoordinate;
                break;
            case SwizzleComponent::R:
            case SwizzleComponent::G:
            case SwizzleComponent::B:
            case SwizzleComponent::A:
                fieldDomain = kColor;
                break;
            case SwizzleComponent::S:
            case SwizzleComponent::T:
            case SwizzleComponent::P:
            case SwizzleComponent::Q:
                fieldDomain = kUV;
                break;
            case SwizzleComponent::UL:
            case SwizzleComponent::UT:
            case SwizzleComponent::UR:
            case SwizzleComponent::UB:
                fieldDomain = kRectangle;
                break;
            case SwizzleComponent::ZERO:
            case SwizzleComponent::ONE:
                continue;
            default:
                return false;
        }

        if (!domain.has_value()) {
            domain = fieldDomain;
        } else if (domain != fieldDomain) {
            return false;
        }
    }

    return true;
}

static char mask_char(int8_t component) {
    switch (component) {
        case SwizzleComponent::X:    return 'x';
        case SwizzleComponent::Y:    return 'y';
        case SwizzleComponent::Z:    return 'z';
        case SwizzleComponent::W:    return 'w';
        case SwizzleComponent::R:    return 'r';
        case SwizzleComponent::G:    return 'g';
        case SwizzleComponent::B:    return 'b';
        case SwizzleComponent::A:    return 'a';
        case SwizzleComponent::S:    return 's';
        case SwizzleComponent::T:    return 't';
        case SwizzleComponent::P:    return 'p';
        case SwizzleComponent::Q:    return 'q';
        case SwizzleComponent::UL:   return 'L';
        case SwizzleComponent::UT:   return 'T';
        case SwizzleComponent::UR:   return 'R';
        case SwizzleComponent::UB:   return 'B';
        case SwizzleComponent::ZERO: return '0';
        case SwizzleComponent::ONE:  return '1';
        default: SkUNREACHABLE;
    }
}

std::string Swizzle::MaskString(const ComponentArray& components) {
    std::string result;
    for (int8_t component : components) {
        result += mask_char(component);
    }
    return result;
}

static std::unique_ptr<Expression> optimize_constructor_swizzle(const Context& context,
                                                                Position pos,
                                                                const ConstructorCompound& base,
                                                                ComponentArray components) {
    auto baseArguments = base.argumentSpan();
    std::unique_ptr<Expression> replacement;
    const Type& exprType = base.type();
    const Type& componentType = exprType.componentType();
    int swizzleSize = components.size();

    // Swizzles can duplicate some elements and discard others, e.g.
    // `half4(1, 2, 3, 4).xxz` --> `half3(1, 1, 3)`. However, there are constraints:
    // - Expressions with side effects need to occur exactly once, even if they would otherwise be
    //   swizzle-eliminated
    // - Non-trivial expressions should not be repeated, but elimination is OK.
    //
    // Look up the argument for the constructor at each index. This is typically simple but for
    // weird cases like `half4(bar.yz, half2(foo))`, it can be harder than it seems. This example
    // would result in:
    //     argMap[0] = {.fArgIndex = 0, .fComponent = 0}   (bar.yz     .x)
    //     argMap[1] = {.fArgIndex = 0, .fComponent = 1}   (bar.yz     .y)
    //     argMap[2] = {.fArgIndex = 1, .fComponent = 0}   (half2(foo) .x)
    //     argMap[3] = {.fArgIndex = 1, .fComponent = 1}   (half2(foo) .y)
    struct ConstructorArgMap {
        int8_t fArgIndex;
        int8_t fComponent;
    };

    int numConstructorArgs = base.type().columns();
    ConstructorArgMap argMap[4] = {};
    int writeIdx = 0;
    for (int argIdx = 0; argIdx < (int)baseArguments.size(); ++argIdx) {
        const Expression& arg = *baseArguments[argIdx];
        const Type& argType = arg.type();

        if (!argType.isScalar() && !argType.isVector()) {
            return nullptr;
        }

        int argSlots = argType.slotCount();
        for (int componentIdx = 0; componentIdx < argSlots; ++componentIdx) {
            argMap[writeIdx].fArgIndex = argIdx;
            argMap[writeIdx].fComponent = componentIdx;
            ++writeIdx;
        }
    }
    SkASSERT(writeIdx == numConstructorArgs);

    // Count up the number of times each constructor argument is used by the swizzle.
    //    `half4(bar.yz, half2(foo)).xwxy` -> { 3, 1 }
    // - bar.yz    is referenced 3 times, by `.x_xy`
    // - half(foo) is referenced 1 time,  by `._w__`
    int8_t exprUsed[4] = {};
    for (int8_t c : components) {
        exprUsed[argMap[c].fArgIndex]++;
    }

    for (int index = 0; index < numConstructorArgs; ++index) {
        int8_t constructorArgIndex = argMap[index].fArgIndex;
        const Expression& baseArg = *baseArguments[constructorArgIndex];

        // Check that non-trivial expressions are not swizzled in more than once.
        if (exprUsed[constructorArgIndex] > 1 && !Analysis::IsTrivialExpression(baseArg)) {
            return nullptr;
        }
        // Check that side-effect-bearing expressions are swizzled in exactly once.
        if (exprUsed[constructorArgIndex] != 1 && Analysis::HasSideEffects(baseArg)) {
            return nullptr;
        }
    }

    struct ReorderedArgument {
        int8_t fArgIndex;
        ComponentArray fComponents;
    };
    STArray<4, ReorderedArgument> reorderedArgs;
    for (int8_t c : components) {
        const ConstructorArgMap& argument = argMap[c];
        const Expression& baseArg = *baseArguments[argument.fArgIndex];

        if (baseArg.type().isScalar()) {
            // This argument is a scalar; add it to the list as-is.
            SkASSERT(argument.fComponent == 0);
            reorderedArgs.push_back({argument.fArgIndex,
                                     ComponentArray{}});
        } else {
            // This argument is a component from a vector.
            SkASSERT(baseArg.type().isVector());
            SkASSERT(argument.fComponent < baseArg.type().columns());
            if (reorderedArgs.empty() ||
                reorderedArgs.back().fArgIndex != argument.fArgIndex) {
                // This can't be combined with the previous argument. Add a new one.
                reorderedArgs.push_back({argument.fArgIndex,
                                         ComponentArray{argument.fComponent}});
            } else {
                // Since we know this argument uses components, it should already have at least one
                // component set.
                SkASSERT(!reorderedArgs.back().fComponents.empty());
                // Build up the current argument with one more component.
                reorderedArgs.back().fComponents.push_back(argument.fComponent);
            }
        }
    }

    // Convert our reordered argument list to an actual array of expressions, with the new order and
    // any new inner swizzles that need to be applied.
    ExpressionArray newArgs;
    newArgs.reserve_exact(swizzleSize);
    for (const ReorderedArgument& reorderedArg : reorderedArgs) {
        std::unique_ptr<Expression> newArg = baseArguments[reorderedArg.fArgIndex]->clone();

        if (reorderedArg.fComponents.empty()) {
            newArgs.push_back(std::move(newArg));
        } else {
            newArgs.push_back(Swizzle::Make(context, pos, std::move(newArg),
                                            reorderedArg.fComponents));
        }
    }

    // Wrap the new argument list in a compound constructor.
    return ConstructorCompound::Make(context,
                                     pos,
                                     componentType.toCompound(context, swizzleSize, /*rows=*/1),
                                     std::move(newArgs));
}

std::unique_ptr<Expression> Swizzle::Convert(const Context& context,
                                             Position pos,
                                             Position maskPos,
                                             std::unique_ptr<Expression> base,
                                             std::string_view maskString) {
    ComponentArray components;
    for (size_t i = 0; i < maskString.length(); ++i) {
        char field = maskString[i];
        switch (field) {
            case '0': components.push_back(SwizzleComponent::ZERO); break;
            case '1': components.push_back(SwizzleComponent::ONE);  break;
            case 'x': components.push_back(SwizzleComponent::X);    break;
            case 'r': components.push_back(SwizzleComponent::R);    break;
            case 's': components.push_back(SwizzleComponent::S);    break;
            case 'L': components.push_back(SwizzleComponent::UL);   break;
            case 'y': components.push_back(SwizzleComponent::Y);    break;
            case 'g': components.push_back(SwizzleComponent::G);    break;
            case 't': components.push_back(SwizzleComponent::T);    break;
            case 'T': components.push_back(SwizzleComponent::UT);   break;
            case 'z': components.push_back(SwizzleComponent::Z);    break;
            case 'b': components.push_back(SwizzleComponent::B);    break;
            case 'p': components.push_back(SwizzleComponent::P);    break;
            case 'R': components.push_back(SwizzleComponent::UR);   break;
            case 'w': components.push_back(SwizzleComponent::W);    break;
            case 'a': components.push_back(SwizzleComponent::A);    break;
            case 'q': components.push_back(SwizzleComponent::Q);    break;
            case 'B': components.push_back(SwizzleComponent::UB);   break;
            default:
                context.fErrors->error(Position::Range(maskPos.startOffset() + i,
                                                       maskPos.startOffset() + i + 1),
                                       String::printf("invalid swizzle component '%c'", field));
                return nullptr;
        }
    }
    return Convert(context, pos, maskPos, std::move(base), std::move(components));
}

// Swizzles are complicated due to constant components. The most difficult case is a mask like
// '.x1w0'. A naive approach might turn that into 'float4(base.x, 1, base.w, 0)', but that evaluates
// 'base' twice. We instead group the swizzle mask ('xw') and constants ('1, 0') together and use a
// secondary swizzle to put them back into the right order, so in this case we end up with
// 'float4(base.xw, 1, 0).xzyw'.
std::unique_ptr<Expression> Swizzle::Convert(const Context& context,
                                             Position pos,
                                             Position maskPos,
                                             std::unique_ptr<Expression> base,
                                             ComponentArray inComponents) {
    if (inComponents.size() > 4) {
        context.fErrors->error(Position::Range(maskPos.startOffset() + 4,
                                               maskPos.endOffset()),
                               "too many components in swizzle mask");
        return nullptr;
    }

    if (!validate_swizzle_domain(inComponents)) {
        context.fErrors->error(maskPos, "invalid swizzle mask '" + MaskString(inComponents) + "'");
        return nullptr;
    }

    const Type& baseType = base->type().scalarTypeForLiteral();

    if (!baseType.isVector() && !baseType.isScalar()) {
        context.fErrors->error(pos, "cannot swizzle value of type '" +
                                    baseType.displayName() + "'");
        return nullptr;
    }

    ComponentArray maskComponents;
    bool foundXYZW = false;
    for (int i = 0; i < inComponents.size(); ++i) {
        switch (inComponents[i]) {
            case SwizzleComponent::ZERO:
            case SwizzleComponent::ONE:
                // Skip over constant fields for now.
                break;
            case SwizzleComponent::X:
            case SwizzleComponent::R:
            case SwizzleComponent::S:
            case SwizzleComponent::UL:
                foundXYZW = true;
                maskComponents.push_back(SwizzleComponent::X);
                break;
            case SwizzleComponent::Y:
            case SwizzleComponent::G:
            case SwizzleComponent::T:
            case SwizzleComponent::UT:
                foundXYZW = true;
                if (baseType.columns() >= 2) {
                    maskComponents.push_back(SwizzleComponent::Y);
                    break;
                }
                [[fallthrough]];
            case SwizzleComponent::Z:
            case SwizzleComponent::B:
            case SwizzleComponent::P:
            case SwizzleComponent::UR:
                foundXYZW = true;
                if (baseType.columns() >= 3) {
                    maskComponents.push_back(SwizzleComponent::Z);
                    break;
                }
                [[fallthrough]];
            case SwizzleComponent::W:
            case SwizzleComponent::A:
            case SwizzleComponent::Q:
            case SwizzleComponent::UB:
                foundXYZW = true;
                if (baseType.columns() >= 4) {
                    maskComponents.push_back(SwizzleComponent::W);
                    break;
                }
                [[fallthrough]];
            default:
                // The swizzle component references a field that doesn't exist in the base type.
                context.fErrors->error(Position::Range(maskPos.startOffset() + i,
                                                       maskPos.startOffset() + i + 1),
                                       String::printf("invalid swizzle component '%c'",
                                                      mask_char(inComponents[i])));
                return nullptr;
        }
    }

    if (!foundXYZW) {
        context.fErrors->error(maskPos, "swizzle must refer to base expression");
        return nullptr;
    }

    // Coerce literals in expressions such as `(12345).xxx` to their actual type.
    base = baseType.coerceExpression(std::move(base), context);
    if (!base) {
        return nullptr;
    }

    // First, we need a vector expression that is the non-constant portion of the swizzle, packed:
    //   scalar.xxx  -> type3(scalar)
    //   scalar.x0x0 -> type2(scalar)
    //   vector.zyx  -> vector.zyx
    //   vector.x0y0 -> vector.xy
    std::unique_ptr<Expression> expr = Swizzle::Make(context, pos, std::move(base), maskComponents);

    // If we have processed the entire swizzle, we're done.
    if (maskComponents.size() == inComponents.size()) {
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
    constructorArgs.reserve_exact(3);
    constructorArgs.push_back(std::move(expr));

    // Apply another swizzle to shuffle the constants into the correct place. Any constant values we
    // need are also tacked on to the end of the constructor.
    //   scalar.x0x0 -> type4(type2(x), 0).xyxy
    //   vector.y111 -> type2(vector.y, 1).xyyy
    //   vector.z10x -> type4(vector.zx, 1, 0).xzwy
    const Type* scalarType = &baseType.componentType();
    ComponentArray swizzleComponents;
    int maskFieldIdx = 0;
    int constantFieldIdx = maskComponents.size();
    int constantZeroIdx = -1, constantOneIdx = -1;

    for (int i = 0; i < inComponents.size(); i++) {
        switch (inComponents[i]) {
            case SwizzleComponent::ZERO:
                if (constantZeroIdx == -1) {
                    // Synthesize a '0' argument at the end of the constructor.
                    constructorArgs.push_back(Literal::Make(pos, /*value=*/0, scalarType));
                    constantZeroIdx = constantFieldIdx++;
                }
                swizzleComponents.push_back(constantZeroIdx);
                break;
            case SwizzleComponent::ONE:
                if (constantOneIdx == -1) {
                    // Synthesize a '1' argument at the end of the constructor.
                    constructorArgs.push_back(Literal::Make(pos, /*value=*/1, scalarType));
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

    expr = ConstructorCompound::Make(context, pos,
                                     scalarType->toCompound(context, constantFieldIdx, /*rows=*/1),
                                     std::move(constructorArgs));

    // Create (and potentially optimize-away) the resulting swizzle-expression.
    return Swizzle::Make(context, pos, std::move(expr), swizzleComponents);
}

std::unique_ptr<Expression> Swizzle::Make(const Context& context,
                                          Position pos,
                                          std::unique_ptr<Expression> expr,
                                          ComponentArray components) {
    const Type& exprType = expr->type();
    SkASSERTF(exprType.isVector() || exprType.isScalar(),
              "cannot swizzle type '%s'", exprType.description().c_str());
    SkASSERT(components.size() >= 1 && components.size() <= 4);

    // Confirm that the component array only contains X/Y/Z/W. (Call MakeWith01 if you want support
    // for ZERO and ONE. Once initial IR generation is complete, no swizzles should have zeros or
    // ones in them.)
    SkASSERT(std::all_of(components.begin(), components.end(), [](int8_t component) {
        return component >= SwizzleComponent::X &&
               component <= SwizzleComponent::W;
    }));

    // SkSL supports splatting a scalar via `scalar.xxxx`, but not all versions of GLSL allow this.
    // Replace swizzles with equivalent splat constructors (`scalar.xxx` --> `half3(value)`).
    if (exprType.isScalar()) {
        return ConstructorSplat::Make(context, pos,
                                      exprType.toCompound(context, components.size(), /*rows=*/1),
                                      std::move(expr));
    }

    // Detect identity swizzles like `color.rgba` and optimize it away.
    if (components.size() == exprType.columns()) {
        bool identity = true;
        for (int i = 0; i < components.size(); ++i) {
            if (components[i] != i) {
                identity = false;
                break;
            }
        }
        if (identity) {
            expr->fPosition = pos;
            return expr;
        }
    }

    // Optimize swizzles of swizzles, e.g. replace `foo.argb.rggg` with `foo.arrr`.
    if (expr->is<Swizzle>()) {
        Swizzle& base = expr->as<Swizzle>();
        ComponentArray combined;
        for (int8_t c : components) {
            combined.push_back(base.components()[c]);
        }

        // It may actually be possible to further simplify this swizzle. Go again.
        // (e.g. `color.abgr.abgr` --> `color.rgba` --> `color`.)
        return Swizzle::Make(context, pos, std::move(base.base()), combined);
    }

    // If we are swizzling a constant expression, we can use its value instead here (so that
    // swizzles like `colorWhite.x` can be simplified to `1`).
    const Expression* value = ConstantFolder::GetConstantValueForVariable(*expr);

    // `half4(scalar).zyy` can be optimized to `half3(scalar)`, and `half3(scalar).y` can be
    // optimized to just `scalar`. The swizzle components don't actually matter, as every field
    // in a splat constructor holds the same value.
    if (value->is<ConstructorSplat>()) {
        const ConstructorSplat& splat = value->as<ConstructorSplat>();
        return ConstructorSplat::Make(
                context, pos,
                splat.type().componentType().toCompound(context, components.size(), /*rows=*/1),
                splat.argument()->clone());
    }

    // Swizzles on casts, like `half4(myFloat4).zyy`, can optimize to `half3(myFloat4.zyy)`.
    if (value->is<ConstructorCompoundCast>()) {
        const ConstructorCompoundCast& cast = value->as<ConstructorCompoundCast>();
        const Type& castType = cast.type().componentType().toCompound(context, components.size(),
                                                                      /*rows=*/1);
        std::unique_ptr<Expression> swizzled = Swizzle::Make(context, pos, cast.argument()->clone(),
                                                             std::move(components));
        return (castType.columns() > 1)
                       ? ConstructorCompoundCast::Make(context, pos, castType, std::move(swizzled))
                       : ConstructorScalarCast::Make(context, pos, castType, std::move(swizzled));
    }

    // Swizzles on compound constructors, like `half4(1, 2, 3, 4).yw`, can become `half2(2, 4)`.
    if (value->is<ConstructorCompound>()) {
        const ConstructorCompound& ctor = value->as<ConstructorCompound>();
        if (auto replacement = optimize_constructor_swizzle(context, pos, ctor, components)) {
            return replacement;
        }
    }

    // The swizzle could not be simplified, so apply the requested swizzle to the base expression.
    return std::make_unique<Swizzle>(context, pos, std::move(expr), components);
}

std::string Swizzle::description(OperatorPrecedence) const {
    return this->base()->description(OperatorPrecedence::kPostfix) + "." +
           MaskString(this->components());
}

}  // namespace SkSL
