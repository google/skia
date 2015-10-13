//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Buffer11.cpp Defines the Buffer11 class.

#include "libANGLE/renderer/d3d/d3d11/Buffer11.h"

#include "common/MemoryBuffer.h"
#include "libANGLE/renderer/d3d/IndexDataManager.h"
#include "libANGLE/renderer/d3d/VertexDataManager.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"

namespace
{

template <typename T>
GLuint ReadIndexValueFromIndices(const uint8_t *data, size_t index)
{
    return reinterpret_cast<const T *>(data)[index];
}
typedef GLuint (*ReadIndexValueFunction)(const uint8_t *data, size_t index);
}

namespace rx
{
PackPixelsParams::PackPixelsParams()
    : format(GL_NONE), type(GL_NONE), outputPitch(0), packBuffer(nullptr), offset(0)
{
}

PackPixelsParams::PackPixelsParams(const gl::Rectangle &areaIn,
                                   GLenum formatIn,
                                   GLenum typeIn,
                                   GLuint outputPitchIn,
                                   const gl::PixelPackState &packIn,
                                   ptrdiff_t offsetIn)
    : area(areaIn),
      format(formatIn),
      type(typeIn),
      outputPitch(outputPitchIn),
      packBuffer(packIn.pixelBuffer.get()),
      pack(packIn.alignment, packIn.reverseRowOrder),
      offset(offsetIn)
{
}

namespace gl_d3d11
{

D3D11_MAP GetD3DMapTypeFromBits(GLbitfield access)
{
    bool readBit  = ((access & GL_MAP_READ_BIT) != 0);
    bool writeBit = ((access & GL_MAP_WRITE_BIT) != 0);

    ASSERT(readBit || writeBit);

    // Note : we ignore the discard bit, because in D3D11, staging buffers
    //  don't accept the map-discard flag (discard only works for DYNAMIC usage)

    if (readBit && !writeBit)
    {
        return D3D11_MAP_READ;
    }
    else if (writeBit && !readBit)
    {
        return D3D11_MAP_WRITE;
    }
    else if (writeBit && readBit)
    {
        return D3D11_MAP_READ_WRITE;
    }
    else
    {
        UNREACHABLE();
        return D3D11_MAP_READ;
    }
}
}

// Each instance of Buffer11::BufferStorage is specialized for a class of D3D binding points
// - vertex/transform feedback buffers
// - index buffers
// - pixel unpack buffers
// - uniform buffers
class Buffer11::BufferStorage : angle::NonCopyable
{
  public:
    virtual ~BufferStorage() {}

    DataRevision getDataRevision() const { return mRevision; }
    BufferUsage getUsage() const { return mUsage; }
    size_t getSize() const { return mBufferSize; }
    void setDataRevision(DataRevision rev) { mRevision = rev; }

    virtual bool isMappable() const = 0;

    virtual bool copyFromStorage(BufferStorage *source,
                                 size_t sourceOffset,
                                 size_t size,
                                 size_t destOffset) = 0;
    virtual gl::Error resize(size_t size, bool preserveData) = 0;

    virtual uint8_t *map(size_t offset, size_t length, GLbitfield access) = 0;
    virtual void unmap() = 0;

    gl::Error setData(const uint8_t *data, size_t offset, size_t size);

  protected:
    BufferStorage(Renderer11 *renderer, BufferUsage usage);

    Renderer11 *mRenderer;
    DataRevision mRevision;
    const BufferUsage mUsage;
    size_t mBufferSize;
};

// A native buffer storage represents an underlying D3D11 buffer for a particular
// type of storage.
class Buffer11::NativeStorage : public Buffer11::BufferStorage
{
  public:
    NativeStorage(Renderer11 *renderer, BufferUsage usage);
    ~NativeStorage() override;

    bool isMappable() const override { return mUsage == BUFFER_USAGE_STAGING; }

    ID3D11Buffer *getNativeStorage() const { return mNativeStorage; }
    bool copyFromStorage(BufferStorage *source,
                         size_t sourceOffset,
                         size_t size,
                         size_t destOffset) override;
    gl::Error resize(size_t size, bool preserveData) override;

    uint8_t *map(size_t offset, size_t length, GLbitfield access) override;
    void unmap() override;

  private:
    static void fillBufferDesc(D3D11_BUFFER_DESC *bufferDesc,
                               Renderer11 *renderer,
                               BufferUsage usage,
                               unsigned int bufferSize);

    ID3D11Buffer *mNativeStorage;
};

// A emulated indexed buffer storage represents an underlying D3D11 buffer for data
// that has been expanded to match the indices list used. This storage is only
// used for FL9_3 pointsprite rendering emulation.
class Buffer11::EmulatedIndexedStorage : public Buffer11::BufferStorage
{
  public:
    EmulatedIndexedStorage(Renderer11 *renderer);
    ~EmulatedIndexedStorage() override;

    bool isMappable() const override { return true; }

    ID3D11Buffer *getNativeStorage();

    bool copyFromStorage(BufferStorage *source,
                         size_t sourceOffset,
                         size_t size,
                         size_t destOffset) override;

    gl::Error resize(size_t size, bool preserveData) override;

    uint8_t *map(size_t offset, size_t length, GLbitfield access) override;
    void unmap() override;
    bool update(SourceIndexData *indexInfo, const TranslatedAttribute *attribute);

  private:
    ID3D11Buffer *mNativeStorage;       // contains expanded data for use by D3D
    MemoryBuffer mMemoryBuffer;         // original data (not expanded)
    MemoryBuffer mIndicesMemoryBuffer;  // indices data
    SourceIndexData mIndexInfo;         // indices information
    size_t mAttributeStride;            // per element stride in bytes
    size_t mAttributeOffset;            // starting offset
};

// Pack storage represents internal storage for pack buffers. We implement pack buffers
// as CPU memory, tied to a staging texture, for asynchronous texture readback.
class Buffer11::PackStorage : public Buffer11::BufferStorage
{
  public:
    explicit PackStorage(Renderer11 *renderer);
    ~PackStorage() override;

