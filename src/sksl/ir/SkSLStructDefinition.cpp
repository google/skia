/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLStructDefinition.h"

#include "include/private/base/SkSpan_impl.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"

#include <utility>

using namespace skia_private;

namespace SkSL {

std::unique_ptr<StructDefinition> StructDefinition::Convert(const Context& context,
                                                            Position pos,
                                                            std::string_view name,
                                                            TArray<Field> fields) {
    std::unique_ptr<Type> ownedType = Type::MakeStructType(context, pos, name,
                                                           std::move(fields),
                                                           /*interfaceBlock=*/false);
    const SkSL::Type* type = context.fSymbolTable->add(context, std::move(ownedType));
    return StructDefinition::Make(pos, *type);
}

std::unique_ptr<StructDefinition> StructDefinition::Make(Position pos, const Type& type) {
    return std::make_unique<SkSL::StructDefinition>(pos, type);
}

std::string StructDefinition::description() const {
    std::string s = "struct ";
    s += this->type().name();
    s += " { ";
    for (const auto& f : this->type().fields()) {
        s += f.fLayout.description();
        s += f.fModifierFlags.description();
        s += ' ';
        s += f.fType->description();
        s += ' ';
        s += f.fName;
        s += "; ";
    }
    s += "};";
    return s;
}

}  // namespace SkSL
