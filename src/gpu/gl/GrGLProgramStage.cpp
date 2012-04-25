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

GrGLProgramStageFactory::~GrGLProgramStageFactory(void) {

}

uint16_t GrGLProgramStageFactory::stageKey(const GrCustomStage*) {
    return 0;
}

void GrGLProgramStage::setupVSUnis(VarArray& vsUnis, int stage) {

}

void GrGLProgramStage::setupFSUnis(VarArray& fsUnis, int stage) {

}

void GrGLProgramStage::initUniforms(const GrGLInterface*, int progID) {

}

void GrGLProgramStage::setData(const GrGLInterface*, GrCustomStage*) {

}

GrStringBuilder GrGLProgramStage::emitTextureSetup(GrStringBuilder* code,
                                                   const char* coordName,
                                                   int stageNum,
                                                   int coordDims,
                                                   int varyingDims) {
    GrStringBuilder retval;

    switch (fSamplerMode) {
        case kDefault_SamplerMode:
            // Fall through
        case kProj_SamplerMode:
            // Do nothing
            retval = coordName;
            break;
        case kExplicitDivide_SamplerMode:
            retval = "inCoord";
            retval.appendS32(stageNum);
            code->appendf("\t %s %s = %s%s / %s%s\n",
                GrGLShaderVar::TypeString(GrSLFloatVectorType(coordDims)),
                fCoordName.c_str(),
                coordName,
                GrGLSLVectorNonhomogCoords(varyingDims),
                coordName,
                GrGLSLVectorHomogCoord(varyingDims));
            break;
    }
    return retval;
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

