/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSL.h"
#include "GrGLProgramStage.h"

GrGLProgramStage::~GrGLProgramStage() {
}

///////////////////////////////////////////////////////////////////////////////

void GrGLProgramStage::setupVSUnis(VarArray* vsUnis, int stage) {

}

void GrGLProgramStage::setupFSUnis(VarArray* fsUnis, int stage) {

}

void GrGLProgramStage::initUniforms(const GrGLInterface*, int progID) {

}

void GrGLProgramStage::setData(const GrGLInterface*, GrCustomStage*,
                               const GrGLTexture*) {

}

void GrGLProgramStage::emitTextureSetup(GrGLShaderBuilder* segments) {
    GrStringBuilder retval;

    switch (fSamplerMode) {
        case kDefault_SamplerMode:
            // Fall through
        case kProj_SamplerMode:
            // Do nothing
            break;
        case kExplicitDivide_SamplerMode:
            retval = "inCoord";
            segments->fFSCode.appendf("\t %s %s = %s%s / %s%s\n",
                GrGLShaderVar::TypeString
                    (GrSLFloatVectorType(segments->fCoordDims)),
                retval.c_str(),
                segments->fSampleCoords.c_str(),
                GrGLSLVectorNonhomogCoords(segments->fVaryingDims),
                segments->fSampleCoords.c_str(),
                GrGLSLVectorHomogCoord(segments->fVaryingDims));
            segments->fSampleCoords = retval;
            break;
    }
}

void GrGLProgramStage::emitTextureLookup(GrStringBuilder* code,
                                         const char* samplerName,
                                         const char* coordName) {
    switch (fSamplerMode) {
        case kDefault_SamplerMode:
            // Fall through
        case kExplicitDivide_SamplerMode:
            code->appendf("texture2D(%s, %s)", samplerName, coordName);
            break;
        case kProj_SamplerMode:
            code->appendf("texture2DProj(%s, %s)", samplerName, coordName);
            break;
    }

}

