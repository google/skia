/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "GrGLBlend.h"
#include "gl/builders/GrGLFragmentShaderBuilder.h"

static bool append_porterduff_term(GrGLFragmentBuilder* fsBuilder, SkXfermode::Coeff coeff,
                                   const char* colorName, const char* srcColorName,
                                   const char* dstColorName, bool hasPrevious) {
    if (SkXfermode::kZero_Coeff == coeff) {
        return hasPrevious;
    } else {
        if (hasPrevious) {
            fsBuilder->codeAppend(" + ");
        }
        fsBuilder->codeAppendf("%s", colorName);
        switch (coeff) {
            case SkXfermode::kOne_Coeff:
                break;
            case SkXfermode::kSC_Coeff:
                fsBuilder->codeAppendf(" * %s", srcColorName);
                break;
            case SkXfermode::kISC_Coeff:
                fsBuilder->codeAppendf(" * (vec4(1.0) - %s)", srcColorName);
                break;
            case SkXfermode::kDC_Coeff:
                fsBuilder->codeAppendf(" * %s", dstColorName);
                break;
            case SkXfermode::kIDC_Coeff:
                fsBuilder->codeAppendf(" * (vec4(1.0) - %s)", dstColorName);
                break;
            case SkXfermode::kSA_Coeff:
                fsBuilder->codeAppendf(" * %s.a", srcColorName);
                break;
            case SkXfermode::kISA_Coeff:
                fsBuilder->codeAppendf(" * (1.0 - %s.a)", srcColorName);
                break;
            case SkXfermode::kDA_Coeff:
                fsBuilder->codeAppendf(" * %s.a", dstColorName);
                break;
            case SkXfermode::kIDA_Coeff:
                fsBuilder->codeAppendf(" * (1.0 - %s.a)", dstColorName);
                break;
            default:
                SkFAIL("Unsupported Blend Coeff");
        }
        return true;
    }
}

void GrGLBlend::AppendPorterDuffBlend(GrGLFragmentBuilder* fsBuilder, const char* srcColor,
                                      const char* dstColor, const char* outColor,
                                      SkXfermode::Mode mode) {

    SkXfermode::Coeff srcCoeff, dstCoeff;
    SkXfermode::ModeAsCoeff(mode, &srcCoeff, &dstCoeff);

    fsBuilder->codeAppendf("%s =", outColor);
    // append src blend
    bool didAppend = append_porterduff_term(fsBuilder, srcCoeff, srcColor, srcColor, dstColor,
                                            false);
    // append dst blend
    SkAssertResult(append_porterduff_term(fsBuilder, dstCoeff, dstColor, srcColor, dstColor,
                                          didAppend));
    fsBuilder->codeAppend(";");
}
