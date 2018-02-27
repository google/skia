/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOpFlushState.h"

#include "GrContextPriv.h"
#include "GrDrawOpAtlas.h"
#include "GrGpu.h"
#include "GrResourceProvider.h"
#include "GrTexture.h"

//////////////////////////////////////////////////////////////////////////////

GrOpFlushState::GrOpFlushState(GrGpu* gpu,
                               GrResourceProvider* resourceProvider,
                               GrTokenTracker* tokenTracker)
        : fVertexPool(gpu)
        , fIndexPool(gpu)
        , fGpu(gpu)
        , fResourceProvider(resourceProvider)
        , fTokenTracker(tokenTracker) {
}

const GrCaps& GrOpFlushState::caps() const {
    return *fGpu->caps();
}

GrGpuRTCommandBuffer* GrOpFlushState::rtCommandBuffer() {
    return fCommandBuffer->asRTCommandBuffer();
}

void GrOpFlushState::executeDrawsAndUploadsForMeshDrawOp(uint32_t opID, const SkRect& opBounds) {
    SkASSERT(this->rtCommandBuffer());
    while (fCurrDraw != fDraws.end() && fCurrDraw->fOpID == opID) {
        GrDeferredUploadToken drawToken = fTokenTracker->nextTokenToFlush();
        while (fCurrUpload != fInlineUploads.end() &&
               fCurrUpload->fUploadBeforeToken == drawToken) {
            this->rtCommandBuffer()->inlineUpload(this, fCurrUpload->fUpload);
            ++fCurrUpload;
        }
        SkASSERT(fCurrDraw->fPipeline->proxy() == this->drawOpArgs().fProxy);
        this->rtCommandBuffer()->draw(*fCurrDraw->fPipeline, *fCurrDraw->fGeometryProcessor,
                                      fMeshes.begin() + fCurrMesh, nullptr, fCurrDraw->fMeshCnt,
                                      opBounds);
        fCurrMesh += fCurrDraw->fMeshCnt;
        fTokenTracker->flushToken();
        ++fCurrDraw;
    }
}

void GrOpFlushState::preExecuteDraws() {
    fVertexPool.unmap();
    fIndexPool.unmap();
    for (auto& upload : fASAPUploads) {
        this->doUpload(upload);
    }
    // Setup execution iterators.
    fCurrDraw = fDraws.begin();
    fCurrUpload = fInlineUploads.begin();
    fCurrMesh = 0;
}

void GrOpFlushState::reset() {
    SkASSERT(fCurrDraw == fDraws.end());
    SkASSERT(fCurrUpload == fInlineUploads.end());
    fVertexPool.reset();
    fIndexPool.reset();
    fArena.reset();
    fASAPUploads.reset();
    fInlineUploads.reset();
    fDraws.reset();
    fMeshes.reset();
    fCurrMesh = 0;
    fBaseDrawToken = GrDeferredUploadToken::AlreadyFlushedToken();
}

void GrOpFlushState::doUpload(GrDeferredTextureUploadFn& upload) {
    GrDeferredTextureUploadWritePixelsFn wp = [this](GrTextureProxy* dstProxy, int left, int top,
                                                     int width, int height,
                                                     GrColorType srcColorType, const void* buffer,
                                                     size_t rowBytes) {
        // We don't allow srgb conversions via op flush state uploads.
        static constexpr auto kSRGBConversion = GrSRGBConversion::kNone;
        GrSurface* dstSurface = dstProxy->priv().peekSurface();
        GrGpu::DrawPreference drawPreference = GrGpu::kNoDraw_DrawPreference;
        GrGpu::WritePixelTempDrawInfo tempInfo;
        if (!fGpu->getWritePixelsInfo(dstSurface, dstProxy->origin(), width, height, srcColorType,
                                      kSRGBConversion, &drawPreference, &tempInfo)) {
            return false;
        }
        if (GrGpu::kNoDraw_DrawPreference == drawPreference) {
            return this->fGpu->writePixels(dstSurface, dstProxy->origin(), left, top, width, height,
                                           srcColorType, buffer, rowBytes);
        }
        // TODO: Shouldn't we be bailing here if a draw is really required instead of a copy?
        // e.g. if (tempInfo.fSwizzle != "RGBA") fail.
        GrSurfaceDesc desc;
        desc.fOrigin = dstProxy->origin();
        desc.fWidth = width;
        desc.fHeight = height;
        desc.fConfig = dstProxy->config();
        sk_sp<GrTexture> temp(this->fResourceProvider->createApproxTexture(
                desc, GrResourceProvider::kNoPendingIO_Flag));
        if (!temp) {
            return false;
        }
        if (!fGpu->writePixels(temp.get(), dstProxy->origin(), 0, 0, width, height,
                               tempInfo.fWriteColorType, buffer, rowBytes)) {
            return false;
        }
        return fGpu->copySurface(dstSurface, dstProxy->origin(), temp.get(), dstProxy->origin(),
                                 SkIRect::MakeWH(width, height), {left, top});
    };
    upload(wp);
}