    bool isMappable() const override { return true; }
    bool copyFromStorage(BufferStorage *source,
                         size_t sourceOffset,
                         size_t size,
                         size_t destOffset) override;
    gl::Error resize(size_t size, bool preserveData) override;

    uint8_t *map(size_t offset, size_t length, GLbitfield access) override;
    void unmap() override;

    gl::Error packPixels(ID3D11Texture2D *srcTexure,
                         UINT srcSubresource,
                         const PackPixelsParams &params);

  private:
    gl::Error flushQueuedPackCommand();

    ID3D11Texture2D *mStagingTexture;
    DXGI_FORMAT mTextureFormat;
    gl::Extents mTextureSize;
    MemoryBuffer mMemoryBuffer;
    PackPixelsParams *mQueuedPackCommand;
    PackPixelsParams mPackParams;
    bool mDataModified;
};

// System memory storage stores a CPU memory buffer with our buffer data.
// For dynamic data, it's much faster to update the CPU memory buffer than
// it is to update a D3D staging buffer and read it back later.
class Buffer11::SystemMemoryStorage : public Buffer11::BufferStorage
{
  public:
    explicit SystemMemoryStorage(Renderer11 *renderer);
    ~SystemMemoryStorage() override {}

    bool isMappable() const override { return true; }
    bool copyFromStorage(BufferStorage *source,
                         size_t sourceOffset,
                         size_t size,
                         size_t destOffset) override;
    gl::Error resize(size_t size, bool preserveData) override;

    uint8_t *map(size_t offset, size_t length, GLbitfield access) override;
    void unmap() override;

    MemoryBuffer *getSystemCopy() { return &mSystemCopy; }

  protected:
    MemoryBuffer mSystemCopy;
};

Buffer11::Buffer11(Renderer11 *renderer)
    : BufferD3D(renderer),
      mRenderer(renderer),
      mSize(0),
      mMappedStorage(nullptr),
      mBufferStorages(BUFFER_USAGE_COUNT, nullptr),
      mConstantBufferStorageAdditionalSize(0),
      mMaxConstantBufferLruCount(0),
      mReadUsageCount(0)
{
}

Buffer11::~Buffer11()
{
    for (auto &storage : mBufferStorages)
    {
        SafeDelete(storage);
    }

    for (auto &p : mConstantBufferRangeStoragesCache)
    {
        SafeDelete(p.second.storage);
    }

    mRenderer->onBufferDelete(this);
}

gl::Error Buffer11::setData(const void *data, size_t size, GLenum usage)
{
    gl::Error error = setSubData(data, size, 0);
    if (error.isError())
    {
        return error;
    }

    updateD3DBufferUsage(usage);
    return error;
}

gl::Error Buffer11::getData(const uint8_t **outData)
{
    SystemMemoryStorage *systemMemoryStorage = nullptr;
    gl::Error error                          = getSystemMemoryStorage(&systemMemoryStorage);

    if (error.isError())
    {
        *outData = nullptr;
        return error;
    }

    mReadUsageCount = 0;

    ASSERT(systemMemoryStorage->getSize() >= mSize);

    *outData = systemMemoryStorage->getSystemCopy()->data();
    return gl::Error(GL_NO_ERROR);
}

gl::Error Buffer11::getSystemMemoryStorage(SystemMemoryStorage **storageOut)
{
    BufferStorage *memStorageUntyped = getBufferStorage(BUFFER_USAGE_SYSTEM_MEMORY);

    if (memStorageUntyped == nullptr)
    {
        // TODO(jmadill): convert all to errors
        return gl::Error(GL_OUT_OF_MEMORY);
    }

    *storageOut = GetAs<SystemMemoryStorage>(memStorageUntyped);
    return gl::Error(GL_NO_ERROR);
}

gl::Error Buffer11::setSubData(const void *data, size_t size, size_t offset)
{
    size_t requiredSize = size + offset;

    if (data && size > 0)
    {
        // Use system memory storage for dynamic buffers.

        BufferStorage *writeBuffer = nullptr;
        if (supportsDirectBinding())
        {
            writeBuffer = getStagingStorage();

            if (!writeBuffer)
            {
                return gl::Error(GL_OUT_OF_MEMORY, "Failed to allocate internal buffer.");
            }
        }
        else
        {
            SystemMemoryStorage *systemMemoryStorage = nullptr;
            gl::Error error = getSystemMemoryStorage(&systemMemoryStorage);
            if (error.isError())
            {
                return error;
            }

            writeBuffer = systemMemoryStorage;
        }

        ASSERT(writeBuffer);

        // Explicitly resize the staging buffer, preserving data if the new data will not
        // completely fill the buffer
        if (writeBuffer->getSize() < requiredSize)
        {
            bool preserveData = (offset > 0);
            gl::Error error = writeBuffer->resize(requiredSize, preserveData);
            if (error.isError())
            {
                return error;
            }
        }

        writeBuffer->setData(static_cast<const uint8_t *>(data), offset, size);
        writeBuffer->setDataRevision(writeBuffer->getDataRevision() + 1);
    }

    mSize = std::max(mSize, requiredSize);
    invalidateStaticData();

    return gl::Error(GL_NO_ERROR);
}

gl::Error Buffer11::copySubData(BufferImpl *source,
                                GLintptr sourceOffset,
                                GLintptr destOffset,
                                GLsizeiptr size)
{
    Buffer11 *sourceBuffer = GetAs<Buffer11>(source);
    ASSERT(sourceBuffer != nullptr);

    BufferStorage *copyDest = getLatestBufferStorage();
    if (!copyDest)
    {
        copyDest = getStagingStorage();
    }

    BufferStorage *copySource = sourceBuffer->getLatestBufferStorage();

    if (!copySource || !copyDest)
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to allocate internal staging buffer.");
    }

