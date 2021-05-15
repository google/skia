/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeTessellationShaderImpl_DEFINED
#define GrStrokeTessellationShaderImpl_DEFINED

#include "src/gpu/tessellate/GrStrokeShader.h"

class GrStrokeTessellationShaderImpl : public GrStrokeShaderImpl {
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override;

    SkString getTessControlShaderGLSL(const GrGeometryProcessor&,
                                      const char* versionAndExtensionDecls,
                                      const GrGLSLUniformHandler&,
                                      const GrShaderCaps&) const override;

    SkString getTessEvaluationShaderGLSL(const GrGeometryProcessor&,
                                         const char* versionAndExtensionDecls,
                                         const GrGLSLUniformHandler&,
                                         const GrShaderCaps&) const override;
};


#endif
