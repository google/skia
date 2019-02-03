/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpuBuffer_DEFINED
#define GrGpuBuffer_DEFINED

#include "GrGpuResource.h"

class GrGpu;

#include "GrNonAtomicRef.h"

class GrBuffer {
public:
    virtual ~GrBuffer() = default;

    // Our subclasses derive from different ref counting base classes. In order to use base
    // class pointers with sk_sp we virtualize ref() and unref().
    virtual void ref() const = 0;
    virtual void unref() const = 0;

    // GPU-memory backed buffers can become invalid.
    virtual bool isValid() const = 0;

    /** Size of the buffer in bytes. */
    virtual size_t size() const  = 0;

    /**
     * Map the buffer into CPU-addressable memory.
     *
     * The previous content of the buffer is invalidated. It is an error to draw from the buffer
     * while it is mapped. It may fail if the backend doesn't support mapping the buffer. If the
     * buffer is CPU backed then it will always succeed and is a free operation. Once a buffer is
     * mapped, subsequent calls to map() are ignored.
     */
    virtual void* map() = 0;
    /**
     * GPU buffers must be unmapped before they can be used in draws. The previous pointer
     * returned by map() may be invalid after this is called.
     */
    virtual void unmap() = 0;
    /** Current mapping status of the buffer. Always true for CPU memory buffers. */
    virtual bool isMapped() const = 0;

    /** Is this an instance of GrCpuBuffer? */
    virtual bool isCpuBuffer() const = 0;

protected:
    GrBuffer() = default;
    GrBuffer(const GrBuffer&) = delete;
    GrBuffer& operator=(const GrBuffer&) = delete;
};

class GrCpuBuffer final : public GrNonAtomicRef<GrCpuBuffer>, public GrBuffer {
public:
    static sk_sp<GrCpuBuffer> Make(size_t size) {
        SkASSERT(size > 0);
        auto mem = ::operator new(sizeof(GrCpuBuffer) + size);
        return sk_sp<GrCpuBuffer>(new (mem) GrCpuBuffer((char*)mem + sizeof(GrCpuBuffer), size));
    }

    static sk_sp<GrCpuBuffer> Make(void* data, size_t size) {
        SkASSERT(size > 0);
        return sk_sp<GrCpuBuffer>(new GrCpuBuffer(data, size));
    }

    void ref() const override { GrNonAtomicRef<GrCpuBuffer>::ref(); }
    void unref() const override { GrNonAtomicRef<GrCpuBuffer>::unref(); }

    size_t size() const override { return fSize; }
    bool isValid() const override { return true; }
    void* map() override { fIsMapped = true; return fData; }
    void unmap() override { fIsMapped = false; }
    bool isMapped() const override { return fIsMapped; }
    bool isCpuBuffer() const override { return true; }

    const char* data() const { return reinterpret_cast<const char*>(fData); }

private:
    GrCpuBuffer(void* data, size_t size) : fData(data), fSize(size) {}
    void* fData;
    size_t fSize;
    bool fIsMapped = false; // TODO: Get rid of this. Get rid of mapping?
};

class GrGpuBuffer : public GrGpuResource, public GrBuffer {
public:
    /**
     * Computes a scratch key for a GPU-side buffer with a "dynamic" access pattern. (Buffers with
     * "static" and "stream" patterns are disqualified by nature from being cached and reused.)
     */
    static void ComputeScratchKeyForDynamicVBO(size_t size, GrBufferType, GrScratchKey*);

    GrAccessPattern accessPattern() const { return fAccessPattern; }

    bool isValid() const final { return !this->wasDestroyed(); }

    size_t size() const final { return fSizeInBytes; }

    /**
     * Maps the buffer to be written by the CPU.
     *
     * The previous content of the buffer is invalidated. It is an error
     * to draw from the buffer while it is mapped. It may fail if the backend
     * doesn't support mapping the buffer. If the buffer is CPU backed then
     * it will always succeed and is a free operation. Once a buffer is mapped,
     * subsequent calls to map() are ignored.
     *
     * Note that buffer mapping does not go through GrContext and therefore is
     * not serialized with other operations.
     *
     * @return a pointer to the data or nullptr if the map fails.
     */
     void* map() final {
         if (!fMapPtr) {
             this->onMap();
         }
         return fMapPtr;
     }

    /**
     * Unmaps the buffer.
     *
     * The pointer returned by the previous map call will no longer be valid.
     */
     void unmap() final {
         SkASSERT(fMapPtr);
         this->onUnmap();
         fMapPtr = nullptr;
     }

    void ref() const final { GrGpuResource::ref(); }

    void unref() const final { GrGpuResource::unref(); }

    /**
     Queries whether the buffer has been mapped.

     @return true if the buffer is mapped, false otherwise.
     */
     bool isMapped() const final { return SkToBool(fMapPtr); }

    bool isCpuBuffer() const final { return false; }

    /**
     * Updates the buffer data.
     *
     * The size of the buffer will be preserved. The src data will be
     * placed at the beginning of the buffer and any remaining contents will
     * be undefined. srcSizeInBytes must be <= to the buffer size.
     *
     * The buffer must not be mapped.
     *
     * Note that buffer updates do not go through GrContext and therefore are
     * not serialized with other operations.
     *
     * @return returns true if the update succeeds, false otherwise.
     */
    bool updateData(const void* src, size_t srcSizeInBytes) {
        SkASSERT(!this->isMapped());
        SkASSERT(srcSizeInBytes <= fSizeInBytes);
        return this->onUpdateData(src, srcSizeInBytes);
    }

protected:
    GrGpuBuffer(GrGpu*, size_t sizeInBytes, GrBufferType, GrAccessPattern);

    void* fMapPtr;

private:
    /**
     * Internal constructor to make a CPU-backed buffer.
     */
    GrGpuBuffer(GrGpu*, size_t sizeInBytes, GrBufferType, void* cpuData);

    virtual void onMap() = 0;
    virtual void onUnmap() = 0;
    virtual bool onUpdateData(const void* src, size_t srcSizeInBytes) = 0;

    size_t onGpuMemorySize() const override { return fSizeInBytes; }
    const char* getResourceType() const override { return "Buffer Object"; }
    void computeScratchKey(GrScratchKey* key) const override;

    size_t            fSizeInBytes;
    GrAccessPattern   fAccessPattern;
    GrBufferType      fIntendedType;

    typedef GrGpuResource INHERITED;
};

#endif
