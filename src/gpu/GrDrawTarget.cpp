
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawTarget.h"

#include "GrBatch.h"
#include "GrContext.h"
#include "GrDrawTargetCaps.h"
#include "GrPath.h"
#include "GrPipeline.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetPriv.h"
#include "GrSurfacePriv.h"
#include "GrTemplates.h"
#include "GrTexture.h"
#include "GrVertexBuffer.h"

#include "SkStrokeRec.h"

////////////////////////////////////////////////////////////////////////////////

GrDrawTarget::DrawInfo& GrDrawTarget::DrawInfo::operator =(const DrawInfo& di) {
    fPrimitiveType  = di.fPrimitiveType;
    fStartVertex    = di.fStartVertex;
    fStartIndex     = di.fStartIndex;
    fVertexCount    = di.fVertexCount;
    fIndexCount     = di.fIndexCount;

    fInstanceCount          = di.fInstanceCount;
    fVerticesPerInstance    = di.fVerticesPerInstance;
    fIndicesPerInstance     = di.fIndicesPerInstance;

    if (di.fDevBounds) {
        SkASSERT(di.fDevBounds == &di.fDevBoundsStorage);
        fDevBoundsStorage = di.fDevBoundsStorage;
        fDevBounds = &fDevBoundsStorage;
    } else {
        fDevBounds = NULL;
    }

    this->setVertexBuffer(di.vertexBuffer());
    this->setIndexBuffer(di.indexBuffer());

    return *this;
}

#ifdef SK_DEBUG
bool GrDrawTarget::DrawInfo::isInstanced() const {
    if (fInstanceCount > 0) {
        SkASSERT(0 == fIndexCount % fIndicesPerInstance);
        SkASSERT(0 == fVertexCount % fVerticesPerInstance);
        SkASSERT(fIndexCount / fIndicesPerInstance == fInstanceCount);
        SkASSERT(fVertexCount / fVerticesPerInstance == fInstanceCount);
        // there is no way to specify a non-zero start index to drawIndexedInstances().
        SkASSERT(0 == fStartIndex);
        return true;
    } else {
        SkASSERT(!fVerticesPerInstance);
        SkASSERT(!fIndicesPerInstance);
        return false;
    }
}
#endif

void GrDrawTarget::DrawInfo::adjustInstanceCount(int instanceOffset) {
    SkASSERT(this->isInstanced());
    SkASSERT(instanceOffset + fInstanceCount >= 0);
    fInstanceCount += instanceOffset;
    fVertexCount = fVerticesPerInstance * fInstanceCount;
    fIndexCount = fIndicesPerInstance * fInstanceCount;
}

void GrDrawTarget::DrawInfo::adjustStartVertex(int vertexOffset) {
    fStartVertex += vertexOffset;
    SkASSERT(fStartVertex >= 0);
}

void GrDrawTarget::DrawInfo::adjustStartIndex(int indexOffset) {
    SkASSERT(this->isIndexed());
    fStartIndex += indexOffset;
    SkASSERT(fStartIndex >= 0);
}

////////////////////////////////////////////////////////////////////////////////

#define DEBUG_INVAL_BUFFER 0xdeadcafe
#define DEBUG_INVAL_START_IDX -1

GrDrawTarget::GrDrawTarget(GrContext* context)
    : fContext(context)
    , fGpuTraceMarkerCount(0) {
    SkASSERT(context);
    GeometrySrcState& geoSrc = fGeoSrcStateStack.push_back();
#ifdef SK_DEBUG
    geoSrc.fVertexCount = DEBUG_INVAL_START_IDX;
    geoSrc.fVertexBuffer = (GrVertexBuffer*)DEBUG_INVAL_BUFFER;
    geoSrc.fIndexCount = DEBUG_INVAL_START_IDX;
    geoSrc.fIndexBuffer = (GrIndexBuffer*)DEBUG_INVAL_BUFFER;
#endif
    geoSrc.fVertexSrc = kNone_GeometrySrcType;
    geoSrc.fIndexSrc  = kNone_GeometrySrcType;
}

GrDrawTarget::~GrDrawTarget() {
    SkASSERT(1 == fGeoSrcStateStack.count());
    SkDEBUGCODE(GeometrySrcState& geoSrc = fGeoSrcStateStack.back());
    SkASSERT(kNone_GeometrySrcType == geoSrc.fIndexSrc);
    SkASSERT(kNone_GeometrySrcType == geoSrc.fVertexSrc);
}

void GrDrawTarget::releaseGeometry() {
    int popCnt = fGeoSrcStateStack.count() - 1;
    while (popCnt) {
        this->popGeometrySource();
        --popCnt;
    }
    this->resetVertexSource();
    this->resetIndexSource();
}

bool GrDrawTarget::reserveVertexSpace(size_t vertexSize,
                                      int vertexCount,
                                      void** vertices) {
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    bool acquired = false;
    if (vertexCount > 0) {
        SkASSERT(vertices);
        this->releasePreviousVertexSource();
        geoSrc.fVertexSrc = kNone_GeometrySrcType;

        acquired = this->onReserveVertexSpace(vertexSize,
                                              vertexCount,
                                              vertices);
    }
    if (acquired) {
        geoSrc.fVertexSrc = kReserved_GeometrySrcType;
        geoSrc.fVertexCount = vertexCount;
        geoSrc.fVertexSize = vertexSize;
    } else if (vertices) {
        *vertices = NULL;
    }
    return acquired;
}

bool GrDrawTarget::reserveIndexSpace(int indexCount,
                                     void** indices) {
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    bool acquired = false;
    if (indexCount > 0) {
        SkASSERT(indices);
        this->releasePreviousIndexSource();
        geoSrc.fIndexSrc = kNone_GeometrySrcType;

        acquired = this->onReserveIndexSpace(indexCount, indices);
    }
    if (acquired) {
        geoSrc.fIndexSrc = kReserved_GeometrySrcType;
        geoSrc.fIndexCount = indexCount;
    } else if (indices) {
        *indices = NULL;
    }
    return acquired;

}

bool GrDrawTarget::reserveVertexAndIndexSpace(int vertexCount,
                                              size_t vertexStride,
                                              int indexCount,
                                              void** vertices,
                                              void** indices) {
    this->willReserveVertexAndIndexSpace(vertexCount, vertexStride, indexCount);
    if (vertexCount) {
        if (!this->reserveVertexSpace(vertexStride, vertexCount, vertices)) {
            if (indexCount) {
                this->resetIndexSource();
            }
            return false;
        }
    }
    if (indexCount) {
        if (!this->reserveIndexSpace(indexCount, indices)) {
            if (vertexCount) {
                this->resetVertexSource();
            }
            return false;
        }
    }
    return true;
}

