/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLTypeReference.h"

#include "include/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLProgramSettings.h"

namespace SkSL {

std::unique_ptr<TypeReference> TypeReference::Convert(const Context& context,
                                                      int line,
                                                      const Type* type) {
    if (!type->isAllowedInES2(context)) {
        context.fErrors->error(line, "type '" + type->displayName() + "' is not supported");
        return nullptr;
    }
    return TypeReference::Make(context, line, type);
}

std::unique_ptr<TypeReference> TypeReference::Make(const Context& context,
                                                   int line,
                                                   const Type* type) {
    SkASSERT(type->isAllowedInES2(context));
    return std::make_unique<TypeReference>(context, line, type);
}

} // namespace SkSL
