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
 * An UploadCommand represents a single set of uploads from a buffer to texture that
 * can be processed in a single command.
 */
class UploadCommand {
public:
    // Process the current UploadList to make a list of UploadCommands
    static UploadCommand Make(ResourceProvider*,
                              sk_sp<TextureProxy> targetProxy,
                              const std::vector<MipLevel>& levels,
                              const SkIRect& dstRect);

    void addCommand(ResourceProvider*, CommandBuffer*) const;

private:
    UploadCommand(sk_sp<Buffer>, sk_sp<TextureProxy>, std::vector<BufferTextureCopyData>);

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
    void appendUpload(ResourceProvider*,
                      sk_sp<TextureProxy> targetProxy,
                      const std::vector<MipLevel>& levels,
                      const SkIRect& dstRect);

private:
    friend class UploadTask;

    std::vector<UploadCommand> fCommands;
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

    ~UploadTask() override;

    void addCommands(ResourceProvider*, CommandBuffer*) override;

private:
    UploadTask(std::vector<UploadCommand>);

    std::vector<UploadCommand> fCommands;
};

} // namespace skgpu

#endif // skgpu_UploadTask_DEFINED
