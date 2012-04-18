/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLCustomStage_DEFINED
#define GrGLCustomStage_DEFINED

#include "../GrAllocator.h"
#include "GrGLShaderVar.h"
#include "GrGLSL.h"
#include "../GrStringBuilder.h"

class GrCustomStage;
class GrGLInterface;

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
    // TODO: redundant with GrGLProgram.cpp
    enum {
        kUnusedUniform = -1,
        kUseUniform = 2000
    };

    typedef GrTAllocator<GrGLShaderVar> VarArray;


    /** Creates any uniform variables the vertex shader requires
        and appends them to vsUnis;
        must guarantee they are unique (typically done by
        appending the stage number). */
    virtual void setupVSUnis(VarArray& vsUnis, int stage);

    /** Creates any uniform variables the fragment shader requires
        and appends them to fsUnis;
        must guarantee they are unique (typically done by
        appending the stage number). */
    virtual void setupFSUnis(VarArray& fsUnis, int stage);

    /** Given an empty GrStringBuilder and the names of variables;
        must write shader code into that GrStringBuilder.
        Vertex shader input is a vec2 of coordinates, which may
        be altered.
        The code will be inside an otherwise-empty block. */
    virtual void emitVS(GrStringBuilder* code,
                        const char* vertexCoords) = 0;

    /** Given an empty GrStringBuilder and the names of variables;
        must write shader code into that GrStringBuilder.
        The code will be inside an otherwise-empty block.
        Fragment shader inputs are a vec2 of coordinates, one texture,
        and a color; output is a color. */
    /* TODO: don't give them the samplerName, just a handle; then give
       a function here for them to call into that'll apply any texture
       domain - but do we force them to be honest about texture domain
       parameters? */
    virtual void emitFS(GrStringBuilder* code,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName,
                        const char* sampleCoords) = 0;

    /** Binds uniforms; we must have already bound the program and
        determined its GL program ID. */
    virtual void initUniforms(const GrGLInterface*, int programID);

    /** A GrGLCustomStage instance can be reused with any GrCustomStage
        that produces the same stage key; this function reads data from
        a stage and uploads any uniform variables required by the shaders
        created in emit*().
        flush() to change the GrCustomStage from which the uniforms
        are to be read.
        TODO: since we don't have a factory, we can't assert to enforce
        this. Shouldn't we? */
    virtual void setData(const GrGLInterface*, GrCustomStage*);

    // TODO: needs a better name
    enum SamplerMode {
        kDefault_SamplerMode,
        kProj_SamplerMode,
        kExplicitDivide_SamplerMode  // must do an explicit divide
    };

    void setSamplerMode(SamplerMode shaderMode) { fSamplerMode = shaderMode; }

protected:

    /** Returns the *effective* coord name after any perspective divide
        or other transform. */
    GrStringBuilder emitTextureSetup(GrStringBuilder* code,
                                     const char* coordName,
                                     int stageNum,
                                     int coordDims,
                                     int varyingDims);

    /** Convenience function for subclasses to write texture2D() or
        texture2DProj(), depending on fSamplerMode. */
    void emitTextureLookup(GrStringBuilder* code,
                           const char* samplerName,
                           const char* coordName);

    SamplerMode fSamplerMode;
    GrStringBuilder fCoordName;

};


/// Every GrGLProgramStage subclass needs a GrGLProgramStageFactory subclass
/// to manage its creation.

class GrGLProgramStageFactory {

public:

    virtual ~GrGLProgramStageFactory();

    /** Returns a short unique identifier for this subclass x its
        parameters. If the key differs, different shader code must
        be generated; if the key matches, shader code can be reused.
        0 == no custom stage.  */
    virtual uint16_t stageKey(const GrCustomStage*);

    virtual GrGLProgramStage* createGLInstance(GrCustomStage*) = 0;

protected:

    /** Disable default constructor - instances should be singletons
        with static factory functions: our test examples are all stateless,
        but we suspect that future implementations may want to cache data?  */
    GrGLProgramStageFactory() { }
};

#endif
