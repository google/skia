/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STRUCTDEFINITION
#define SKSL_STRUCTDEFINITION

#include <memory>

#include "include/private/SkSLProgramElement.h"
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
    inline static constexpr Kind kIRNodeKind = Kind::kStructDefinition;

    StructDefinition(Position pos, const Type& type)
    : INHERITED(pos, kIRNodeKind)
    , fType(&type) {}

    const Type& type() const {
        return *fType;
    }

    std::unique_ptr<ProgramElement> clone() const override {
        return std::make_unique<StructDefinition>(fPosition, this->type());
    }

    std::string description() const override {
        std::string s = "struct ";
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

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