    // If copying to/from a pixel pack buffer, we must have a staging or
    // pack buffer partner, because other native buffers can't be mapped
    if (copyDest->getUsage() == BUFFER_USAGE_PIXEL_PACK && !copySource->isMappable())
    {
        copySource = sourceBuffer->getStagingStorage();
    }
    else if (copySource->getUsage() == BUFFER_USAGE_PIXEL_PACK && !copyDest->isMappable())
    {
        copyDest = getStagingStorage();
    }

    // D3D11 does not allow overlapped copies until 11.1, and only if the
    // device supports D3D11_FEATURE_DATA_D3D11_OPTIONS::CopyWithOverlap
    // Get around this via a different source buffer
    if (copySource == copyDest)
    {
        if (copySource->getUsage() == BUFFER_USAGE_STAGING)
        {
            copySource = getBufferStorage(BUFFER_USAGE_VERTEX_OR_TRANSFORM_FEEDBACK);
        }
        else
        {
            copySource = getStagingStorage();
        }
    }

    copyDest->copyFromStorage(copySource, sourceOffset, size, destOffset);
    copyDest->setDataRevision(copyDest->getDataRevision() + 1);

    mSize = std::max<size_t>(mSize, destOffset + size);
    invalidateStaticData();

    return gl::Error(GL_NO_ERROR);
}

gl::Error Buffer11::map(GLenum access, GLvoid **mapPtr)
{
    // GL_OES_mapbuffer uses an enum instead of a bitfield for it's access, convert to a bitfield
    // and call mapRange.
    ASSERT(access == GL_WRITE_ONLY_OES);
    return mapRange(0, mSize, GL_MAP_WRITE_BIT, mapPtr);
}

gl::Error Buffer11::mapRange(size_t offset, size_t length, GLbitfield access, GLvoid **mapPtr)
{
    ASSERT(!mMappedStorage);

    BufferStorage *latestStorage = getLatestBufferStorage();
    if (latestStorage && (latestStorage->getUsage() == BUFFER_USAGE_PIXEL_PACK ||
                          latestStorage->getUsage() == BUFFER_USAGE_STAGING))
    {
        // Latest storage is mappable.
        mMappedStorage = latestStorage;
    }
    else
    {
        // Fall back to using the staging buffer if the latest storage does
        // not exist or is not CPU-accessible.
        mMappedStorage = getStagingStorage();
    }

    if (!mMappedStorage)
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to allocate mappable internal buffer.");
    }

    if ((access & GL_MAP_WRITE_BIT) > 0)
    {
        // Update the data revision immediately, since the data might be changed at any time
        mMappedStorage->setDataRevision(mMappedStorage->getDataRevision() + 1);
        invalidateStaticData();
    }

    uint8_t *mappedBuffer = mMappedStorage->map(offset, length, access);
    if (!mappedBuffer)
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to map internal buffer.");
    }

    *mapPtr = static_cast<GLvoid *>(mappedBuffer);
    return gl::Error(GL_NO_ERROR);
}

gl::Error Buffer11::unmap(GLboolean *result)
{
    ASSERT(mMappedStorage);
    mMappedStorage->unmap();
    mMappedStorage = nullptr;

    // TODO: detect if we had corruption. if so, return false.
    *result = GL_TRUE;

    return gl::Error(GL_NO_ERROR);
}

void Buffer11::markTransformFeedbackUsage()
{
    BufferStorage *transformFeedbackStorage =
        getBufferStorage(BUFFER_USAGE_VERTEX_OR_TRANSFORM_FEEDBACK);

    if (transformFeedbackStorage)
    {
        transformFeedbackStorage->setDataRevision(transformFeedbackStorage->getDataRevision() + 1);
    }

    invalidateStaticData();
}

void Buffer11::markBufferUsage()
{
    mReadUsageCount++;

    // Free the system memory storage if we decide it isn't being used very often.
    const unsigned int usageLimit = 5;

    BufferStorage *&sysMemUsage = mBufferStorages[BUFFER_USAGE_SYSTEM_MEMORY];
    if (mReadUsageCount > usageLimit && sysMemUsage != nullptr)
    {
        if (getLatestBufferStorage() != sysMemUsage)
        {
            SafeDelete(sysMemUsage);
        }
    }
}

ID3D11Buffer *Buffer11::getBuffer(BufferUsage usage)
{
    markBufferUsage();

    BufferStorage *bufferStorage = getBufferStorage(usage);

    if (!bufferStorage)
    {
        // Storage out-of-memory
        return nullptr;
    }

    return GetAs<NativeStorage>(bufferStorage)->getNativeStorage();
}

ID3D11Buffer *Buffer11::getEmulatedIndexedBuffer(SourceIndexData *indexInfo,
                                                 const TranslatedAttribute *attribute)
{
    markBufferUsage();

    assert(indexInfo != nullptr);
    assert(attribute != nullptr);

    BufferStorage *bufferStorage = getBufferStorage(BUFFER_USAGE_EMULATED_INDEXED_VERTEX);
    if (!bufferStorage)
    {
        // Storage out-of-memory
        return nullptr;
    }

    EmulatedIndexedStorage *emulatedStorage = GetAs<EmulatedIndexedStorage>(bufferStorage);
    if (!emulatedStorage->update(indexInfo, attribute))
    {
        // Storage out-of-memory
        return nullptr;
    }

    return emulatedStorage->getNativeStorage();
}

ID3D11Buffer *Buffer11::getConstantBufferRange(GLintptr offset, GLsizeiptr size)
{
    markBufferUsage();

    BufferStorage *bufferStorage;

    if (offset == 0)
    {
        bufferStorage = getBufferStorage(BUFFER_USAGE_UNIFORM);
    }
    else
    {
        bufferStorage = getConstantBufferRangeStorage(offset, size);
    }

    if (!bufferStorage)
    {
        // Storage out-of-memory
        return nullptr;
    }

    return GetAs<NativeStorage>(bufferStorage)->getNativeStorage();
}