bool GrDrawTarget::geometryHints(size_t vertexStride,
                                 int32_t* vertexCount,
                                 int32_t* indexCount) const {
    if (vertexCount) {
        *vertexCount = -1;
    }
    if (indexCount) {
        *indexCount = -1;
    }
    return false;
}

void GrDrawTarget::releasePreviousVertexSource() {
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    switch (geoSrc.fVertexSrc) {
        case kNone_GeometrySrcType:
            break;
        case kReserved_GeometrySrcType:
            this->releaseReservedVertexSpace();
            break;
        case kBuffer_GeometrySrcType:
            geoSrc.fVertexBuffer->unref();
#ifdef SK_DEBUG
            geoSrc.fVertexBuffer = (GrVertexBuffer*)DEBUG_INVAL_BUFFER;
#endif
            break;
        default:
            SkFAIL("Unknown Vertex Source Type.");
            break;
    }
}

void GrDrawTarget::releasePreviousIndexSource() {
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    switch (geoSrc.fIndexSrc) {
        case kNone_GeometrySrcType:   // these two don't require
            break;
        case kReserved_GeometrySrcType:
            this->releaseReservedIndexSpace();
            break;
        case kBuffer_GeometrySrcType:
            geoSrc.fIndexBuffer->unref();
#ifdef SK_DEBUG
            geoSrc.fIndexBuffer = (GrIndexBuffer*)DEBUG_INVAL_BUFFER;
#endif
            break;
        default:
            SkFAIL("Unknown Index Source Type.");
            break;
    }
}

void GrDrawTarget::setVertexSourceToBuffer(const GrVertexBuffer* buffer, size_t vertexStride) {
    this->releasePreviousVertexSource();
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    geoSrc.fVertexSrc    = kBuffer_GeometrySrcType;
    geoSrc.fVertexBuffer = buffer;
    buffer->ref();
    geoSrc.fVertexSize = vertexStride;
}

void GrDrawTarget::setIndexSourceToBuffer(const GrIndexBuffer* buffer) {
    this->releasePreviousIndexSource();
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    geoSrc.fIndexSrc     = kBuffer_GeometrySrcType;
    geoSrc.fIndexBuffer  = buffer;
    buffer->ref();
}

void GrDrawTarget::resetVertexSource() {
    this->releasePreviousVertexSource();
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    geoSrc.fVertexSrc = kNone_GeometrySrcType;
}

void GrDrawTarget::resetIndexSource() {
    this->releasePreviousIndexSource();
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    geoSrc.fIndexSrc = kNone_GeometrySrcType;
}

void GrDrawTarget::pushGeometrySource() {
    this->geometrySourceWillPush();
    GeometrySrcState& newState = fGeoSrcStateStack.push_back();
    newState.fIndexSrc = kNone_GeometrySrcType;
    newState.fVertexSrc = kNone_GeometrySrcType;
#ifdef SK_DEBUG
    newState.fVertexCount  = ~0;
    newState.fVertexBuffer = (GrVertexBuffer*)~0;
    newState.fIndexCount   = ~0;
    newState.fIndexBuffer = (GrIndexBuffer*)~0;
#endif
}

void GrDrawTarget::popGeometrySource() {
    // if popping last element then pops are unbalanced with pushes
    SkASSERT(fGeoSrcStateStack.count() > 1);

    this->geometrySourceWillPop(fGeoSrcStateStack.fromBack(1));
    this->releasePreviousVertexSource();
    this->releasePreviousIndexSource();
    fGeoSrcStateStack.pop_back();
}

////////////////////////////////////////////////////////////////////////////////

bool GrDrawTarget::checkDraw(const GrPipelineBuilder& pipelineBuilder,
                             const GrGeometryProcessor* gp,
                             GrPrimitiveType type,
                             int startVertex,
                             int startIndex,
                             int vertexCount,
                             int indexCount) const {
#ifdef SK_DEBUG
    const GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    int maxVertex = startVertex + vertexCount;
    int maxValidVertex;
    switch (geoSrc.fVertexSrc) {
        case kNone_GeometrySrcType:
            SkFAIL("Attempting to draw without vertex src.");
        case kReserved_GeometrySrcType: // fallthrough
            maxValidVertex = geoSrc.fVertexCount;
            break;
        case kBuffer_GeometrySrcType:
            maxValidVertex = static_cast<int>(geoSrc.fVertexBuffer->gpuMemorySize() /
                                              geoSrc.fVertexSize);
            break;
    }
    if (maxVertex > maxValidVertex) {
        SkFAIL("Drawing outside valid vertex range.");
    }
    if (indexCount > 0) {
        int maxIndex = startIndex + indexCount;
        int maxValidIndex;
        switch (geoSrc.fIndexSrc) {
            case kNone_GeometrySrcType:
                SkFAIL("Attempting to draw indexed geom without index src.");
            case kReserved_GeometrySrcType: // fallthrough
                maxValidIndex = geoSrc.fIndexCount;
                break;
            case kBuffer_GeometrySrcType:
                maxValidIndex = static_cast<int>(geoSrc.fIndexBuffer->gpuMemorySize() /
                                                 sizeof(uint16_t));
                break;
        }
        if (maxIndex > maxValidIndex) {
            SkFAIL("Index reads outside valid index range.");
        }
    }

    SkASSERT(pipelineBuilder.getRenderTarget());

    if (gp) {
        int numTextures = gp->numTextures();
        for (int t = 0; t < numTextures; ++t) {
            GrTexture* texture = gp->texture(t);
            SkASSERT(texture->asRenderTarget() != pipelineBuilder.getRenderTarget());
        }
    }

    for (int s = 0; s < pipelineBuilder.numColorFragmentStages(); ++s) {
        const GrProcessor* effect = pipelineBuilder.getColorFragmentStage(s).processor();
        int numTextures = effect->numTextures();
        for (int t = 0; t < numTextures; ++t) {
            GrTexture* texture = effect->texture(t);
            SkASSERT(texture->asRenderTarget() != pipelineBuilder.getRenderTarget());
        }
    }
    for (int s = 0; s < pipelineBuilder.numCoverageFragmentStages(); ++s) {
        const GrProcessor* effect = pipelineBuilder.getCoverageFragmentStage(s).processor();
        int numTextures = effect->numTextures();
        for (int t = 0; t < numTextures; ++t) {
            GrTexture* texture = effect->texture(t);
            SkASSERT(texture->asRenderTarget() != pipelineBuilder.getRenderTarget());
        }
    }

#endif
    if (NULL == pipelineBuilder.getRenderTarget()) {
        return false;
    }
    return true;
}

