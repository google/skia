/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MODIFIERS
#define SKSL_MODIFIERS

#include "include/private/base/SkTo.h"
#include "src/base/SkEnumBitMask.h"
#include "src/sksl/ir/SkSLLayout.h"

#include <cstddef>
#include <memory>
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
    // Corresponds to the GLSL 'shared' modifier. Only allowed in a compute program.
    kWorkgroup      = 1 << 12,
    // SkSL extensions, not present in GLSL
    kExport         = 1 << 13,
    kES3            = 1 << 14,
    kPure           = 1 << 15,
    kInline         = 1 << 16,
    kNoInline       = 1 << 17,
};

}  // namespace SkSL

SK_MAKE_BITMASK_OPS(SkSL::ModifierFlag);

namespace SkSL {

using ModifierFlags = SkEnumBitMask<SkSL::ModifierFlag>;

/**
 * A set of modifier keywords (in, out, uniform, etc.) appearing before a declaration.
 */
struct Modifiers {
    /**
     * OpenGL requires modifiers to be in a strict order:
     * - invariant-qualifier:     (invariant)
     * - interpolation-qualifier: flat, noperspective, (smooth)
     * - storage-qualifier:       const, uniform
     * - parameter-qualifier:     in, out, inout
     * - precision-qualifier:     highp, mediump, lowp
     *
     * SkSL does not have `invariant` or `smooth`.
     */

    Modifiers() : fLayout(Layout()), fFlags(ModifierFlag::kNone) {}

    Modifiers(const Layout& layout, ModifierFlags flags) : fLayout(layout), fFlags(flags) {}

    std::string description() const {
        return fLayout.description() + DescribeFlags(fFlags) + " ";
    }

    bool isConst() const     { return SkToBool(fFlags & ModifierFlag::kConst); }
    bool isUniform() const   { return SkToBool(fFlags & ModifierFlag::kUniform); }
    bool isReadOnly() const  { return SkToBool(fFlags & ModifierFlag::kReadOnly); }
    bool isWriteOnly() const { return SkToBool(fFlags & ModifierFlag::kWriteOnly); }
    bool isBuffer() const    { return SkToBool(fFlags & ModifierFlag::kBuffer); }
    bool isWorkgroup() const { return SkToBool(fFlags & ModifierFlag::kWorkgroup); }
    bool isPure() const      { return SkToBool(fFlags & ModifierFlag::kPure); }

    static std::string DescribeFlags(ModifierFlags flags) {
        // SkSL extensions
        std::string result;
        if (flags & ModifierFlag::kExport) {
            result += "$export ";
        }
        if (flags & ModifierFlag::kES3) {
            result += "$es3 ";
        }
        if (flags & ModifierFlag::kPure) {
            result += "$pure ";
        }
        if (flags & ModifierFlag::kInline) {
            result += "inline ";
        }
        if (flags & ModifierFlag::kNoInline) {
            result += "noinline ";
        }

        // Real GLSL qualifiers (must be specified in order in GLSL 4.1 and below)
        if (flags & ModifierFlag::kFlat) {
            result += "flat ";
        }
        if (flags & ModifierFlag::kNoPerspective) {
            result += "noperspective ";
        }
        if (flags & ModifierFlag::kConst) {
            result += "const ";
        }
        if (flags & ModifierFlag::kUniform) {
            result += "uniform ";
        }
        if ((flags & ModifierFlag::kIn) && (flags & ModifierFlag::kOut)) {
            result += "inout ";
        } else if (flags & ModifierFlag::kIn) {
            result += "in ";
        } else if (flags & ModifierFlag::kOut) {
            result += "out ";
        }
        if (flags & ModifierFlag::kHighp) {
            result += "highp ";
        }
        if (flags & ModifierFlag::kMediump) {
            result += "mediump ";
        }
        if (flags & ModifierFlag::kLowp) {
            result += "lowp ";
        }
        if (flags & ModifierFlag::kReadOnly) {
            result += "readonly ";
        }
        if (flags & ModifierFlag::kWriteOnly) {
            result += "writeonly ";
        }
        if (flags & ModifierFlag::kBuffer) {
            result += "buffer ";
        }

        // We're using a non-GLSL name for this one; the GLSL equivalent is "shared"
        if (flags & ModifierFlag::kWorkgroup) {
            result += "workgroup ";
        }

        if (!result.empty()) {
            result.pop_back();
        }
        return result;
    }

    bool operator==(const Modifiers& other) const {
        return fLayout == other.fLayout && fFlags == other.fFlags;
    }

    bool operator!=(const Modifiers& other) const {
        return !(*this == other);
    }

    /**
     * Verifies that only permitted modifiers and layout flags are included. Reports errors and
     * returns false in the event of a violation.
     */
    bool checkPermitted(const Context& context,
                        Position pos,
                        ModifierFlags permittedModifierFlags,
                        LayoutFlags permittedLayoutFlags) const;

    Layout fLayout;
    ModifierFlags fFlags;
};

} // namespace SkSL

namespace std {

template <>
struct hash<SkSL::Modifiers> {
    size_t operator()(const SkSL::Modifiers& key) const {
        return ((size_t) key.fFlags.value()) ^
               ((size_t) key.fLayout.fFlags.value() << 8) ^
               ((size_t) key.fLayout.fBuiltin << 16);
    }
};

} // namespace std

#endif
