/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrOpFlushState.h"

#include "src/core/SkConvertPixels.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDrawOpAtlas.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrTexture.h"

//////////////////////////////////////////////////////////////////////////////

GrOpFlushState::GrOpFlushState(GrGpu* gpu, GrResourceProvider* resourceProvider,
                               GrTokenTracker* tokenTracker,
                               sk_sp<GrBufferAllocPool::CpuBufferCache> cpuBufferCache)
        : fVertexPool(gpu, cpuBufferCache)
        , fIndexPool(gpu, std::move(cpuBufferCache))
        , fGpu(gpu)
        , fResourceProvider(resourceProvider)
        , fTokenTracker(tokenTracker) {}

const GrCaps& GrOpFlushState::caps() const {
    return *fGpu->caps();
}

void GrOpFlushState::executeDrawsAndUploadsForMeshDrawOp(
        const GrOp* op, const SkRect& chainBounds, const GrPipeline* pipeline) {
    SkASSERT(this->opsRenderPass());

    while (fCurrDraw != fDraws.end() && fCurrDraw->fOp == op) {
        GrDeferredUploadToken drawToken = fTokenTracker->nextTokenToFlush();
        while (fCurrUpload != fInlineUploads.end() &&
               fCurrUpload->fUploadBeforeToken == drawToken) {
            this->opsRenderPass()->inlineUpload(this, fCurrUpload->fUpload);
            ++fCurrUpload;
        }

        GrProgramInfo programInfo(this->proxy()->numSamples(),
                                  this->proxy()->numStencilSamples(),
                                  this->proxy()->backendFormat(),
                                  this->outputView()->origin(),
                                  pipeline,
                                  fCurrDraw->fGeometryProcessor,
                                  fCurrDraw->fPrimitiveType);

        this->bindPipelineAndScissorClip(programInfo, chainBounds);
        this->bindTextures(programInfo.primProc(), fCurrDraw->fPrimProcProxies,
                           programInfo.pipeline());
        for (int i = 0; i < fCurrDraw->fMeshCnt; ++i) {
            this->drawMesh(fCurrDraw->fMeshes[i]);
        }

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
    fBaseDrawToken = GrDeferredUploadToken::AlreadyFlushedToken();
}

void GrOpFlushState::doUpload(GrDeferredTextureUploadFn& upload,
                              bool shouldPrepareSurfaceForSampling) {
    GrDeferredTextureUploadWritePixelsFn wp = [this, shouldPrepareSurfaceForSampling](
            GrTextureProxy* dstProxy, int left, int top, int width, int height,
            GrColorType colorType, const void* buffer, size_t rowBytes) {
        GrSurface* dstSurface = dstProxy->peekSurface();
        if (!fGpu->caps()->surfaceSupportsWritePixels(dstSurface)) {
            return false;
        }
        GrCaps::SupportedWrite supportedWrite = fGpu->caps()->supportedWritePixelsColorType(
                colorType, dstSurface->backendFormat(), colorType);
        size_t tightRB = width * GrColorTypeBytesPerPixel(supportedWrite.fColorType);
        SkASSERT(rowBytes >= tightRB);
        std::unique_ptr<char[]> tmpPixels;
        if (supportedWrite.fColorType != colorType ||
            (!fGpu->caps()->writePixelsRowBytesSupport() && rowBytes != tightRB)) {
            tmpPixels.reset(new char[height * tightRB]);
            // Use kUnpremul to ensure no alpha type conversions or clamping occur.
            static constexpr auto kAT = kUnpremul_SkAlphaType;
            GrImageInfo srcInfo(colorType, kAT, nullptr, width, height);
            GrImageInfo tmpInfo(supportedWrite.fColorType, kAT, nullptr, width,
                                height);
            if (!GrConvertPixels(tmpInfo, tmpPixels.get(), tightRB, srcInfo, buffer, rowBytes)) {
                return false;
            }
            rowBytes = tightRB;
            buffer = tmpPixels.get();
        }
        return this->fGpu->writePixels(dstSurface, left, top, width, height, colorType,
                                       supportedWrite.fColorType, buffer, rowBytes,
                                       shouldPrepareSurfaceForSampling);
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

void GrOpFlushState::recordDraw(
        const GrGeometryProcessor* gp,
        const GrSimpleMesh meshes[],
        int meshCnt,
        const GrSurfaceProxy* const primProcProxies[],
        GrPrimitiveType primitiveType) {
    SkASSERT(fOpArgs);
    SkDEBUGCODE(fOpArgs->validate());
    bool firstDraw = fDraws.begin() == fDraws.end();
    auto& draw = fDraws.append(&fArena);
    GrDeferredUploadToken token = fTokenTracker->issueDrawToken();
    for (int i = 0; i < gp->numTextureSamplers(); ++i) {
        SkASSERT(primProcProxies && primProcProxies[i]);
        primProcProxies[i]->ref();
    }
    draw.fGeometryProcessor = gp;
    draw.fPrimProcProxies = primProcProxies;
    draw.fMeshes = meshes;
    draw.fMeshCnt = meshCnt;
    draw.fOp = fOpArgs->op();
    draw.fPrimitiveType = primitiveType;
    if (firstDraw) {
        fBaseDrawToken = token;
    }
}

void* GrOpFlushState::makeVertexSpace(size_t vertexSize, int vertexCount,
                                      sk_sp<const GrBuffer>* buffer, int* startVertex) {
    return fVertexPool.makeSpace(vertexSize, vertexCount, buffer, startVertex);
}

uint16_t* GrOpFlushState::makeIndexSpace(int indexCount, sk_sp<const GrBuffer>* buffer,
                                         int* startIndex) {
    return reinterpret_cast<uint16_t*>(fIndexPool.makeSpace(indexCount, buffer, startIndex));
}

void* GrOpFlushState::makeVertexSpaceAtLeast(size_t vertexSize, int minVertexCount,
                                             int fallbackVertexCount, sk_sp<const GrBuffer>* buffer,
                                             int* startVertex, int* actualVertexCount) {
    return fVertexPool.makeSpaceAtLeast(vertexSize, minVertexCount, fallbackVertexCount, buffer,
                                        startVertex, actualVertexCount);
}

uint16_t* GrOpFlushState::makeIndexSpaceAtLeast(int minIndexCount, int fallbackIndexCount,
                                                sk_sp<const GrBuffer>* buffer, int* startIndex,
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
    return fOpArgs->appliedClip() ? std::move(*fOpArgs->appliedClip()) : GrAppliedClip();
}

GrStrikeCache* GrOpFlushState::glyphCache() const {
    return fGpu->getContext()->priv().getGrStrikeCache();
}

GrAtlasManager* GrOpFlushState::atlasManager() const {
    return fGpu->getContext()->priv().getAtlasManager();
}

void GrOpFlushState::drawMesh(const GrSimpleMesh& mesh) {
    SkASSERT(mesh.fIsInitialized);
    if (!mesh.fIndexBuffer) {
        this->bindBuffers(nullptr, nullptr, mesh.fVertexBuffer.get());
        this->draw(mesh.fVertexCount, mesh.fBaseVertex);
    } else {
        this->bindBuffers(mesh.fIndexBuffer.get(), nullptr, mesh.fVertexBuffer.get(),
                          mesh.fPrimitiveRestart);
        if (0 == mesh.fPatternRepeatCount) {
            this->drawIndexed(mesh.fIndexCount, mesh.fBaseIndex, mesh.fMinIndexValue,
                              mesh.fMaxIndexValue, mesh.fBaseVertex);
        } else {
            this->drawIndexPattern(mesh.fIndexCount, mesh.fPatternRepeatCount,
                                   mesh.fMaxPatternRepetitionsInIndexBuffer, mesh.fVertexCount,
                                   mesh.fBaseVertex);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

GrOpFlushState::Draw::~Draw() {
    for (int i = 0; i < fGeometryProcessor->numTextureSamplers(); ++i) {
        SkASSERT(fPrimProcProxies && fPrimProcProxies[i]);
        fPrimProcProxies[i]->unref();
    }
}
