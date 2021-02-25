/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SETTING
#define SKSL_SETTING

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * Represents a compile-time constant setting, such as sk_Caps.fbFetchSupport. These are generally
 * collapsed down to their constant representations during the compilation process.
 */
class Setting final : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kSetting;

    Setting(int offset, String name, const Type* type)
        : INHERITED(offset, kExpressionKind, type)
        , fName(std::move(name)) {}

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override;

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new Setting(fOffset, this->name(), &this->type()));
    }

    const String& name() const {
        return fName;
    }

    String description() const override {
        return this->name();
    }

    bool hasProperty(Property property) const override {
        return false;
    }

    bool isCompileTimeConstant() const override {
        return true;
    }

private:
    String fName;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
