/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/ir/SkSLConstructorScalarCast.h"

namespace SkSL {

static std::unique_ptr<Expression> cast_scalar_literal(const Type& constructorType,
                                                       const Expression& expr) {
    if (expr.is<IntLiteral>()) {
        SKSL_INT value = expr.as<IntLiteral>().value();
        if (constructorType.isFloat()) {
            // promote float(1) to 1.0
            return FloatLiteral::Make(expr.fOffset, (SKSL_FLOAT)value, &constructorType);
        } else if (constructorType.isInteger()) {
            // promote uint(1) to 1u
            return IntLiteral::Make(expr.fOffset, value, &constructorType);
        } else if (constructorType.isBoolean()) {
            // promote bool(1) to true/false
            return BoolLiteral::Make(expr.fOffset, value != 0, &constructorType);
        }
    } else if (expr.is<FloatLiteral>()) {
        float value = expr.as<FloatLiteral>().value();
        if (constructorType.isFloat()) {
            // promote float(1.23) to 1.23
            return FloatLiteral::Make(expr.fOffset, value, &constructorType);
        } else if (constructorType.isInteger()) {
            // promote uint(1.23) to 1u
            return IntLiteral::Make(expr.fOffset, (SKSL_INT)value, &constructorType);
        } else if (constructorType.isBoolean()) {
            // promote bool(1.23) to true/false
            return BoolLiteral::Make(expr.fOffset, value != 0.0f, &constructorType);
        }
    } else if (expr.is<BoolLiteral>()) {
        bool value = expr.as<BoolLiteral>().value();
        if (constructorType.isFloat()) {
            // promote float(true) to 1.0
            return FloatLiteral::Make(expr.fOffset, value ? 1.0f : 0.0f, &constructorType);
        } else if (constructorType.isInteger()) {
            // promote uint(true) to 1u
            return IntLiteral::Make(expr.fOffset, value ? 1 : 0, &constructorType);
        } else if (constructorType.isBoolean()) {
            // promote bool(true) to true/false
            return BoolLiteral::Make(expr.fOffset, value, &constructorType);
        }
    }
    return nullptr;
}

std::unique_ptr<Expression> ConstructorScalarCast::Convert(const Context& context,
                                                           int offset,
                                                           const Type& rawType,
                                                           ExpressionArray args) {
    // As you might expect, scalar-cast constructors should only be created with scalar types.
    const Type& type = rawType.scalarTypeForLiteral();
    SkASSERT(type.isScalar());

    if (args.size() != 1) {
        context.errors().error(offset, "invalid arguments to '" + type.displayName() +
                                       "' constructor, (expected exactly 1 argument, but found " +
                                       to_string((uint64_t)args.size()) + ")");
        return nullptr;
    }

    const Type& argType = args[0]->type();
    if (!argType.isScalar()) {
        // Casting a vector-type into its scalar component type is treated as a slice in GLSL.
        // We don't allow those casts in SkSL; recommend a .x swizzle instead.
        const char* swizzleHint = "";
        if (argType.componentType() == type) {
            if (argType.isVector()) {
                swizzleHint = "; use '.x' instead";
            } else if (argType.isMatrix()) {
                swizzleHint = "; use '[0][0]' instead";
            }
        }

        context.errors().error(offset,
                               "'" + argType.displayName() + "' is not a valid parameter to '" +
                               type.displayName() + "' constructor" + swizzleHint);
        return nullptr;
    }

    return ConstructorScalarCast::Make(context, offset, type, std::move(args[0]));
}

std::unique_ptr<Expression> ConstructorScalarCast::Make(const Context& context,
                                                        int offset,
                                                        const Type& type,
                                                        std::unique_ptr<Expression> arg) {
    SkASSERT(type.isScalar());
    SkASSERT(arg->type().isScalar());

    // No cast required when the types match.
    if (arg->type() == type) {
        return arg;
    }
    // When optimization is on, look up the value of constant variables. This allows expressions
    // like `int(zero)` to be replaced with a literal zero.
    if (context.fConfig->fSettings.fOptimize) {
        arg = ConstantFolder::MakeConstantValueForVariable(std::move(arg));
    }
    // We can cast scalar literals at compile-time.
    if (std::unique_ptr<Expression> converted = cast_scalar_literal(type, *arg)) {
        return converted;
    }
    return std::make_unique<ConstructorScalarCast>(offset, type, std::move(arg));
}

}  // namespace SkSL