bool GrDrawTarget::setupDstReadIfNecessary(const GrPipelineBuilder& pipelineBuilder,
                                           const GrProcOptInfo& colorPOI,
                                           const GrProcOptInfo& coveragePOI,
                                           GrDeviceCoordTexture* dstCopy,
                                           const SkRect* drawBounds) {
    if (!pipelineBuilder.willXPNeedDstCopy(*this->caps(), colorPOI, coveragePOI)) {
        return true;
    }
    SkIRect copyRect;
    GrRenderTarget* rt = pipelineBuilder.getRenderTarget();
    pipelineBuilder.clip().getConservativeBounds(rt, &copyRect);

    if (drawBounds) {
        SkIRect drawIBounds;
        drawBounds->roundOut(&drawIBounds);
        if (!copyRect.intersect(drawIBounds)) {
#ifdef SK_DEBUG
            SkDebugf("Missed an early reject. Bailing on draw from setupDstReadIfNecessary.\n");
#endif
            return false;
        }
    } else {
#ifdef SK_DEBUG
        //SkDebugf("No dev bounds when dst copy is made.\n");
#endif
    }

    // MSAA consideration: When there is support for reading MSAA samples in the shader we could
    // have per-sample dst values by making the copy multisampled.
    GrSurfaceDesc desc;
    this->initCopySurfaceDstDesc(rt, &desc);
    desc.fWidth = copyRect.width();
    desc.fHeight = copyRect.height();

    SkAutoTUnref<GrTexture> copy(
        fContext->refScratchTexture(desc, GrContext::kApprox_ScratchTexMatch));

    if (!copy) {
        SkDebugf("Failed to create temporary copy of destination texture.\n");
        return false;
    }
    SkIPoint dstPoint = {0, 0};
    if (this->copySurface(copy, rt, copyRect, dstPoint)) {
        dstCopy->setTexture(copy);
        dstCopy->setOffset(copyRect.fLeft, copyRect.fTop);
        return true;
    } else {
        return false;
    }
}

void GrDrawTarget::drawIndexed(GrPipelineBuilder* pipelineBuilder,
                               const GrGeometryProcessor* gp,
                               GrPrimitiveType type,
                               int startVertex,
                               int startIndex,
                               int vertexCount,
                               int indexCount,
                               const SkRect* devBounds) {
    SkASSERT(pipelineBuilder);
    if (indexCount > 0 &&
        this->checkDraw(*pipelineBuilder, gp, type, startVertex, startIndex, vertexCount,
                        indexCount)) {

        // Setup clip
        GrScissorState scissorState;
        GrPipelineBuilder::AutoRestoreFragmentProcessors arfp;
        GrPipelineBuilder::AutoRestoreStencil ars;
        if (!this->setupClip(pipelineBuilder, &arfp, &ars, &scissorState, devBounds)) {
            return;
        }

        DrawInfo info;
        info.fPrimitiveType = type;
        info.fStartVertex   = startVertex;
        info.fStartIndex    = startIndex;
        info.fVertexCount   = vertexCount;
        info.fIndexCount    = indexCount;

        info.fInstanceCount         = 0;
        info.fVerticesPerInstance   = 0;
        info.fIndicesPerInstance    = 0;

        if (devBounds) {
            info.setDevBounds(*devBounds);
        }

        GrDrawTarget::PipelineInfo pipelineInfo(pipelineBuilder, &scissorState, gp, devBounds,
                                                this);
        if (pipelineInfo.mustSkipDraw()) {
            return;
        }

        this->setDrawBuffers(&info, gp->getVertexStride());

        this->onDraw(gp, info, pipelineInfo);
    }
}

void GrDrawTarget::drawNonIndexed(GrPipelineBuilder* pipelineBuilder,
                                  const GrGeometryProcessor* gp,
                                  GrPrimitiveType type,
                                  int startVertex,
                                  int vertexCount,
                                  const SkRect* devBounds) {
    SkASSERT(pipelineBuilder);
    if (vertexCount > 0 && this->checkDraw(*pipelineBuilder, gp, type, startVertex, -1, vertexCount,
                                           -1)) {

        // Setup clip
        GrScissorState scissorState;
        GrPipelineBuilder::AutoRestoreFragmentProcessors arfp;
        GrPipelineBuilder::AutoRestoreStencil ars;
        if (!this->setupClip(pipelineBuilder, &arfp, &ars, &scissorState, devBounds)) {
            return;
        }

        DrawInfo info;
        info.fPrimitiveType = type;
        info.fStartVertex   = startVertex;
        info.fStartIndex    = 0;
        info.fVertexCount   = vertexCount;
        info.fIndexCount    = 0;

        info.fInstanceCount         = 0;
        info.fVerticesPerInstance   = 0;
        info.fIndicesPerInstance    = 0;

        if (devBounds) {
            info.setDevBounds(*devBounds);
        }

        GrDrawTarget::PipelineInfo pipelineInfo(pipelineBuilder, &scissorState, gp, devBounds,
                                                this);
        if (pipelineInfo.mustSkipDraw()) {
            return;
        }

        this->setDrawBuffers(&info, gp->getVertexStride());

        this->onDraw(gp, info, pipelineInfo);
    }
}


void GrDrawTarget::drawBatch(GrPipelineBuilder* pipelineBuilder,
                             GrBatch* batch,
                             const SkRect* devBounds) {
    SkASSERT(pipelineBuilder);
    // TODO some kind of checkdraw, but not at this level

    // Setup clip
    GrScissorState scissorState;
    GrPipelineBuilder::AutoRestoreFragmentProcessors arfp;
    GrPipelineBuilder::AutoRestoreStencil ars;
    if (!this->setupClip(pipelineBuilder, &arfp, &ars, &scissorState, devBounds)) {
        return;
    }

    GrDrawTarget::PipelineInfo pipelineInfo(pipelineBuilder, &scissorState, batch, devBounds, this);
    if (pipelineInfo.mustSkipDraw()) {
        return;
    }

    this->onDrawBatch(batch, pipelineInfo);
}

