/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SETTING
#define SKSL_SETTING

#include "SkSLContext.h"
#include "SkSLExpression.h"

namespace SkSL {

/**
 * Represents a compile-time constant setting, such as sk_Caps.fbFetchSupport. These are generally
 * collapsed down to their constant representations during the compilation process.
 */
struct Setting : public Expression {
    Setting(Position position, String name, std::unique_ptr<Expression> value)
    : INHERITED(position, kSetting_Kind, value->fType)
    , fName(std::move(name))
    , fValue(std::move(value)) {
        ASSERT(fValue->isConstant());
    }

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override;

    String description() const override {
        return fName;
    }

    bool hasSideEffects() const override {
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
