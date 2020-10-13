/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrDirectContext.h"

#include "include/core/SkDeferredDisplayList.h"
#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkTaskGroup.h"
#include "src/gpu/GrClientMappedBufferManager.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrImageContextPriv.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrPathRendererChain.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/GrSoftwarePathRenderer.h"
#include "src/gpu/GrThreadSafeCache.h"
#include "src/gpu/GrTracing.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/ccpr/GrCoverageCountingPathRenderer.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/text/GrSDFTOptions.h"
#include "src/gpu/text/GrStrikeCache.h"
#include "src/gpu/text/GrTextBlobCache.h"
#include "src/image/SkImage_GpuBase.h"
#include "src/image/SkSurface_Gpu.h"
#include <atomic>
#include <memory>

GrContext::GrContext(sk_sp<GrContextThreadSafeProxy> proxy) : INHERITED(std::move(proxy)) { }

GrContext::~GrContext() = default;

//////////////////////////////////////////////////////////////////////////////

static GrBackendTexture create_and_update_compressed_backend_texture(
        GrDirectContext* context,
        SkISize dimensions,
        const GrBackendFormat& backendFormat,
        GrMipmapped mipMapped,
        GrProtected isProtected,
        sk_sp<GrRefCntedCallback> finishedCallback,
        const GrGpu::BackendTextureData* data) {
    GrGpu* gpu = context->priv().getGpu();

    GrBackendTexture beTex = gpu->createCompressedBackendTexture(dimensions, backendFormat,
                                                                 mipMapped, isProtected);
    if (!beTex.isValid()) {
        return {};
    }

    if (!context->priv().getGpu()->updateCompressedBackendTexture(
                beTex, std::move(finishedCallback), data)) {
        context->deleteBackendTexture(beTex);
        return {};
    }
    return beTex;
}

GrBackendTexture GrContext::createCompressedBackendTexture(int width, int height,
                                                           const GrBackendFormat& backendFormat,
                                                           const SkColor4f& color,
                                                           GrMipmapped mipMapped,
                                                           GrProtected isProtected,
                                                           GrGpuFinishedProc finishedProc,
                                                           GrGpuFinishedContext finishedContext) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    sk_sp<GrRefCntedCallback> finishedCallback;
    if (finishedProc) {
        finishedCallback.reset(new GrRefCntedCallback(finishedProc, finishedContext));
    }

    if (!this->asDirectContext()) {
        return {};
    }

    if (this->abandoned()) {
        return {};
    }

    GrGpu::BackendTextureData data(color);
    return create_and_update_compressed_backend_texture(this->asDirectContext(), {width, height},
                                                        backendFormat, mipMapped, isProtected,
                                                        std::move(finishedCallback), &data);
}

GrBackendTexture GrContext::createCompressedBackendTexture(int width, int height,
                                                           SkImage::CompressionType compression,
                                                           const SkColor4f& color,
                                                           GrMipmapped mipMapped,
                                                           GrProtected isProtected,
                                                           GrGpuFinishedProc finishedProc,
                                                           GrGpuFinishedContext finishedContext) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    GrBackendFormat format = this->compressedBackendFormat(compression);
    return this->createCompressedBackendTexture(width, height, format, color,
                                                mipMapped, isProtected, finishedProc,
                                                finishedContext);
}

GrBackendTexture GrContext::createCompressedBackendTexture(int width, int height,
                                                           const GrBackendFormat& backendFormat,
                                                           const void* compressedData,
                                                           size_t dataSize,
                                                           GrMipmapped mipMapped,
                                                           GrProtected isProtected,
                                                           GrGpuFinishedProc finishedProc,
                                                           GrGpuFinishedContext finishedContext) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    sk_sp<GrRefCntedCallback> finishedCallback;
    if (finishedProc) {
        finishedCallback.reset(new GrRefCntedCallback(finishedProc, finishedContext));
    }

    if (!this->asDirectContext()) {
        return {};
    }

    if (this->abandoned()) {
        return {};
    }

    GrGpu::BackendTextureData data(compressedData, dataSize);
    return create_and_update_compressed_backend_texture(this->asDirectContext(), {width, height},
                                                        backendFormat, mipMapped, isProtected,
                                                        std::move(finishedCallback), &data);
}

GrBackendTexture GrContext::createCompressedBackendTexture(int width, int height,
                                                           SkImage::CompressionType compression,
                                                           const void* data, size_t dataSize,
                                                           GrMipmapped mipMapped,
                                                           GrProtected isProtected,
                                                           GrGpuFinishedProc finishedProc,
                                                           GrGpuFinishedContext finishedContext) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    GrBackendFormat format = this->compressedBackendFormat(compression);
    return this->createCompressedBackendTexture(width, height, format, data, dataSize, mipMapped,
                                                isProtected, finishedProc, finishedContext);
}