ID3D11ShaderResourceView *Buffer11::getSRV(DXGI_FORMAT srvFormat)
{
    BufferStorage *storage = getBufferStorage(BUFFER_USAGE_PIXEL_UNPACK);

    if (!storage)
    {
        // Storage out-of-memory
        return nullptr;
    }

    ID3D11Buffer *buffer = GetAs<NativeStorage>(storage)->getNativeStorage();

    auto bufferSRVIt = mBufferResourceViews.find(srvFormat);

    if (bufferSRVIt != mBufferResourceViews.end())
    {
        if (bufferSRVIt->second.first == buffer)
        {
            return bufferSRVIt->second.second;
        }
        else
        {
            // The underlying buffer has changed since the SRV was created: recreate the SRV.
            SafeRelease(bufferSRVIt->second.second);
        }
    }

    ID3D11Device *device                = mRenderer->getDevice();
    ID3D11ShaderResourceView *bufferSRV = nullptr;

    const d3d11::DXGIFormat &dxgiFormatInfo = d3d11::GetDXGIFormatInfo(srvFormat);

    D3D11_SHADER_RESOURCE_VIEW_DESC bufferSRVDesc;
    bufferSRVDesc.Buffer.ElementOffset = 0;
    bufferSRVDesc.Buffer.ElementWidth =
        static_cast<unsigned int>(mSize) / dxgiFormatInfo.pixelBytes;
    bufferSRVDesc.ViewDimension        = D3D11_SRV_DIMENSION_BUFFER;
    bufferSRVDesc.Format               = srvFormat;

    HRESULT result = device->CreateShaderResourceView(buffer, &bufferSRVDesc, &bufferSRV);
    UNUSED_ASSERTION_VARIABLE(result);
    ASSERT(SUCCEEDED(result));

    mBufferResourceViews[srvFormat] = BufferSRVPair(buffer, bufferSRV);

    return bufferSRV;
}

gl::Error Buffer11::packPixels(ID3D11Texture2D *srcTexture,
                               UINT srcSubresource,
                               const PackPixelsParams &params)
{
    PackStorage *packStorage     = getPackStorage();
    BufferStorage *latestStorage = getLatestBufferStorage();

    if (packStorage)
    {
        gl::Error error = packStorage->packPixels(srcTexture, srcSubresource, params);
        if (error.isError())
        {
            return error;
        }
        packStorage->setDataRevision(latestStorage ? latestStorage->getDataRevision() + 1 : 1);
    }

    return gl::Error(GL_NO_ERROR);
}

size_t Buffer11::getTotalCPUBufferMemoryBytes() const
{
    size_t allocationSize = 0;

    BufferStorage *staging = mBufferStorages[BUFFER_USAGE_STAGING];
    allocationSize += staging ? staging->getSize() : 0;

    BufferStorage *sysMem = mBufferStorages[BUFFER_USAGE_SYSTEM_MEMORY];
    allocationSize += sysMem ? sysMem->getSize() : 0;

    return allocationSize;
}

Buffer11::BufferStorage *Buffer11::getBufferStorage(BufferUsage usage)
{
    ASSERT(0 <= usage && usage < BUFFER_USAGE_COUNT);
    BufferStorage *&newStorage = mBufferStorages[usage];

    if (!newStorage)
    {
        if (usage == BUFFER_USAGE_PIXEL_PACK)
        {
            newStorage = new PackStorage(mRenderer);
        }
        else if (usage == BUFFER_USAGE_SYSTEM_MEMORY)
        {
            newStorage = new SystemMemoryStorage(mRenderer);
        }
        else if (usage == BUFFER_USAGE_EMULATED_INDEXED_VERTEX)
        {
            newStorage = new EmulatedIndexedStorage(mRenderer);
        }
        else
        {
            // buffer is not allocated, create it
            newStorage = new NativeStorage(mRenderer, usage);
        }
    }

    // resize buffer
    if (newStorage->getSize() < mSize)
    {
        if (newStorage->resize(mSize, true).isError())
        {
            // Out of memory error
            return nullptr;
        }
    }

    updateBufferStorage(newStorage, 0, mSize);

    return newStorage;
}

Buffer11::BufferStorage *Buffer11::getConstantBufferRangeStorage(GLintptr offset, GLsizeiptr size)
{
    BufferStorage *newStorage;

    {
        // Keep the cacheEntry in a limited scope because it may be invalidated later in the code if
        // we need to reclaim some space.
        ConstantBufferCacheEntry *cacheEntry = &mConstantBufferRangeStoragesCache[offset];

        if (!cacheEntry->storage)
        {
            cacheEntry->storage  = new NativeStorage(mRenderer, BUFFER_USAGE_UNIFORM);
            cacheEntry->lruCount = ++mMaxConstantBufferLruCount;
        }

        cacheEntry->lruCount = ++mMaxConstantBufferLruCount;
        newStorage           = cacheEntry->storage;
    }

    if (newStorage->getSize() < static_cast<size_t>(size))
    {
        size_t maximumAllowedAdditionalSize = 2 * getSize();

        size_t sizeDelta = size - newStorage->getSize();

        while (mConstantBufferStorageAdditionalSize + sizeDelta > maximumAllowedAdditionalSize)
        {
            auto iter = std::min_element(std::begin(mConstantBufferRangeStoragesCache),
                                         std::end(mConstantBufferRangeStoragesCache),
                                         [](const ConstantBufferCache::value_type &a,
                                            const ConstantBufferCache::value_type &b)
                                         {
                                             return a.second.lruCount < b.second.lruCount;
                                         });

            ASSERT(iter->second.storage != newStorage);
            ASSERT(mConstantBufferStorageAdditionalSize >= iter->second.storage->getSize());

            mConstantBufferStorageAdditionalSize -= iter->second.storage->getSize();
            SafeDelete(iter->second.storage);
            mConstantBufferRangeStoragesCache.erase(iter);
        }

        if (newStorage->resize(size, false).isError())
        {
            // Out of memory error
            return nullptr;
        }

        mConstantBufferStorageAdditionalSize += sizeDelta;

        // We don't copy the old data when resizing the constant buffer because the data may be
        // out-of-date therefore we reset the data revision and let updateBufferStorage() handle the
        // copy.
        newStorage->setDataRevision(0);
    }

    updateBufferStorage(newStorage, offset, size);

    return newStorage;
}

