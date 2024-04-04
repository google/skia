/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

GrShaderVar GrGLSLUniformHandler::getUniformMapping(const GrProcessor& owner,
                                                    SkString rawName) const {
    for (int i = this->numUniforms() - 1; i >= 0; i--) {
        const UniformInfo& u = this->uniform(i);
        if (u.fOwner == &owner && u.fRawName == rawName) {
            return u.fVariable;
        }
    }
    return GrShaderVar();
}

GrShaderVar GrGLSLUniformHandler::liftUniformToVertexShader(const GrProcessor& owner,
                                                            SkString rawName) {
    for (int i = this->numUniforms() - 1; i >= 0; i--) {
        UniformInfo& u = this->uniform(i);
        if (u.fOwner == &owner && u.fRawName == rawName) {
            u.fVisibility |= kVertex_GrShaderFlag;
            return u.fVariable;
        }
    }
    // Uniform not found; it's better to return a void variable than to assert because sample
    // matrices that are uniform are treated the same for most of the code. When the sample
    // matrix expression can't be found as a uniform, we can infer it's a constant.
    return GrShaderVar();
}
