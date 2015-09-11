/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLPathProcessor.h"

#include "GrPathProcessor.h"
#include "GrGLGpu.h"
#include "GrGLPathRendering.h"

GrGLPathProcessor::GrGLPathProcessor()
    : fColor(GrColor_ILLEGAL) {}

void GrGLPathProcessor::emitCode(EmitArgs& args) {
    GrGLGPBuilder* pb = args.fPB;
    GrGLFragmentBuilder* fs = args.fPB->getFragmentShaderBuilder();
    const GrPathProcessor& pathProc = args.fGP.cast<GrPathProcessor>();

    // emit transforms
    this->emitTransforms(args.fPB, args.fTransformsIn, args.fTransformsOut);

    // Setup uniform color
    if (pathProc.opts().readsColor()) {
        const char* stagedLocalVarName;
        fColorUniform = pb->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                       kVec4f_GrSLType,
                                       kDefault_GrSLPrecision,
                                       "Color",
                                       &stagedLocalVarName);
        fs->codeAppendf("%s = %s;", args.fOutputColor, stagedLocalVarName);
    }

    // setup constant solid coverage
    if (pathProc.opts().readsCoverage()) {
        fs->codeAppendf("%s = vec4(1);", args.fOutputCoverage);
    }
}

void GrGLPathProcessor::GenKey(const GrPathProcessor& pathProc,
                               const GrGLSLCaps&,
                               GrProcessorKeyBuilder* b) {
    b->add32(SkToInt(pathProc.opts().readsColor()) |
             SkToInt(pathProc.opts().readsCoverage()) << 16);
}

void GrGLPathProcessor::setData(const GrGLProgramDataManager& pdman,
                                const GrPrimitiveProcessor& primProc) {
    const GrPathProcessor& pathProc = primProc.cast<GrPathProcessor>();
    if (pathProc.opts().readsColor() && pathProc.color() != fColor) {
        GrGLfloat c[4];
        GrColorToRGBAFloat(pathProc.color(), c);
        pdman.set4fv(fColorUniform, 1, c);
        fColor = pathProc.color();
    }
}

void GrGLPathProcessor::emitTransforms(GrGLGPBuilder* pb, const TransformsIn& tin,
                                       TransformsOut* tout) {
    tout->push_back_n(tin.count());
    fInstalledTransforms.push_back_n(tin.count());
    for (int i = 0; i < tin.count(); i++) {
        const ProcCoords& coordTransforms = tin[i];
        fInstalledTransforms[i].push_back_n(coordTransforms.count());
        for (int t = 0; t < coordTransforms.count(); t++) {
            GrSLType varyingType =
                    coordTransforms[t]->getMatrix().hasPerspective() ? kVec3f_GrSLType :
                                                                       kVec2f_GrSLType;

            SkString strVaryingName("MatrixCoord");
            strVaryingName.appendf("_%i_%i", i, t);
            GrGLVertToFrag v(varyingType);
            fInstalledTransforms[i][t].fHandle =
                    pb->addSeparableVarying(strVaryingName.c_str(), &v).toIndex();
            fInstalledTransforms[i][t].fType = varyingType;

            SkNEW_APPEND_TO_TARRAY(&(*tout)[i], GrGLProcessor::TransformedCoords,
                                   (SkString(v.fsIn()), varyingType));
        }
    }
}

void GrGLPathProcessor::setTransformData(
        const GrPrimitiveProcessor& primProc,
        const GrGLProgramDataManager& pdman,
        int index,
        const SkTArray<const GrCoordTransform*, true>& coordTransforms) {
    const GrPathProcessor& pathProc = primProc.cast<GrPathProcessor>();
    SkSTArray<2, Transform, true>& transforms = fInstalledTransforms[index];
    int numTransforms = transforms.count();
    for (int t = 0; t < numTransforms; ++t) {
        SkASSERT(transforms[t].fHandle.isValid());
        const SkMatrix& transform = GetTransformMatrix(pathProc.localMatrix(),
                                                       *coordTransforms[t]);
        if (transforms[t].fCurrentValue.cheapEqualTo(transform)) {
            continue;
        }
        transforms[t].fCurrentValue = transform;

        SkASSERT(transforms[t].fType == kVec2f_GrSLType ||
                 transforms[t].fType == kVec3f_GrSLType);
        unsigned components = transforms[t].fType == kVec2f_GrSLType ? 2 : 3;
        pdman.setPathFragmentInputTransform(transforms[t].fHandle, components, transform);
    }
}
