/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrResourceProvider.h"

#include "GrBuffer.h"
#include "GrGpu.h"
#include "GrPathRendering.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetPriv.h"
#include "GrResourceCache.h"
#include "GrResourceKey.h"
#include "GrStencilAttachment.h"

GR_DECLARE_STATIC_UNIQUE_KEY(gQuadIndexBufferKey);

GrResourceProvider::GrResourceProvider(GrGpu* gpu, GrResourceCache* cache, GrSingleOwner* owner)
    : INHERITED(gpu, cache, owner) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gQuadIndexBufferKey);
    fQuadIndexBufferKey = gQuadIndexBufferKey;
}

const GrBuffer* GrResourceProvider::createInstancedIndexBuffer(const uint16_t* pattern,
                                                               int patternSize,
                                                               int reps,
                                                               int vertCount,
                                                               const GrUniqueKey& key) {
    size_t bufferSize = patternSize * reps * sizeof(uint16_t);

    // This is typically used in GrBatchs, so we assume kNoPendingIO.
    GrBuffer* buffer = this->createBuffer(bufferSize, kIndex_GrBufferType, kStatic_GrAccessPattern,
                                          kNoPendingIO_Flag);
    if (!buffer) {
        return nullptr;
    }
    uint16_t* data = (uint16_t*) buffer->map();
    bool useTempData = (nullptr == data);
    if (useTempData) {
        data = new uint16_t[reps * patternSize];
    }
    for (int i = 0; i < reps; ++i) {
        int baseIdx = i * patternSize;
        uint16_t baseVert = (uint16_t)(i * vertCount);
        for (int j = 0; j < patternSize; ++j) {
            data[baseIdx+j] = baseVert + pattern[j];
        }
    }
    if (useTempData) {
        if (!buffer->updateData(data, bufferSize)) {
            buffer->unref();
            return nullptr;
        }
        delete[] data;
    } else {
        buffer->unmap();
    }
    this->assignUniqueKeyToResource(key, buffer);
    return buffer;
}

const GrBuffer* GrResourceProvider::createQuadIndexBuffer() {
    static const int kMaxQuads = 1 << 12; // max possible: (1 << 14) - 1;
    GR_STATIC_ASSERT(4 * kMaxQuads <= 65535);
    static const uint16_t kPattern[] = { 0, 1, 2, 0, 2, 3 };

    return this->createInstancedIndexBuffer(kPattern, 6, kMaxQuads, 4, fQuadIndexBufferKey);
}

GrPath* GrResourceProvider::createPath(const SkPath& path, const GrStrokeInfo& stroke) {
    SkASSERT(this->gpu()->pathRendering());
    return this->gpu()->pathRendering()->createPath(path, stroke);
}

GrPathRange* GrResourceProvider::createPathRange(GrPathRange::PathGenerator* gen,
                                                 const GrStrokeInfo& stroke) {
    SkASSERT(this->gpu()->pathRendering());
    return this->gpu()->pathRendering()->createPathRange(gen, stroke);
}

GrPathRange* GrResourceProvider::createGlyphs(const SkTypeface* tf, const SkDescriptor* desc,
                                              const GrStrokeInfo& stroke) {

    SkASSERT(this->gpu()->pathRendering());
    return this->gpu()->pathRendering()->createGlyphs(tf, desc, stroke);
}

GrBuffer* GrResourceProvider::createBuffer(size_t size, GrBufferType intendedType,
                                           GrAccessPattern accessPattern, uint32_t flags) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    if (kDynamic_GrAccessPattern == accessPattern) {
        // bin by pow2 with a reasonable min
        static const uint32_t MIN_SIZE = 1 << 12;
        size = SkTMax(MIN_SIZE, GrNextPow2(SkToUInt(size)));

        GrScratchKey key;
        GrBuffer::ComputeScratchKeyForDynamicBuffer(size, intendedType, &key);
        uint32_t scratchFlags = 0;
        if (flags & kNoPendingIO_Flag) {
            scratchFlags = GrResourceCache::kRequireNoPendingIO_ScratchFlag;
        } else {
            scratchFlags = GrResourceCache::kPreferNoPendingIO_ScratchFlag;
        }
        GrGpuResource* resource = this->cache()->findAndRefScratchResource(key, size, scratchFlags);
        if (resource) {
            return static_cast<GrBuffer*>(resource);
        }
    }
    return this->gpu()->createBuffer(size, intendedType, accessPattern);
}

GrBatchAtlas* GrResourceProvider::createAtlas(GrPixelConfig config,
                                              int width, int height,
                                              int numPlotsX, int numPlotsY,
                                              GrBatchAtlas::EvictionFunc func, void* data) {
    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = config;

    // We don't want to flush the context so we claim we're in the middle of flushing so as to
    // guarantee we do not recieve a texture with pending IO
    // TODO: Determine how to avoid having to do this. (https://bug.skia.org/4156)
    static const uint32_t kFlags = GrResourceProvider::kNoPendingIO_Flag;
    GrTexture* texture = this->createApproxTexture(desc, kFlags);
    if (!texture) {
        return nullptr;
    }
    GrBatchAtlas* atlas = new GrBatchAtlas(texture, numPlotsX, numPlotsY);
    atlas->registerEvictionCallback(func, data);
    return atlas;
}

GrStencilAttachment* GrResourceProvider::attachStencilAttachment(GrRenderTarget* rt) {
    SkASSERT(rt);
    if (rt->renderTargetPriv().getStencilAttachment()) {
        return rt->renderTargetPriv().getStencilAttachment();
    }

    if (!rt->wasDestroyed() && rt->canAttemptStencilAttachment()) {
        GrUniqueKey sbKey;

        int width = rt->width();
        int height = rt->height();
#if 0
        if (this->caps()->oversizedStencilSupport()) {
            width  = SkNextPow2(width);
            height = SkNextPow2(height);
        }
#endif
        bool newStencil = false;
        GrStencilAttachment::ComputeSharedStencilAttachmentKey(width, height,
                                                               rt->numStencilSamples(), &sbKey);
        GrStencilAttachment* stencil = static_cast<GrStencilAttachment*>(
            this->findAndRefResourceByUniqueKey(sbKey));
        if (!stencil) {
            // Need to try and create a new stencil
            stencil = this->gpu()->createStencilAttachmentForRenderTarget(rt, width, height);
            if (stencil) {
                stencil->resourcePriv().setUniqueKey(sbKey);
                newStencil = true;
            }
        }
        if (rt->renderTargetPriv().attachStencilAttachment(stencil)) {
            if (newStencil) {
                // Right now we're clearing the stencil attachment here after it is
                // attached to a RT for the first time. When we start matching
                // stencil buffers with smaller color targets this will no longer
                // be correct because it won't be guaranteed to clear the entire
                // sb.
                // We used to clear down in the GL subclass using a special purpose
                // FBO. But iOS doesn't allow a stencil-only FBO. It reports unsupported
                // FBO status.
                this->gpu()->clearStencil(rt);
            }
        }
    }
    return rt->renderTargetPriv().getStencilAttachment();
}

GrRenderTarget* GrResourceProvider::wrapBackendTextureAsRenderTarget(
        const GrBackendTextureDesc& desc) {
    if (this->isAbandoned()) {
        return nullptr;
    }
    return this->gpu()->wrapBackendTextureAsRenderTarget(desc);
}