void Buffer11::updateBufferStorage(BufferStorage *storage, size_t sourceOffset, size_t storageSize)
{
    BufferStorage *latestBuffer = getLatestBufferStorage();
    if (latestBuffer && latestBuffer->getDataRevision() > storage->getDataRevision())
    {
        // Copy through a staging buffer if we're copying from or to a non-staging, mappable
        // buffer storage. This is because we can't map a GPU buffer, and copy CPU
        // data directly. If we're already using a staging buffer we're fine.
        if (latestBuffer->getUsage() != BUFFER_USAGE_STAGING &&
            storage->getUsage() != BUFFER_USAGE_STAGING &&
            (!latestBuffer->isMappable() || !storage->isMappable()))
        {
            NativeStorage *stagingBuffer = getStagingStorage();

            stagingBuffer->copyFromStorage(latestBuffer, 0, latestBuffer->getSize(), 0);
            stagingBuffer->setDataRevision(latestBuffer->getDataRevision());

            latestBuffer = stagingBuffer;
        }

        // if copyFromStorage returns true, the D3D buffer has been recreated
        // and we should update our serial
        if (storage->copyFromStorage(latestBuffer, sourceOffset, storageSize, 0))
        {
            updateSerial();
        }
        storage->setDataRevision(latestBuffer->getDataRevision());
    }
}

Buffer11::BufferStorage *Buffer11::getLatestBufferStorage() const
{
    // Even though we iterate over all the direct buffers, it is expected that only
    // 1 or 2 will be present.
    BufferStorage *latestStorage = nullptr;
    DataRevision latestRevision = 0;
    for (auto &storage : mBufferStorages)
    {
        if (storage && (!latestStorage || storage->getDataRevision() > latestRevision))
        {
            latestStorage  = storage;
            latestRevision = storage->getDataRevision();
        }
    }

    // resize buffer
    if (latestStorage && latestStorage->getSize() < mSize)
    {
        if (latestStorage->resize(mSize, true).isError())
        {
            // Out of memory error
            return nullptr;
        }
    }

    return latestStorage;
}

Buffer11::NativeStorage *Buffer11::getStagingStorage()
{
    BufferStorage *stagingStorage = getBufferStorage(BUFFER_USAGE_STAGING);

    if (!stagingStorage)
    {
        // Out-of-memory
        return nullptr;
    }

    return GetAs<NativeStorage>(stagingStorage);
}

Buffer11::PackStorage *Buffer11::getPackStorage()
{
    BufferStorage *packStorage = getBufferStorage(BUFFER_USAGE_PIXEL_PACK);

    if (!packStorage)
    {
        // Out-of-memory
        return nullptr;
    }

    return GetAs<PackStorage>(packStorage);
}

bool Buffer11::supportsDirectBinding() const
{
    // Do not support direct buffers for dynamic data. The streaming buffer
    // offers better performance for data which changes every frame.
    // Check for absence of static buffer interfaces to detect dynamic data.
    return (mStaticVertexBuffer && mStaticIndexBuffer);
}

Buffer11::BufferStorage::BufferStorage(Renderer11 *renderer, BufferUsage usage)
    : mRenderer(renderer), mRevision(0), mUsage(usage), mBufferSize(0)
{
}

gl::Error Buffer11::BufferStorage::setData(const uint8_t *data, size_t offset, size_t size)
{
    ASSERT(isMappable());

    uint8_t *writePointer = map(offset, size, GL_MAP_WRITE_BIT);
    if (!writePointer)
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to map internal buffer.");
    }

    memcpy(writePointer, data, size);

    unmap();

    return gl::Error(GL_NO_ERROR);
}

Buffer11::NativeStorage::NativeStorage(Renderer11 *renderer, BufferUsage usage)
    : BufferStorage(renderer, usage), mNativeStorage(nullptr)
{
}

Buffer11::NativeStorage::~NativeStorage()
{
    SafeRelease(mNativeStorage);
}

// Returns true if it recreates the direct buffer
bool Buffer11::NativeStorage::copyFromStorage(BufferStorage *source,
                                              size_t sourceOffset,
                                              size_t size,
                                              size_t destOffset)
{
    ID3D11DeviceContext *context = mRenderer->getDeviceContext();

    size_t requiredSize = destOffset + size;
    bool createBuffer   = !mNativeStorage || mBufferSize < requiredSize;

    // (Re)initialize D3D buffer if needed
    if (createBuffer)
    {
        bool preserveData = (destOffset > 0);
        resize(requiredSize, preserveData);
    }

    if (source->getUsage() == BUFFER_USAGE_PIXEL_PACK ||
        source->getUsage() == BUFFER_USAGE_SYSTEM_MEMORY)
    {
        ASSERT(source->isMappable());

        uint8_t *sourcePointer = source->map(sourceOffset, size, GL_MAP_READ_BIT);

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        HRESULT hr = context->Map(mNativeStorage, 0, D3D11_MAP_WRITE, 0, &mappedResource);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
        {
            source->unmap();
            return false;
        }

        uint8_t *destPointer = static_cast<uint8_t *>(mappedResource.pData) + destOffset;

        // Offset bounds are validated at the API layer
        ASSERT(sourceOffset + size <= destOffset + mBufferSize);
        memcpy(destPointer, sourcePointer, size);

        context->Unmap(mNativeStorage, 0);
        source->unmap();
    }
    else
    {
        D3D11_BOX srcBox;
        srcBox.left   = static_cast<unsigned int>(sourceOffset);
        srcBox.right  = static_cast<unsigned int>(sourceOffset + size);
        srcBox.top    = 0;
        srcBox.bottom = 1;
        srcBox.front  = 0;
        srcBox.back   = 1;

        ID3D11Buffer *sourceBuffer = GetAs<NativeStorage>(source)->getNativeStorage();

        context->CopySubresourceRegion(mNativeStorage, 0, static_cast<unsigned int>(destOffset), 0,
                                       0, sourceBuffer, 0, &srcBox);
    }

    return createBuffer;
}

