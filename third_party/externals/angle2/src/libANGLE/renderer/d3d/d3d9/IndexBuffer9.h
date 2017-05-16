//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Indexffer9.h: Defines the D3D9 IndexBuffer implementation.

#ifndef LIBANGLE_RENDERER_D3D_D3D9_INDEXBUFFER9_H_
#define LIBANGLE_RENDERER_D3D_D3D9_INDEXBUFFER9_H_

#include "libANGLE/renderer/d3d/IndexBuffer.h"

namespace rx
{
class Renderer9;

class IndexBuffer9 : public IndexBuffer
{
  public:
    explicit IndexBuffer9(Renderer9 *const renderer);
    virtual ~IndexBuffer9();

    virtual gl::Error initialize(unsigned int bufferSize, GLenum indexType, bool dynamic);

    virtual gl::Error mapBuffer(unsigned int offset, unsigned int size, void** outMappedMemory);
    virtual gl::Error unmapBuffer();

    virtual GLenum getIndexType() const;
    virtual unsigned int getBufferSize() const;
    virtual gl::Error setSize(unsigned int bufferSize, GLenum indexType);

    virtual gl::Error discard();

    D3DFORMAT getIndexFormat() const;
    IDirect3DIndexBuffer9 *getBuffer() const;

  private:
    Renderer9 *const mRenderer;

    IDirect3DIndexBuffer9 *mIndexBuffer;
    unsigned int mBufferSize;
    GLenum mIndexType;
    bool mDynamic;
};

}

#endif // LIBANGLE_RENDERER_D3D_D3D9_INDEXBUFFER9_H_
