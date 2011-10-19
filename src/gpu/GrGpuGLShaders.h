
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrGpuGLShaders_DEFINED
#define GrGpuGLShaders_DEFINED

#include "GrGpuGL.h"
#include "GrGLProgram.h"

class GrGpuGLProgram;

// Programmable OpenGL or OpenGL ES 2.0
class GrGpuGLShaders : public GrGpuGL {
public:
             GrGpuGLShaders(const GrGLInterface* glInterface);
    virtual ~GrGpuGLShaders();

    virtual void resetContext();

    virtual void abandonResources();

    bool programUnitTest();

protected:
    // overrides from GrGpu
    virtual bool flushGraphicsState(GrPrimitiveType type);
    virtual void setupGeometry(int* startVertex,
                               int* startIndex,
                               int vertexCount,
                               int indexCount);
    virtual void postDraw();

private:

    // for readability of function impls
    typedef GrGLProgram::ProgramDesc ProgramDesc;
    typedef ProgramDesc::StageDesc   StageDesc;
    typedef GrGLProgram::CachedData  CachedData;

    class ProgramCache;

    // Helpers to make code more readable
    const GrMatrix& getHWSamplerMatrix(int stage);
    void recordHWSamplerMatrix(int stage, const GrMatrix& matrix);

    // sets the texture matrix uniform for currently bound program
    void flushTextureMatrix(int stage);

    // sets the texture domain uniform for currently bound program
    void flushTextureDomain(int stage);

    // sets the color specified by GrDrawTarget::setColor()
    void flushColor(GrColor color);

    // sets the MVP matrix uniform for currently bound program
    void flushViewMatrix();

    // flushes the parameters to two point radial gradient
    void flushRadial2(int stage);

    // flushes the parameters for convolution
    void flushConvolution(int stage);

    // flushes the normalized texel size
    void flushTexelSize(int stage);

    // flushes the edges for edge AA
    void flushEdgeAAData();

    static void DeleteProgram(const GrGLInterface* gl,
                              CachedData* programData);

    void buildProgram(GrPrimitiveType typeBlend,
                      BlendOptFlags blendOpts,
                      GrBlendCoeff dstCoeff);

    ProgramCache*               fProgramCache;
    CachedData*                 fProgramData;
    GrGLuint                    fHWProgramID;
    GrGLProgram                 fCurrentProgram;
    // If we get rid of fixed function subclass this should move
    // to the GLCaps struct in parent class
    GrGLint                     fMaxVertexAttribs;

    typedef GrGpuGL INHERITED;
};

#endif

