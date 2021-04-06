/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorArray.h"

namespace SkSL {

std::unique_ptr<Expression> ConstructorArray::Convert(const Context& context,
                                                      int offset,
                                                      const Type& type,
                                                      ExpressionArray args) {
    SkASSERTF(type.isArray() && type.columns() > 0, "%s", type.description().c_str());

    // ES2 doesn't support first-class array types.
    if (context.fConfig->strictES2Mode()) {
        context.fErrors.error(offset, "construction of array type '" + type.displayName() +
                                      "' is not supported");
        return nullptr;
    }

    // Check that the number of constructor arguments matches the array size.
    if (type.columns() != args.count()) {
        context.fErrors.error(offset, String::printf("invalid arguments to '%s' constructor "
                                                     "(expected %d elements, but found %d)",
                                                     type.displayName().c_str(), type.columns(),
                                                     args.count()));
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

    return ConstructorArray::Make(context, offset, type, std::move(args));
}

std::unique_ptr<Expression> ConstructorArray::Make(const Context& context,
                                                   int offset,
                                                   const Type& type,
                                                   ExpressionArray args) {
    SkASSERT(!context.fConfig->strictES2Mode());
    SkASSERT(type.columns() == args.count());
    SkASSERT(std::all_of(args.begin(), args.end(), [&](const std::unique_ptr<Expression>& arg) {
        return type.componentType() == arg->type();
    }));

    return std::make_unique<ConstructorArray>(offset, type, std::move(args));
}

}  // namespace SkSL
