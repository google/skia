
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrGpuGLFixed_DEFINED
#define GrGpuGLFixed_DEFINED

#include "GrDrawState.h"
#include "GrGpuGL.h"

// Fixed Pipeline OpenGL or OpenGL ES 1.x
class GrGpuGLFixed : public GrGpuGL {
public:
             GrGpuGLFixed(const GrGLInterface* glInterface);
    virtual ~GrGpuGLFixed();

protected:
    // overrides from GrGpu
    virtual bool flushGraphicsState(GrPrimitiveType type);
    virtual void setupGeometry(int* startVertex,
                               int* startIndex,
                               int vertexCount,
                               int indexCount);

private:
    virtual void resetContext();

    // Helpers to make code more readable
    const GrMatrix& getHWSamplerMatrix(int stage) const {
        return fHWDrawState.fSamplerStates[stage].getMatrix();
    }
    void recordHWSamplerMatrix(int stage, const GrMatrix& matrix) {
        fHWDrawState.fSamplerStates[stage].setMatrix(matrix);
    }

    // when the texture is GL_RGBA we set the GL_COMBINE texture
    // environment rgb operand 0 to be GL_COLOR to modulate each incoming
    // R,G, & B by the texture's R, G, & B. When the texture is alpha-only we
    // set the operand to GL_ALPHA so that the incoming frag's R, G, &B are all
    // modulated by the texture's A.
    enum TextureEnvRGBOperands {
        kAlpha_TextureEnvRGBOperand,
        kColor_TextureEnvRGBOperand,
    };
    TextureEnvRGBOperands fHWRGBOperand0[GrDrawState::kNumStages];

    void flushProjectionMatrix();

    // are the currently bound vertex buffers/arrays laid
    // out for text or other drawing.
    bool fTextVerts;

    // On GL we have to build the base vertex offset into the
    // glVertexPointer/glTexCoordPointer/etc
    int fBaseVertex;

    typedef GrGpuGL INHERITED;
};

#endif
