/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorVectorCast.h"

#include "src/sksl/ir/SkSLConstructor.h"

namespace SkSL {

template <typename T>
static std::unique_ptr<Expression> make_constructor_vector(const Context& context,
                                                           std::unique_ptr<Expression> arg) {
    int columns = arg->type().columns();
    ExpressionArray ctorArgs;
    ctorArgs.reserve_back(columns);
    for (int index=0; index<columns; ++index) {
        ctorArgs.push_back(Literal<T>::Make(context, arg->fOffset, arg->getVecComponent<T>(index)));
    }
    auto ctor = Constructor::Convert(context, arg->fOffset, arg->type(), std::move(ctorArgs));
    SkASSERT(ctor);
    return ctor;
}

std::unique_ptr<Expression> ConstructorVectorCast::Make(const Context& context,
                                                        int offset,
                                                        const Type& type,
                                                        std::unique_ptr<Expression> arg) {
    if (arg->isCompileTimeConstant()) {
        // `arg` is a compile-time constant; cast each field and create a ConstructorVector instead.
        const Type& argType = arg->type().componentType();
        if (argType.isFloat()) {
            return make_constructor_vector<SKSL_FLOAT>(context, std::move(arg));
        }
        if (argType.isInteger()) {
            return make_constructor_vector<SKSL_INT>(context, std::move(arg));
        }
        if (argType.isBoolean()) {
            return make_constructor_vector<bool>(context, std::move(arg));
        }
        SkDEBUGFAILF("unexpected compile-time type: %s", arg->type().description().c_str());
    }

    if (type == arg->type()) {
        return arg;
    }

    return std::make_unique<ConstructorVectorCast>(offset, type, std::move(arg));
}

}  // namespace SkSL