static const GrStencilSettings& winding_path_stencil_settings() {
    GR_STATIC_CONST_SAME_STENCIL_STRUCT(gSettings,
        kIncClamp_StencilOp,
        kIncClamp_StencilOp,
        kAlwaysIfInClip_StencilFunc,
        0xFFFF, 0xFFFF, 0xFFFF);
    return *GR_CONST_STENCIL_SETTINGS_PTR_FROM_STRUCT_PTR(&gSettings);
}

static const GrStencilSettings& even_odd_path_stencil_settings() {
    GR_STATIC_CONST_SAME_STENCIL_STRUCT(gSettings,
        kInvert_StencilOp,
        kInvert_StencilOp,
        kAlwaysIfInClip_StencilFunc,
        0xFFFF, 0xFFFF, 0xFFFF);
    return *GR_CONST_STENCIL_SETTINGS_PTR_FROM_STRUCT_PTR(&gSettings);
}

void GrDrawTarget::getPathStencilSettingsForFilltype(GrPathRendering::FillType fill,
                                                     const GrStencilBuffer* sb,
                                                     GrStencilSettings* outStencilSettings) {

    switch (fill) {
        default:
            SkFAIL("Unexpected path fill.");
        case GrPathRendering::kWinding_FillType:
            *outStencilSettings = winding_path_stencil_settings();
            break;
        case GrPathRendering::kEvenOdd_FillType:
            *outStencilSettings = even_odd_path_stencil_settings();
            break;
    }
    this->clipMaskManager()->adjustPathStencilParams(sb, outStencilSettings);
}

void GrDrawTarget::stencilPath(GrPipelineBuilder* pipelineBuilder,
                               const GrPathProcessor* pathProc,
                               const GrPath* path,
                               GrPathRendering::FillType fill) {
    // TODO: extract portions of checkDraw that are relevant to path stenciling.
    SkASSERT(path);
    SkASSERT(this->caps()->pathRenderingSupport());
    SkASSERT(pipelineBuilder);

    // Setup clip
    GrScissorState scissorState;
    GrPipelineBuilder::AutoRestoreFragmentProcessors arfp;
    GrPipelineBuilder::AutoRestoreStencil ars;
    if (!this->setupClip(pipelineBuilder, &arfp, &ars, &scissorState, NULL)) {
        return;
    }

    // set stencil settings for path
    GrStencilSettings stencilSettings;
    GrRenderTarget* rt = pipelineBuilder->getRenderTarget();
    GrStencilBuffer* sb = rt->renderTargetPriv().attachStencilBuffer();
    this->getPathStencilSettingsForFilltype(fill, sb, &stencilSettings);

    this->onStencilPath(*pipelineBuilder, pathProc, path, scissorState, stencilSettings);
}

void GrDrawTarget::drawPath(GrPipelineBuilder* pipelineBuilder,
                            const GrPathProcessor* pathProc,
                            const GrPath* path,
                            GrPathRendering::FillType fill) {
    // TODO: extract portions of checkDraw that are relevant to path rendering.
    SkASSERT(path);
    SkASSERT(this->caps()->pathRenderingSupport());
    SkASSERT(pipelineBuilder);

    SkRect devBounds = path->getBounds();
    pathProc->viewMatrix().mapRect(&devBounds);

    // Setup clip
    GrScissorState scissorState;
    GrPipelineBuilder::AutoRestoreFragmentProcessors arfp;
    GrPipelineBuilder::AutoRestoreStencil ars;
    if (!this->setupClip(pipelineBuilder, &arfp, &ars, &scissorState, &devBounds)) {
       return;
    }

    // set stencil settings for path
    GrStencilSettings stencilSettings;
    GrRenderTarget* rt = pipelineBuilder->getRenderTarget();
    GrStencilBuffer* sb = rt->renderTargetPriv().attachStencilBuffer();
    this->getPathStencilSettingsForFilltype(fill, sb, &stencilSettings);

    GrDrawTarget::PipelineInfo pipelineInfo(pipelineBuilder, &scissorState, pathProc, &devBounds,
                                            this);
    if (pipelineInfo.mustSkipDraw()) {
        return;
    }

    this->onDrawPath(pathProc, path, stencilSettings, pipelineInfo);
}

void GrDrawTarget::drawPaths(GrPipelineBuilder* pipelineBuilder,
                             const GrPathProcessor* pathProc,
                             const GrPathRange* pathRange,
                             const void* indices,
                             PathIndexType indexType,
                             const float transformValues[],
                             PathTransformType transformType,
                             int count,
                             GrPathRendering::FillType fill) {
    SkASSERT(this->caps()->pathRenderingSupport());
    SkASSERT(pathRange);
    SkASSERT(indices);
    SkASSERT(0 == reinterpret_cast<long>(indices) % GrPathRange::PathIndexSizeInBytes(indexType));
    SkASSERT(transformValues);
    SkASSERT(pipelineBuilder);

    // Setup clip
    GrScissorState scissorState;
    GrPipelineBuilder::AutoRestoreFragmentProcessors arfp;
    GrPipelineBuilder::AutoRestoreStencil ars;

    if (!this->setupClip(pipelineBuilder, &arfp, &ars, &scissorState, NULL)) {
        return;
    }

    // set stencil settings for path
    GrStencilSettings stencilSettings;
    GrRenderTarget* rt = pipelineBuilder->getRenderTarget();
    GrStencilBuffer* sb = rt->renderTargetPriv().attachStencilBuffer();
    this->getPathStencilSettingsForFilltype(fill, sb, &stencilSettings);

    // Don't compute a bounding box for dst copy texture, we'll opt
    // instead for it to just copy the entire dst. Realistically this is a moot
    // point, because any context that supports NV_path_rendering will also
    // support NV_blend_equation_advanced.
    GrDrawTarget::PipelineInfo pipelineInfo(pipelineBuilder, &scissorState, pathProc, NULL, this);
    if (pipelineInfo.mustSkipDraw()) {
        return;
    }

    this->onDrawPaths(pathProc, pathRange, indices, indexType, transformValues,
                      transformType, count, stencilSettings, pipelineInfo);
}

