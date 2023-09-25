/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorScalarCast.h"

#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLType.h"

#include <string>

namespace SkSL {

std::unique_ptr<Expression> ConstructorScalarCast::Convert(const Context& context,
                                                           Position pos,
                                                           const Type& rawType,
                                                           ExpressionArray args) {
    // As you might expect, scalar-cast constructors should only be created with scalar types.
    const Type& type = rawType.scalarTypeForLiteral();
    SkASSERT(type.isScalar());

    if (args.size() != 1) {
        context.fErrors->error(pos, "invalid arguments to '" + type.displayName() +
                                    "' constructor, (expected exactly 1 argument, but found " +
                                    std::to_string(args.size()) + ")");
        return nullptr;
    }

    const Type& argType = args[0]->type();
    if (!argType.isScalar()) {
        // Casting a vector-type into its scalar component type is treated as a slice in GLSL.
        // We don't allow those casts in SkSL; recommend a .x swizzle instead.
        const char* swizzleHint = "";
        if (argType.componentType().matches(type)) {
            if (argType.isVector()) {
                swizzleHint = "; use '.x' instead";
            } else if (argType.isMatrix()) {
                swizzleHint = "; use '[0][0]' instead";
            }
        }

        context.fErrors->error(pos,
                               "'" + argType.displayName() + "' is not a valid parameter to '" +
                               type.displayName() + "' constructor" + swizzleHint);
        return nullptr;
    }
    if (type.checkForOutOfRangeLiteral(context, *args[0])) {
        return nullptr;
    }

    return ConstructorScalarCast::Make(context, pos, type, std::move(args[0]));
}

std::unique_ptr<Expression> ConstructorScalarCast::Make(const Context& context,
                                                        Position pos,
                                                        const Type& type,
                                                        std::unique_ptr<Expression> arg) {
    SkASSERT(type.isScalar());
    SkASSERT(type.isAllowedInES2(context));
    SkASSERT(arg->type().isScalar());

    // No cast required when the types match.
    if (arg->type().matches(type)) {
        arg->setPosition(pos);
        return arg;
    }
    // Look up the value of constant variables. This allows constant-expressions like `int(zero)` to
    // be replaced with a literal zero.
    arg = ConstantFolder::MakeConstantValueForVariable(pos, std::move(arg));

    // We can cast scalar literals at compile-time when possible. (If the resulting literal would be
    // out of range for its type, we report an error and return zero to minimize error cascading.
    // This can occur when code is inlined, so we can't necessarily catch it during Convert. As
    // such, it's not safe to return null or assert.)
    if (arg->is<Literal>()) {
        double value = arg->as<Literal>().value();
        if (type.checkForOutOfRangeLiteral(context, value, arg->fPosition)) {
            value = 0.0;
        }
        return Literal::Make(pos, value, &type);
    }

    // We allow scalar casts to abstract types `$floatLiteral` or `$intLiteral`. This can be used to
    // represent various expressions where SkSL still allows type flexibility. For instance, the
    // expression `float x = myBool ? 1 : 0` is allowed in SkSL despite the apparent type mismatch,
    // and the resolved type of expression `myBool ? 1 : 0` is actually `$intLiteral`. This
    // expression could also be rewritten as `$intLiteral(myBool)` to replace a ternary with a cast.
    //
    // If we are casting an expression of the form `$intLiteral(...)` or `$floatLiteral(...)`, we
    // can eliminate the intermediate constructor-cast since it no longer adds value.
    if (arg->is<ConstructorScalarCast>() && arg->type().isLiteral()) {
        std::unique_ptr<Expression> inner = std::move(arg->as<ConstructorScalarCast>().argument());
        return ConstructorScalarCast::Make(context, pos, type, std::move(inner));
    }

    return std::make_unique<ConstructorScalarCast>(pos, type, std::move(arg));
}

}  // namespace SkSL
