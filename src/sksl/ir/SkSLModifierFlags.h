/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MODIFIERFLAGS
#define SKSL_MODIFIERFLAGS

#include "include/private/base/SkTo.h"
#include "src/base/SkEnumBitMask.h"

#include <string>

namespace SkSL {

class Context;
class Position;

enum class ModifierFlag : int {
    kNone           =       0,
    // Real GLSL modifiers
    kFlat           = 1 <<  0,
    kNoPerspective  = 1 <<  1,
    kConst          = 1 <<  2,
    kUniform        = 1 <<  3,
    kIn             = 1 <<  4,
    kOut            = 1 <<  5,
    kHighp          = 1 <<  6,
    kMediump        = 1 <<  7,
    kLowp           = 1 <<  8,
    kReadOnly       = 1 <<  9,
    kWriteOnly      = 1 << 10,
    kBuffer         = 1 << 11,
    kPixelLocal     = 1 << 12,
    // Corresponds to the GLSL 'shared' modifier. Only allowed in a compute program.
    kWorkgroup      = 1 << 13,
    // SkSL extensions, not present in GLSL
    kExport         = 1 << 14,
    kES3            = 1 << 15,
    kPure           = 1 << 16,
    kInline         = 1 << 17,
    kNoInline       = 1 << 18,
};

}  // namespace SkSL

SK_MAKE_BITMASK_OPS(SkSL::ModifierFlag);

namespace SkSL {

class ModifierFlags : public SkEnumBitMask<SkSL::ModifierFlag> {
public:
    using SkEnumBitMask<SkSL::ModifierFlag>::SkEnumBitMask;
    ModifierFlags(SkEnumBitMask<SkSL::ModifierFlag> that)
            : SkEnumBitMask<SkSL::ModifierFlag>(that) {}

    std::string description() const;
    std::string paddedDescription() const;

    /**
     * Verifies that only permitted modifier flags are included. Reports errors and returns false in
     * the event of a violation.
     */
    bool checkPermittedFlags(const Context& context,
                             Position pos,
                             ModifierFlags permittedModifierFlags) const;

    bool isConst() const      { return SkToBool(*this & ModifierFlag::kConst); }
    bool isUniform() const    { return SkToBool(*this & ModifierFlag::kUniform); }
    bool isReadOnly() const   { return SkToBool(*this & ModifierFlag::kReadOnly); }
    bool isWriteOnly() const  { return SkToBool(*this & ModifierFlag::kWriteOnly); }
    bool isBuffer() const     { return SkToBool(*this & ModifierFlag::kBuffer); }
    bool isPixelLocal() const { return SkToBool(*this & ModifierFlag::kPixelLocal); }
    bool isWorkgroup() const  { return SkToBool(*this & ModifierFlag::kWorkgroup); }
    bool isExport() const     { return SkToBool(*this & ModifierFlag::kExport); }
    bool isES3() const        { return SkToBool(*this & ModifierFlag::kES3); }
    bool isPure() const       { return SkToBool(*this & ModifierFlag::kPure); }
    bool isInline() const     { return SkToBool(*this & ModifierFlag::kInline); }
    bool isNoInline() const   { return SkToBool(*this & ModifierFlag::kNoInline); }
};

}  // namespace SkSL

#endif
