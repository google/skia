//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// IndexBuffer11.h: Defines the D3D11 IndexBuffer implementation.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_INDEXBUFFER11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_INDEXBUFFER11_H_

#include "libANGLE/renderer/d3d/IndexBuffer.h"

namespace rx
{
class Renderer11;

class IndexBuffer11 : public IndexBuffer
{
  public:
    explicit IndexBuffer11(Renderer11 *const renderer);
    virtual ~IndexBuffer11();

    virtual gl::Error initialize(unsigned int bufferSize, GLenum indexType, bool dynamic);

    virtual gl::Error mapBuffer(unsigned int offset, unsigned int size, void** outMappedMemory);
    virtual gl::Error unmapBuffer();

    virtual GLenum getIndexType() const;
    virtual unsigned int getBufferSize() const;
    virtual gl::Error setSize(unsigned int bufferSize, GLenum indexType);

    virtual gl::Error discard();

    DXGI_FORMAT getIndexFormat() const;
    ID3D11Buffer *getBuffer() const;

  private:
    Renderer11 *const mRenderer;

    ID3D11Buffer *mBuffer;
    unsigned int mBufferSize;
    GLenum mIndexType;
    bool mDynamicUsage;
};

}

#endif // LIBANGLE_RENDERER_D3D_D3D11_INDEXBUFFER11_H_
