/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FIELDACCESS
#define SKSL_FIELDACCESS

#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

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

    static constexpr Kind kExpressionKind = Kind::kFieldAccess;

    FieldAccess(std::unique_ptr<Expression> base, int fieldIndex,
                OwnerKind ownerKind = OwnerKind::kDefault)
    : INHERITED(base->fOffset, kExpressionKind, base->type().fields()[fieldIndex].fType)
    , fFieldIndex(fieldIndex)
    , fOwnerKind(ownerKind)
    , fBase(std::move(base)) {}

    // Returns a field-access expression; reports errors via the ErrorReporter.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               std::unique_ptr<Expression> base,
                                               skstd::string_view field);

    // Returns a field-access expression; reports errors via ASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
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

    bool hasProperty(Property property) const override {
        return this->base()->hasProperty(property);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new FieldAccess(this->base()->clone(),
                                                           this->fieldIndex(),
                                                           this->ownerKind()));
    }

    String description() const override {
        return this->base()->description() + "." +
               this->base()->type().fields()[this->fieldIndex()].fName;
    }

private:
    int fFieldIndex;
    FieldAccessOwnerKind fOwnerKind;
    std::unique_ptr<Expression> fBase;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
