/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorArray.h"

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/ir/SkSLConstructorArrayCast.h"
#include "src/sksl/ir/SkSLType.h"

#include <algorithm>
#include <string>

namespace SkSL {

std::unique_ptr<Expression> ConstructorArray::Convert(const Context& context,
                                                      Position pos,
                                                      const Type& type,
                                                      ExpressionArray args) {
    SkASSERTF(type.isArray() && type.columns() > 0, "%s", type.description().c_str());

    // ES2 doesn't support first-class array types.
    if (context.fConfig->strictES2Mode()) {
        context.fErrors->error(pos, "construction of array type '" + type.displayName() +
                "' is not supported");
        return nullptr;
    }

    // An array of atomics cannot be constructed.
    if (type.isOrContainsAtomic()) {
        context.fErrors->error(
                pos,
                String::printf("construction of array type '%s' with atomic member is not allowed",
                               type.displayName().c_str()));
        return nullptr;
    }

    // If there is a single argument containing an array of matching size and the types are
    // coercible, this is actually a cast. i.e., `half[10](myFloat10Array)`. This isn't a GLSL
    // feature, but the Pipeline stage code generator needs this functionality so that code which
    // was originally compiled with "allow narrowing conversions" enabled can be later recompiled
    // without narrowing conversions (we patch over these conversions with an explicit cast).
    if (args.size() == 1) {
        const Expression& expr = *args.front();
        const Type& exprType = expr.type();

        if (exprType.isArray() && exprType.canCoerceTo(type, /*allowNarrowing=*/true)) {
            return ConstructorArrayCast::Make(context, pos, type, std::move(args.front()));
        }
    }

    // Check that the number of constructor arguments matches the array size.
    if (type.columns() != args.size()) {
        context.fErrors->error(pos, String::printf("invalid arguments to '%s' constructor "
                "(expected %d elements, but found %d)", type.displayName().c_str(), type.columns(),
                args.size()));
        return nullptr;
    }

    // Convert each constructor argument to the array's component type.
    const Type& baseType = type.componentType();
    for (std::unique_ptr<Expression>& argument : args) {
        argument = baseType.coerceExpression(std::move(argument), context);
        if (!argument) {
            return nullptr;
        }
    }

    return ConstructorArray::Make(context, pos, type, std::move(args));
}

std::unique_ptr<Expression> ConstructorArray::Make(const Context& context,
                                                   Position pos,
                                                   const Type& type,
                                                   ExpressionArray args) {
    SkASSERT(!context.fConfig->strictES2Mode());
    SkASSERT(type.isAllowedInES2(context));
    SkASSERT(type.columns() == args.size());
    SkASSERT(!type.isOrContainsAtomic());
    SkASSERT(std::all_of(args.begin(), args.end(), [&](const std::unique_ptr<Expression>& arg) {
        return type.componentType().matches(arg->type());
    }));

    return std::make_unique<ConstructorArray>(pos, type, std::move(args));
}

}  // namespace SkSL
