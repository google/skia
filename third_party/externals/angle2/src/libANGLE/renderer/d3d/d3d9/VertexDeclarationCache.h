//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexDeclarationCache.h: Defines a helper class to construct and cache vertex declarations.

#ifndef LIBANGLE_RENDERER_D3D_D3D9_VERTEXDECLARATIONCACHE_H_
#define LIBANGLE_RENDERER_D3D_D3D9_VERTEXDECLARATIONCACHE_H_

#include "libANGLE/Error.h"
#include "libANGLE/renderer/d3d/VertexDataManager.h"

namespace gl
{
class VertexDataManager;
class Program;
}

namespace rx
{

class VertexDeclarationCache
{
  public:
    VertexDeclarationCache();
    ~VertexDeclarationCache();

    gl::Error applyDeclaration(IDirect3DDevice9 *device,
                               const std::vector<TranslatedAttribute> &attributes,
                               gl::Program *program,
                               GLsizei instances,
                               GLsizei *repeatDraw);

    void markStateDirty();

  private:
    UINT mMaxLru;

    enum { NUM_VERTEX_DECL_CACHE_ENTRIES = 32 };

    struct VBData
    {
        unsigned int serial;
        unsigned int stride;
        unsigned int offset;
    };

    VBData mAppliedVBs[gl::MAX_VERTEX_ATTRIBS];
    IDirect3DVertexDeclaration9 *mLastSetVDecl;
    bool mInstancingEnabled;

    struct VertexDeclCacheEntry
    {
        D3DVERTEXELEMENT9 cachedElements[gl::MAX_VERTEX_ATTRIBS + 1];
        UINT lruCount;
        IDirect3DVertexDeclaration9 *vertexDeclaration;
    } mVertexDeclCache[NUM_VERTEX_DECL_CACHE_ENTRIES];
};

}

#endif // LIBANGLE_RENDERER_D3D_D3D9_VERTEXDECLARATIONCACHE_H_
