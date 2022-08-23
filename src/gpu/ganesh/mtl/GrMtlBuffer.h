/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlBuffer_DEFINED
#define GrMtlBuffer_DEFINED

#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/mtl/GrMtlUniformHandler.h"

#import <Metal/Metal.h>

class GrMtlCaps;
class GrMtlGpu;

class GrMtlBuffer: public GrGpuBuffer {
public:
    static sk_sp<GrMtlBuffer> Make(GrMtlGpu*,
                                   size_t size,
                                   GrGpuBufferType intendedType,
                                   GrAccessPattern);

    ~GrMtlBuffer() override;

    id<MTLBuffer> mtlBuffer() const { return fMtlBuffer; }

protected:
    GrMtlBuffer(GrMtlGpu*,
                size_t size,
                GrGpuBufferType intendedType,
                GrAccessPattern,
                std::string_view label);

    void onAbandon() override;
    void onRelease() override;

private:
    GrMtlGpu* mtlGpu() const;

    void onMap(MapType) override;
    bool onClearToZero() override;
    void onUnmap(MapType) override;
    bool onUpdateData(const void* src, size_t offset, size_t size, bool preserve) override;

    void internalMap();
    void internalUnmap(size_t writtenOffset, size_t writtenSize);

#ifdef SK_DEBUG
    void validate() const;
#endif

    void onSetLabel() override;

    bool fIsDynamic;
    id<MTLBuffer> fMtlBuffer;

    using INHERITED = GrGpuBuffer;
};

#endif
