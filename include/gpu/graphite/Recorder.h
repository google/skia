/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Recorder_DEFINED
#define skgpu_graphite_Recorder_DEFINED

#include "include/core/SkCPURecorder.h"
#include "include/core/SkRecorder.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recording.h"
#include "include/private/base/SingleOwner.h"
#include "include/private/base/SkAPI.h"
#include "include/private/base/SkTArray.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

class SkCanvas;
class SkPixmap;
class SkTraceMemoryDump;
struct SkISize;
struct SkImageInfo;

#if defined(SK_BUILD_FOR_ANDROID)
struct AHardwareBuffer;
#endif

namespace skgpu {
enum class BackendApi : unsigned int;
class RefCntedCallback;
class TokenTracker;
}

namespace sktext::gpu {
class StrikeCache;
class TextBlobRedrawCoordinator;
}

namespace skgpu::graphite {

class AtlasProvider;
class BackendTexture;
class Context;
class Device;
class DrawBufferManager;
class FloatStorageManager;
class ImageProvider;
class ProxyReadCountMap;
class RecorderPriv;
class ResourceProvider;
class RuntimeEffectDictionary;
class SharedContext;
class TaskList;
class TextureInfo;
class UploadBufferManager;
class UploadList;

struct RecorderOptionsPriv;

struct SK_API RecorderOptions final {
    RecorderOptions();
    RecorderOptions(const RecorderOptions&);
    ~RecorderOptions();

    sk_sp<ImageProvider> fImageProvider;

    static constexpr size_t kDefaultRecorderBudget = 256 * (1 << 20);
    // What is the budget for GPU resources allocated and held by this Recorder.
    size_t fGpuBudgetInBytes = kDefaultRecorderBudget;
    // If Recordings are known to be played back in the order they are recorded, then Graphite
    // may be able to make certain assumptions that improve performance. This is often the case
    // if the content being drawn triggers the use of internal atlasing in Graphite (e.g. text).
    std::optional<bool> fRequireOrderedRecordings;

    // Private options that are only meant for testing within Skia's tools.
    RecorderOptionsPriv* fRecorderOptionsPriv = nullptr;
};

class SK_API Recorder final : public SkRecorder {
public:
    Recorder(const Recorder&) = delete;
    Recorder(Recorder&&) = delete;
    Recorder& operator=(const Recorder&) = delete;
    Recorder& operator=(Recorder&&) = delete;

    ~Recorder() override;

    BackendApi backend() const;

    Type type() const override { return SkRecorder::Type::kGraphite; }
    skcpu::Recorder* cpuRecorder() override;

    std::unique_ptr<Recording> snap();

    ImageProvider* clientImageProvider() { return fClientImageProvider.get(); }
    const ImageProvider* clientImageProvider() const { return fClientImageProvider.get(); }

    /**
     * Gets the maximum supported texture size.
     */
    int maxTextureSize() const;

    /**
     * Creates a new backend gpu texture matching the dimensions and TextureInfo. If an invalid
     * TextureInfo or a TextureInfo Skia can't support is passed in, this will return an invalid
     * BackendTexture. Thus the client should check isValid on the returned BackendTexture to know
     * if it succeeded or not.
     *
     * If this does return a valid BackendTexture, the caller is required to use
     * Recorder::deleteBackendTexture or Context::deleteBackendTexture to delete the texture. It is
     * safe to use the Context that created this Recorder or any other Recorder created from the
     * same Context to call deleteBackendTexture.
     */
    BackendTexture createBackendTexture(SkISize dimensions, const TextureInfo&);

#ifdef SK_BUILD_FOR_ANDROID
    BackendTexture createBackendTexture(AHardwareBuffer*,
                                        bool isRenderable,
                                        bool isProtectedContent,
                                        SkISize dimensions,
                                        bool fromAndroidWindow = false) const;
#endif

