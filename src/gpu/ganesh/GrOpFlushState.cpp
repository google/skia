/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/GrOpFlushState.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkRect.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDataUtils.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrPixmap.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/GrSimpleMesh.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrTextureProxy.h"

#include <functional>
#include <memory>

//////////////////////////////////////////////////////////////////////////////

GrOpFlushState::GrOpFlushState(GrGpu* gpu, GrResourceProvider* resourceProvider,
                               skgpu::TokenTracker* tokenTracker,
                               sk_sp<GrBufferAllocPool::CpuBufferCache> cpuBufferCache)
        : fVertexPool(gpu, cpuBufferCache)
        , fIndexPool(gpu, cpuBufferCache)
        , fDrawIndirectPool(gpu, std::move(cpuBufferCache))
        , fGpu(gpu)
        , fResourceProvider(resourceProvider)
        , fTokenTracker(tokenTracker) {}

const GrCaps& GrOpFlushState::caps() const {
    return *fGpu->caps();
}

GrThreadSafeCache* GrOpFlushState::threadSafeCache() const {
    return fGpu->getContext()->priv().threadSafeCache();
}

void GrOpFlushState::executeDrawsAndUploadsForMeshDrawOp(
        const GrOp* op, const SkRect& chainBounds, const GrPipeline* pipeline,
        const GrUserStencilSettings* userStencilSettings) {
    SkASSERT(this->opsRenderPass());

    while (fCurrDraw != fDraws.end() && fCurrDraw->fOp == op) {
        skgpu::AtlasToken drawToken = fTokenTracker->nextFlushToken();
        while (fCurrUpload != fInlineUploads.end() &&
               fCurrUpload->fUploadBeforeToken == drawToken) {
            this->opsRenderPass()->inlineUpload(this, fCurrUpload->fUpload);
            ++fCurrUpload;
        }

        GrProgramInfo programInfo(this->caps(),
                                  this->writeView(),
                                  this->usesMSAASurface(),
                                  pipeline,
                                  userStencilSettings,
                                  fCurrDraw->fGeometryProcessor,
                                  fCurrDraw->fPrimitiveType,
                                  this->renderPassBarriers(),
                                  this->colorLoadOp());

        this->bindPipelineAndScissorClip(programInfo, chainBounds);
        this->bindTextures(programInfo.geomProc(), fCurrDraw->fGeomProcProxies,
                           programInfo.pipeline());
        for (int i = 0; i < fCurrDraw->fMeshCnt; ++i) {
            this->drawMesh(fCurrDraw->fMeshes[i]);
        }

        fTokenTracker->issueFlushToken();
        ++fCurrDraw;
    }
}

void GrOpFlushState::preExecuteDraws() {
    fVertexPool.unmap();
    fIndexPool.unmap();
    fDrawIndirectPool.unmap();
    for (auto& upload : fASAPUploads) {
        this->doUpload(upload);
    }
    // Setup execution iterators.
    fCurrDraw = fDraws.begin();
    fCurrUpload = fInlineUploads.begin();
    fGpu->willExecute();
}

void GrOpFlushState::reset() {
    SkASSERT(fCurrDraw == fDraws.end());
    SkASSERT(fCurrUpload == fInlineUploads.end());
    fVertexPool.reset();
    fIndexPool.reset();
    fDrawIndirectPool.reset();
    fArena.reset();
    fASAPUploads.reset();
    fInlineUploads.reset();
    fDraws.reset();
    fBaseDrawToken = skgpu::AtlasToken::InvalidToken();
}

