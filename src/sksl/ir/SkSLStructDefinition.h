/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STRUCTDEFINITION
#define SKSL_STRUCTDEFINITION

#include <memory>

#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

/**
 * A struct at global scope, as in:
 *
 * struct RenderData {
 *   float3 color;
 *   bool highQuality;
 * };
 */
class StructDefinition final : public ProgramElement {
public:
    static constexpr Kind kProgramElementKind = Kind::kStructDefinition;

    StructDefinition(int offset, const Type& type, std::shared_ptr<SymbolTable> typeOwner)
    : INHERITED(offset, kProgramElementKind)
    , fType(&type)
    , fTypeOwner(std::move(typeOwner)) {}

    const Type& type() const {
        return *fType;
    }

    std::shared_ptr<SymbolTable> typeOwner() const {
        return fTypeOwner;
    }

    std::unique_ptr<ProgramElement> clone() const override {
        return std::make_unique<StructDefinition>(fOffset, this->type(), this->typeOwner());
    }

    String description() const override {
        String s = "struct ";
        s += this->type().name();
        s += " { ";
        for (const auto& f : this->type().fields()) {
            s += f.fModifiers.description();
            s += f.fType->description();
            s += " ";
            s += f.fName;
            s += "; ";
        }
        s += "};";
        return s;
    }

private:
    const Type* fType = nullptr;
    std::shared_ptr<SymbolTable> fTypeOwner;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
