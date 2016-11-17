/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_ASTMODIFIERS
#define SKSL_ASTMODIFIERS

#include "SkSLASTLayout.h"
#include "SkSLASTNode.h"

namespace SkSL {

/**
 * A set of modifier keywords (in, out, uniform, etc.) appearing before a declaration. 
 */
struct ASTModifiers : public ASTNode {
    enum Flag {
        kNo_Flag            =  0,
        kConst_Flag         =  1,
        kIn_Flag            =  2,
        kOut_Flag           =  4,
        kLowp_Flag          =  8,
        kMediump_Flag       = 16,
        kHighp_Flag         = 32,
        kUniform_Flag       = 64,
        kFlat_Flag          = 128,
        kNoPerspective_Flag = 256
    };

    ASTModifiers(ASTLayout layout, int flags)
    : fLayout(layout)
    , fFlags(flags) {}

    SkString description() const override {
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

    const ASTLayout fLayout;
    const int fFlags;
};

} // namespace

#endif
