/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "glsl/GrGLSLXferProcessor.h"

#include "GrXferProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"

void GrGLSLXferProcessor::emitCode(const EmitArgs& args) {
    if (!args.fXP.willReadDstColor()) {
        this->emitOutputsForBlendState(args);
        return;
    }

    GrGLSLXPFragmentBuilder* fragBuilder = args.fXPFragBuilder;
    const char* dstColor = fragBuilder->dstColor();

    if (args.fXP.getDstTexture()) {
        bool topDown = kTopLeft_GrSurfaceOrigin == args.fXP.getDstTexture()->origin();

        if (args.fInputCoverage) {
            // We don't think any shaders actually output negative coverage, but just as a safety
            // check for floating point precision errors we compare with <= here
            fragBuilder->codeAppendf("if (all(lessThanEqual(%s, vec4(0)))) {"
                                     "    discard;"
                                     "}", args.fInputCoverage);
        }

        const char* dstTopLeftName;
        const char* dstCoordScaleName;

        fDstTopLeftUni = args.fPB->addUniform(GrGLSLProgramBuilder::kFragment_Visibility,
                                              kVec2f_GrSLType,
                                              kDefault_GrSLPrecision,
                                              "DstTextureUpperLeft",
                                              &dstTopLeftName);
        fDstScaleUni = args.fPB->addUniform(GrGLSLProgramBuilder::kFragment_Visibility,
                                            kVec2f_GrSLType,
                                            kDefault_GrSLPrecision,
                                            "DstTextureCoordScale",
                                            &dstCoordScaleName);
        const char* fragPos = fragBuilder->fragmentPosition();

        fragBuilder->codeAppend("// Read color from copy of the destination.\n");
        fragBuilder->codeAppendf("vec2 _dstTexCoord = (%s.xy - %s) * %s;",
                                 fragPos, dstTopLeftName, dstCoordScaleName);

        if (!topDown) {
            fragBuilder->codeAppend("_dstTexCoord.y = 1.0 - _dstTexCoord.y;");
        }

        fragBuilder->codeAppendf("vec4 %s = ", dstColor);
        fragBuilder->appendTextureLookup(args.fSamplers[0], "_dstTexCoord", kVec2f_GrSLType);
        fragBuilder->codeAppend(";");
    }

    this->emitBlendCodeForDstRead(args.fPB,
                                  fragBuilder,
                                  args.fInputColor,
                                  args.fInputCoverage,
                                  dstColor,
                                  args.fOutputPrimary,
                                  args.fOutputSecondary,
                                  args.fXP);
}

void GrGLSLXferProcessor::setData(const GrGLSLProgramDataManager& pdm, const GrXferProcessor& xp) {
    if (xp.getDstTexture()) {
        if (fDstTopLeftUni.isValid()) {
            pdm.set2f(fDstTopLeftUni, static_cast<float>(xp.dstTextureOffset().fX),
                      static_cast<float>(xp.dstTextureOffset().fY));
            pdm.set2f(fDstScaleUni, 1.f / xp.getDstTexture()->width(),
                      1.f / xp.getDstTexture()->height());
        } else {
            SkASSERT(!fDstScaleUni.isValid());
        }
    } else {
        SkASSERT(!fDstTopLeftUni.isValid());
        SkASSERT(!fDstScaleUni.isValid());
    }
    this->onSetData(pdm, xp);
}

