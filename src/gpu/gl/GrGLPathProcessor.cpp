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

GrGLPathProcessor::GrGLPathProcessor(const GrPathProcessor&, const GrBatchTracker&)
    : fColor(GrColor_ILLEGAL) {}

void GrGLPathProcessor::emitCode(EmitArgs& args) {
    GrGLGPBuilder* pb = args.fPB;
    GrGLFragmentBuilder* fs = args.fPB->getFragmentShaderBuilder();
    const PathBatchTracker& local = args.fBT.cast<PathBatchTracker>();

    // emit transforms
    this->emitTransforms(args.fPB, args.fTransformsIn, args.fTransformsOut);

    // Setup uniform color
    if (kUniform_GrGPInput == local.fInputColorType) {
        const char* stagedLocalVarName;
        fColorUniform = pb->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                       kVec4f_GrSLType,
                                       kDefault_GrSLPrecision,
                                       "Color",
                                       &stagedLocalVarName);
        fs->codeAppendf("%s = %s;", args.fOutputColor, stagedLocalVarName);
    }

    // setup constant solid coverage
    if (kAllOnes_GrGPInput == local.fInputCoverageType) {
        fs->codeAppendf("%s = vec4(1);", args.fOutputCoverage);
    }
}

void GrGLPathProcessor::GenKey(const GrPathProcessor&,
                               const GrBatchTracker& bt,
                               const GrGLSLCaps&,
                               GrProcessorKeyBuilder* b) {
    const PathBatchTracker& local = bt.cast<PathBatchTracker>();
    b->add32(local.fInputColorType | local.fInputCoverageType << 16);
}

void GrGLPathProcessor::setData(const GrGLProgramDataManager& pdman,
                                const GrPrimitiveProcessor& primProc,
                                const GrBatchTracker& bt) {
    const PathBatchTracker& local = bt.cast<PathBatchTracker>();
    if (kUniform_GrGPInput == local.fInputColorType && local.fColor != fColor) {
        GrGLfloat c[4];
        GrColorToRGBAFloat(local.fColor, c);
        pdman.set4fv(fColorUniform, 1, c);
        fColor = local.fColor;
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
            pb->addVarying(strVaryingName.c_str(), &v);
            SeparableVaryingInfo& varyingInfo = fSeparableVaryingInfos.push_back();
            varyingInfo.fVariable = pb->getFragmentShaderBuilder()->fInputs.back();
            varyingInfo.fLocation = fSeparableVaryingInfos.count() - 1;
            varyingInfo.fType = varyingType;
            fInstalledTransforms[i][t].fHandle = ShaderVarHandle(varyingInfo.fLocation);
            fInstalledTransforms[i][t].fType = varyingType;

            SkNEW_APPEND_TO_TARRAY(&(*tout)[i], GrGLProcessor::TransformedCoords,
                                   (SkString(v.fsIn()), varyingType));
        }
    }
}

void GrGLPathProcessor::resolveSeparableVaryings(GrGLGpu* gpu, GrGLuint programId) {
    int count = fSeparableVaryingInfos.count();
    for (int i = 0; i < count; ++i) {
        GrGLint location;
        GR_GL_CALL_RET(gpu->glInterface(),
                       location,
                       GetProgramResourceLocation(programId,
                                                  GR_GL_FRAGMENT_INPUT,
                                                  fSeparableVaryingInfos[i].fVariable.c_str()));
        fSeparableVaryingInfos[i].fLocation = location;
    }
}

void GrGLPathProcessor::setTransformData(
        const GrPrimitiveProcessor& primProc,
        int index,
        const SkTArray<const GrCoordTransform*, true>& coordTransforms,
        GrGLPathRendering* glpr,
        GrGLuint programID) {
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
        const SeparableVaryingInfo& fragmentInput =
                fSeparableVaryingInfos[transforms[t].fHandle.handle()];
        SkASSERT(transforms[t].fType == kVec2f_GrSLType ||
                 transforms[t].fType == kVec3f_GrSLType);
        unsigned components = transforms[t].fType == kVec2f_GrSLType ? 2 : 3;
        glpr->setProgramPathFragmentInputTransform(programID,
                                                   fragmentInput.fLocation,
                                                   GR_GL_OBJECT_LINEAR,
                                                   components,
                                                   transform);
    }
}
