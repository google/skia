/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_task_UploadTask_DEFINED
#define skgpu_graphite_task_UploadTask_DEFINED

#include "src/gpu/graphite/task/Task.h"

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/CommandTypes.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>

class SkColorInfo;
struct SkIRect;
enum class SkTextureCompressionType;

namespace skgpu::graphite {

class Buffer;
class Caps;
class CommandBuffer;
class Context;
class Recorder;
class ResourceProvider;
class RuntimeEffectDictionary;
class ScratchResourceManager;
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

    // Return true if the upload needs to occur; false if it should be skipped this time.
    virtual bool needsUpload(Context*) = 0;

    // Return true if the upload should be kept in the task (and possibly re-executed on replay
    // depending on needsUpload()'s return value), or false if it should be discarded and never
    // attempt to be uploaded on any replay.
    virtual bool uploadSubmitted() { return true; }
};

/**
 * ImageUploadContext is an implementation of ConditionalUploadContext that returns true on
 * the first call to needsUpload() and then returns false on subsequent calls. This is used to
 * upload an image once and then avoid redundant uploads after that.
 */
class ImageUploadContext : public ConditionalUploadContext {
public:
    ~ImageUploadContext() override {}

    // Always upload, since it will be discarded right afterwards
    bool needsUpload(Context*) override { return true; }

    // Always return false so the upload instance is discarded after the first execution
    bool uploadSubmitted() override { return false; }
};

/**
 * A set of `MipLevel`s, comprising the source data for an upload operation.
 *
 * While preparing the upload source, this class additionally caches some needed information, such
 * as whether the upload can be done on the host.
 */
class UploadSource {
public:
    static UploadSource Make(const Caps*,
                             const TextureProxy& textureProxy,
                             const SkColorInfo& srcColorInfo,
                             const SkColorInfo& dstColorInfo,
                             SkSpan<const MipLevel> levels,
                             const SkIRect& dstRect);
    static UploadSource MakeCompressed(const Caps*,
                                       const TextureProxy& textureProxy,
                                       const void* data,
                                       size_t dataSize);

    UploadSource(UploadSource&&);
    UploadSource& operator=(UploadSource&&);
    ~UploadSource();

    bool isValid() const { return !fLevels.empty(); }

    SkSpan<const MipLevel> levels() const { return fLevels; }
    bool canUploadOnHost() const { return fCanUploadOnHost; }
    bool isRGB888Format() const { return fIsRGB888Format; }
    SkTextureCompressionType compression() const { return fCompression; }
    size_t bytesPerPixel() const { return fBytesPerPixel; }

private:
    static UploadSource Invalid() { return {}; }

    UploadSource();

    skia_private::STArray<16, MipLevel> fLevels;

    // Whether the texture supports uploads directly from host memory.
    bool fCanUploadOnHost = false;
    // Whether the texture is RGB888, which is typically emulated by RGBA8888.
    bool fIsRGB888Format = false;
    // Compression type, if any.
    SkTextureCompressionType fCompression;
    // Bytes per pixel or block (if compressed)
    size_t fBytesPerPixel = 0;
};

/**
 * An UploadInstance represents a single set of uploads from a buffer to texture that
 * can be processed in a single command.
 */
class UploadInstance {
public:
    static UploadInstance Make(Recorder*,
                               sk_sp<TextureProxy> textureProxy,
                               const SkColorInfo& srcColorInfo,
                               const SkColorInfo& dstColorInfo,
                               const UploadSource& source,
                               const SkIRect& dstRect,
                               std::unique_ptr<ConditionalUploadContext>);
    static UploadInstance MakeCompressed(Recorder*,
                                         sk_sp<TextureProxy> textureProxy,
                                         const UploadSource& source);

    static UploadInstance Invalid() { return {}; }

    UploadInstance(UploadInstance&&);
    UploadInstance& operator=(UploadInstance&&);
    ~UploadInstance();

    bool isValid() const { return fBuffer != nullptr && fTextureProxy != nullptr; }

    bool prepareResources(ResourceProvider*);

    // Adds upload command to the given CommandBuffer, returns false if the instance should be
    // discarded.
    Task::Status addCommand(Context*, CommandBuffer*, Task::ReplayTargetData) const;

private:
    friend class UploadTask;

    UploadInstance();
    // Copy data is appended directly after the object is created
    UploadInstance(const Buffer*,
                   size_t bytesPerPixel,
                   sk_sp<TextureProxy>,
                   std::unique_ptr<ConditionalUploadContext> = nullptr);

    const Buffer* fBuffer;
    size_t fBytesPerPixel;
    sk_sp<TextureProxy> fTextureProxy;
    skia_private::STArray<1, BufferTextureCopyData> fCopyData;
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
                      const UploadSource& source,
                      const SkIRect& dstRect,
                      std::unique_ptr<ConditionalUploadContext>);

    int size() { return fInstances.size(); }

private:
    friend class UploadTask;

    skia_private::STArray<1, UploadInstance> fInstances;
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

    Status prepareResources(ResourceProvider*,
                            ScratchResourceManager*,
                            sk_sp<const RuntimeEffectDictionary>) override;

    Status addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

    bool visitProxies(const std::function<bool(const TextureProxy*)>& visitor,
                      bool readsOnly) override {
        // Textures being uploaded to are never read from, so skip all visiting unless readsOnly
        // is false.
        if (!readsOnly) {
            for (int32_t i = 0; i < fInstances.size(); ++i) {
                if (fInstances[i].isValid() && !visitor(fInstances[i].fTextureProxy.get())) {
                    return false;
                }
            }
        }
        return true;
    }

    SK_DUMP_TASKS_CODE(const char* getTaskName() const override { return "Upload Task"; })

private:
    UploadTask(skia_private::TArray<UploadInstance>&&);
    UploadTask(UploadInstance);

    skia_private::STArray<1, UploadInstance> fInstances;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_task_UploadTask_DEFINED
