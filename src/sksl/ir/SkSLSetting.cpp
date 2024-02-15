/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLSetting.h"

#include "include/core/SkTypes.h"
#include "src/base/SkNoDestructor.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLLiteral.h"

#include <initializer_list>

using namespace skia_private;

namespace SkSL {
namespace {

using CapsLookupTable = THashMap<std::string_view, Setting::CapsPtr>;

static const CapsLookupTable& caps_lookup_table() {
    // Create a lookup table that converts strings into the equivalent ShaderCaps member-pointers.
    static SkNoDestructor<CapsLookupTable> sCapsLookupTable(CapsLookupTable{
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
        CapsLookupTable::Pair("PerlinNoiseRoundingFix",
                              &ShaderCaps::fPerlinNoiseRoundingFix),
    });
    return *sCapsLookupTable;
}

}  // namespace

std::string_view Setting::name() const {
    for (const auto& [name, capsPtr] : caps_lookup_table()) {
        if (capsPtr == fCapsPtr) {
            return name;
        }
    }
    SkUNREACHABLE;
}

std::unique_ptr<Expression> Setting::Convert(const Context& context,
                                             Position pos,
                                             const std::string_view& name) {
    SkASSERT(context.fConfig);

    if (!ProgramConfig::AllowsPrivateIdentifiers(context.fConfig->fKind)) {
        context.fErrors->error(pos, "name 'sk_Caps' is reserved");
        return nullptr;
    }

    const CapsPtr* capsPtr = caps_lookup_table().find(name);
    if (!capsPtr) {
        context.fErrors->error(pos, "unknown capability flag '" + std::string(name) + "'");
        return nullptr;
    }

    return Setting::Make(context, pos, *capsPtr);
}

std::unique_ptr<Expression> Setting::Make(const Context& context, Position pos, CapsPtr capsPtr) {
    SkASSERT(ProgramConfig::AllowsPrivateIdentifiers(context.fConfig->fKind));

    return std::make_unique<Setting>(pos, capsPtr, context.fTypes.fBool.get());
}

std::unique_ptr<Expression> Setting::toLiteral(const ShaderCaps& caps) const {
    return Literal::MakeBool(fPosition, caps.*fCapsPtr, &this->type());
}

}  // namespace SkSL
