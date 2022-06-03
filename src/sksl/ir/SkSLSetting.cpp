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

using CapsFn = bool (ShaderCaps::*)() const;
using CapsLookupTable = SkTHashMap<std::string_view, CapsFn>;

static const CapsLookupTable& caps_lookup_table() {
    // Create a lookup table that converts strings into the equivalent ShaderCaps methods.
    static CapsLookupTable* sCapsLookupTable = new CapsLookupTable({
    #define CAP(name) CapsLookupTable::Pair{#name, &ShaderCaps::name}
        CAP(mustDoOpBetweenFloorAndAbs),
        CAP(mustGuardDivisionEvenAfterExplicitZeroCheck),
        CAP(atan2ImplementedAsAtanYOverX),
        CAP(floatIs32Bits),
        CAP(integerSupport),
        CAP(builtinDeterminantSupport),
        CAP(rewriteMatrixVectorMultiply),
    #undef CAP
    });
    return *sCapsLookupTable;
}

}  // namespace

std::unique_ptr<Expression> Setting::Convert(const Context& context, Position pos,
                                             const std::string_view& name) {
    SkASSERT(context.fConfig);

    const CapsFn* getCap = caps_lookup_table().find(name);

    if (!getCap) {
        context.fErrors->error(pos, "unknown capability flag '" + std::string(name) + "'");
        return nullptr;
    }

    if (context.fConfig->fSettings.fReplaceSettings) {
        // Insert the settings value directly into the IR.
        return Literal::MakeBool(context, pos, (context.fCaps.*(*getCap))());
    }

    // Generate a Setting IRNode.
    return std::make_unique<Setting>(pos, name, context.fTypes.fBool.get());
}

}  // namespace SkSL
