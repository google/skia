/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLTypeReference.h"

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLProgramSettings.h"

namespace SkSL {

bool TypeReference::VerifyType(const Context& context, const SkSL::Type* type, Position pos) {
    if (!context.fConfig->isBuiltinCode() && type) {
        if (type->isGeneric() || type->isLiteral()) {
            context.fErrors->error(pos, "type '" + std::string(type->name()) + "' is generic");
            return false;
        }
        if (!type->isAllowedInES2(context)) {
            context.fErrors->error(pos, "type '" + std::string(type->name()) +"' is not supported");
            return false;
        }
    }
    return true;
}

std::unique_ptr<TypeReference> TypeReference::Convert(const Context& context,
                                                      Position pos,
                                                      const Type* type) {
    return VerifyType(context, type, pos) ? TypeReference::Make(context, pos, type)
                                          : nullptr;
}

std::unique_ptr<TypeReference> TypeReference::Make(const Context& context,
                                                   Position pos,
                                                   const Type* type) {
    SkASSERT(type->isAllowedInES2(context));
    return std::make_unique<TypeReference>(context, pos, type);
}

} // namespace SkSL
