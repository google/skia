/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLSymbol.h"

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFieldSymbol.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionReference.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLTypeReference.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <utility>

namespace SkSL {

std::unique_ptr<Expression> Symbol::instantiate(const Context& context, Position pos) const {
    switch (this->kind()) {
        case Symbol::Kind::kFunctionDeclaration:
            return std::make_unique<FunctionReference>(
                    context, pos, &this->as<FunctionDeclaration>());

        case Symbol::Kind::kVariable: {
            const Variable* var = &this->as<Variable>();
            // default to kRead_RefKind; this will be corrected later if the variable is written to
            return VariableReference::Make(pos, var, VariableReference::RefKind::kRead);
        }
        case Symbol::Kind::kField: {
            const FieldSymbol* field = &this->as<FieldSymbol>();
            auto base = VariableReference::Make(
                    pos, &field->owner(), VariableReference::RefKind::kRead);
            return FieldAccess::Make(context,
                                     pos,
                                     std::move(base),
                                     field->fieldIndex(),
                                     FieldAccess::OwnerKind::kAnonymousInterfaceBlock);
        }
        case Symbol::Kind::kType:
            return TypeReference::Convert(context, pos, &this->as<Type>());

        default:
            SkDEBUGFAILF("unsupported symbol type %d\n", fKind);
            return nullptr;
    }
}

}  // namespace SkSL
