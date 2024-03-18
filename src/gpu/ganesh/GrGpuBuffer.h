/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpuBuffer_DEFINED
#define GrGpuBuffer_DEFINED

#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrBuffer.h"
#include "src/gpu/ganesh/GrGpuResource.h"

#include <cstddef>
#include <string_view>

class GrGpu;

namespace skgpu {
class ScratchKey;
}

class GrGpuBuffer : public GrGpuResource, public GrBuffer {
public:
    /**
     * Computes a scratch key for a GPU-side buffer with a "dynamic" access pattern. (Buffers with
     * "static" and "stream" patterns are disqualified by nature from being cached and reused.)
     */
    static void ComputeScratchKeyForDynamicBuffer(size_t size, GrGpuBufferType, skgpu::ScratchKey*);

    GrAccessPattern accessPattern() const { return fAccessPattern; }

    size_t size() const final { return fSizeInBytes; }

    void ref() const final { GrGpuResource::ref(); }

    void unref() const final { GrGpuResource::unref(); }

    /**
     * Maps the buffer to be read or written by the CPU.
     *
     * It is an error to draw from the buffer while it is mapped or transfer to/from the buffer. It
     * may fail if the backend doesn't support mapping the buffer. Once a buffer is mapped,
     * subsequent calls to map() trivially succeed. No matter how many times map() is called,
     * umap() will unmap the buffer on the first call if it is mapped.
     *
     * If the buffer is of type GrGpuBufferType::kXferGpuToCpu then it is mapped for reading only.
     * Otherwise it is mapped writing only. Writing to a buffer that is mapped for reading or vice
     * versa produces undefined results. If the buffer is mapped for writing then the buffer's
     * previous contents are invalidated.
     *
     * @return a pointer to the data or nullptr if the map fails.
     */
    void* map();

    /**
     * Unmaps the buffer if it is mapped.
     *
     * The pointer returned by the previous map call will no longer be valid.
     */
    void unmap();

    /**
     * Queries whether the buffer has been mapped.
     *
     * @return true if the buffer is mapped, false otherwise.
     */
    bool isMapped() const;

    bool isCpuBuffer() const final { return false; }

    /**
     * Overwrites the buffer with zero bytes. Always fails for GrGpuBufferType::kXferGpuToCpu
     * buffers. The buffer must not currently be mapped.
     */
    bool clearToZero();

    /**
     * Updates the buffer data.
     *
     * The size of the buffer will be preserved. The src data will be
     * placed at offset. If preserve is false then any remaining content
     * before/after the range [offset, offset+size) becomes undefined.
     * Preserving updates will fail if the size and offset are not aligned
     * to GrCaps::bufferUpdateDataPreserveAlignment().
     *
     * The buffer must not be mapped.
     *
     * Fails for GrGpuBufferType::kXferGpuToCpu.
     *
     * Note that buffer updates do not go through GrContext and therefore are
     * not serialized with other operations.
     *
     * @return returns true if the update succeeds, false otherwise.
     */
    bool updateData(const void* src, size_t offset, size_t size, bool preserve);

    GrGpuBufferType intendedType() const { return fIntendedType; }

protected:
    GrGpuBuffer(GrGpu*,
                size_t sizeInBytes,
                GrGpuBufferType,
                GrAccessPattern,
                std::string_view label);

    enum class MapType {
        /** Maps for reading. The effect of writes is undefined. */
        kRead,
        /**
         * Maps for writing. The existing contents are discarded and the initial contents of the
         * buffer. Reads (even after overwriting initial contents) should be avoided for performance
         * reasons as the memory may not be cached.
         */
        kWriteDiscard,
    };

    void* fMapPtr;

private:
    /** Currently MapType is determined entirely by the buffer type, as documented in map(). */
    MapType mapType() const {
        return this->intendedType() == GrGpuBufferType::kXferGpuToCpu ? MapType::kRead
                                                                      : MapType::kWriteDiscard;
    }

    virtual void onMap(MapType) = 0;
    virtual void onUnmap(MapType) = 0;
    virtual bool onClearToZero() = 0;
    virtual bool onUpdateData(const void* src, size_t offset, size_t size, bool preserve) = 0;

    size_t onGpuMemorySize() const override { return fSizeInBytes; }
    void onSetLabel() override{}
    const char* getResourceType() const override { return "Buffer Object"; }
    void computeScratchKey(skgpu::ScratchKey* key) const override;

    size_t            fSizeInBytes;
    GrAccessPattern   fAccessPattern;
    GrGpuBufferType   fIntendedType;
};

#endif