void GrDrawTarget::clear(const SkIRect* rect,
                         GrColor color,
                         bool canIgnoreRect,
                         GrRenderTarget* renderTarget) {
    if (fCaps->useDrawInsteadOfClear()) {
        // This works around a driver bug with clear by drawing a rect instead.
        // The driver will ignore a clear if it is the only thing rendered to a
        // target before the target is read.
        SkIRect rtRect = SkIRect::MakeWH(renderTarget->width(), renderTarget->height());
        if (NULL == rect || canIgnoreRect || rect->contains(rtRect)) {
            rect = &rtRect;
            // We first issue a discard() since that may help tilers.
            this->discard(renderTarget);
        }

        GrPipelineBuilder pipelineBuilder;
        pipelineBuilder.setRenderTarget(renderTarget);

        this->drawSimpleRect(&pipelineBuilder, color, SkMatrix::I(), *rect);
    } else {       
        this->onClear(rect, color, canIgnoreRect, renderTarget);
    }
}

typedef GrTraceMarkerSet::Iter TMIter;
void GrDrawTarget::saveActiveTraceMarkers() {
    if (this->caps()->gpuTracingSupport()) {
        SkASSERT(0 == fStoredTraceMarkers.count());
        fStoredTraceMarkers.addSet(fActiveTraceMarkers);
        for (TMIter iter = fStoredTraceMarkers.begin(); iter != fStoredTraceMarkers.end(); ++iter) {
            this->removeGpuTraceMarker(&(*iter));
        }
    }
}

void GrDrawTarget::restoreActiveTraceMarkers() {
    if (this->caps()->gpuTracingSupport()) {
        SkASSERT(0 == fActiveTraceMarkers.count());
        for (TMIter iter = fStoredTraceMarkers.begin(); iter != fStoredTraceMarkers.end(); ++iter) {
            this->addGpuTraceMarker(&(*iter));
        }
        for (TMIter iter = fActiveTraceMarkers.begin(); iter != fActiveTraceMarkers.end(); ++iter) {
            this->fStoredTraceMarkers.remove(*iter);
        }
    }
}

void GrDrawTarget::addGpuTraceMarker(const GrGpuTraceMarker* marker) {
    if (this->caps()->gpuTracingSupport()) {
        SkASSERT(fGpuTraceMarkerCount >= 0);
        this->fActiveTraceMarkers.add(*marker);
        ++fGpuTraceMarkerCount;
    }
}

void GrDrawTarget::removeGpuTraceMarker(const GrGpuTraceMarker* marker) {
    if (this->caps()->gpuTracingSupport()) {
        SkASSERT(fGpuTraceMarkerCount >= 1);
        this->fActiveTraceMarkers.remove(*marker);
        --fGpuTraceMarkerCount;
    }
}

////////////////////////////////////////////////////////////////////////////////

void GrDrawTarget::drawIndexedInstances(GrPipelineBuilder* pipelineBuilder,
                                        const GrGeometryProcessor* gp,
                                        GrPrimitiveType type,
                                        int instanceCount,
                                        int verticesPerInstance,
                                        int indicesPerInstance,
                                        const SkRect* devBounds) {
    SkASSERT(pipelineBuilder);

    if (!verticesPerInstance || !indicesPerInstance) {
        return;
    }

    int maxInstancesPerDraw = this->indexCountInCurrentSource() / indicesPerInstance;
    if (!maxInstancesPerDraw) {
        return;
    }

    // Setup clip
    GrScissorState scissorState;
    GrPipelineBuilder::AutoRestoreFragmentProcessors arfp;
    GrPipelineBuilder::AutoRestoreStencil ars;
    if (!this->setupClip(pipelineBuilder, &arfp, &ars, &scissorState, devBounds)) {
        return;
    }

    DrawInfo info;
    info.fPrimitiveType = type;
    info.fStartIndex = 0;
    info.fStartVertex = 0;
    info.fIndicesPerInstance = indicesPerInstance;
    info.fVerticesPerInstance = verticesPerInstance;

    // Set the same bounds for all the draws.
    if (devBounds) {
        info.setDevBounds(*devBounds);
    }

    while (instanceCount) {
        info.fInstanceCount = SkTMin(instanceCount, maxInstancesPerDraw);
        info.fVertexCount = info.fInstanceCount * verticesPerInstance;
        info.fIndexCount = info.fInstanceCount * indicesPerInstance;

        if (this->checkDraw(*pipelineBuilder,
                            gp,
                            type,
                            info.fStartVertex,
                            info.fStartIndex,
                            info.fVertexCount,
                            info.fIndexCount)) {

            GrDrawTarget::PipelineInfo pipelineInfo(pipelineBuilder, &scissorState, gp, devBounds,
                                                    this);
            if (pipelineInfo.mustSkipDraw()) {
                return;
            }

            this->setDrawBuffers(&info, gp->getVertexStride());
            this->onDraw(gp, info, pipelineInfo);
        }
        info.fStartVertex += info.fVertexCount;
        instanceCount -= info.fInstanceCount;
    }
}

////////////////////////////////////////////////////////////////////////////////

GrDrawTarget::AutoReleaseGeometry::AutoReleaseGeometry(
                                         GrDrawTarget*  target,
                                         int vertexCount,
                                         size_t vertexStride,
                                         int indexCount) {
    fTarget = NULL;
    this->set(target, vertexCount, vertexStride, indexCount);
}

GrDrawTarget::AutoReleaseGeometry::AutoReleaseGeometry() {
    fTarget = NULL;
}

GrDrawTarget::AutoReleaseGeometry::~AutoReleaseGeometry() {
    this->reset();
}

bool GrDrawTarget::AutoReleaseGeometry::set(GrDrawTarget*  target,
                                            int vertexCount,
                                            size_t vertexStride,
                                            int indexCount) {
    this->reset();
    fTarget = target;
    bool success = true;
    if (fTarget) {
        success = target->reserveVertexAndIndexSpace(vertexCount,
                                                     vertexStride,
                                                     indexCount,
                                                     &fVertices,
                                                     &fIndices);
        if (!success) {
            fTarget = NULL;
            this->reset();
        }
    }
    SkASSERT(success == SkToBool(fTarget));
    return success;
}

void GrDrawTarget::AutoReleaseGeometry::reset() {
    if (fTarget) {
        if (fVertices) {
            fTarget->resetVertexSource();
        }
        if (fIndices) {
            fTarget->resetIndexSource();
        }
        fTarget = NULL;
    }
    fVertices = NULL;
    fIndices = NULL;
}