gl::Error Buffer11::NativeStorage::resize(size_t size, bool preserveData)
{
    ID3D11Device *device         = mRenderer->getDevice();
    ID3D11DeviceContext *context = mRenderer->getDeviceContext();

    D3D11_BUFFER_DESC bufferDesc;
    fillBufferDesc(&bufferDesc, mRenderer, mUsage, static_cast<unsigned int>(size));

    ID3D11Buffer *newBuffer;
    HRESULT result = device->CreateBuffer(&bufferDesc, nullptr, &newBuffer);

    if (FAILED(result))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal buffer, result: 0x%X.",
                         result);
    }

    d3d11::SetDebugName(newBuffer, "Buffer11::NativeStorage");

    if (mNativeStorage && preserveData)
    {
        // We don't call resize if the buffer is big enough already.
        ASSERT(mBufferSize <= size);

        D3D11_BOX srcBox;
        srcBox.left   = 0;
        srcBox.right  = static_cast<unsigned int>(mBufferSize);
        srcBox.top    = 0;
        srcBox.bottom = 1;
        srcBox.front  = 0;
        srcBox.back   = 1;

        context->CopySubresourceRegion(newBuffer, 0, 0, 0, 0, mNativeStorage, 0, &srcBox);
    }

    // No longer need the old buffer
    SafeRelease(mNativeStorage);
    mNativeStorage = newBuffer;

    mBufferSize = bufferDesc.ByteWidth;

    return gl::Error(GL_NO_ERROR);
}

void Buffer11::NativeStorage::fillBufferDesc(D3D11_BUFFER_DESC *bufferDesc,
                                             Renderer11 *renderer,
                                             BufferUsage usage,
                                             unsigned int bufferSize)
{
    bufferDesc->ByteWidth           = bufferSize;
    bufferDesc->MiscFlags           = 0;
    bufferDesc->StructureByteStride = 0;

    switch (usage)
    {
        case BUFFER_USAGE_STAGING:
            bufferDesc->Usage          = D3D11_USAGE_STAGING;
            bufferDesc->BindFlags      = 0;
            bufferDesc->CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
            break;

        case BUFFER_USAGE_VERTEX_OR_TRANSFORM_FEEDBACK:
            bufferDesc->Usage     = D3D11_USAGE_DEFAULT;
            bufferDesc->BindFlags = D3D11_BIND_VERTEX_BUFFER;

            if (renderer->isES3Capable())
            {
                bufferDesc->BindFlags |= D3D11_BIND_STREAM_OUTPUT;
            }

            bufferDesc->CPUAccessFlags = 0;
            break;

        case BUFFER_USAGE_INDEX:
            bufferDesc->Usage          = D3D11_USAGE_DEFAULT;
            bufferDesc->BindFlags      = D3D11_BIND_INDEX_BUFFER;
            bufferDesc->CPUAccessFlags = 0;
            break;

        case BUFFER_USAGE_PIXEL_UNPACK:
            bufferDesc->Usage          = D3D11_USAGE_DEFAULT;
            bufferDesc->BindFlags      = D3D11_BIND_SHADER_RESOURCE;
            bufferDesc->CPUAccessFlags = 0;
            break;

        case BUFFER_USAGE_UNIFORM:
            bufferDesc->Usage          = D3D11_USAGE_DYNAMIC;
            bufferDesc->BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
            bufferDesc->CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            // Constant buffers must be of a limited size, and aligned to 16 byte boundaries
            // For our purposes we ignore any buffer data past the maximum constant buffer size
            bufferDesc->ByteWidth = roundUp(bufferDesc->ByteWidth, 16u);
            bufferDesc->ByteWidth =
                std::min<UINT>(bufferDesc->ByteWidth,
                               static_cast<UINT>(renderer->getRendererCaps().maxUniformBlockSize));
            break;

        default:
            UNREACHABLE();
    }
}

uint8_t *Buffer11::NativeStorage::map(size_t offset, size_t length, GLbitfield access)
{
    ASSERT(mUsage == BUFFER_USAGE_STAGING);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ID3D11DeviceContext *context = mRenderer->getDeviceContext();
    D3D11_MAP d3dMapType         = gl_d3d11::GetD3DMapTypeFromBits(access);
    UINT d3dMapFlag              = ((access & GL_MAP_UNSYNCHRONIZED_BIT) != 0 ? D3D11_MAP_FLAG_DO_NOT_WAIT : 0);

    HRESULT result = context->Map(mNativeStorage, 0, d3dMapType, d3dMapFlag, &mappedResource);
    ASSERT(SUCCEEDED(result));
    if (FAILED(result))
    {
        return nullptr;
    }
    return static_cast<uint8_t *>(mappedResource.pData) + offset;
}

void Buffer11::NativeStorage::unmap()
{
    ASSERT(mUsage == BUFFER_USAGE_STAGING);
    ID3D11DeviceContext *context = mRenderer->getDeviceContext();
    context->Unmap(mNativeStorage, 0);
}

Buffer11::EmulatedIndexedStorage::EmulatedIndexedStorage(Renderer11 *renderer)
    : BufferStorage(renderer, BUFFER_USAGE_EMULATED_INDEXED_VERTEX), mNativeStorage(nullptr)
{
}

Buffer11::EmulatedIndexedStorage::~EmulatedIndexedStorage()
{
    SafeRelease(mNativeStorage);
}

