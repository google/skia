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
struct Setting : public Expression {
    Setting(int offset, String name, std::unique_ptr<Expression> value)
    : INHERITED(offset, kSetting_Kind, value->fType)
    , fName(std::move(name))
    , fValue(std::move(value)) {
        SkASSERT(fValue->isConstant());
    }

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override;

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new Setting(fOffset, fName, fValue->clone()));
    }

#ifdef SK_DEBUG
    String description() const override {
        return fName;
    }
#endif

    bool hasProperty(Property property) const override {
        return false;
    }

    bool isConstant() const override {
        return true;
    }

    const String fName;
    std::unique_ptr<Expression> fValue;

    typedef Expression INHERITED;
};

} // namespace

#endif
