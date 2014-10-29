/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLNvprProgramBuilder_DEFINED
#define GrGLNvprProgramBuilder_DEFINED

#include "GrGLProgramBuilder.h"

class GrGLNvprProgramBuilder : public GrGLProgramBuilder {
public:
    GrGLNvprProgramBuilder(GrGpuGL*, const GrOptDrawState&);

    /*
     * The separable varying info must be passed to GrGLProgram so this must
     * be part of the public interface
     */
    struct SeparableVaryingInfo {
        GrGLShaderVar fVariable;
        GrGLint       fLocation;
    };

    typedef GrTAllocator<SeparableVaryingInfo> SeparableVaryingInfoArray;

    virtual GrGLProgram* createProgram(GrGLuint programID);

private:
    virtual void emitTransforms(const GrFragmentStage&,
                                GrGLProcessor::TransformedCoordsArray* outCoords,
                                GrGLInstalledFragProc*) SK_OVERRIDE;

    typedef GrGLInstalledFragProc::ShaderVarHandle ShaderVarHandle;

    /**
     * Add a separable varying input variable to the current program.
     * A separable varying (fragment shader input) is a varying that can be used also when vertex
     * shaders are not used. With a vertex shader, the operation is same as with other
     * varyings. Without a vertex shader, such as with NV_path_rendering, GL APIs are used to
     * populate the variable. The APIs can refer to the variable through the returned handle.
     */
    ShaderVarHandle addSeparableVarying(const char* name, GrGLVarying* v);

    void resolveSeparableVaryings(GrGLuint programId);

    SeparableVaryingInfoArray fSeparableVaryingInfos;

    typedef GrGLProgramBuilder INHERITED;
};

#endif
