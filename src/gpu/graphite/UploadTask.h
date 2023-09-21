/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_UploadTask_DEFINED
#define skgpu_graphite_UploadTask_DEFINED

#include "src/gpu/graphite/Task.h"

#include <memory>
#include <vector>

#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"

namespace skgpu::graphite {

class Buffer;
struct BufferTextureCopyData;
class Recorder;
class TextureProxy;

struct MipLevel {
    const void* fPixels = nullptr;
    size_t fRowBytes = 0;
};

/**
 * The ConditionalUploadContext, if set, is used to determine whether an upload needs to occur
 * on Recording playback. Clients will need to create their own subclasses to store the
 * necessary data and override the needsUpload() method to do this check.
 */
class ConditionalUploadContext {
public:
    virtual ~ConditionalUploadContext() {}

    virtual bool needsUpload(Context*) const = 0;

    virtual void uploadSubmitted() {}
};

/**
 * ImageUploadContext is an implementation of ConditionalUploadContext that returns true on
 * the first call to needsUpload() and then returns false on subsequent calls. This is used to
 * upload an image once and then avoid redundant uploads after that.
 */
class ImageUploadContext : public ConditionalUploadContext {
public:
    ~ImageUploadContext() override {}

    bool needsUpload(Context* context) const override {
        return fNeedsUpload;
    }

    void uploadSubmitted() override {
        fNeedsUpload = false;
    }

private:
    bool fNeedsUpload = true;
};

/**
 * An UploadInstance represents a single set of uploads from a buffer to texture that
 * can be processed in a single command.
 */
class UploadInstance {
public:
    static UploadInstance Make(Recorder*,
                               sk_sp<TextureProxy> targetProxy,
                               const SkColorInfo& srcColorInfo,
                               const SkColorInfo& dstColorInfo,
                               const std::vector<MipLevel>& levels,
                               const SkIRect& dstRect,
                               std::unique_ptr<ConditionalUploadContext>);
    UploadInstance(UploadInstance&&);
    UploadInstance& operator=(UploadInstance&&);
    ~UploadInstance();

    bool isValid() const { return fBuffer != nullptr && fTextureProxy != nullptr; }

    bool prepareResources(ResourceProvider*);

    // Adds upload command to the given CommandBuffer
    void addCommand(Context*, CommandBuffer*, Task::ReplayTargetData) const;

private:
    UploadInstance();
    UploadInstance(const Buffer*,
                   size_t bytesPerPixel,
                   sk_sp<TextureProxy>,
                   std::vector<BufferTextureCopyData>,
                   std::unique_ptr<ConditionalUploadContext>);

    const Buffer* fBuffer;
    size_t fBytesPerPixel;
    sk_sp<TextureProxy> fTextureProxy;
    std::vector<BufferTextureCopyData> fCopyData;
    std::unique_ptr<ConditionalUploadContext> fConditionalContext;
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
    bool recordUpload(Recorder*,
                      sk_sp<TextureProxy> targetProxy,
                      const SkColorInfo& srcColorInfo,
                      const SkColorInfo& dstColorInfo,
                      const std::vector<MipLevel>& levels,
                      const SkIRect& dstRect,
                      std::unique_ptr<ConditionalUploadContext>);

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
    static sk_sp<UploadTask> Make(UploadInstance);

    ~UploadTask() override;

    bool prepareResources(ResourceProvider*, const RuntimeEffectDictionary*) override;

    bool addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

private:
    UploadTask(std::vector<UploadInstance>);
    UploadTask(UploadInstance);

    std::vector<UploadInstance> fInstances;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_UploadTask_DEFINED
