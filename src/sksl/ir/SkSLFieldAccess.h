/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FIELDACCESS
#define SKSL_FIELDACCESS

#include "include/core/SkSpan.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLType.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace SkSL {

class Context;
enum class OperatorPrecedence : uint8_t;

enum class FieldAccessOwnerKind : int8_t {
    kDefault,
    // this field access is to a field of an anonymous interface block (and thus, the field name
    // is actually in global scope, so only the field name needs to be written in GLSL)
    kAnonymousInterfaceBlock
};

/**
 * An expression which extracts a field from a struct, as in 'foo.bar'.
 */
class FieldAccess final : public Expression {
public:
    using OwnerKind = FieldAccessOwnerKind;

    inline static constexpr Kind kIRNodeKind = Kind::kFieldAccess;

    FieldAccess(Position pos, std::unique_ptr<Expression> base, int fieldIndex,
                OwnerKind ownerKind = OwnerKind::kDefault)
    : INHERITED(pos, kIRNodeKind, base->type().fields()[fieldIndex].fType)
    , fFieldIndex(fieldIndex)
    , fOwnerKind(ownerKind)
    , fBase(std::move(base)) {}

    // Returns a field-access expression; reports errors via the ErrorReporter.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               Position pos,
                                               std::unique_ptr<Expression> base,
                                               std::string_view field);

    // Returns a field-access expression; reports errors via ASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            Position pos,
                                            std::unique_ptr<Expression> base,
                                            int fieldIndex,
                                            OwnerKind ownerKind = OwnerKind::kDefault);

    std::unique_ptr<Expression>& base() {
        return fBase;
    }

    const std::unique_ptr<Expression>& base() const {
        return fBase;
    }

    int fieldIndex() const {
        return fFieldIndex;
    }

    OwnerKind ownerKind() const {
        return fOwnerKind;
    }

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::make_unique<FieldAccess>(pos,
                                             this->base()->clone(),
                                             this->fieldIndex(),
                                             this->ownerKind());
    }

    size_t initialSlot() const;

    std::string description(OperatorPrecedence) const override;

private:
    int fFieldIndex;
    FieldAccessOwnerKind fOwnerKind;
    std::unique_ptr<Expression> fBase;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
