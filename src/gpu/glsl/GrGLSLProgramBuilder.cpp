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
    this->onAppendUniformDecls(visibility, out);
}

