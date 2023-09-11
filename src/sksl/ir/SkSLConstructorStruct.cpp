/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorStruct.h"

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/ir/SkSLType.h"

#include <string>

namespace SkSL {

std::unique_ptr<Expression> ConstructorStruct::Convert(const Context& context,
                                                       Position pos,
                                                       const Type& type,
                                                       ExpressionArray args) {
    SkASSERTF(type.isStruct() && type.fields().size() > 0, "%s", type.description().c_str());

    // Check that the number of constructor arguments matches the array size.
    if (type.fields().size() != SkToSizeT(args.size())) {
        context.fErrors->error(pos,
                               String::printf("invalid arguments to '%s' constructor "
                                              "(expected %zu elements, but found %d)",
                                              type.displayName().c_str(), type.fields().size(),
                                              args.size()));
        return nullptr;
    }

    // A struct with atomic members cannot be constructed.
    if (type.isOrContainsAtomic()) {
        context.fErrors->error(
                pos,
                String::printf("construction of struct type '%s' with atomic member is not allowed",
                               type.displayName().c_str()));
        return nullptr;
    }

    // Convert each constructor argument to the struct's field type.
    for (int index=0; index<args.size(); ++index) {
        std::unique_ptr<Expression>& argument = args[index];
        const Field& field = type.fields()[index];

        argument = field.fType->coerceExpression(std::move(argument), context);
        if (!argument) {
            return nullptr;
        }
    }

    return ConstructorStruct::Make(context, pos, type, std::move(args));
}

[[maybe_unused]] static bool arguments_match_field_types(const ExpressionArray& args,
                                                         const Type& type) {
    SkASSERT(type.fields().size() == SkToSizeT(args.size()));

    for (int index = 0; index < args.size(); ++index) {
        const std::unique_ptr<Expression>& argument = args[index];
        const Field& field = type.fields()[index];
        if (!argument->type().matches(*field.fType)) {
            return false;
        }
    }

    return true;
}

std::unique_ptr<Expression> ConstructorStruct::Make(const Context& context,
                                                    Position pos,
                                                    const Type& type,
                                                    ExpressionArray args) {
    SkASSERT(type.isAllowedInES2(context));
    SkASSERT(arguments_match_field_types(args, type));
    SkASSERT(!type.isOrContainsAtomic());
    return std::make_unique<ConstructorStruct>(pos, type, std::move(args));
}

}  // namespace SkSL
