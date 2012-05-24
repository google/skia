/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLCustomStage_DEFINED
#define GrGLCustomStage_DEFINED

#include "GrAllocator.h"
#include "GrCustomStage.h"
#include "GrGLProgram.h"
#include "GrGLShaderBuilder.h"
#include "GrGLShaderVar.h"
#include "GrGLSL.h"
#include "GrStringBuilder.h"

struct GrGLInterface;
class GrGLTexture;

/** @file
    This file contains specializations for OpenGL of the shader stages
    declared in src/gpu/GrCustomStage.h. All the functions emit
    GLSL shader code and OpenGL calls.

    These objects are created by a factory function on the
    GrCustomStage.
    TODO: lifetime management.
*/

class GrGLProgramStage {

public:
    typedef GrCustomStage::StageKey StageKey ;
    // TODO: redundant with GrGLProgram.cpp
    enum {
        kUnusedUniform = -1,
        kUseUniform = 2000
    };

    GrGLProgramStage(const GrProgramStageFactory&);

    virtual ~GrGLProgramStage();

    /** Creates any uniform variables the vertex shader requires
        and appends them to vsUnis;
        must guarantee they are unique (typically done by
        appending the stage number). */
    virtual void setupVSUnis(VarArray* vsUnis, int stage);

    /** Creates any uniform variables the fragment shader requires
        and appends them to fsUnis;
        must guarantee they are unique (typically done by
        appending the stage number). */
    virtual void setupFSUnis(VarArray* fsUnis, int stage);

    /** Creates any varying variables shared between the shaders;
        must guarantee they are unique (typically done by
        appending the stage number). */
    virtual void setupVaryings(GrGLShaderBuilder* state, int stage);

    /** Appends vertex code to the appropriate GrStringBuilder
        on the state.
        The code will be inside an otherwise-empty block.
        Vertex shader input is a vec2 of coordinates, which may
        be altered.
        The code will be inside an otherwise-empty block. */
    virtual void emitVS(GrGLShaderBuilder* state,
                        const char* vertexCoords) = 0;

    /** Appends fragment code to the appropriate GrStringBuilder
        on the state.
        The code will be inside an otherwise-empty block.
        Fragment shader inputs are a vec2 of coordinates, one texture,
        and a color; output is a color. */
    /* TODO: don't give them the samplerName, just a handle; then give
       a function here for them to call into that'll apply any texture
       domain - but do we force them to be honest about texture domain
       parameters? */
    virtual void emitFS(GrGLShaderBuilder* state,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) = 0;

    /** Binds uniforms; we must have already bound the program and
        determined its GL program ID. */
    virtual void initUniforms(const GrGLInterface* gl, int programID);

    /** A GrGLCustomStage instance can be reused with any GrCustomStage
        that produces the same stage key; this function reads data from
        a stage and uploads any uniform variables required by the shaders
        created in emit*(). */
    virtual void setData(const GrGLInterface* gl,
                         const GrGLTexture& texture,
                         GrCustomStage* stage,
                         int stageNum);

    const char* name() const { return fFactory.name(); }

protected:

    const GrProgramStageFactory& fFactory;
};

#endif
