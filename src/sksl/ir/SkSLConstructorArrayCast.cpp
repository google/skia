/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorArrayCast.h"

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/ir/SkSLConstructorArray.h"
#include "src/sksl/ir/SkSLConstructorCompoundCast.h"
#include "src/sksl/ir/SkSLConstructorScalarCast.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

static std::unique_ptr<Expression> cast_constant_array(const Context& context,
                                                       Position pos,
                                                       const Type& destType,
                                                       std::unique_ptr<Expression> constCtor) {
    const Type& scalarType = destType.componentType();

    // Create a ConstructorArray(...) which typecasts each argument inside.
    auto inputArgs = constCtor->as<ConstructorArray>().argumentSpan();
    ExpressionArray typecastArgs;
    typecastArgs.reserve_exact(inputArgs.size());
    for (std::unique_ptr<Expression>& arg : inputArgs) {
        Position argPos = arg->fPosition;
        if (arg->type().isScalar()) {
            typecastArgs.push_back(ConstructorScalarCast::Make(context, argPos, scalarType,
                                                               std::move(arg)));
        } else {
            typecastArgs.push_back(ConstructorCompoundCast::Make(context, argPos, scalarType,
                                                                 std::move(arg)));
        }
    }

    return ConstructorArray::Make(context, pos, destType, std::move(typecastArgs));
}

std::unique_ptr<Expression> ConstructorArrayCast::Make(const Context& context,
                                                       Position pos,
                                                       const Type& type,
                                                       std::unique_ptr<Expression> arg) {
    // Only arrays of the same size are allowed.
    SkASSERT(type.isArray());
    SkASSERT(type.isAllowedInES2(context));
    SkASSERT(arg->type().isArray());
    SkASSERT(type.columns() == arg->type().columns());

    // If this is a no-op cast, return the expression as-is.
    if (type.matches(arg->type())) {
        arg->fPosition = pos;
        return arg;
    }

    // Look up the value of constant variables. This allows constant-expressions like `myArray` to
    // be replaced with the compile-time constant `int[2](0, 1)`.
    arg = ConstantFolder::MakeConstantValueForVariable(pos, std::move(arg));

    // We can cast a vector of compile-time constants at compile-time.
    if (Analysis::IsCompileTimeConstant(*arg)) {
        return cast_constant_array(context, pos, type, std::move(arg));
    }
    return std::make_unique<ConstructorArrayCast>(pos, type, std::move(arg));
}

}  // namespace SkSL
