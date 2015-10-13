//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// InputLayoutCache.h: Defines InputLayoutCache, a class that builds and caches
// D3D11 input layouts.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_INPUTLAYOUTCACHE_H_
#define LIBANGLE_RENDERER_D3D_D3D11_INPUTLAYOUTCACHE_H_

#include <GLES2/gl2.h>

#include <cstddef>
#include <map>
#include <unordered_map>

#include "common/angleutils.h"
#include "libANGLE/Constants.h"
#include "libANGLE/Error.h"
#include "libANGLE/formatutils.h"

namespace gl
{
class Program;
}

namespace rx
{
struct TranslatedAttribute;
struct TranslatedIndexData;
struct SourceIndexData;

class InputLayoutCache : angle::NonCopyable
{
  public:
    InputLayoutCache();
    virtual ~InputLayoutCache();

    void initialize(ID3D11Device *device, ID3D11DeviceContext *context);
    void clear();
    void markDirty();

    gl::Error applyVertexBuffers(const std::vector<TranslatedAttribute> &attributes,
                                 GLenum mode, gl::Program *program, SourceIndexData *sourceInfo);

    // Useful for testing
    void setCacheSize(unsigned int cacheSize) { mCacheSize = cacheSize; }

  private:
    struct PackedAttributeLayout
    {
        PackedAttributeLayout()
            : numAttributes(0),
              flags(0)
        {
        }

        void addAttributeData(GLenum glType,
                              UINT semanticIndex,
                              gl::VertexFormatType vertexFormatType,
                              unsigned int divisor);

        bool operator<(const PackedAttributeLayout &other) const;

        enum Flags
        {
            FLAG_USES_INSTANCED_SPRITES = 0x1,
            FLAG_MOVE_FIRST_INDEXED = 0x2,
            FLAG_INSTANCED_SPRITES_ACTIVE = 0x4,
        };

        size_t numAttributes;
        unsigned int flags;
        uint32_t attributeData[gl::MAX_VERTEX_ATTRIBS];
    };

    std::map<PackedAttributeLayout, ID3D11InputLayout *> mLayoutMap;

    ID3D11InputLayout *mCurrentIL;
    ID3D11Buffer *mCurrentBuffers[gl::MAX_VERTEX_ATTRIBS];
    UINT mCurrentVertexStrides[gl::MAX_VERTEX_ATTRIBS];
    UINT mCurrentVertexOffsets[gl::MAX_VERTEX_ATTRIBS];

    ID3D11Buffer *mPointSpriteVertexBuffer;
    ID3D11Buffer *mPointSpriteIndexBuffer;

    unsigned int mCacheSize;
    unsigned long long mCounter;

    ID3D11Device *mDevice;
    ID3D11DeviceContext *mDeviceContext;
    D3D_FEATURE_LEVEL mFeatureLevel;
};

}

#endif // LIBANGLE_RENDERER_D3D_D3D11_INPUTLAYOUTCACHE_H_