void GrOpFlushState::doUpload(GrDeferredTextureUploadFn& upload,
                              bool shouldPrepareSurfaceForSampling) {
    GrDeferredTextureUploadWritePixelsFn wp = [this, shouldPrepareSurfaceForSampling](
                                                      GrTextureProxy* dstProxy,
                                                      SkIRect rect,
                                                      GrColorType colorType,
                                                      const void* buffer,
                                                      size_t rowBytes) {
        GrSurface* dstSurface = dstProxy->peekSurface();
        if (!fGpu->caps()->surfaceSupportsWritePixels(dstSurface)) {
            return false;
        }
        GrCaps::SupportedWrite supportedWrite = fGpu->caps()->supportedWritePixelsColorType(
                colorType, dstSurface->backendFormat(), colorType);
        size_t tightRB = rect.width()*GrColorTypeBytesPerPixel(supportedWrite.fColorType);
        SkASSERT(rowBytes >= tightRB);
        std::unique_ptr<char[]> tmpPixels;
        if (supportedWrite.fColorType != colorType ||
            (!fGpu->caps()->writePixelsRowBytesSupport() && rowBytes != tightRB)) {
            tmpPixels.reset(new char[rect.height()*tightRB]);
            // Use kUnknown to ensure no alpha type conversions or clamping occur.
            static constexpr auto kAT = kUnknown_SkAlphaType;
            GrImageInfo srcInfo(colorType,                 kAT, nullptr, rect.size());
            GrImageInfo tmpInfo(supportedWrite.fColorType, kAT, nullptr, rect.size());
            if (!GrConvertPixels( GrPixmap(tmpInfo, tmpPixels.get(), tightRB ),
                                 GrCPixmap(srcInfo,          buffer, rowBytes))) {
                return false;
            }
            rowBytes = tightRB;
            buffer = tmpPixels.get();
        }
        return this->fGpu->writePixels(dstSurface,
                                       rect,
                                       colorType,
                                       supportedWrite.fColorType,
                                       buffer,
                                       rowBytes,
                                       shouldPrepareSurfaceForSampling);
    };
    upload(wp);
}

skgpu::AtlasToken GrOpFlushState::addInlineUpload(GrDeferredTextureUploadFn&& upload) {
    return fInlineUploads.append(&fArena, std::move(upload), fTokenTracker->nextDrawToken())
            .fUploadBeforeToken;
}

skgpu::AtlasToken GrOpFlushState::addASAPUpload(GrDeferredTextureUploadFn&& upload) {
    fASAPUploads.append(&fArena, std::move(upload));
    return fTokenTracker->nextFlushToken();
}

void GrOpFlushState::recordDraw(
        const GrGeometryProcessor* geomProc,
        const GrSimpleMesh meshes[],
        int meshCnt,
        const GrSurfaceProxy* const geomProcProxies[],
        GrPrimitiveType primitiveType) {
    SkASSERT(fOpArgs);
    SkDEBUGCODE(fOpArgs->validate());
    bool firstDraw = fDraws.begin() == fDraws.end();
    auto& draw = fDraws.append(&fArena);
    skgpu::AtlasToken token = fTokenTracker->issueDrawToken();
    for (int i = 0; i < geomProc->numTextureSamplers(); ++i) {
        SkASSERT(geomProcProxies && geomProcProxies[i]);
        geomProcProxies[i]->ref();
    }
    draw.fGeometryProcessor = geomProc;
    draw.fGeomProcProxies = geomProcProxies;
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
    return fOpArgs->appliedClip() ? std::move(*fOpArgs->appliedClip()) : GrAppliedClip::Disabled();
}

sktext::gpu::StrikeCache* GrOpFlushState::strikeCache() const {
    return fGpu->getContext()->priv().getStrikeCache();
}

GrAtlasManager* GrOpFlushState::atlasManager() const {
    return fGpu->getContext()->priv().getAtlasManager();
}

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
skgpu::ganesh::SmallPathAtlasMgr* GrOpFlushState::smallPathAtlasManager() const {
    return fGpu->getContext()->priv().getSmallPathAtlasMgr();
}
#endif

void GrOpFlushState::drawMesh(const GrSimpleMesh& mesh) {
    SkASSERT(mesh.fIsInitialized);
    if (!mesh.fIndexBuffer) {
        this->bindBuffers(nullptr, nullptr, mesh.fVertexBuffer);
        this->draw(mesh.fVertexCount, mesh.fBaseVertex);
    } else {
        this->bindBuffers(mesh.fIndexBuffer, nullptr, mesh.fVertexBuffer, mesh.fPrimitiveRestart);
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
        SkASSERT(fGeomProcProxies && fGeomProcProxies[i]);
        fGeomProcProxies[i]->unref();
    }
}
