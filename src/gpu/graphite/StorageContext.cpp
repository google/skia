/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/StorageContext.h"

#include "include/private/SkAlign.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/task/UploadTask.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

namespace skgpu::graphite {

namespace {

sk_sp<TextureProxy> create_texture_proxy(Recorder* recorder, int width, int height) {
    const Caps* caps = recorder->priv().caps();
    TextureInfo info = caps->getDefaultReadableTextureInfo(
            StorageContext::kTextureFormat,
            recorder->priv().isProtected());

    sk_sp<TextureProxy> proxy = TextureProxy::Make(caps,
                                                   recorder->priv().resourceProvider(),
                                                   SkISize::Make(width, height),
                                                   info,
                                                   Budgeted::kYes,
                                                   "StorageFallbackTexture");
    return proxy;
}

}  // anonymous namespace

StorageContext::StorageContext(int maxFallbackTextureSize, bool storageBufferSupport)
        : fRunningLCM(1)
        , fMaxFallbackTextureSize(maxFallbackTextureSize)
        , fStorageBufferSupport(storageBufferSupport) {
    SkASSERT(fMaxFallbackTextureSize > 0);
}

StorageContext::~StorageContext() {
    this->resetCache();
}

void StorageContext::GradientCache::reset() {
    fLocalGradientOffsetCache.foreach([](const SkGradientBaseShader* shader, int*) {
        shader->unref();
    });
    fLocalGradientOffsetCache.reset();
    fGradientData.clear();
    fGradientDataSize = 0;
}

void StorageContext::resetCache() {
    fGradientCache.reset();
    fVertexData.clear();
    fRunningLCM = 1;
    SkDEBUGCODE(fFinalized = false;)
}

std::pair<float*, int> StorageContext::allocateGradientData(int numStops,
                                                            const SkGradientBaseShader* shader) {
    SkASSERT(!fFinalized);
    if (numStops > GradientCache::kMaxGradientStops) {
        return {nullptr, -1};
    }

    int* existingLocalOffset = fGradientCache.fLocalGradientOffsetCache.find(shader);
    if (existingLocalOffset) {
        return {nullptr, *existingLocalOffset};
    }

    int floatOffset = fGradientCache.fGradientData.size();
    const int floatCount = fStorageBufferSupport
            ? (numStops * 5)
            : (SkAlign4(numStops) + numStops * 4);
    SkASSERT(fStorageBufferSupport || SkIsAlign4(floatCount));
    if (GradientCache::kMaxStorageFloats - floatCount < floatOffset) {
        return {nullptr, -1};
    }

    fGradientCache.fGradientData.resize(floatOffset + floatCount);
    float* dstData = fGradientCache.fGradientData.data() + floatOffset;
    memset(dstData, 0, floatCount * sizeof(float));

    shader->ref();
    fGradientCache.fLocalGradientOffsetCache.set(shader, floatOffset);
    fGradientCache.fGradientDataSize = fGradientCache.fGradientData.size_bytes();

    return {dstData, floatOffset};
}

void StorageContext::recordAlignment(size_t stride, size_t align) {
    SkASSERT(stride > 0 && align > 0);
    SkASSERT(!fFinalized);
    if (!fStorageBufferSupport) {
        align = std::max<size_t>(align, kTexelBytes);
        stride = SkAlignTo<size_t>(stride, kTexelBytes);
    }
    uint32_t align32 = BufferAligner::LcmAlignment(SkTo<uint32_t>(align), SkTo<uint32_t>(stride));
    fRunningLCM = BufferAligner::LcmAlignment(fRunningLCM, align32);
}

uint32_t StorageContext::appendVertices(const void* data,
                                        size_t count,
                                        size_t stride,
                                        size_t align) {
    SkASSERT(data && count > 0 && stride > 0 && align > 0);

    size_t paddedStride = stride;
    size_t paddedAlign  = align;
    if (!fStorageBufferSupport) {
        paddedStride = SkAlignTo<size_t>(stride, kTexelBytes);
        paddedAlign  = std::max<size_t>(align, static_cast<size_t>(kTexelBytes));
    }

    uint32_t align32 = BufferAligner::LcmAlignment(SkTo<uint32_t>(paddedAlign),
                                                   SkTo<uint32_t>(paddedStride));
    SkASSERT(fRunningLCM % align32 == 0);

    uint32_t requiredBytes =
            BufferAligner::ValidateCountAndStride(count, paddedStride, /*headroom=*/0, align32);
    if (requiredBytes == 0) {
        return 0;
    }

    uint32_t alignedVertOffset = SkAlignNonPow2(static_cast<uint32_t>(fVertexData.size()), align32);
    if (alignedVertOffset > static_cast<uint32_t>(fVertexData.size())) {
        int padBytes = alignedVertOffset - fVertexData.size();
        memset(fVertexData.append(padBytes), 0, padBytes);
    }

    char* dst = fVertexData.append(requiredBytes);

    // Because the fallback texture format is kRGBA32F (16 bytes per texel), the unpack shader
    // addresses instances by whole texels (index * nTexels). Pad each instance's stride to a
    // 16-byte texel boundary so subsequent instances align with the shader's texel indexing.
    if (stride == paddedStride) {
        memcpy(dst, data, requiredBytes);
    } else {
        const char* src = static_cast<const char*>(data);
        size_t diff = paddedStride - stride;
        for (size_t i = 0; i < count; ++i) {
            memcpy(dst, src, stride);
            memset(dst + stride, 0, diff);
            dst += paddedStride;
            src += stride;
        }
    }

    return SkTo<uint32_t>(fGradientCache.fGradientDataSize) + alignedVertOffset;
}

void StorageContext::finalizePrecachedStorageData() {
    fGradientCache.fGradientDataSize =
            SkAlignNonPow2<size_t>(fGradientCache.fGradientDataSize, fRunningLCM);
    SkDEBUGCODE(fFinalized = true;)
}

std::optional<StorageContextResult> StorageContext::finalize(Recorder* recorder,
                                                             DrawContext* drawContext) {
    SkASSERT(recorder);
    SkASSERT(fFinalized);
    SkDEBUGCODE(fFinalized = false;)

    if (this->isEmpty()) {
        return std::nullopt;
    }

    if (fStorageBufferSupport) {
        if (BindBufferInfo info = this->finalizeStorageBuffer(recorder)) {
            return info;
        }
    } else {
        if (sk_sp<TextureProxy> proxy = this->finalizeTexture(recorder, drawContext)) {
            return proxy;
        }
    }

    return std::nullopt;
}

BindBufferInfo StorageContext::finalizeStorageBuffer(Recorder* recorder) {
    DrawBufferManager* bufferMgr = recorder->priv().drawBufferManager();
    SkASSERT(bufferMgr);

    size_t totalBytes = fGradientCache.fGradientDataSize + fVertexData.size_bytes();

    BindBufferInfo result;
    if (totalBytes > 0) {
        auto [writer, bufferInfo, _] = bufferMgr->getMappedStorageBuffer(totalBytes, /*stride=*/1);
        if (writer) {
            if (!fGradientCache.isEmpty()) {
                writer.write(fGradientCache.fGradientData.data(),
                             fGradientCache.fGradientData.size_bytes());
                if (fGradientCache.fGradientDataSize > fGradientCache.fGradientData.size_bytes()) {
                    writer.zeroBytes(fGradientCache.fGradientDataSize -
                                     fGradientCache.fGradientData.size_bytes());
                }
            }
            if (!fVertexData.empty()) {
                writer.write(fVertexData.data(), fVertexData.size_bytes());
            }

            result = bufferInfo;
        }
    }

    return result;
}

// TODO (thomsmit): Currently UploadSource holds its own copy of the data. Create an alternative
// path for uploading which allows writing to the mapped gpu buffer directly.
sk_sp<TextureProxy> StorageContext::finalizeTexture(Recorder* recorder, DrawContext* drawContext) {
    SkASSERT(drawContext);
    size_t gradSize = fGradientCache.fGradientDataSize;
    size_t vertSize = fVertexData.size_bytes();
    size_t totalBytes = gradSize + vertSize;
    if (totalBytes == 0) {
        return nullptr;
    }

    int totalTexels = (totalBytes + kTexelBytes - 1) / kTexelBytes;
    int width = std::min(totalTexels, fMaxFallbackTextureSize);
    int height = (totalTexels + fMaxFallbackTextureSize - 1) / fMaxFallbackTextureSize;
    if (height > fMaxFallbackTextureSize) {
        return nullptr;
    }

    int atlasRowBytes = width * kTexelBytes;
    size_t paddedBytes = static_cast<size_t>(height) * atlasRowBytes;
    SkTDArray<char> uploadBuffer;
    uploadBuffer.resize(paddedBytes);
    memset(uploadBuffer.data(), 0, paddedBytes);

    if (gradSize > 0) {
        memcpy(uploadBuffer.data(), fGradientCache.fGradientData.data(),
               fGradientCache.fGradientData.size_bytes());
    }

    if (!fVertexData.empty()) {
        memcpy(uploadBuffer.data() + gradSize, fVertexData.data(), fVertexData.size_bytes());
    }

    sk_sp<TextureProxy> proxy = create_texture_proxy(recorder, width, height);
    if (!proxy) {
        return nullptr;
    }

    MipLevel level;
    level.fPixels = uploadBuffer.data();
    level.fRowBytes = atlasRowBytes;
    SkIRect dstRect = SkIRect::MakeWH(width, height);

    Swizzle readSwizzle = ReadSwizzleForColorType(StorageContext::kColorType, proxy->format());
    TextureProxyView proxyView(std::move(proxy), readSwizzle);

    SkColorInfo colorInfo(StorageContext::kColorType, kPremul_SkAlphaType, nullptr);
    UploadSource source = UploadSource::Make(recorder->priv().caps(),
                                             proxyView,
                                             colorInfo,
                                             colorInfo,
                                             SkSpan<const MipLevel>(&level, 1),
                                             dstRect);

    UploadInstance uploadInstance = UploadInstance::Make(recorder, source, nullptr);
    if (!uploadInstance.isValid()) {
        return nullptr;
    }

    sk_sp<Task> uploadTask = UploadTask::Make(std::move(uploadInstance));
    if (!uploadTask) {
        return nullptr;
    }
    drawContext->recordDependency(std::move(uploadTask));

    return proxyView.refProxy();
}

}  // namespace skgpu::graphite
