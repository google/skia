//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexBuffer9.h: Defines the D3D9 VertexBuffer implementation.

#ifndef LIBANGLE_RENDERER_D3D_D3D9_VERTEXBUFFER9_H_
#define LIBANGLE_RENDERER_D3D_D3D9_VERTEXBUFFER9_H_

#include "libANGLE/renderer/d3d/VertexBuffer.h"

namespace rx
{
class Renderer9;

class VertexBuffer9 : public VertexBuffer
{
  public:
    explicit VertexBuffer9(Renderer9 *renderer);
    virtual ~VertexBuffer9();

    virtual gl::Error initialize(unsigned int size, bool dynamicUsage);

    gl::Error storeVertexAttributes(const gl::VertexAttribute &attrib,
                                    GLenum currentValueType,
                                    GLint start,
                                    GLsizei count,
                                    GLsizei instances,
                                    unsigned int offset,
                                    const uint8_t *sourceData) override;

    virtual gl::Error getSpaceRequired(const gl::VertexAttribute &attrib, GLsizei count, GLsizei instances, unsigned int *outSpaceRequired) const;

    virtual unsigned int getBufferSize() const;
    virtual gl::Error setBufferSize(unsigned int size);
    virtual gl::Error discard();

    IDirect3DVertexBuffer9 *getBuffer() const;

  private:
    Renderer9 *mRenderer;

    IDirect3DVertexBuffer9 *mVertexBuffer;
    unsigned int mBufferSize;
    bool mDynamicUsage;

    gl::Error spaceRequired(const gl::VertexAttribute &attrib, std::size_t count, GLsizei instances,
                            unsigned int *outSpaceRequired) const;
};

}

#endif // LIBANGLE_RENDERER_D3D_D3D9_VERTEXBUFFER9_H_
