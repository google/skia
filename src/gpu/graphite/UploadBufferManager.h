/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_UploadBufferManager_DEFINED
#define skgpu_graphite_UploadBufferManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/ResourceTypes.h"

#include <string_view>
#include <tuple>
#include <vector>

namespace skgpu::graphite {

class Buffer;
class Caps;
class CommandBuffer;
class Recorder;
class Recording;
class ResourceProvider;

class UploadBufferManager {
public:
    UploadBufferManager(ResourceProvider*, const Caps*,
                        // TODO(b/407062399): Remove these fields post debugging
                        int* maxReusedBufferCount,
                        int* maxUsedBufferCount);
    ~UploadBufferManager();

    std::tuple<TextureUploadWriter, BindBufferInfo> getTextureUploadWriter(
            size_t requiredBytes, size_t requiredAlignment);

    // Finalizes all buffers and transfers ownership of them to a Recording.
    void transferToRecording(Recording*);
    void transferToCommandBuffer(CommandBuffer*);

private:
    friend class DrawBufferManager; // to access makeBindInfo
    friend class StaticBufferManager; // to access makeBindInfo

    std::tuple<void* /*mappedPtr*/, BindBufferInfo> makeBindInfo(size_t requiredBytes,
                                                                 size_t requiredAlignment,
                                                                 std::string_view label);

    ResourceProvider* fResourceProvider;

    sk_sp<Buffer> fReusedBuffer;
    const uint32_t fMinAlignment;
    uint32_t fReusedBufferOffset = 0;

    std::vector<sk_sp<Buffer>> fUsedBuffers;

    // TODO(b/407062399): Debugging fields to track pathologic resource situations.
    // Cloud dumps don't always fetch the UploadBufferManager, but reliably include the Recorder,
    // so these point to storage fields in the Recorder.
    int* fMaxReusedBufferCount;
    int* fMaxUsedBufferCount;

    int fReusedBufferCount = 0; // reset after each transfer, should be less than fUsedBuffers.count
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_UploadBufferManager_DEFINED