    /**
     * If possible, updates a backend texture with the provided pixmap data. The client
     * should check the return value to see if the update was successful. The client is required
     * to insert a Recording into the Context and call `submit` to send the upload work to the gpu.
     * The backend texture must be compatible with the provided pixmap(s). Compatible, in this case,
     * means that the backend format is compatible with the base pixmap's colortype. The src data
     * can be deleted when this call returns. When the BackendTexture is safe to be destroyed by the
     * client, Skia will call the passed in GpuFinishedProc. The BackendTexture should not be
     * destroyed before that.
     * If the backend texture is mip mapped, the data for all the mipmap levels must be provided.
     * In the mipmapped case all the colortypes of the provided pixmaps must be the same.
     * Additionally, all the miplevels must be sized correctly (please see
     * SkMipmap::ComputeLevelSize and ComputeLevelCount).
     * Note: the pixmap's alphatypes and colorspaces are ignored.
     * For the Vulkan backend after a successful update the layout of the created VkImage will be:
     *      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
     */
    bool updateBackendTexture(const BackendTexture&,
                              const SkPixmap srcData[],
                              int numLevels,
                              GpuFinishedProc = nullptr,
                              GpuFinishedContext = nullptr);

    /**
     * If possible, updates a compressed backend texture filled with the provided raw data. The
     * client should check the return value to see if the update was successful. The client is
     * required to insert a Recording into the Context and call `submit` to send the upload work to
     * the gpu. When the BackendTexture is safe to be destroyed by the client, Skia will call the
     * passed in GpuFinishedProc. The BackendTexture should not be destroyed before that.
     * If the backend texture is mip mapped, the data for all the mipmap levels must be provided.
     * Additionally, all the miplevels must be sized correctly (please see
     * SkMipMap::ComputeLevelSize and ComputeLevelCount).
     * For the Vulkan backend after a successful update the layout of the created VkImage will be:
     *      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
     */
    bool updateCompressedBackendTexture(const BackendTexture&,
                                        const void* data,
                                        size_t dataSize,
                                        GpuFinishedProc = nullptr,
                                        GpuFinishedContext = nullptr);

    /**
     * Called to delete the passed in BackendTexture. This should only be called if the
     * BackendTexture was created by calling Recorder::createBackendTexture on a Recorder that is
     * associated with the same Context. If the BackendTexture is not valid or does not match the
     * BackendApi of the Recorder then nothing happens.
     *
     * Otherwise this will delete/release the backend object that is wrapped in the BackendTexture.
     * The BackendTexture will be reset to an invalid state and should not be used again.
     */
    void deleteBackendTexture(const BackendTexture&);

    // Adds a proc that will be moved to the Recording upon snap, subsequently attached to the
    // CommandBuffer when the Recording is added, and called when that CommandBuffer is submitted
    // and finishes. If the Recorder or Recording is deleted before the proc is added to the
    // CommandBuffer, it will be called with result Failure.
    void addFinishInfo(const InsertFinishInfo&);

    // Returns a canvas that will record to a proxy surface, which must be instantiated on replay.
    // This can only be called once per Recording; subsequent calls will return null until a
    // Recording is snapped. Additionally, the returned SkCanvas is only valid until the next
    // Recording snap, at which point it is deleted.
    SkCanvas* makeDeferredCanvas(const SkImageInfo&, const TextureInfo&);

    /**
     * Frees GPU resources created and held by the Recorder. Can be called to reduce GPU memory
     * pressure. Any resources that are still in use (e.g. being used by work submitted to the GPU)
     * will not be deleted by this call. If the caller wants to make sure all resources are freed,
     * then they should first make sure to submit and wait on any outstanding work.
     */
    void freeGpuResources();

    /**
     * Purge GPU resources on the Recorder that haven't been used in the past 'msNotUsed'
     * milliseconds or are otherwise marked for deletion, regardless of whether the context is under
     * budget.
     */
    void performDeferredCleanup(std::chrono::milliseconds msNotUsed);

    /**
     * Returns the number of bytes of the Recorder's gpu memory cache budget that are currently in
     * use.
     */
    size_t currentBudgetedBytes() const;

    /**
     * Returns the number of bytes of the Recorder's resource cache that are currently purgeable.
     */
    size_t currentPurgeableBytes() const;

    /**
     * Returns the size of Recorder's gpu memory cache budget in bytes.
     */
    size_t maxBudgetedBytes() const;

    /**
     * Sets the size of Recorders's gpu memory cache budget in bytes. If the new budget is lower
     * than the current budget, the cache will try to free resources to get under the new budget.
     */
    void setMaxBudgetedBytes(size_t bytes);

    /**
     * Enumerates all cached GPU resources owned by the Recorder and dumps their memory to
     * traceMemoryDump.
     */
    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const;