ID3D11Buffer *Buffer11::EmulatedIndexedStorage::getNativeStorage()
{
    if (!mNativeStorage)
    {
        // Expand the memory storage upon request and cache the results.
        unsigned int expandedDataSize =
            static_cast<unsigned int>((mIndexInfo.srcCount * mAttributeStride) + mAttributeOffset);
        MemoryBuffer expandedData;
        if (!expandedData.resize(expandedDataSize))
        {
            return nullptr;
        }

        // Clear the contents of the allocated buffer
        ZeroMemory(expandedData.data(), expandedDataSize);

        uint8_t *curr      = expandedData.data();
        const uint8_t *ptr = static_cast<const uint8_t *>(mIndexInfo.srcIndices);

        // Ensure that we start in the correct place for the emulated data copy operation to
        // maintain offset behaviors.
        curr += mAttributeOffset;

        ReadIndexValueFunction readIndexValue = ReadIndexValueFromIndices<GLushort>;

        switch (mIndexInfo.srcIndexType)
        {
            case GL_UNSIGNED_INT:
                readIndexValue = ReadIndexValueFromIndices<GLuint>;
                break;
            case GL_UNSIGNED_SHORT:
                readIndexValue = ReadIndexValueFromIndices<GLushort>;
                break;
            case GL_UNSIGNED_BYTE:
                readIndexValue = ReadIndexValueFromIndices<GLubyte>;
                break;
        }

        // Iterate over the cached index data and copy entries indicated into the emulated buffer.
        for (GLuint i = 0; i < mIndexInfo.srcCount; i++)
        {
            GLuint idx = readIndexValue(ptr, i);
            memcpy(curr, mMemoryBuffer.data() + (mAttributeStride * idx), mAttributeStride);
            curr += mAttributeStride;
        }

        // Finally, initialize the emulated indexed native storage object with the newly copied data
        // and free the temporary buffers used.
        ID3D11Device *device = mRenderer->getDevice();

        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.ByteWidth           = expandedDataSize;
        bufferDesc.MiscFlags           = 0;
        bufferDesc.StructureByteStride = 0;
        bufferDesc.Usage               = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags      = 0;

        D3D11_SUBRESOURCE_DATA subResourceData = {expandedData.data(), 0, 0};

        HRESULT result = device->CreateBuffer(&bufferDesc, &subResourceData, &mNativeStorage);
        if (FAILED(result))
        {
            ERR("Could not create emulated index data buffer: %08lX", result);
            return nullptr;
        }
        d3d11::SetDebugName(mNativeStorage, "Buffer11::EmulatedIndexedStorage");
    }

    return mNativeStorage;
}

bool Buffer11::EmulatedIndexedStorage::update(SourceIndexData *indexInfo,
                                              const TranslatedAttribute *attribute)
{
    // If a change in the indices applied from the last draw call is detected, then the emulated
    // indexed buffer needs to be invalidated.  After invalidation, the change detected flag should
    // be cleared to avoid unnecessary recreation of the buffer.
    if (mNativeStorage == nullptr || indexInfo->srcIndicesChanged)
    {
        SafeRelease(mNativeStorage);

        // Copy attribute offset and stride information
        mAttributeStride = attribute->stride;
        mAttributeOffset = attribute->offset;

        // Copy the source index data. This ensures that the lifetime of the indices pointer
        // stays with this storage until the next time we invalidate.
        size_t indicesDataSize = 0;
        switch (indexInfo->srcIndexType)
        {
            case GL_UNSIGNED_INT:
                indicesDataSize = sizeof(GLuint) * indexInfo->srcCount;
                break;
            case GL_UNSIGNED_SHORT:
                indicesDataSize = sizeof(GLushort) * indexInfo->srcCount;
                break;
            case GL_UNSIGNED_BYTE:
                indicesDataSize = sizeof(GLubyte) * indexInfo->srcCount;
                break;
            default:
                indicesDataSize = sizeof(GLushort) * indexInfo->srcCount;
                break;
        }

        if (!mIndicesMemoryBuffer.resize(indicesDataSize))
        {
            return false;
        }

        memcpy(mIndicesMemoryBuffer.data(), indexInfo->srcIndices, indicesDataSize);

        // Copy the source index data description and update the srcIndices pointer to point
        // to our cached index data.
        mIndexInfo            = *indexInfo;
        mIndexInfo.srcIndices = mIndicesMemoryBuffer.data();

        indexInfo->srcIndicesChanged = false;
    }
    return true;
}

bool Buffer11::EmulatedIndexedStorage::copyFromStorage(BufferStorage *source,
                                                       size_t sourceOffset,
                                                       size_t size,
                                                       size_t destOffset)
{
    ASSERT(source->isMappable());
    const uint8_t *sourceData = source->map(sourceOffset, size, GL_MAP_READ_BIT);
    ASSERT(destOffset + size <= mMemoryBuffer.size());
    memcpy(mMemoryBuffer.data() + destOffset, sourceData, size);
    source->unmap();
    return true;
}

gl::Error Buffer11::EmulatedIndexedStorage::resize(size_t size, bool preserveData)
{
    if (mMemoryBuffer.size() < size)
    {
        if (!mMemoryBuffer.resize(size))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to resize EmulatedIndexedStorage");
        }
        mBufferSize = size;
    }

    return gl::Error(GL_NO_ERROR);
}

uint8_t *Buffer11::EmulatedIndexedStorage::map(size_t offset, size_t length, GLbitfield access)
{
    ASSERT(!mMemoryBuffer.empty() && offset + length <= mMemoryBuffer.size());
    return mMemoryBuffer.data() + offset;
}

void Buffer11::EmulatedIndexedStorage::unmap()
{
    // No-op
}

Buffer11::PackStorage::PackStorage(Renderer11 *renderer)
    : BufferStorage(renderer, BUFFER_USAGE_PIXEL_PACK),
      mStagingTexture(nullptr),
      mTextureFormat(DXGI_FORMAT_UNKNOWN),
      mQueuedPackCommand(nullptr),
      mDataModified(false)
{
}

Buffer11::PackStorage::~PackStorage()
{
    SafeRelease(mStagingTexture);
    SafeDelete(mQueuedPackCommand);
}

