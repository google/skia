/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLSetting.h"

#include "include/core/SkTypes.h"
#include "include/private/SkTHash.h"
#include "include/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLLiteral.h"

#include <initializer_list>

namespace SkSL {

namespace {

using CapsPtr = bool ShaderCaps::*;
using CapsLookupTable = SkTHashMap<std::string_view, CapsPtr>;

static const CapsLookupTable& caps_lookup_table() {
    // Create a lookup table that converts strings into the equivalent ShaderCaps member-pointers.
    static CapsLookupTable* sCapsLookupTable = new CapsLookupTable({
        CapsLookupTable::Pair("mustDoOpBetweenFloorAndAbs",
                              &ShaderCaps::fMustDoOpBetweenFloorAndAbs),
        CapsLookupTable::Pair("mustGuardDivisionEvenAfterExplicitZeroCheck",
                              &ShaderCaps::fMustGuardDivisionEvenAfterExplicitZeroCheck),
        CapsLookupTable::Pair("atan2ImplementedAsAtanYOverX",
                              &ShaderCaps::fAtan2ImplementedAsAtanYOverX),
        CapsLookupTable::Pair("floatIs32Bits",
                              &ShaderCaps::fFloatIs32Bits),
        CapsLookupTable::Pair("integerSupport",
                              &ShaderCaps::fIntegerSupport),
        CapsLookupTable::Pair("builtinDeterminantSupport",
                              &ShaderCaps::fBuiltinDeterminantSupport),
        CapsLookupTable::Pair("rewriteMatrixVectorMultiply",
                              &ShaderCaps::fRewriteMatrixVectorMultiply),
    });
    return *sCapsLookupTable;
}

}  // namespace

std::unique_ptr<Expression> Setting::Convert(const Context& context, Position pos,
                                             const std::string_view& name) {
    SkASSERT(context.fConfig);

    const CapsPtr* capsPtr = caps_lookup_table().find(name);
    if (!capsPtr) {
        context.fErrors->error(pos, "unknown capability flag '" + std::string(name) + "'");
        return nullptr;
    }

    if (context.fConfig->fSettings.fReplaceSettings) {
        // Insert the settings value directly into the IR.
        return Literal::MakeBool(context, pos, context.fCaps.*(*capsPtr));
    }

    // Generate a Setting IRNode.
    return std::make_unique<Setting>(pos, name, context.fTypes.fBool.get());
}

std::unique_ptr<Expression> Setting::toLiteral(const Context& context) const {
    const CapsPtr* capsPtr = caps_lookup_table().find(fName);
    SkASSERT(capsPtr);

    return Literal::MakeBool(fPosition, context.fCaps.*(*capsPtr), &this->type());
}

}  // namespace SkSL