    // Provides access to functions that aren't part of the public API.
    RecorderPriv priv();
    const RecorderPriv priv() const;  // NOLINT(readability-const-return-type)

private:
    friend class Context; // For ctor
    friend class Device; // For registering and deregistering Devices;
    friend class RecorderPriv; // for ctor and hidden methods

    // If Context is non-null, the Recorder will use the Context's resource provider
    // instead of creating its own.
    Recorder(sk_sp<SharedContext>, const RecorderOptions&, const Context*);

    SingleOwner* singleOwner() const { return &fSingleOwner; }

    // We keep track of all Devices that are connected to a Recorder. This allows the client to
    // safely delete an SkSurface or a Recorder in any order. If the client deletes the Recorder
    // we need to notify all Devices that the Recorder is no longer valid. If we delete the
    // SkSurface/Device first we will flush all the Device's into the Recorder before deregistering
    // it from the Recorder.
    //
    // We take a ref on the Device so that ~Device() does not have to deregister the recorder
    // (which can happen on any thread if the Device outlives the Surface via an Image view).
    // Recorder::flushTrackedDevices() cleans up uniquely held and immutable Devices on the recorder
    // thread so this extra ref is not significantly increasing the Device lifetime.
    //
    // Note: We could probably get by with only registering Devices directly connected to
    // SkSurfaces. All other one off Devices will be created in a controlled scope where the
    // Recorder should still be valid by the time they need to flush their work when the Device is
    // deleted. We would have to make sure we safely handle cases where a client calls saveLayer
    // then either deletes the SkSurface or Recorder before calling restore. For simplicity we just
    // register every device for now, but if we see extra overhead in pushing back the extra
    // pointers, we can look into only registering SkSurface Devices.
    void registerDevice(sk_sp<Device>);
    void deregisterDevice(const Device*);

    SkCanvas* makeCaptureCanvas(SkCanvas*) override;

    sk_sp<SharedContext> fSharedContext;
    ResourceProvider* fResourceProvider; // May point to the Context's resource provider
    std::unique_ptr<ResourceProvider> fOwnedResourceProvider; // May be null

    sk_sp<RuntimeEffectDictionary> fRuntimeEffectDict;

    // NOTE: These are stored by pointer to allow them to be forward declared.
    std::unique_ptr<TaskList> fRootTaskList;
    // Aggregated one-time uploads that preceed all tasks in the root task list.
    std::unique_ptr<UploadList> fRootUploads;

    std::unique_ptr<DrawBufferManager> fDrawBufferManager;
    std::unique_ptr<UploadBufferManager> fUploadBufferManager;
    sk_sp<FloatStorageManager> fFloatStorageManager;
    std::unique_ptr<ProxyReadCountMap> fProxyReadCounts;

    // Iterating over tracked devices in flushTrackedDevices() needs to be re-entrant and support
    // additions to fTrackedDevices if registerDevice() is triggered by a temporary device during
    // flushing. Removals are handled by setting elements to null; final clean up is handled at the
    // end of the initial call to flushTrackedDevices().
    skia_private::TArray<sk_sp<Device>> fTrackedDevices;
    int fFlushingDevicesIndex = -1;

    uint32_t fUniqueID;  // Needed for MessageBox handling for text
    uint32_t fNextRecordingID = 1;
    const bool fRequireOrderedRecordings;

    std::unique_ptr<AtlasProvider> fAtlasProvider;
    std::unique_ptr<TokenTracker> fTokenTracker;
    std::unique_ptr<sktext::gpu::StrikeCache> fStrikeCache;
    std::unique_ptr<sktext::gpu::TextBlobRedrawCoordinator> fTextBlobCache;
    sk_sp<ImageProvider> fClientImageProvider;

    // In debug builds we guard against improper thread handling
    // This guard is passed to the ResourceCache.
    // TODO: Should we also pass this to Device, DrawContext, and similar classes?
    mutable SingleOwner fSingleOwner;

    sk_sp<Device> fTargetProxyDevice;
    std::unique_ptr<SkCanvas> fTargetProxyCanvas;
    std::unique_ptr<Recording::LazyProxyData> fTargetProxyData;

    skia_private::TArray<sk_sp<RefCntedCallback>> fFinishedProcs;

#if defined(GPU_TEST_UTILS)
    // For testing use only -- the Context used to create this Recorder
    Context* fContext = nullptr;
#endif
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Recorder_DEFINED
