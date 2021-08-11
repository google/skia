/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLSetting.h"

namespace SkSL {

std::unique_ptr<Expression> FieldAccess::Convert(const Context& context,
                                                 std::unique_ptr<Expression> base,
                                                 skstd::string_view field) {
    const Type& baseType = base->type();
    if (baseType.isStruct()) {
        const std::vector<Type::Field>& fields = baseType.fields();
        for (size_t i = 0; i < fields.size(); i++) {
            if (fields[i].fName == field) {
                return FieldAccess::Make(context, std::move(base), (int) i);
            }
        }
    }
    if (baseType == *context.fTypes.fSkCaps) {
        return Setting::Convert(context, base->fOffset, field);
    }

    context.errors().error(base->fOffset, "type '" + baseType.displayName() +
                                          "' does not have a field named '" + field + "'");
    return nullptr;
}

std::unique_ptr<Expression> FieldAccess::Make(const Context& context,
                                              std::unique_ptr<Expression> base,
                                              int fieldIndex,
                                              OwnerKind ownerKind) {
    SkASSERT(base->type().isStruct());
    SkASSERT(fieldIndex >= 0);
    SkASSERT(fieldIndex < (int) base->type().fields().size());
    return std::make_unique<FieldAccess>(std::move(base), fieldIndex, ownerKind);
}

}  // namespace SkSL
