/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorScalarCast.h"

namespace SkSL {

std::unique_ptr<Expression> IntLiteral::castConstantExpression(const Context& context,
                                                               const Type& type) const {
    SkASSERT(type.isScalar());
    if (type.isFloat()) {
        return FloatLiteral::Make(fOffset, (SKSL_FLOAT)this->value(), &type);
    }
    if (type.isInteger()) {
        return IntLiteral::Make(fOffset, (SKSL_INT)this->value(), &type);
    }
    if (type.isBoolean()) {
        return BoolLiteral::Make(fOffset, this->value() != 0, &type);
    }
    return nullptr;
}

std::unique_ptr<Expression> FloatLiteral::castConstantExpression(const Context& context,
                                                                 const Type& type) const {
    SkASSERT(type.isScalar());
    if (type.isFloat()) {
        return FloatLiteral::Make(fOffset, (SKSL_FLOAT)this->value(), &type);
    }
    if (type.isInteger()) {
        return IntLiteral::Make(fOffset, (SKSL_INT)this->value(), &type);
    }
    if (type.isBoolean()) {
        return BoolLiteral::Make(fOffset, this->value() != 0, &type);
    }
    return nullptr;
}

std::unique_ptr<Expression> BoolLiteral::castConstantExpression(const Context& context,
                                                                const Type& type) const {
    SkASSERT(type.isScalar());
    if (type.isFloat()) {
        return FloatLiteral::Make(fOffset, (SKSL_FLOAT)this->value(), &type);
    }
    if (type.isInteger()) {
        return IntLiteral::Make(fOffset, (SKSL_INT)this->value(), &type);
    }
    if (type.isBoolean()) {
        return BoolLiteral::Make(fOffset, this->value() != 0, &type);
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
        context.fErrors.error(offset, "invalid arguments to '" + type.displayName() +
                                      "' constructor, (expected exactly 1 argument, but found " +
                                      to_string((uint64_t)args.size()) + ")");
        return nullptr;
    }

    const Type& argType = args[0]->type();
    if (!argType.isScalar()) {
        context.fErrors.error(offset, "invalid argument to '" + type.displayName() +
                                      "' constructor (expected a number or bool, but found '" +
                                      argType.displayName() + "')");
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
    // We can cast scalar literals at compile-time.
    if (std::unique_ptr<Expression> converted = arg->castConstantExpression(context, type)) {
        return converted;
    }
    return std::make_unique<ConstructorScalarCast>(offset, type, std::move(arg));
}

}  // namespace SkSL
