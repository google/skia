/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_MODIFIERS
#define SKSL_MODIFIERS

#include "../ast/SkSLASTModifiers.h"
#include "SkSLLayout.h"

namespace SkSL {

/**
 * A set of modifier keywords (in, out, uniform, etc.) appearing before a declaration. 
 */
struct Modifiers {
    enum Flag {
        kNo_Flag            = ASTModifiers::kNo_Flag,
        kConst_Flag         = ASTModifiers::kConst_Flag,
        kIn_Flag            = ASTModifiers::kIn_Flag,
        kOut_Flag           = ASTModifiers::kOut_Flag,
        kLowp_Flag          = ASTModifiers::kLowp_Flag,
        kMediump_Flag       = ASTModifiers::kMediump_Flag,
        kHighp_Flag         = ASTModifiers::kHighp_Flag,
        kUniform_Flag       = ASTModifiers::kUniform_Flag,
        kFlat_Flag          = ASTModifiers::kFlat_Flag,
        kNoPerspective_Flag = ASTModifiers::kNoPerspective_Flag
    };

    Modifiers(const ASTModifiers& modifiers)
    : fLayout(modifiers.fLayout)
    , fFlags(modifiers.fFlags) {}

    Modifiers(Layout& layout, int flags)
    : fLayout(layout)
    , fFlags(flags) {}

    SkString description() const {
        SkString result = fLayout.description();
        if (fFlags & kUniform_Flag) {
            result += "uniform ";
        }
        if (fFlags & kConst_Flag) {
            result += "const ";
        }
        if (fFlags & kLowp_Flag) {
            result += "lowp ";
        }
        if (fFlags & kMediump_Flag) {
            result += "mediump ";
        }
        if (fFlags & kHighp_Flag) {
            result += "highp ";
        }
        if (fFlags & kFlat_Flag) {
            result += "flat ";
        }
        if (fFlags & kNoPerspective_Flag) {
            result += "noperspective ";
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

} // namespace

#endif
