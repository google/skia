//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Buffer11.h: Defines the rx::Buffer11 class which implements rx::BufferImpl via rx::BufferD3D.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_BUFFER11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_BUFFER11_H_

#include <map>

#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/d3d/BufferD3D.h"

namespace rx
{
class Renderer11;
struct SourceIndexData;
struct TranslatedAttribute;

enum BufferUsage
{
    BUFFER_USAGE_STAGING,
    BUFFER_USAGE_VERTEX_OR_TRANSFORM_FEEDBACK,
    BUFFER_USAGE_INDEX,
    BUFFER_USAGE_PIXEL_UNPACK,
    BUFFER_USAGE_PIXEL_PACK,
    BUFFER_USAGE_UNIFORM,
    BUFFER_USAGE_SYSTEM_MEMORY,
    BUFFER_USAGE_EMULATED_INDEXED_VERTEX,

    BUFFER_USAGE_COUNT,
};

struct PackPixelsParams
{
    PackPixelsParams();
    PackPixelsParams(const gl::Rectangle &area, GLenum format, GLenum type, GLuint outputPitch,
                     const gl::PixelPackState &pack, ptrdiff_t offset);

    gl::Rectangle area;
    GLenum format;
    GLenum type;
    GLuint outputPitch;
    gl::Buffer *packBuffer;
    gl::PixelPackState pack;
    ptrdiff_t offset;
};

typedef size_t DataRevision;

class Buffer11 : public BufferD3D
{
  public:
    Buffer11(Renderer11 *renderer);
    virtual ~Buffer11();

    ID3D11Buffer *getBuffer(BufferUsage usage);
    ID3D11Buffer *getEmulatedIndexedBuffer(SourceIndexData *indexInfo, const TranslatedAttribute *attribute);
    ID3D11Buffer *getConstantBufferRange(GLintptr offset, GLsizeiptr size);
    ID3D11ShaderResourceView *getSRV(DXGI_FORMAT srvFormat);
    bool isMapped() const { return mMappedStorage != NULL; }
    gl::Error packPixels(ID3D11Texture2D *srcTexure, UINT srcSubresource, const PackPixelsParams &params);
    size_t getTotalCPUBufferMemoryBytes() const;

    // BufferD3D implementation
    virtual size_t getSize() const { return mSize; }
    virtual bool supportsDirectBinding() const;
    gl::Error getData(const uint8_t **outData) override;

    // BufferImpl implementation
    virtual gl::Error setData(const void* data, size_t size, GLenum usage);
    virtual gl::Error setSubData(const void* data, size_t size, size_t offset);
    virtual gl::Error copySubData(BufferImpl* source, GLintptr sourceOffset, GLintptr destOffset, GLsizeiptr size);
    virtual gl::Error map(GLenum access, GLvoid **mapPtr);
    virtual gl::Error mapRange(size_t offset, size_t length, GLbitfield access, GLvoid **mapPtr);
    virtual gl::Error unmap(GLboolean *result);
    virtual void markTransformFeedbackUsage();

  private:
    class BufferStorage;
    class EmulatedIndexedStorage;
    class NativeStorage;
    class PackStorage;
    class SystemMemoryStorage;

    Renderer11 *mRenderer;
    size_t mSize;

    BufferStorage *mMappedStorage;

    std::vector<BufferStorage*> mBufferStorages;

    struct ConstantBufferCacheEntry
    {
        ConstantBufferCacheEntry() : storage(nullptr), lruCount(0) { }

        BufferStorage *storage;
        unsigned int lruCount;
    };

    // Cache of D3D11 constant buffer for specific ranges of buffer data.
    // This is used to emulate UBO ranges on 11.0 devices.
    // Constant buffers are indexed by there start offset.
    typedef std::map<GLintptr /*offset*/, ConstantBufferCacheEntry> ConstantBufferCache;
    ConstantBufferCache mConstantBufferRangeStoragesCache;
    size_t mConstantBufferStorageAdditionalSize;
    unsigned int mMaxConstantBufferLruCount;

    typedef std::pair<ID3D11Buffer *, ID3D11ShaderResourceView *> BufferSRVPair;
    std::map<DXGI_FORMAT, BufferSRVPair> mBufferResourceViews;

    unsigned int mReadUsageCount;

    void markBufferUsage();
    NativeStorage *getStagingStorage();
    PackStorage *getPackStorage();
    gl::Error getSystemMemoryStorage(SystemMemoryStorage **storageOut);

    void updateBufferStorage(BufferStorage *storage, size_t sourceOffset, size_t storageSize);
    BufferStorage *getBufferStorage(BufferUsage usage);
    BufferStorage *getLatestBufferStorage() const;

    BufferStorage *getConstantBufferRangeStorage(GLintptr offset, GLsizeiptr size);

    void invalidateEmulatedIndexedBuffer();
};

}

#endif // LIBANGLE_RENDERER_D3D_D3D11_BUFFER11_H_
