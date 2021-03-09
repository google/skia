/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MODIFIERS
#define SKSL_MODIFIERS

#include "include/private/SkSLLayout.h"

#include <vector>

namespace SkSL {

/**
 * A set of modifier keywords (in, out, uniform, etc.) appearing before a declaration.
 */
struct Modifiers {
    enum Flag {
        kNo_Flag             =       0,
        kConst_Flag          = 1 <<  0,
        kIn_Flag             = 1 <<  1,
        kOut_Flag            = 1 <<  2,
        kUniform_Flag        = 1 <<  3,
        kFlat_Flag           = 1 <<  4,
        kNoPerspective_Flag  = 1 <<  5,
        kHasSideEffects_Flag = 1 <<  6,
        kVarying_Flag        = 1 <<  7,
        kInline_Flag         = 1 <<  8,
        kNoInline_Flag       = 1 <<  9,
    };

    Modifiers()
    : fLayout(Layout())
    , fFlags(0) {}

    Modifiers(const Layout& layout, int flags)
    : fLayout(layout)
    , fFlags(flags) {}

    String description() const {
        String result = fLayout.description();
        if (fFlags & kUniform_Flag) {
            result += "uniform ";
        }
        if (fFlags & kConst_Flag) {
            result += "const ";
        }
        if (fFlags & kFlat_Flag) {
            result += "flat ";
        }
        if (fFlags & kNoPerspective_Flag) {
            result += "noperspective ";
        }
        if (fFlags & kHasSideEffects_Flag) {
            result += "sk_has_side_effects ";
        }
        if (fFlags & kVarying_Flag) {
            result += "varying ";
        }
        if (fFlags & kNoInline_Flag) {
            result += "noinline ";
        }
        if ((fFlags & kIn_Flag) && (fFlags & kOut_Flag)) {
            result += "inout ";
        } else if (fFlags & kIn_Flag) {
            result += "in ";
        } else if (fFlags & kOut_Flag) {
            result += "out ";
        }

        return result;
    }

    bool operator==(const Modifiers& other) const {
        return fLayout == other.fLayout && fFlags == other.fFlags;
    }

    bool operator!=(const Modifiers& other) const {
        return !(*this == other);
    }

    Layout fLayout;
    int fFlags;
};

} // namespace SkSL

namespace std {

template <>
struct hash<SkSL::Modifiers> {
    size_t operator()(const SkSL::Modifiers& key) const {
        return (size_t) key.fFlags ^ ((size_t) key.fLayout.fFlags << 8) ^
               ((size_t) key.fLayout.fBuiltin << 16);
    }
};

} // namespace std

#endif
