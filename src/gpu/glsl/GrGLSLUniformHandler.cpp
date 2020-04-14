/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/glsl/GrGLSLUniformHandler.h"

#include "src/gpu/glsl/GrGLSL.h"
#include "src/gpu/glsl/GrGLSLShaderBuilder.h"

void GrGLSLUniformHandler::writeUniformMappings(GrFragmentProcessor* owner,
                                                GrGLSLShaderBuilder* b) {
    for (int i = this->numUniforms() - 1; i >= 0; i--) {
        UniformInfo& u = this->uniform(i);
        if (u.fOwner == owner) {
            u.fVisibility |= kVertex_GrShaderFlag;
            b->codeAppendf("%s %s = %s;\n", GrGLSLTypeString(u.fVariable.getType()),
                           u.fRawName.c_str(), u.fVariable.getName().c_str());
        }
    }
}