namespace {
// returns true if the read/written rect intersects the src/dst and false if not.
bool clip_srcrect_and_dstpoint(const GrSurface* dst,
                               const GrSurface* src,
                               const SkIRect& srcRect,
                               const SkIPoint& dstPoint,
                               SkIRect* clippedSrcRect,
                               SkIPoint* clippedDstPoint) {
    *clippedSrcRect = srcRect;
    *clippedDstPoint = dstPoint;

    // clip the left edge to src and dst bounds, adjusting dstPoint if necessary
    if (clippedSrcRect->fLeft < 0) {
        clippedDstPoint->fX -= clippedSrcRect->fLeft;
        clippedSrcRect->fLeft = 0;
    }
    if (clippedDstPoint->fX < 0) {
        clippedSrcRect->fLeft -= clippedDstPoint->fX;
        clippedDstPoint->fX = 0;
    }

    // clip the top edge to src and dst bounds, adjusting dstPoint if necessary
    if (clippedSrcRect->fTop < 0) {
        clippedDstPoint->fY -= clippedSrcRect->fTop;
        clippedSrcRect->fTop = 0;
    }
    if (clippedDstPoint->fY < 0) {
        clippedSrcRect->fTop -= clippedDstPoint->fY;
        clippedDstPoint->fY = 0;
    }

    // clip the right edge to the src and dst bounds.
    if (clippedSrcRect->fRight > src->width()) {
        clippedSrcRect->fRight = src->width();
    }
    if (clippedDstPoint->fX + clippedSrcRect->width() > dst->width()) {
        clippedSrcRect->fRight = clippedSrcRect->fLeft + dst->width() - clippedDstPoint->fX;
    }

    // clip the bottom edge to the src and dst bounds.
    if (clippedSrcRect->fBottom > src->height()) {
        clippedSrcRect->fBottom = src->height();
    }
    if (clippedDstPoint->fY + clippedSrcRect->height() > dst->height()) {
        clippedSrcRect->fBottom = clippedSrcRect->fTop + dst->height() - clippedDstPoint->fY;
    }

    // The above clipping steps may have inverted the rect if it didn't intersect either the src or
    // dst bounds.
    return !clippedSrcRect->isEmpty();
}
}

bool GrDrawTarget::copySurface(GrSurface* dst,
                               GrSurface* src,
                               const SkIRect& srcRect,
                               const SkIPoint& dstPoint) {
    SkASSERT(dst);
    SkASSERT(src);

    SkIRect clippedSrcRect;
    SkIPoint clippedDstPoint;
    // If the rect is outside the src or dst then we've already succeeded.
    if (!clip_srcrect_and_dstpoint(dst,
                                   src,
                                   srcRect,
                                   dstPoint,
                                   &clippedSrcRect,
                                   &clippedDstPoint)) {
        return true;
    }

    if (this->onCopySurface(dst, src, clippedSrcRect, clippedDstPoint)) {
        return true;
    }

    GrRenderTarget* rt = dst->asRenderTarget();
    GrTexture* tex = src->asTexture();

    if ((dst == src) || !rt || !tex) {
        return false;
    }

    GrPipelineBuilder pipelineBuilder;
    pipelineBuilder.setRenderTarget(rt);
    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(clippedSrcRect.fLeft - clippedDstPoint.fX),
                        SkIntToScalar(clippedSrcRect.fTop - clippedDstPoint.fY));
    matrix.postIDiv(tex->width(), tex->height());
    pipelineBuilder.addColorTextureProcessor(tex, matrix);
    SkIRect dstRect = SkIRect::MakeXYWH(clippedDstPoint.fX,
                                        clippedDstPoint.fY,
                                        clippedSrcRect.width(),
                                        clippedSrcRect.height());
    this->drawSimpleRect(&pipelineBuilder, GrColor_WHITE, SkMatrix::I(), dstRect);
    return true;
}

bool GrDrawTarget::canCopySurface(const GrSurface* dst,
                                  const GrSurface* src,
                                  const SkIRect& srcRect,
                                  const SkIPoint& dstPoint) {
    SkASSERT(dst);
    SkASSERT(src);

    SkIRect clippedSrcRect;
    SkIPoint clippedDstPoint;
    // If the rect is outside the src or dst then we're guaranteed success
    if (!clip_srcrect_and_dstpoint(dst,
                                   src,
                                   srcRect,
                                   dstPoint,
                                   &clippedSrcRect,
                                   &clippedDstPoint)) {
        return true;
    }
    return this->internalCanCopySurface(dst, src, clippedSrcRect, clippedDstPoint);
}

bool GrDrawTarget::internalCanCopySurface(const GrSurface* dst,
                                          const GrSurface* src,
                                          const SkIRect& clippedSrcRect,
                                          const SkIPoint& clippedDstPoint) {
    // Check that the read/write rects are contained within the src/dst bounds.
    SkASSERT(!clippedSrcRect.isEmpty());
    SkASSERT(SkIRect::MakeWH(src->width(), src->height()).contains(clippedSrcRect));
    SkASSERT(clippedDstPoint.fX >= 0 && clippedDstPoint.fY >= 0);
    SkASSERT(clippedDstPoint.fX + clippedSrcRect.width() <= dst->width() &&
             clippedDstPoint.fY + clippedSrcRect.height() <= dst->height());

    // The base class can do it as a draw or the subclass may be able to handle it.
    return ((dst != src) && dst->asRenderTarget() && src->asTexture()) ||
           this->onCanCopySurface(dst, src, clippedSrcRect, clippedDstPoint);
}

void GrDrawTarget::setupPipeline(const PipelineInfo& pipelineInfo,
                                 GrPipeline* pipeline) {
    SkNEW_PLACEMENT_ARGS(pipeline, GrPipeline, (*pipelineInfo.fPipelineBuilder,
                                                pipelineInfo.fColorPOI,
                                                pipelineInfo.fCoveragePOI,
                                                *this->caps(),
                                                *pipelineInfo.fScissor,
                                                &pipelineInfo.fDstCopy));
}
///////////////////////////////////////////////////////////////////////////////

