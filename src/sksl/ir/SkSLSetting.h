/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SETTING
#define SKSL_SETTING

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>


namespace SkSL {

class Context;
class Type;
enum class OperatorPrecedence : uint8_t;

/**
 * Represents a compile-time constant setting, such as sk_Caps.integerSupport. These IRNodes are
 * used when assembling a module. These nodes are replaced with the value of the setting during
 * compilation when ShaderCaps are available.
 */
class Setting final : public Expression {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kSetting;

    using CapsPtr = const bool ShaderCaps::*;

    Setting(Position pos, CapsPtr capsPtr, const Type* type)
        : INHERITED(pos, kIRNodeKind, type)
        , fCapsPtr(capsPtr) {}

    // Creates the current value of the associated caps bit as a Literal if ShaderCaps are
    // available, or a Setting IRNode when ShaderCaps are not known. Reports errors via the
    // ErrorReporter.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               Position pos,
                                               const std::string_view& name);

    // Creates the current value of the passed-in caps bit as a Literal if ShaderCaps are
    // available, or a Setting IRNode when ShaderCaps are not known.
    static std::unique_ptr<Expression> Make(const Context& context, Position pos, CapsPtr capsPtr);

    // Converts a Setting expression to its actual ShaderCaps value (boolean true/false).
    std::unique_ptr<Expression> toLiteral(const ShaderCaps& caps) const;

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::make_unique<Setting>(pos, fCapsPtr, &this->type());
    }

    std::string_view name() const;

    CapsPtr capsPtr() const { return fCapsPtr; }

    std::string description(OperatorPrecedence) const override {
        return "sk_Caps." + std::string(this->name());
    }

private:
    CapsPtr fCapsPtr;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
