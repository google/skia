/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "glsl/GrGLSLProgramBuilder.h"

const int GrGLSLProgramBuilder::kVarsPerBlock = 8;

GrGLSLProgramBuilder::GrGLSLProgramBuilder(const DrawArgs& args)
    : fVS(this)
    , fGS(this)
    , fFS(this, args.fDesc->header().fFragPosKey)
    , fStageIndex(-1)
    , fArgs(args) {
}

void GrGLSLProgramBuilder::nameVariable(SkString* out, char prefix, const char* name, bool mangle) {
    if ('\0' == prefix) {
        *out = name;
    } else {
        out->printf("%c%s", prefix, name);
    }
    if (mangle) {
        if (out->endsWith('_')) {
            // Names containing "__" are reserved.
            out->append("x");
        }
        out->appendf("_Stage%d%s", fStageIndex, fFS.getMangleString().c_str());
    }
}

void GrGLSLProgramBuilder::appendUniformDecls(ShaderVisibility visibility,
                                              SkString* out) const {
    this->uniformHandler()->appendUniformDecls(visibility, out);
}

void GrGLSLProgramBuilder::addRTAdjustmentUniform(GrSLPrecision precision,
                                                  const char* name,
                                                  const char** outName) {
        SkASSERT(!fUniformHandles.fRTAdjustmentUni.isValid());
        fUniformHandles.fRTAdjustmentUni =
            this->uniformHandler()->addUniform(GrGLSLUniformHandler::kVertex_Visibility,
                                               kVec4f_GrSLType,
                                               precision,
                                               name,
                                               outName);
}

void GrGLSLProgramBuilder::addRTHeightUniform(const char* name, const char** outName) {
        SkASSERT(!fUniformHandles.fRTHeightUni.isValid());
        GrGLSLUniformHandler* uniformHandler = this->uniformHandler();
        fUniformHandles.fRTHeightUni =
            uniformHandler->internalAddUniformArray(GrGLSLUniformHandler::kFragment_Visibility,
                                                    kFloat_GrSLType, kDefault_GrSLPrecision,
                                                    name, false, 0, outName);
}