GrDeferredUploadToken GrOpFlushState::addInlineUpload(GrDeferredTextureUploadFn&& upload) {
    return fInlineUploads.append(&fArena, std::move(upload), fTokenTracker->nextDrawToken())
            .fUploadBeforeToken;
}

GrDeferredUploadToken GrOpFlushState::addASAPUpload(GrDeferredTextureUploadFn&& upload) {
    fASAPUploads.append(&fArena, std::move(upload));
    return fTokenTracker->nextTokenToFlush();
}

void GrOpFlushState::draw(const GrGeometryProcessor* gp, const GrPipeline* pipeline,
                          const GrMesh& mesh) {
    SkASSERT(fOpArgs);
    SkASSERT(fOpArgs->fOp);
    fMeshes.push_back(mesh);
    bool firstDraw = fDraws.begin() == fDraws.end();
    if (!firstDraw) {
        Draw& lastDraw = *fDraws.begin();
        // If the last draw shares a geometry processor and pipeline and there are no intervening
        // uploads, add this mesh to it.
        if (lastDraw.fGeometryProcessor == gp && lastDraw.fPipeline == pipeline) {
            if (fInlineUploads.begin() == fInlineUploads.end() ||
                fInlineUploads.tail()->fUploadBeforeToken != fTokenTracker->nextDrawToken()) {
                ++lastDraw.fMeshCnt;
                return;
            }
        }
    }
    auto& draw = fDraws.append(&fArena);
    GrDeferredUploadToken token = fTokenTracker->issueDrawToken();

    draw.fGeometryProcessor.reset(gp);
    draw.fPipeline = pipeline;
    draw.fMeshCnt = 1;
    draw.fOpID = fOpArgs->fOp->uniqueID();
    if (firstDraw) {
        fBaseDrawToken = token;
    }
}

void* GrOpFlushState::makeVertexSpace(size_t vertexSize, int vertexCount, const GrBuffer** buffer,
                                      int* startVertex) {
    return fVertexPool.makeSpace(vertexSize, vertexCount, buffer, startVertex);
}

uint16_t* GrOpFlushState::makeIndexSpace(int indexCount, const GrBuffer** buffer, int* startIndex) {
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

void GrOpFlushState::putBackIndices(int indexCount) {
    fIndexPool.putBack(indexCount * sizeof(uint16_t));
}

void GrOpFlushState::putBackVertices(int vertices, size_t vertexStride) {
    fVertexPool.putBack(vertices * vertexStride);
}

GrAppliedClip GrOpFlushState::detachAppliedClip() {
    return fOpArgs->fAppliedClip ? std::move(*fOpArgs->fAppliedClip) : GrAppliedClip();
}

GrGlyphCache* GrOpFlushState::glyphCache() const {
    return fGpu->getContext()->contextPriv().getGlyphCache();
}

GrAtlasManager* GrOpFlushState::fullAtlasManager() const {
    return fGpu->getContext()->contextPriv().getFullAtlasManager();
}
