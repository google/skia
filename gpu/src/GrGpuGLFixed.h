/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrGpuGLFixed_DEFINED
#define GrGpuGLFixed_DEFINED

#include "GrGpuGL.h"

// Fixed Pipeline OpenGL or OpenGL ES 1.x
class GrGpuGLFixed : public GrGpuGL {
public:
             GrGpuGLFixed();
    virtual ~GrGpuGLFixed();
    
    virtual void resetContext();

protected:
    // overrides from GrGpu
    virtual bool flushGraphicsState(PrimitiveType type);
    virtual void setupGeometry(uint32_t startVertex,
                               uint32_t startIndex,
                               uint32_t vertexCount,
                               uint32_t indexCount);

private:
    void resetContextHelper();

    // when the texture is GL_RGBA we set the GL_COMBINE texture
    // environment rgb operand 0 to be GL_COLOR to modulate each incoming frag's
    // RGB by the texture's RGB. When the texture is GL_ALPHA we set
    // the operand to GL_ALPHA so that the incoming frag's RGB is modulated
    // by the texture's alpha.
    enum TextureEnvRGBOperands {
        kAlpha_TextureEnvRGBOperand,
        kColor_TextureEnvRGBOperand,
    };
    TextureEnvRGBOperands fHWRGBOperand0;

    void flushProjectionMatrix();

    // are the currently bound vertex buffers/arrays laid
    // out for text or other drawing.
    bool fTextVerts;

    // On GL we have to build the base vertex offset into the
    // glVertexPointer/glTexCoordPointer/etc
    int fBaseVertex;

    GrGLTexture::Orientation fHWTextureOrientation;

    typedef GrGpuGL INHERITED;
};

#endif
