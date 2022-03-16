/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_UploadTask_DEFINED
#define skgpu_UploadTask_DEFINED

#include "experimental/graphite/src/Task.h"

#include <vector>

#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"

namespace skgpu {

class Buffer;
struct BufferTextureCopyData;
class CommandBuffer;
class Recorder;
class ResourceProvider;
class TextureProxy;

struct MipLevel {
    const void* fPixels = nullptr;
    size_t fRowBytes = 0;
};

/**
 * An UploadInstance represents a single set of uploads from a buffer to texture that
 * can be processed in a single command.
 */
class UploadInstance {
public:
    static UploadInstance Make(Recorder*,
                               sk_sp<TextureProxy> targetProxy,
                               SkColorType colorType,
                               const std::vector<MipLevel>& levels,
                               const SkIRect& dstRect);

    bool isValid() const { return fBuffer != nullptr; }

    // Adds upload command to the given CommandBuffer
    void addCommand(ResourceProvider*, CommandBuffer*) const;

private:
    UploadInstance() {}
    UploadInstance(sk_sp<Buffer>, sk_sp<TextureProxy>, std::vector<BufferTextureCopyData>);

    sk_sp<Buffer> fBuffer;
    sk_sp<TextureProxy> fTextureProxy;
    std::vector<BufferTextureCopyData> fCopyData;
};


/**
 * An UploadList is a mutable collection of UploadCommands.
 *
 * Currently commands are accumulated in order and processed in the same order. Dependency
 * management is expected to be handled by the TaskGraph.
 *
 * When an upload is appended to the list its data will be copied to a Buffer in
 * preparation for a deferred upload.
 */
class UploadList {
public:
    bool appendUpload(Recorder*,
                      sk_sp<TextureProxy> targetProxy,
                      SkColorType colorType,
                      const std::vector<MipLevel>& levels,
                      const SkIRect& dstRect);

    int size() { return fInstances.size(); }

private:
    friend class UploadTask;

    std::vector<UploadInstance> fInstances;
};

/*
 * An UploadTask is a immutable collection of UploadCommands.
 *
 * When adding commands to the commandBuffer the texture proxies in those
 * commands will be instantiated and the copy command added.
 */
class UploadTask final : public Task {
public:
    static sk_sp<UploadTask> Make(UploadList*);
    static sk_sp<UploadTask> Make(const UploadInstance&);

    ~UploadTask() override;

    void addCommands(ResourceProvider*, CommandBuffer*) override;

private:
    UploadTask(std::vector<UploadInstance>);
    UploadTask(const UploadInstance&);

    std::vector<UploadInstance> fInstances;
};

} // namespace skgpu

#endif // skgpu_UploadTask_DEFINED
