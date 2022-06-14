/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlGpu.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/mtl/MtlCommandBuffer.h"
#include "src/gpu/graphite/mtl/MtlResourceProvider.h"
#include "src/gpu/graphite/mtl/MtlTexture.h"

namespace skgpu::graphite {

sk_sp<skgpu::graphite::Gpu> MtlGpu::Make(const MtlBackendContext& context,
                                         const ContextOptions& options) {
    // TODO: This was taken from GrMtlGpu.mm's Make, does graphite deserve a higher version?
    if (@available(macOS 10.14, iOS 11.0, *)) {
        // no warning needed
    } else {
        SKGPU_LOG_E("Skia's Graphite backend no longer supports this OS version.");
#ifdef SK_BUILD_FOR_IOS
        SKGPU_LOG_E("Minimum supported version is iOS 11.0.");
#else
        SKGPU_LOG_E("Minimum supported version is MacOS 10.14.");
#endif
        return nullptr;
    }

    sk_cfp<id<MTLDevice>> device = sk_ret_cfp((id<MTLDevice>)(context.fDevice.get()));
    sk_cfp<id<MTLCommandQueue>> queue = sk_ret_cfp((id<MTLCommandQueue>)(context.fQueue.get()));

    sk_sp<const MtlCaps> caps(new MtlCaps(device.get(), options));

    return sk_sp<skgpu::graphite::Gpu>(new MtlGpu(std::move(device),
                                                  std::move(queue),
                                                  std::move(caps)));
}

MtlGpu::MtlGpu(sk_cfp<id<MTLDevice>> device,
               sk_cfp<id<MTLCommandQueue>> queue,
               sk_sp<const MtlCaps> caps)
    : skgpu::graphite::Gpu(std::move(caps))
    , fDevice(std::move(device))
    , fQueue(std::move(queue)) {
    this->initCompiler();
}

MtlGpu::~MtlGpu() {
}

std::unique_ptr<ResourceProvider> MtlGpu::makeResourceProvider(
        sk_sp<GlobalCache> globalCache, SingleOwner* singleOwner) const {
    return std::unique_ptr<ResourceProvider>(new MtlResourceProvider(this,
                                                                     std::move(globalCache),
                                                                     singleOwner));
}

class WorkSubmission final : public GpuWorkSubmission {
public:
    WorkSubmission(sk_sp<CommandBuffer> cmdBuffer)
        : GpuWorkSubmission(std::move(cmdBuffer)) {}
    ~WorkSubmission() override {}

    bool isFinished() override {
        return static_cast<MtlCommandBuffer*>(this->commandBuffer())->isFinished();
    }
    void waitUntilFinished(const Gpu*) override {
        return static_cast<MtlCommandBuffer*>(this->commandBuffer())->waitUntilFinished();
    }
};

skgpu::graphite::Gpu::OutstandingSubmission MtlGpu::onSubmit(sk_sp<CommandBuffer> commandBuffer) {
    SkASSERT(commandBuffer);
    MtlCommandBuffer* mtlCmdBuffer = static_cast<MtlCommandBuffer*>(commandBuffer.get());
    if (!mtlCmdBuffer->commit()) {
        commandBuffer->callFinishedProcs(/*success=*/false);
        return nullptr;
    }

    std::unique_ptr<GpuWorkSubmission> submission(new WorkSubmission(std::move(commandBuffer)));
    return submission;
}

BackendTexture MtlGpu::onCreateBackendTexture(SkISize dimensions, const TextureInfo& info) {
    sk_cfp<id<MTLTexture>> texture = MtlTexture::MakeMtlTexture(this, dimensions, info);
    if (!texture) {
        return {};
    }
    return BackendTexture(dimensions, (Handle)texture.release());
}

void MtlGpu::onDeleteBackendTexture(BackendTexture& texture) {
    SkASSERT(texture.backend() == BackendApi::kMetal);
    MtlHandle texHandle = texture.getMtlTexture();
    SkCFSafeRelease(texHandle);
}

#if GRAPHITE_TEST_UTILS
void MtlGpu::testingOnly_startCapture() {
    if (@available(macOS 10.13, iOS 11.0, *)) {
        // TODO: add newer Metal interface as well
        MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];
        if (captureManager.isCapturing) {
            return;
        }
        if (@available(macOS 10.15, iOS 13.0, *)) {
            MTLCaptureDescriptor* captureDescriptor = [[MTLCaptureDescriptor alloc] init];
            captureDescriptor.captureObject = fQueue.get();

            NSError *error;
            if (![captureManager startCaptureWithDescriptor: captureDescriptor error:&error])
            {
                NSLog(@"Failed to start capture, error %@", error);
            }
        } else {
            [captureManager startCaptureWithCommandQueue: fQueue.get()];
        }
     }
}

void MtlGpu::testingOnly_endCapture() {
    if (@available(macOS 10.13, iOS 11.0, *)) {
        MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];
        if (captureManager.isCapturing) {
            [captureManager stopCapture];
        }
    }
}
#endif

} // namespace skgpu::graphite
