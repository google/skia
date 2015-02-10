/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLXferProcessor.h"

#include "GrXferProcessor.h"
#include "gl/builders/GrGLFragmentShaderBuilder.h"
#include "gl/builders/GrGLProgramBuilder.h"

void GrGLXferProcessor::emitCode(const EmitArgs& args) {
    if (args.fXP.getDstCopyTexture()) {
        bool topDown = kTopLeft_GrSurfaceOrigin == args.fXP.getDstCopyTexture()->origin();

        GrGLFPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();

        // We don't think any shaders actually output negative coverage, but just as a safety check
        // for floating point precision errors we compare with <= here
        fsBuilder->codeAppendf("if (all(lessThanEqual(%s, vec4(0)))) {"
                               "    discard;"
                               "}", args.fInputCoverage);
        const char* dstColor = fsBuilder->dstColor();

        const char* dstCopyTopLeftName;
        const char* dstCopyCoordScaleName;

        fDstCopyTopLeftUni = args.fPB->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                                   kVec2f_GrSLType,
                                                   kDefault_GrSLPrecision,
                                                   "DstCopyUpperLeft",
                                                   &dstCopyTopLeftName);
        fDstCopyScaleUni = args.fPB->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                                kVec2f_GrSLType,
                                                kDefault_GrSLPrecision,
                                                "DstCopyCoordScale",
                                                &dstCopyCoordScaleName);
        const char* fragPos = fsBuilder->fragmentPosition();

        fsBuilder->codeAppend("// Read color from copy of the destination.\n");
        fsBuilder->codeAppendf("vec2 _dstTexCoord = (%s.xy - %s) * %s;",
                               fragPos, dstCopyTopLeftName, dstCopyCoordScaleName);

        if (!topDown) {
            fsBuilder->codeAppend("_dstTexCoord.y = 1.0 - _dstTexCoord.y;");
        }

        fsBuilder->codeAppendf("vec4 %s = ", dstColor);
        fsBuilder->appendTextureLookup(args.fSamplers[0], "_dstTexCoord", kVec2f_GrSLType);
        fsBuilder->codeAppend(";");
    }

    this->onEmitCode(args);
}

void GrGLXferProcessor::setData(const GrGLProgramDataManager& pdm, const GrXferProcessor& xp) {
    if (xp.getDstCopyTexture()) {
        if (fDstCopyTopLeftUni.isValid()) {
            pdm.set2f(fDstCopyTopLeftUni, static_cast<GrGLfloat>(xp.dstCopyTextureOffset().fX),
                      static_cast<GrGLfloat>(xp.dstCopyTextureOffset().fY));
            pdm.set2f(fDstCopyScaleUni, 1.f / xp.getDstCopyTexture()->width(),
                      1.f / xp.getDstCopyTexture()->height());
        } else {
            SkASSERT(!fDstCopyScaleUni.isValid());
        }
    } else {
        SkASSERT(!fDstCopyTopLeftUni.isValid());
        SkASSERT(!fDstCopyScaleUni.isValid());
    }
    this->onSetData(pdm, xp);
}

