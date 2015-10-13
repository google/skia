//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexBuffer11.h: Defines the D3D11 VertexBuffer implementation.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_VERTEXBUFFER11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_VERTEXBUFFER11_H_

#include <stdint.h>

#include "libANGLE/renderer/d3d/VertexBuffer.h"

namespace rx
{
class Renderer11;

class VertexBuffer11 : public VertexBuffer
{
  public:
    explicit VertexBuffer11(Renderer11 *const renderer);
    virtual ~VertexBuffer11();

    virtual gl::Error initialize(unsigned int size, bool dynamicUsage);

    gl::Error storeVertexAttributes(const gl::VertexAttribute &attrib,
                                    GLenum currentValueType,
                                    GLint start,
                                    GLsizei count,
                                    GLsizei instances,
                                    unsigned int offset,
                                    const uint8_t *sourceData) override;

    virtual gl::Error getSpaceRequired(const gl::VertexAttribute &attrib, GLsizei count, GLsizei instances,
                                       unsigned int *outSpaceRequired) const;

    virtual unsigned int getBufferSize() const;
    virtual gl::Error setBufferSize(unsigned int size);
    virtual gl::Error discard();

    virtual void hintUnmapResource();

    ID3D11Buffer *getBuffer() const;

  private:
    gl::Error mapResource();

    Renderer11 *const mRenderer;

    ID3D11Buffer *mBuffer;
    unsigned int mBufferSize;
    bool mDynamicUsage;

    uint8_t *mMappedResourceData;
};

}

#endif // LIBANGLE_RENDERER_D3D_D3D11_VERTEXBUFFER11_H_
