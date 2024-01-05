/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STRUCTDEFINITION
#define SKSL_STRUCTDEFINITION

#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLType.h"  // IWYU pragma: keep

#include <memory>
#include <string>
#include <string_view>

namespace SkSL {

class Context;

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

    static std::unique_ptr<StructDefinition> Convert(const Context& context,
                                                     Position pos,
                                                     std::string_view name,
                                                     skia_private::TArray<Field> fields);

    static std::unique_ptr<StructDefinition> Make(Position pos, const Type& type);

    const Type& type() const {
        return *fType;
    }

    std::string description() const override;

private:
    const Type* fType = nullptr;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
