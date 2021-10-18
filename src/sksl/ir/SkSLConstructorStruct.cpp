/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorStruct.h"

#include "include/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLContext.h"

namespace SkSL {

std::unique_ptr<Expression> ConstructorStruct::Convert(const Context& context,
                                                       int line,
                                                       const Type& type,
                                                       ExpressionArray args) {
    SkASSERTF(type.isStruct() && type.fields().size() > 0, "%s", type.description().c_str());

    // Check that the number of constructor arguments matches the array size.
    if (type.fields().size() != args.size()) {
        context.fErrors->error(line,
                               String::printf("invalid arguments to '%s' constructor "
                                              "(expected %zu elements, but found %zu)",
                                              type.displayName().c_str(), type.fields().size(),
                                              args.size()));
        return nullptr;
    }

    // Convert each constructor argument to the struct's field type.
    for (int index=0; index<args.count(); ++index) {
        std::unique_ptr<Expression>& argument = args[index];
        const Type::Field& field = type.fields()[index];

        argument = field.fType->coerceExpression(std::move(argument), context);
        if (!argument) {
            return nullptr;
        }
    }

    return ConstructorStruct::Make(context, line, type, std::move(args));
}

[[maybe_unused]] static bool arguments_match_field_types(const ExpressionArray& args,
                                                         const Type& type) {
    SkASSERT(type.fields().size() == args.size());

    for (int index = 0; index < args.count(); ++index) {
        const std::unique_ptr<Expression>& argument = args[index];
        const Type::Field& field = type.fields()[index];
        if (argument->type() != *field.fType) {
            return false;
        }
    }

    return true;
}

std::unique_ptr<Expression> ConstructorStruct::Make(const Context& context,
                                                    int line,
                                                    const Type& type,
                                                    ExpressionArray args) {
    SkASSERT(type.isAllowedInES2(context));
    SkASSERT(arguments_match_field_types(args, type));
    return std::make_unique<ConstructorStruct>(line, type, std::move(args));
}

}  // namespace SkSL
