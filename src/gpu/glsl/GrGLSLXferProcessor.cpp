/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "glsl/GrGLSLXferProcessor.h"

#include "GrShaderCaps.h"
#include "GrTexture.h"
#include "GrXferProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

// This is only called for cases where we are doing LCD coverage and not using in shader blending.
// For these cases we assume the the src alpha is 1, thus we can just use the max for the alpha
// coverage since src alpha will always be greater than or equal to dst alpha.
static void adjust_for_lcd_coverage(GrGLSLXPFragmentBuilder* fragBuilder,
                                    const char* srcCoverage,
                                    const GrXferProcessor& proc) {
    if (srcCoverage && proc.isLCD()) {
        fragBuilder->codeAppendf("%s.a = max(max(%s.r, %s.g), %s.b);",
                                 srcCoverage, srcCoverage, srcCoverage, srcCoverage);
    }
}


void GrGLSLXferProcessor::emitCode(const EmitArgs& args) {
    if (!args.fXP.willReadDstColor()) {
        adjust_for_lcd_coverage(args.fXPFragBuilder, args.fInputCoverage, args.fXP);
        this->emitOutputsForBlendState(args);
        return;
    }

    GrGLSLXPFragmentBuilder* fragBuilder = args.fXPFragBuilder;
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    const char* dstColor = fragBuilder->dstColor();

    bool needsLocalOutColor = false;

    if (args.fDstTextureSamplerHandle.isValid()) {
        bool flipY = kBottomLeft_GrSurfaceOrigin == args.fDstTextureOrigin;

        if (args.fInputCoverage) {
            // We don't think any shaders actually output negative coverage, but just as a safety
            // check for floating point precision errors we compare with <= here. We just check the
            // rgb values of the coverage since the alpha may not have been set when using lcd. If
            // we are using single channel coverage alpha will equal to rgb anyways.
            //
            // The discard here also helps for batching text draws together which need to read from
            // a dst copy for blends. Though this only helps the case where the outer bounding boxes
            // of each letter overlap and not two actually parts of the text.
            fragBuilder->codeAppendf("if (all(lessThanEqual(%s.rgb, half3(0)))) {"
                                     "    discard;"
                                     "}", args.fInputCoverage);
        }

        const char* dstTopLeftName;
        const char* dstCoordScaleName;

        fDstTopLeftUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                    kHalf2_GrSLType,
                                                    "DstTextureUpperLeft",
                                                    &dstTopLeftName);
        fDstScaleUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                  kHalf2_GrSLType,
                                                  "DstTextureCoordScale",
                                                  &dstCoordScaleName);

        fragBuilder->codeAppend("// Read color from copy of the destination.\n");
        fragBuilder->codeAppendf("half2 _dstTexCoord = (half2(sk_FragCoord.xy) - %s) * %s;",
                                 dstTopLeftName, dstCoordScaleName);

        if (flipY) {
            fragBuilder->codeAppend("_dstTexCoord.y = 1.0 - _dstTexCoord.y;");
        }

        fragBuilder->codeAppendf("half4 %s = ", dstColor);
        fragBuilder->appendTextureLookup(args.fDstTextureSamplerHandle, "_dstTexCoord",
                                         kHalf2_GrSLType);
        fragBuilder->codeAppend(";");
    } else {
        needsLocalOutColor = args.fShaderCaps->requiresLocalOutputColorForFBFetch();
    }

    const char* outColor = "_localColorOut";
    if (!needsLocalOutColor) {
        outColor = args.fOutputPrimary;
    } else {
        fragBuilder->codeAppendf("half4 %s;", outColor);
    }

    this->emitBlendCodeForDstRead(fragBuilder,
                                  uniformHandler,
                                  args.fInputColor,
                                  args.fInputCoverage,
                                  dstColor,
                                  outColor,
                                  args.fOutputSecondary,
                                  args.fXP);
    if (needsLocalOutColor) {
        fragBuilder->codeAppendf("%s = %s;", args.fOutputPrimary, outColor);
    }
}

void GrGLSLXferProcessor::setData(const GrGLSLProgramDataManager& pdm, const GrXferProcessor& xp,
                                  const GrTexture* dstTexture, const SkIPoint& dstTextureOffset) {
    if (dstTexture) {
        if (fDstTopLeftUni.isValid()) {
            pdm.set2f(fDstTopLeftUni, static_cast<float>(dstTextureOffset.fX),
                      static_cast<float>(dstTextureOffset.fY));
            pdm.set2f(fDstScaleUni, 1.f / dstTexture->width(), 1.f / dstTexture->height());
        } else {
            SkASSERT(!fDstScaleUni.isValid());
        }
    } else {
        SkASSERT(!fDstTopLeftUni.isValid());
        SkASSERT(!fDstScaleUni.isValid());
    }
    this->onSetData(pdm, xp);
}

void GrGLSLXferProcessor::DefaultCoverageModulation(GrGLSLXPFragmentBuilder* fragBuilder,
                                                    const char* srcCoverage,
                                                    const char* dstColor,
                                                    const char* outColor,
                                                    const char* outColorSecondary,
                                                    const GrXferProcessor& proc) {
    if (proc.dstReadUsesMixedSamples()) {
        if (srcCoverage) {
            // TODO: Once we are no longer using legacy mesh ops, it will not be possible to even
            // create a mixed sample with lcd so we can uncomment the below assert. In practice
            // today this never happens except for GLPrograms test which can make one. skia:6661
            // SkASSERT(!proc.isLCD());
            fragBuilder->codeAppendf("%s *= %s;", outColor, srcCoverage);
            fragBuilder->codeAppendf("%s = %s;", outColorSecondary, srcCoverage);
        } else {
            fragBuilder->codeAppendf("%s = half4(1.0);", outColorSecondary);
        }
    } else if (srcCoverage) {
        if (proc.isLCD()) {
            fragBuilder->codeAppendf("half lerpRed = mix(%s.a, %s.a, %s.r);",
                                     dstColor, outColor, srcCoverage);
            fragBuilder->codeAppendf("half lerpBlue = mix(%s.a, %s.a, %s.g);",
                                     dstColor, outColor, srcCoverage);
            fragBuilder->codeAppendf("half lerpGreen = mix(%s.a, %s.a, %s.b);",
                                     dstColor, outColor, srcCoverage);
        }
        fragBuilder->codeAppendf("%s = %s * %s + (half4(1.0) - %s) * %s;",
                                 outColor, srcCoverage, outColor, srcCoverage, dstColor);
        if (proc.isLCD()) {
            fragBuilder->codeAppendf("%s.a = max(max(lerpRed, lerpBlue), lerpGreen);", outColor);
        }
    }
}