bool Buffer11::PackStorage::copyFromStorage(BufferStorage *source,
                                            size_t sourceOffset,
                                            size_t size,
                                            size_t destOffset)
{
    // We copy through a staging buffer when drawing with a pack buffer,
    // or for other cases where we access the pack buffer
    UNREACHABLE();
    return false;
}

gl::Error Buffer11::PackStorage::resize(size_t size, bool preserveData)
{
    if (size != mBufferSize)
    {
        if (!mMemoryBuffer.resize(size))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to resize internal buffer storage.");
        }
        mBufferSize = size;
    }

    return gl::Error(GL_NO_ERROR);
}

uint8_t *Buffer11::PackStorage::map(size_t offset, size_t length, GLbitfield access)
{
    ASSERT(offset + length <= getSize());
    // TODO: fast path
    //  We might be able to optimize out one or more memcpy calls by detecting when
    //  and if D3D packs the staging texture memory identically to how we would fill
    //  the pack buffer according to the current pack state.

    gl::Error error = flushQueuedPackCommand();
    if (error.isError())
    {
        return nullptr;
    }

    mDataModified = (mDataModified || (access & GL_MAP_WRITE_BIT) != 0);

    return mMemoryBuffer.data() + offset;
}

void Buffer11::PackStorage::unmap()
{
    // No-op
}

gl::Error Buffer11::PackStorage::packPixels(ID3D11Texture2D *srcTexure,
                                            UINT srcSubresource,
                                            const PackPixelsParams &params)
{
    gl::Error error = flushQueuedPackCommand();
    if (error.isError())
    {
        return error;
    }

    mQueuedPackCommand = new PackPixelsParams(params);

    D3D11_TEXTURE2D_DESC textureDesc;
    srcTexure->GetDesc(&textureDesc);

    if (mStagingTexture != nullptr &&
        (mTextureFormat != textureDesc.Format || mTextureSize.width != params.area.width ||
         mTextureSize.height != params.area.height))
    {
        SafeRelease(mStagingTexture);
        mTextureSize.width  = 0;
        mTextureSize.height = 0;
        mTextureFormat      = DXGI_FORMAT_UNKNOWN;
    }

    if (mStagingTexture == nullptr)
    {
        ID3D11Device *device = mRenderer->getDevice();
        HRESULT hr;

        mTextureSize.width  = params.area.width;
        mTextureSize.height = params.area.height;
        mTextureFormat      = textureDesc.Format;

        D3D11_TEXTURE2D_DESC stagingDesc;
        stagingDesc.Width              = params.area.width;
        stagingDesc.Height             = params.area.height;
        stagingDesc.MipLevels          = 1;
        stagingDesc.ArraySize          = 1;
        stagingDesc.Format             = mTextureFormat;
        stagingDesc.SampleDesc.Count   = 1;
        stagingDesc.SampleDesc.Quality = 0;
        stagingDesc.Usage              = D3D11_USAGE_STAGING;
        stagingDesc.BindFlags          = 0;
        stagingDesc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ;
        stagingDesc.MiscFlags          = 0;

        hr = device->CreateTexture2D(&stagingDesc, nullptr, &mStagingTexture);
        if (FAILED(hr))
        {
            ASSERT(hr == E_OUTOFMEMORY);
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to allocate internal staging texture.");
        }
    }

    // ReadPixels from multisampled FBOs isn't supported in current GL
    ASSERT(textureDesc.SampleDesc.Count <= 1);

    ID3D11DeviceContext *immediateContext = mRenderer->getDeviceContext();
    D3D11_BOX srcBox;
    srcBox.left   = params.area.x;
    srcBox.right  = params.area.x + params.area.width;
    srcBox.top    = params.area.y;
    srcBox.bottom = params.area.y + params.area.height;
    srcBox.front  = 0;
    srcBox.back   = 1;

    // Asynchronous copy
    immediateContext->CopySubresourceRegion(mStagingTexture, 0, 0, 0, 0, srcTexure, srcSubresource,
                                            &srcBox);

    return gl::Error(GL_NO_ERROR);
}

gl::Error Buffer11::PackStorage::flushQueuedPackCommand()
{
    ASSERT(mMemoryBuffer.size() > 0);

    if (mQueuedPackCommand)
    {
        gl::Error error =
            mRenderer->packPixels(mStagingTexture, *mQueuedPackCommand, mMemoryBuffer.data());
        SafeDelete(mQueuedPackCommand);
        if (error.isError())
        {
            return error;
        }
    }

    return gl::Error(GL_NO_ERROR);
}

Buffer11::SystemMemoryStorage::SystemMemoryStorage(Renderer11 *renderer)
    : Buffer11::BufferStorage(renderer, BUFFER_USAGE_SYSTEM_MEMORY)
{
}

bool Buffer11::SystemMemoryStorage::copyFromStorage(BufferStorage *source,
                                                    size_t sourceOffset,
                                                    size_t size,
                                                    size_t destOffset)
{
    ASSERT(source->isMappable());
    const uint8_t *sourceData = source->map(sourceOffset, size, GL_MAP_READ_BIT);
    ASSERT(destOffset + size <= mSystemCopy.size());
    memcpy(mSystemCopy.data() + destOffset, sourceData, size);
    source->unmap();
    return true;
}

gl::Error Buffer11::SystemMemoryStorage::resize(size_t size, bool preserveData)
{
    if (mSystemCopy.size() < size)
    {
        if (!mSystemCopy.resize(size))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to resize SystemMemoryStorage");
        }
        mBufferSize = size;
    }

    return gl::Error(GL_NO_ERROR);
}

uint8_t *Buffer11::SystemMemoryStorage::map(size_t offset, size_t length, GLbitfield access)
{
    ASSERT(!mSystemCopy.empty() && offset + length <= mSystemCopy.size());
    return mSystemCopy.data() + offset;
}

void Buffer11::SystemMemoryStorage::unmap()
{
    // No-op
}
}
