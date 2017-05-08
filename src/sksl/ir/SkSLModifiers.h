/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MODIFIERS
#define SKSL_MODIFIERS

#include "SkSLLayout.h"

namespace SkSL {

/**
 * A set of modifier keywords (in, out, uniform, etc.) appearing before a declaration.
 */
struct Modifiers {
    enum Flag {
        kNo_Flag            =    0,
        kConst_Flag         =    1,
        kIn_Flag            =    2,
        kOut_Flag           =    4,
        kLowp_Flag          =    8,
        kMediump_Flag       =   16,
        kHighp_Flag         =   32,
        kUniform_Flag       =   64,
        kFlat_Flag          =  128,
        kNoPerspective_Flag =  256,
        kReadOnly_Flag      =  512,
        kWriteOnly_Flag     = 1024,
        kCoherent_Flag      = 2048,
        kVolatile_Flag      = 4096,
        kRestrict_Flag      = 8192
    };

    Modifiers()
    : fLayout(Layout())
    , fFlags(0) {}

    Modifiers(Layout& layout, int flags)
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
        if (fFlags & kReadOnly_Flag) {
            result += "readonly ";
        }
        if (fFlags & kWriteOnly_Flag) {
            result += "writeonly ";
        }
        if (fFlags & kCoherent_Flag) {
            result += "coherent ";
        }
        if (fFlags & kVolatile_Flag) {
            result += "volatile ";
        }
        if (fFlags & kRestrict_Flag) {
            result += "restrict ";
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