bool GrContext::setBackendTextureState(const GrBackendTexture& backendTexture,
                                       const GrBackendSurfaceMutableState& state,
                                       GrBackendSurfaceMutableState* previousState,
                                       GrGpuFinishedProc finishedProc,
                                       GrGpuFinishedContext finishedContext) {
    sk_sp<GrRefCntedCallback> callback;
    if (finishedProc) {
        callback.reset(new GrRefCntedCallback(finishedProc, finishedContext));
    }

    if (!this->asDirectContext()) {
        return false;
    }

    if (this->abandoned()) {
        return false;
    }

    return fGpu->setBackendTextureState(backendTexture, state, previousState, std::move(callback));
}

bool GrContext::updateCompressedBackendTexture(const GrBackendTexture& backendTexture,
                                               const SkColor4f& color,
                                               GrGpuFinishedProc finishedProc,
                                               GrGpuFinishedContext finishedContext) {
    sk_sp<GrRefCntedCallback> finishedCallback;
    if (finishedProc) {
        finishedCallback.reset(new GrRefCntedCallback(finishedProc, finishedContext));
    }

    if (!this->asDirectContext()) {
        return false;
    }

    if (this->abandoned()) {
        return false;
    }

    GrGpu::BackendTextureData data(color);
    return fGpu->updateCompressedBackendTexture(backendTexture, std::move(finishedCallback), &data);
}

bool GrContext::updateCompressedBackendTexture(const GrBackendTexture& backendTexture,
                                               const void* compressedData,
                                               size_t dataSize,
                                               GrGpuFinishedProc finishedProc,
                                               GrGpuFinishedContext finishedContext) {
    sk_sp<GrRefCntedCallback> finishedCallback;
    if (finishedProc) {
        finishedCallback.reset(new GrRefCntedCallback(finishedProc, finishedContext));
    }

    if (!this->asDirectContext()) {
        return false;
    }

    if (this->abandoned()) {
        return false;
    }

    if (!compressedData) {
        return false;
    }

    GrGpu::BackendTextureData data(compressedData, dataSize);

    return fGpu->updateCompressedBackendTexture(backendTexture, std::move(finishedCallback), &data);
}

//////////////////////////////////////////////////////////////////////////////

bool GrContext::setBackendRenderTargetState(const GrBackendRenderTarget& backendRenderTarget,
                                            const GrBackendSurfaceMutableState& state,
                                            GrBackendSurfaceMutableState* previousState,
                                            GrGpuFinishedProc finishedProc,
                                            GrGpuFinishedContext finishedContext) {
    sk_sp<GrRefCntedCallback> callback;
    if (finishedProc) {
        callback.reset(new GrRefCntedCallback(finishedProc, finishedContext));
    }

    if (!this->asDirectContext()) {
        return false;
    }

    if (this->abandoned()) {
        return false;
    }

    return fGpu->setBackendRenderTargetState(backendRenderTarget, state, previousState,
                                             std::move(callback));
}

void GrContext::deleteBackendTexture(GrBackendTexture backendTex) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    // For the Vulkan backend we still must destroy the backend texture when the context is
    // abandoned.
    if ((this->abandoned() && this->backend() != GrBackendApi::kVulkan) || !backendTex.isValid()) {
        return;
    }

    fGpu->deleteBackendTexture(backendTex);
}

//////////////////////////////////////////////////////////////////////////////

bool GrContext::precompileShader(const SkData& key, const SkData& data) {
    return fGpu->precompileShader(key, data);
}

#ifdef SK_ENABLE_DUMP_GPU
#include "include/core/SkString.h"
#include "src/utils/SkJSONWriter.h"
SkString GrContext::dump() const {
    SkDynamicMemoryWStream stream;
    SkJSONWriter writer(&stream, SkJSONWriter::Mode::kPretty);
    writer.beginObject();

    writer.appendString("backend", GrBackendApiToStr(this->backend()));

    writer.appendName("caps");
    this->caps()->dumpJSON(&writer);

    writer.appendName("gpu");
    this->fGpu->dumpJSON(&writer);

    writer.appendName("context");
    this->dumpJSON(&writer);

    // Flush JSON to the memory stream
    writer.endObject();
    writer.flush();

    // Null terminate the JSON data in the memory stream
    stream.write8(0);

    // Allocate a string big enough to hold all the data, then copy out of the stream
    SkString result(stream.bytesWritten());
    stream.copyToAndReset(result.writable_str());
    return result;
}
#endif
