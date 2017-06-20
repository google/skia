/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOpFlushState.h"

#include "GrDrawOpAtlas.h"
#include "GrGpu.h"
#include "GrResourceProvider.h"
#include "GrTexture.h"

GrOpFlushState::GrOpFlushState(GrGpu* gpu, GrResourceProvider* resourceProvider)
        : fGpu(gpu)
        , fResourceProvider(resourceProvider)
        , fCommandBuffer(nullptr)
        , fVertexPool(gpu)
        , fIndexPool(gpu)
        , fLastIssuedToken(GrDrawOpUploadToken::AlreadyFlushedToken())
        , fLastFlushedToken(0)
        , fOpArgs(nullptr) {}

const GrCaps& GrOpFlushState::caps() const {
    return *fGpu->caps();
}

void* GrOpFlushState::makeVertexSpace(size_t vertexSize, int vertexCount,
                                         const GrBuffer** buffer, int* startVertex) {
    return fVertexPool.makeSpace(vertexSize, vertexCount, buffer, startVertex);
}

uint16_t* GrOpFlushState::makeIndexSpace(int indexCount,
                                            const GrBuffer** buffer, int* startIndex) {
    return reinterpret_cast<uint16_t*>(fIndexPool.makeSpace(indexCount, buffer, startIndex));
}

void* GrOpFlushState::makeVertexSpaceAtLeast(size_t vertexSize, int minVertexCount,
                                             int fallbackVertexCount, const GrBuffer** buffer,
                                             int* startVertex, int* actualVertexCount) {
    return fVertexPool.makeSpaceAtLeast(vertexSize, minVertexCount, fallbackVertexCount, buffer,
                                        startVertex, actualVertexCount);
}

uint16_t* GrOpFlushState::makeIndexSpaceAtLeast(int minIndexCount, int fallbackIndexCount,
                                                const GrBuffer** buffer, int* startIndex,
                                                int* actualIndexCount) {
    return reinterpret_cast<uint16_t*>(fIndexPool.makeSpaceAtLeast(
        minIndexCount, fallbackIndexCount, buffer, startIndex, actualIndexCount));
}

void GrOpFlushState::doUpload(GrDrawOp::DeferredUploadFn& upload) {
    GrDrawOp::WritePixelsFn wp = [this](GrSurface* surface, int left, int top, int width,
                                        int height, GrPixelConfig config, const void* buffer,
                                        size_t rowBytes) {
        GrGpu::DrawPreference drawPreference = GrGpu::kNoDraw_DrawPreference;
        GrGpu::WritePixelTempDrawInfo tempInfo;
        fGpu->getWritePixelsInfo(surface, width, height, surface->config(), &drawPreference,
                                 &tempInfo);
        if (GrGpu::kNoDraw_DrawPreference == drawPreference) {
            return this->fGpu->writePixels(surface, left, top, width, height, config, buffer,
                                           rowBytes);
        }
        GrSurfaceDesc desc;
        desc.fConfig = surface->config();
        desc.fWidth = width;
        desc.fHeight = height;
        desc.fOrigin = surface->origin();
        sk_sp<GrTexture> temp(this->fResourceProvider->createApproxTexture(
                desc, GrResourceProvider::kNoPendingIO_Flag));
        if (!temp) {
            return false;
        }
        if (!fGpu->writePixels(temp.get(), 0, 0, width, height, desc.fConfig, buffer, rowBytes)) {
            return false;
        }
        return fGpu->copySurface(surface, temp.get(), SkIRect::MakeWH(width, height), {left, top});
    };
    upload(wp);
}