GrDrawTarget::PipelineInfo::PipelineInfo(GrPipelineBuilder* pipelineBuilder,
                                         GrScissorState* scissor,
                                         const GrPrimitiveProcessor* primProc,
                                         const SkRect* devBounds,
                                         GrDrawTarget* target)
    : fPipelineBuilder(pipelineBuilder)
    , fScissor(scissor) {
    fColorPOI = fPipelineBuilder->colorProcInfo(primProc);
    fCoveragePOI = fPipelineBuilder->coverageProcInfo(primProc);
    if (!target->setupDstReadIfNecessary(*fPipelineBuilder, fColorPOI, fCoveragePOI,
                                         &fDstCopy, devBounds)) {
        fPipelineBuilder = NULL;
    }
}

GrDrawTarget::PipelineInfo::PipelineInfo(GrPipelineBuilder* pipelineBuilder,
                                         GrScissorState* scissor,
                                         const GrBatch* batch,
                                         const SkRect* devBounds,
                                         GrDrawTarget* target)
    : fPipelineBuilder(pipelineBuilder)
    , fScissor(scissor) {
    fColorPOI = fPipelineBuilder->colorProcInfo(batch);
    fCoveragePOI = fPipelineBuilder->coverageProcInfo(batch);
    if (!target->setupDstReadIfNecessary(*fPipelineBuilder, fColorPOI, fCoveragePOI,
                                         &fDstCopy, devBounds)) {
        fPipelineBuilder = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrDrawTargetCaps::reset() {
    fMipMapSupport = false;
    fNPOTTextureTileSupport = false;
    fTwoSidedStencilSupport = false;
    fStencilWrapOpsSupport = false;
    fHWAALineSupport = false;
    fShaderDerivativeSupport = false;
    fGeometryShaderSupport = false;
    fDualSourceBlendingSupport = false;
    fPathRenderingSupport = false;
    fDstReadInShaderSupport = false;
    fDiscardRenderTargetSupport = false;
    fReuseScratchTextures = true;
    fGpuTracingSupport = false;
    fCompressedTexSubImageSupport = false;
    fOversizedStencilSupport = false;

    fUseDrawInsteadOfClear = false;

    fMapBufferFlags = kNone_MapFlags;

    fMaxRenderTargetSize = 0;
    fMaxTextureSize = 0;
    fMaxSampleCount = 0;

    fShaderPrecisionVaries = false;

    memset(fConfigRenderSupport, 0, sizeof(fConfigRenderSupport));
    memset(fConfigTextureSupport, 0, sizeof(fConfigTextureSupport));
}

GrDrawTargetCaps& GrDrawTargetCaps::operator=(const GrDrawTargetCaps& other) {
    fMipMapSupport = other.fMipMapSupport;
    fNPOTTextureTileSupport = other.fNPOTTextureTileSupport;
    fTwoSidedStencilSupport = other.fTwoSidedStencilSupport;
    fStencilWrapOpsSupport = other.fStencilWrapOpsSupport;
    fHWAALineSupport = other.fHWAALineSupport;
    fShaderDerivativeSupport = other.fShaderDerivativeSupport;
    fGeometryShaderSupport = other.fGeometryShaderSupport;
    fDualSourceBlendingSupport = other.fDualSourceBlendingSupport;
    fPathRenderingSupport = other.fPathRenderingSupport;
    fDstReadInShaderSupport = other.fDstReadInShaderSupport;
    fDiscardRenderTargetSupport = other.fDiscardRenderTargetSupport;
    fReuseScratchTextures = other.fReuseScratchTextures;
    fGpuTracingSupport = other.fGpuTracingSupport;
    fCompressedTexSubImageSupport = other.fCompressedTexSubImageSupport;
    fOversizedStencilSupport = other.fOversizedStencilSupport;

    fUseDrawInsteadOfClear = other.fUseDrawInsteadOfClear;

    fMapBufferFlags = other.fMapBufferFlags;

    fMaxRenderTargetSize = other.fMaxRenderTargetSize;
    fMaxTextureSize = other.fMaxTextureSize;
    fMaxSampleCount = other.fMaxSampleCount;

    memcpy(fConfigRenderSupport, other.fConfigRenderSupport, sizeof(fConfigRenderSupport));
    memcpy(fConfigTextureSupport, other.fConfigTextureSupport, sizeof(fConfigTextureSupport));

    fShaderPrecisionVaries = other.fShaderPrecisionVaries;
    for (int s = 0; s < kGrShaderTypeCount; ++s) {
        for (int p = 0; p < kGrSLPrecisionCount; ++p) {
            fFloatPrecisions[s][p] = other.fFloatPrecisions[s][p];
        }
    }
    return *this;
}

static SkString map_flags_to_string(uint32_t flags) {
    SkString str;
    if (GrDrawTargetCaps::kNone_MapFlags == flags) {
        str = "none";
    } else {
        SkASSERT(GrDrawTargetCaps::kCanMap_MapFlag & flags);
        SkDEBUGCODE(flags &= ~GrDrawTargetCaps::kCanMap_MapFlag);
        str = "can_map";

        if (GrDrawTargetCaps::kSubset_MapFlag & flags) {
            str.append(" partial");
        } else {
            str.append(" full");
        }
        SkDEBUGCODE(flags &= ~GrDrawTargetCaps::kSubset_MapFlag);
    }
    SkASSERT(0 == flags); // Make sure we handled all the flags.
    return str;
}

static const char* shader_type_to_string(GrShaderType type) {
    switch (type) {
        case kVertex_GrShaderType:
            return "vertex";
        case kGeometry_GrShaderType:
            return "geometry";
        case kFragment_GrShaderType:
            return "fragment";
    }
    return "";
}

static const char* precision_to_string(GrSLPrecision p) {
    switch (p) {
        case kLow_GrSLPrecision:
            return "low";
        case kMedium_GrSLPrecision:
            return "medium";
        case kHigh_GrSLPrecision:
            return "high";
    }
    return "";
}

SkString GrDrawTargetCaps::dump() const {
    SkString r;
    static const char* gNY[] = {"NO", "YES"};
    r.appendf("MIP Map Support                    : %s\n", gNY[fMipMapSupport]);
    r.appendf("NPOT Texture Tile Support          : %s\n", gNY[fNPOTTextureTileSupport]);
    r.appendf("Two Sided Stencil Support          : %s\n", gNY[fTwoSidedStencilSupport]);
    r.appendf("Stencil Wrap Ops  Support          : %s\n", gNY[fStencilWrapOpsSupport]);
    r.appendf("HW AA Lines Support                : %s\n", gNY[fHWAALineSupport]);
    r.appendf("Shader Derivative Support          : %s\n", gNY[fShaderDerivativeSupport]);
    r.appendf("Geometry Shader Support            : %s\n", gNY[fGeometryShaderSupport]);
    r.appendf("Dual Source Blending Support       : %s\n", gNY[fDualSourceBlendingSupport]);
    r.appendf("Path Rendering Support             : %s\n", gNY[fPathRenderingSupport]);
    r.appendf("Dst Read In Shader Support         : %s\n", gNY[fDstReadInShaderSupport]);
    r.appendf("Discard Render Target Support      : %s\n", gNY[fDiscardRenderTargetSupport]);
    r.appendf("Reuse Scratch Textures             : %s\n", gNY[fReuseScratchTextures]);
    r.appendf("Gpu Tracing Support                : %s\n", gNY[fGpuTracingSupport]);
    r.appendf("Compressed Update Support          : %s\n", gNY[fCompressedTexSubImageSupport]);
    r.appendf("Oversized Stencil Support          : %s\n", gNY[fOversizedStencilSupport]);
    r.appendf("Draw Instead of Clear [workaround] : %s\n", gNY[fUseDrawInsteadOfClear]);

    r.appendf("Max Texture Size                   : %d\n", fMaxTextureSize);
    r.appendf("Max Render Target Size             : %d\n", fMaxRenderTargetSize);
    r.appendf("Max Sample Count                   : %d\n", fMaxSampleCount);

    r.appendf("Map Buffer Support                 : %s\n",
              map_flags_to_string(fMapBufferFlags).c_str());

    static const char* kConfigNames[] = {
        "Unknown",  // kUnknown_GrPixelConfig
        "Alpha8",   // kAlpha_8_GrPixelConfig,
        "Index8",   // kIndex_8_GrPixelConfig,
        "RGB565",   // kRGB_565_GrPixelConfig,
        "RGBA444",  // kRGBA_4444_GrPixelConfig,
        "RGBA8888", // kRGBA_8888_GrPixelConfig,
        "BGRA8888", // kBGRA_8888_GrPixelConfig,
        "SRGBA8888",// kSRGBA_8888_GrPixelConfig,
        "ETC1",     // kETC1_GrPixelConfig,
        "LATC",     // kLATC_GrPixelConfig,
        "R11EAC",   // kR11_EAC_GrPixelConfig,
        "ASTC12x12",// kASTC_12x12_GrPixelConfig,
        "RGBAFloat",// kRGBA_float_GrPixelConfig
        "AlphaHalf",// kAlpha_half_GrPixelConfig
    };
    GR_STATIC_ASSERT(0  == kUnknown_GrPixelConfig);
    GR_STATIC_ASSERT(1  == kAlpha_8_GrPixelConfig);
    GR_STATIC_ASSERT(2  == kIndex_8_GrPixelConfig);
    GR_STATIC_ASSERT(3  == kRGB_565_GrPixelConfig);
    GR_STATIC_ASSERT(4  == kRGBA_4444_GrPixelConfig);
    GR_STATIC_ASSERT(5  == kRGBA_8888_GrPixelConfig);
    GR_STATIC_ASSERT(6  == kBGRA_8888_GrPixelConfig);
    GR_STATIC_ASSERT(7  == kSRGBA_8888_GrPixelConfig);
    GR_STATIC_ASSERT(8  == kETC1_GrPixelConfig);
    GR_STATIC_ASSERT(9  == kLATC_GrPixelConfig);
    GR_STATIC_ASSERT(10  == kR11_EAC_GrPixelConfig);
    GR_STATIC_ASSERT(11 == kASTC_12x12_GrPixelConfig);
    GR_STATIC_ASSERT(12 == kRGBA_float_GrPixelConfig);
    GR_STATIC_ASSERT(13 == kAlpha_half_GrPixelConfig);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kConfigNames) == kGrPixelConfigCnt);

    SkASSERT(!fConfigRenderSupport[kUnknown_GrPixelConfig][0]);
    SkASSERT(!fConfigRenderSupport[kUnknown_GrPixelConfig][1]);

    for (size_t i = 1; i < SK_ARRAY_COUNT(kConfigNames); ++i)  {
        r.appendf("%s is renderable: %s, with MSAA: %s\n",
                  kConfigNames[i],
                  gNY[fConfigRenderSupport[i][0]],
                  gNY[fConfigRenderSupport[i][1]]);
    }

    SkASSERT(!fConfigTextureSupport[kUnknown_GrPixelConfig]);

    for (size_t i = 1; i < SK_ARRAY_COUNT(kConfigNames); ++i)  {
        r.appendf("%s is uploadable to a texture: %s\n",
                  kConfigNames[i],
                  gNY[fConfigTextureSupport[i]]);
    }

    r.appendf("Shader Float Precisions (varies: %s):\n", gNY[fShaderPrecisionVaries]);

    for (int s = 0; s < kGrShaderTypeCount; ++s) {
        GrShaderType shaderType = static_cast<GrShaderType>(s);
        r.appendf("\t%s:\n", shader_type_to_string(shaderType));
        for (int p = 0; p < kGrSLPrecisionCount; ++p) {
            if (fFloatPrecisions[s][p].supported()) {
                GrSLPrecision precision = static_cast<GrSLPrecision>(p);
                r.appendf("\t\t%s: log_low: %d log_high: %d bits: %d\n",
                          precision_to_string(precision),
                          fFloatPrecisions[s][p].fLogRangeLow,
                          fFloatPrecisions[s][p].fLogRangeHigh,
                          fFloatPrecisions[s][p].fBits);
            }
        }
    }

    return r;
}

uint32_t GrDrawTargetCaps::CreateUniqueID() {
    static int32_t gUniqueID = SK_InvalidUniqueID;
    uint32_t id;
    do {
        id = static_cast<uint32_t>(sk_atomic_inc(&gUniqueID) + 1);
    } while (id == SK_InvalidUniqueID);
    return id;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool GrClipTarget::setupClip(GrPipelineBuilder* pipelineBuilder,
                             GrPipelineBuilder::AutoRestoreFragmentProcessors* arfp,
                             GrPipelineBuilder::AutoRestoreStencil* ars,
                             GrScissorState* scissorState,
                             const SkRect* devBounds) {
    return fClipMaskManager.setupClipping(pipelineBuilder,
                                          arfp,
                                          ars,
                                          scissorState,
                                          devBounds);
}
