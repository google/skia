/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLConstructorScalarCast.h"
#include "src/sksl/ir/SkSLLiteral.h"

namespace SkSL {

std::unique_ptr<Expression> ConstructorScalarCast::Convert(const Context& context,
                                                           int line,
                                                           const Type& rawType,
                                                           ExpressionArray args) {
    // As you might expect, scalar-cast constructors should only be created with scalar types.
    const Type& type = rawType.scalarTypeForLiteral();
    SkASSERT(type.isScalar());

    if (args.size() != 1) {
        context.fErrors->error(line, "invalid arguments to '" + type.displayName() +
                                     "' constructor, (expected exactly 1 argument, but found " +
                                     to_string((uint64_t)args.size()) + ")");
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

        context.fErrors->error(line,
                               "'" + argType.displayName() + "' is not a valid parameter to '" +
                               type.displayName() + "' constructor" + swizzleHint);
        return nullptr;
    }
    if (type.checkForOutOfRangeLiteral(context, *args[0])) {
        return nullptr;
    }

    return ConstructorScalarCast::Make(context, line, type, std::move(args[0]));
}

std::unique_ptr<Expression> ConstructorScalarCast::Make(const Context& context,
                                                        int line,
                                                        const Type& type,
                                                        std::unique_ptr<Expression> arg) {
    SkASSERT(type.isScalar());
    SkASSERT(type.isAllowedInES2(context));
    SkASSERT(arg->type().isScalar());

    // No cast required when the types match.
    if (arg->type().matches(type)) {
        return arg;
    }
    // Look up the value of constant variables. This allows constant-expressions like `int(zero)` to
    // be replaced with a literal zero.
    arg = ConstantFolder::MakeConstantValueForVariable(std::move(arg));

    // We can cast scalar literals at compile-time when possible. (If the resulting literal would be
    // out of range for its type, we report an error and return the constructor. This can occur when
    // code is inlined, so we can't necessarily catch it during Convert. As such, it's not safe to
    // return null or assert.)
    if (arg->is<Literal>() &&
        !type.checkForOutOfRangeLiteral(context, arg->as<Literal>().value(), arg->fLine)) {
        return Literal::Make(line, arg->as<Literal>().value(), &type);
    }
    return std::make_unique<ConstructorScalarCast>(line, type, std::move(arg));
}

}  // namespace SkSL
