
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "GrDrawTarget.h"
#include "GrContext.h"
#include "GrDrawTargetCaps.h"
#include "GrPath.h"
#include "GrRenderTarget.h"
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

    if (NULL != di.fDevBounds) {
        SkASSERT(di.fDevBounds == &di.fDevBoundsStorage);
        fDevBoundsStorage = di.fDevBoundsStorage;
        fDevBounds = &fDevBoundsStorage;
    } else {
        fDevBounds = NULL;
    }

    fDstCopy = di.fDstCopy;

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
    : fClip(NULL)
    , fContext(context) {
    SkASSERT(NULL != context);

    fDrawState = &fDefaultDrawState;
    // We assume that fDrawState always owns a ref to the object it points at.
    fDefaultDrawState.ref();
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
    fDrawState->unref();
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

void GrDrawTarget::setClip(const GrClipData* clip) {
    clipWillBeSet(clip);
    fClip = clip;
}

const GrClipData* GrDrawTarget::getClip() const {
    return fClip;
}

void GrDrawTarget::setDrawState(GrDrawState*  drawState) {
    SkASSERT(NULL != fDrawState);
    if (NULL == drawState) {
        drawState = &fDefaultDrawState;
    }
    if (fDrawState != drawState) {
        fDrawState->unref();
        drawState->ref();
        fDrawState = drawState;
    }
}

bool GrDrawTarget::reserveVertexSpace(size_t vertexSize,
                                      int vertexCount,
                                      void** vertices) {
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    bool acquired = false;
    if (vertexCount > 0) {
        SkASSERT(NULL != vertices);
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
    } else if (NULL != vertices) {
        *vertices = NULL;
    }
    return acquired;
}

bool GrDrawTarget::reserveIndexSpace(int indexCount,
                                     void** indices) {
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    bool acquired = false;
    if (indexCount > 0) {
        SkASSERT(NULL != indices);
        this->releasePreviousIndexSource();
        geoSrc.fIndexSrc = kNone_GeometrySrcType;

        acquired = this->onReserveIndexSpace(indexCount, indices);
    }
    if (acquired) {
        geoSrc.fIndexSrc = kReserved_GeometrySrcType;
        geoSrc.fIndexCount = indexCount;
    } else if (NULL != indices) {
        *indices = NULL;
    }
    return acquired;

}

bool GrDrawTarget::reserveVertexAndIndexSpace(int vertexCount,
                                              int indexCount,
                                              void** vertices,
                                              void** indices) {
    size_t vertexSize = this->drawState()->getVertexSize();
    this->willReserveVertexAndIndexSpace(vertexCount, indexCount);
    if (vertexCount) {
        if (!this->reserveVertexSpace(vertexSize, vertexCount, vertices)) {
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

bool GrDrawTarget::geometryHints(int32_t* vertexCount,
                                 int32_t* indexCount) const {
    if (NULL != vertexCount) {
        *vertexCount = -1;
    }
    if (NULL != indexCount) {
        *indexCount = -1;
    }
    return false;
}

void GrDrawTarget::releasePreviousVertexSource() {
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    switch (geoSrc.fVertexSrc) {
        case kNone_GeometrySrcType:
            break;
        case kArray_GeometrySrcType:
            this->releaseVertexArray();
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
            GrCrash("Unknown Vertex Source Type.");
            break;
    }
}

void GrDrawTarget::releasePreviousIndexSource() {
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    switch (geoSrc.fIndexSrc) {
        case kNone_GeometrySrcType:   // these two don't require
            break;
        case kArray_GeometrySrcType:
            this->releaseIndexArray();
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
            GrCrash("Unknown Index Source Type.");
            break;
    }
}

void GrDrawTarget::setVertexSourceToArray(const void* vertexArray,
                                          int vertexCount) {
    this->releasePreviousVertexSource();
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    geoSrc.fVertexSrc = kArray_GeometrySrcType;
    geoSrc.fVertexSize = this->drawState()->getVertexSize();
    geoSrc.fVertexCount = vertexCount;
    this->onSetVertexSourceToArray(vertexArray, vertexCount);
}

void GrDrawTarget::setIndexSourceToArray(const void* indexArray,
                                         int indexCount) {
    this->releasePreviousIndexSource();
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    geoSrc.fIndexSrc = kArray_GeometrySrcType;
    geoSrc.fIndexCount = indexCount;
    this->onSetIndexSourceToArray(indexArray, indexCount);
}

void GrDrawTarget::setVertexSourceToBuffer(const GrVertexBuffer* buffer) {
    this->releasePreviousVertexSource();
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    geoSrc.fVertexSrc    = kBuffer_GeometrySrcType;
    geoSrc.fVertexBuffer = buffer;
    buffer->ref();
    geoSrc.fVertexSize = this->drawState()->getVertexSize();
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

bool GrDrawTarget::checkDraw(GrPrimitiveType type, int startVertex,
                             int startIndex, int vertexCount,
                             int indexCount) const {
    const GrDrawState& drawState = this->getDrawState();
#ifdef SK_DEBUG
    const GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    int maxVertex = startVertex + vertexCount;
    int maxValidVertex;
    switch (geoSrc.fVertexSrc) {
        case kNone_GeometrySrcType:
            GrCrash("Attempting to draw without vertex src.");
        case kReserved_GeometrySrcType: // fallthrough
        case kArray_GeometrySrcType:
            maxValidVertex = geoSrc.fVertexCount;
            break;
        case kBuffer_GeometrySrcType:
            maxValidVertex = static_cast<int>(geoSrc.fVertexBuffer->sizeInBytes() / geoSrc.fVertexSize);
            break;
    }
    if (maxVertex > maxValidVertex) {
        GrCrash("Drawing outside valid vertex range.");
    }
    if (indexCount > 0) {
        int maxIndex = startIndex + indexCount;
        int maxValidIndex;
        switch (geoSrc.fIndexSrc) {
            case kNone_GeometrySrcType:
                GrCrash("Attempting to draw indexed geom without index src.");
            case kReserved_GeometrySrcType: // fallthrough
            case kArray_GeometrySrcType:
                maxValidIndex = geoSrc.fIndexCount;
                break;
            case kBuffer_GeometrySrcType:
                maxValidIndex = static_cast<int>(geoSrc.fIndexBuffer->sizeInBytes() / sizeof(uint16_t));
                break;
        }
        if (maxIndex > maxValidIndex) {
            GrCrash("Index reads outside valid index range.");
        }
    }

    SkASSERT(NULL != drawState.getRenderTarget());

    for (int s = 0; s < drawState.numColorStages(); ++s) {
        const GrEffectRef& effect = *drawState.getColorStage(s).getEffect();
        int numTextures = effect->numTextures();
        for (int t = 0; t < numTextures; ++t) {
            GrTexture* texture = effect->texture(t);
            SkASSERT(texture->asRenderTarget() != drawState.getRenderTarget());
        }
    }
    for (int s = 0; s < drawState.numCoverageStages(); ++s) {
        const GrEffectRef& effect = *drawState.getCoverageStage(s).getEffect();
        int numTextures = effect->numTextures();
        for (int t = 0; t < numTextures; ++t) {
            GrTexture* texture = effect->texture(t);
            SkASSERT(texture->asRenderTarget() != drawState.getRenderTarget());
        }
    }

    SkASSERT(drawState.validateVertexAttribs());
#endif
    if (NULL == drawState.getRenderTarget()) {
        return false;
    }
    return true;
}

bool GrDrawTarget::setupDstReadIfNecessary(GrDeviceCoordTexture* dstCopy, const SkRect* drawBounds) {
    if (this->caps()->dstReadInShaderSupport() || !this->getDrawState().willEffectReadDstColor()) {
        return true;
    }
    GrRenderTarget* rt = this->drawState()->getRenderTarget();
    SkIRect copyRect;
    const GrClipData* clip = this->getClip();
    clip->getConservativeBounds(rt, &copyRect);

    if (NULL != drawBounds) {
        SkIRect drawIBounds;
        drawBounds->roundOut(&drawIBounds);
        if (!copyRect.intersect(drawIBounds)) {
#ifdef SK_DEBUG
            GrPrintf("Missed an early reject. Bailing on draw from setupDstReadIfNecessary.\n");
#endif
            return false;
        }
    } else {
#ifdef SK_DEBUG
        //GrPrintf("No dev bounds when dst copy is made.\n");
#endif
    }

    // MSAA consideration: When there is support for reading MSAA samples in the shader we could
    // have per-sample dst values by making the copy multisampled.
    GrTextureDesc desc;
    this->initCopySurfaceDstDesc(rt, &desc);
    desc.fWidth = copyRect.width();
    desc.fHeight = copyRect.height();

    GrAutoScratchTexture ast(fContext, desc, GrContext::kApprox_ScratchTexMatch);

    if (NULL == ast.texture()) {
        GrPrintf("Failed to create temporary copy of destination texture.\n");
        return false;
    }
    SkIPoint dstPoint = {0, 0};
    if (this->copySurface(ast.texture(), rt, copyRect, dstPoint)) {
        dstCopy->setTexture(ast.texture());
        dstCopy->setOffset(copyRect.fLeft, copyRect.fTop);
        return true;
    } else {
        return false;
    }
}

void GrDrawTarget::drawIndexed(GrPrimitiveType type,
                               int startVertex,
                               int startIndex,
                               int vertexCount,
                               int indexCount,
                               const SkRect* devBounds) {
    if (indexCount > 0 && this->checkDraw(type, startVertex, startIndex, vertexCount, indexCount)) {
        DrawInfo info;
        info.fPrimitiveType = type;
        info.fStartVertex   = startVertex;
        info.fStartIndex    = startIndex;
        info.fVertexCount   = vertexCount;
        info.fIndexCount    = indexCount;

        info.fInstanceCount         = 0;
        info.fVerticesPerInstance   = 0;
        info.fIndicesPerInstance    = 0;

        if (NULL != devBounds) {
            info.setDevBounds(*devBounds);
        }
        // TODO: We should continue with incorrect blending.
        if (!this->setupDstReadIfNecessary(&info)) {
            return;
        }
        this->onDraw(info);
    }
}

void GrDrawTarget::drawNonIndexed(GrPrimitiveType type,
                                  int startVertex,
                                  int vertexCount,
                                  const SkRect* devBounds) {
    if (vertexCount > 0 && this->checkDraw(type, startVertex, -1, vertexCount, -1)) {
        DrawInfo info;
        info.fPrimitiveType = type;
        info.fStartVertex   = startVertex;
        info.fStartIndex    = 0;
        info.fVertexCount   = vertexCount;
        info.fIndexCount    = 0;

        info.fInstanceCount         = 0;
        info.fVerticesPerInstance   = 0;
        info.fIndicesPerInstance    = 0;

        if (NULL != devBounds) {
            info.setDevBounds(*devBounds);
        }
        // TODO: We should continue with incorrect blending.
        if (!this->setupDstReadIfNecessary(&info)) {
            return;
        }
        this->onDraw(info);
    }
}

void GrDrawTarget::stencilPath(const GrPath* path, SkPath::FillType fill) {
    // TODO: extract portions of checkDraw that are relevant to path stenciling.
    SkASSERT(NULL != path);
    SkASSERT(this->caps()->pathRenderingSupport());
    SkASSERT(!SkPath::IsInverseFillType(fill));
    this->onStencilPath(path, fill);
}

void GrDrawTarget::drawPath(const GrPath* path, SkPath::FillType fill) {
    // TODO: extract portions of checkDraw that are relevant to path rendering.
    SkASSERT(NULL != path);
    SkASSERT(this->caps()->pathRenderingSupport());
    const GrDrawState* drawState = &getDrawState();

    SkRect devBounds;
    if (SkPath::IsInverseFillType(fill)) {
        devBounds = SkRect::MakeWH(SkIntToScalar(drawState->getRenderTarget()->width()),
                                   SkIntToScalar(drawState->getRenderTarget()->height()));
    } else {
        devBounds = path->getBounds();
    }
    SkMatrix viewM = drawState->getViewMatrix();
    viewM.mapRect(&devBounds);

    GrDeviceCoordTexture dstCopy;
    if (!this->setupDstReadIfNecessary(&dstCopy, &devBounds)) {
        return;
    }

    this->onDrawPath(path, fill, dstCopy.texture() ? &dstCopy : NULL);
}

////////////////////////////////////////////////////////////////////////////////

bool GrDrawTarget::willUseHWAALines() const {
    // There is a conflict between using smooth lines and our use of premultiplied alpha. Smooth
    // lines tweak the incoming alpha value but not in a premul-alpha way. So we only use them when
    // our alpha is 0xff and tweaking the color for partial coverage is OK
    if (!this->caps()->hwAALineSupport() ||
        !this->getDrawState().isHWAntialiasState()) {
        return false;
    }
    GrDrawState::BlendOptFlags opts = this->getDrawState().getBlendOpts();
    return (GrDrawState::kDisableBlend_BlendOptFlag & opts) &&
           (GrDrawState::kCoverageAsAlpha_BlendOptFlag & opts);
}

bool GrDrawTarget::canApplyCoverage() const {
    // we can correctly apply coverage if a) we have dual source blending
    // or b) one of our blend optimizations applies.
    return this->caps()->dualSourceBlendingSupport() ||
           GrDrawState::kNone_BlendOpt != this->getDrawState().getBlendOpts(true);
}

////////////////////////////////////////////////////////////////////////////////

void GrDrawTarget::drawIndexedInstances(GrPrimitiveType type,
                                        int instanceCount,
                                        int verticesPerInstance,
                                        int indicesPerInstance,
                                        const SkRect* devBounds) {
    if (!verticesPerInstance || !indicesPerInstance) {
        return;
    }

    int maxInstancesPerDraw = this->indexCountInCurrentSource() / indicesPerInstance;
    if (!maxInstancesPerDraw) {
        return;
    }

    DrawInfo info;
    info.fPrimitiveType = type;
    info.fStartIndex = 0;
    info.fStartVertex = 0;
    info.fIndicesPerInstance = indicesPerInstance;
    info.fVerticesPerInstance = verticesPerInstance;

    // Set the same bounds for all the draws.
    if (NULL != devBounds) {
        info.setDevBounds(*devBounds);
    }
    // TODO: We should continue with incorrect blending.
    if (!this->setupDstReadIfNecessary(&info)) {
        return;
    }

    while (instanceCount) {
        info.fInstanceCount = GrMin(instanceCount, maxInstancesPerDraw);
        info.fVertexCount = info.fInstanceCount * verticesPerInstance;
        info.fIndexCount = info.fInstanceCount * indicesPerInstance;

        if (this->checkDraw(type,
                            info.fStartVertex,
                            info.fStartIndex,
                            info.fVertexCount,
                            info.fIndexCount)) {
            this->onDraw(info);
        }
        info.fStartVertex += info.fVertexCount;
        instanceCount -= info.fInstanceCount;
    }
}

////////////////////////////////////////////////////////////////////////////////

namespace {

// position + (optional) texture coord
extern const GrVertexAttrib gBWRectPosUVAttribs[] = {
    {kVec2f_GrVertexAttribType, 0,               kPosition_GrVertexAttribBinding},
    {kVec2f_GrVertexAttribType, sizeof(GrPoint), kLocalCoord_GrVertexAttribBinding}
};

void set_vertex_attributes(GrDrawState* drawState, bool hasUVs) {
    if (hasUVs) {
        drawState->setVertexAttribs<gBWRectPosUVAttribs>(2);
    } else {
        drawState->setVertexAttribs<gBWRectPosUVAttribs>(1);
    }
}

};

void GrDrawTarget::onDrawRect(const SkRect& rect,
                              const SkMatrix* matrix,
                              const SkRect* localRect,
                              const SkMatrix* localMatrix) {

    GrDrawState::AutoViewMatrixRestore avmr;
    if (NULL != matrix) {
        avmr.set(this->drawState(), *matrix);
    }

    set_vertex_attributes(this->drawState(), NULL != localRect);

    AutoReleaseGeometry geo(this, 4, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return;
    }

    size_t vsize = this->drawState()->getVertexSize();
    geo.positions()->setRectFan(rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, vsize);
    if (NULL != localRect) {
        GrPoint* coords = GrTCast<GrPoint*>(GrTCast<intptr_t>(geo.vertices()) +
                                            sizeof(GrPoint));
        coords->setRectFan(localRect->fLeft, localRect->fTop,
                           localRect->fRight, localRect->fBottom,
                           vsize);
        if (NULL != localMatrix) {
            localMatrix->mapPointsWithStride(coords, vsize, 4);
        }
    }
    SkTLazy<SkRect> bounds;
    if (this->getDrawState().willEffectReadDstColor()) {
        bounds.init();
        this->getDrawState().getViewMatrix().mapRect(bounds.get(), rect);
    }

    this->drawNonIndexed(kTriangleFan_GrPrimitiveType, 0, 4, bounds.getMaybeNull());
}

void GrDrawTarget::clipWillBeSet(const GrClipData* clipData) {
}

////////////////////////////////////////////////////////////////////////////////

GrDrawTarget::AutoStateRestore::AutoStateRestore() {
    fDrawTarget = NULL;
}

GrDrawTarget::AutoStateRestore::AutoStateRestore(GrDrawTarget* target,
                                                 ASRInit init,
                                                 const SkMatrix* vm) {
    fDrawTarget = NULL;
    this->set(target, init, vm);
}

GrDrawTarget::AutoStateRestore::~AutoStateRestore() {
    if (NULL != fDrawTarget) {
        fDrawTarget->setDrawState(fSavedState);
        fSavedState->unref();
    }
}

void GrDrawTarget::AutoStateRestore::set(GrDrawTarget* target, ASRInit init, const SkMatrix* vm) {
    SkASSERT(NULL == fDrawTarget);
    fDrawTarget = target;
    fSavedState = target->drawState();
    SkASSERT(fSavedState);
    fSavedState->ref();
    if (kReset_ASRInit == init) {
        if (NULL == vm) {
            // calls the default cons
            fTempState.init();
        } else {
            SkNEW_IN_TLAZY(&fTempState, GrDrawState, (*vm));
        }
    } else {
        SkASSERT(kPreserve_ASRInit == init);
        if (NULL == vm) {
            fTempState.set(*fSavedState);
        } else {
            SkNEW_IN_TLAZY(&fTempState, GrDrawState, (*fSavedState, *vm));
        }
    }
    target->setDrawState(fTempState.get());
}

bool GrDrawTarget::AutoStateRestore::setIdentity(GrDrawTarget* target, ASRInit init) {
    SkASSERT(NULL == fDrawTarget);
    fDrawTarget = target;
    fSavedState = target->drawState();
    SkASSERT(fSavedState);
    fSavedState->ref();
    if (kReset_ASRInit == init) {
        // calls the default cons
        fTempState.init();
    } else {
        SkASSERT(kPreserve_ASRInit == init);
        // calls the copy cons
        fTempState.set(*fSavedState);
        if (!fTempState.get()->setIdentityViewMatrix()) {
            // let go of any resources held by the temp
            fTempState.get()->reset();
            fDrawTarget = NULL;
            fSavedState->unref();
            fSavedState = NULL;
            return false;
        }
    }
    target->setDrawState(fTempState.get());
    return true;
}

////////////////////////////////////////////////////////////////////////////////

GrDrawTarget::AutoReleaseGeometry::AutoReleaseGeometry(
                                         GrDrawTarget*  target,
                                         int vertexCount,
                                         int indexCount) {
    fTarget = NULL;
    this->set(target, vertexCount, indexCount);
}

GrDrawTarget::AutoReleaseGeometry::AutoReleaseGeometry() {
    fTarget = NULL;
}

GrDrawTarget::AutoReleaseGeometry::~AutoReleaseGeometry() {
    this->reset();
}

bool GrDrawTarget::AutoReleaseGeometry::set(GrDrawTarget*  target,
                                            int vertexCount,
                                            int indexCount) {
    this->reset();
    fTarget = target;
    bool success = true;
    if (NULL != fTarget) {
        fTarget = target;
        success = target->reserveVertexAndIndexSpace(vertexCount,
                                                     indexCount,
                                                     &fVertices,
                                                     &fIndices);
        if (!success) {
            fTarget = NULL;
            this->reset();
        }
    }
    SkASSERT(success == (NULL != fTarget));
    return success;
}

void GrDrawTarget::AutoReleaseGeometry::reset() {
    if (NULL != fTarget) {
        if (NULL != fVertices) {
            fTarget->resetVertexSource();
        }
        if (NULL != fIndices) {
            fTarget->resetIndexSource();
        }
        fTarget = NULL;
    }
    fVertices = NULL;
    fIndices = NULL;
}

GrDrawTarget::AutoClipRestore::AutoClipRestore(GrDrawTarget* target, const SkIRect& newClip) {
    fTarget = target;
    fClip = fTarget->getClip();
    fStack.init();
    fStack.get()->clipDevRect(newClip, SkRegion::kReplace_Op);
    fReplacementClip.fClipStack = fStack.get();
    target->setClip(&fReplacementClip);
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
    SkASSERT(NULL != dst);
    SkASSERT(NULL != src);

    SkIRect clippedSrcRect;
    SkIPoint clippedDstPoint;
    // If the rect is outside the src or dst then we've already succeeded.
    if (!clip_srcrect_and_dstpoint(dst,
                                   src,
                                   srcRect,
                                   dstPoint,
                                   &clippedSrcRect,
                                   &clippedDstPoint)) {
        SkASSERT(this->canCopySurface(dst, src, srcRect, dstPoint));
        return true;
    }

    bool result = this->onCopySurface(dst, src, clippedSrcRect, clippedDstPoint);
    SkASSERT(result == this->canCopySurface(dst, src, clippedSrcRect, clippedDstPoint));
    return result;
}

bool GrDrawTarget::canCopySurface(GrSurface* dst,
                                  GrSurface* src,
                                  const SkIRect& srcRect,
                                  const SkIPoint& dstPoint) {
    SkASSERT(NULL != dst);
    SkASSERT(NULL != src);

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
    return this->onCanCopySurface(dst, src, clippedSrcRect, clippedDstPoint);
}

bool GrDrawTarget::onCanCopySurface(GrSurface* dst,
                                    GrSurface* src,
                                    const SkIRect& srcRect,
                                    const SkIPoint& dstPoint) {
    // Check that the read/write rects are contained within the src/dst bounds.
    SkASSERT(!srcRect.isEmpty());
    SkASSERT(SkIRect::MakeWH(src->width(), src->height()).contains(srcRect));
    SkASSERT(dstPoint.fX >= 0 && dstPoint.fY >= 0);
    SkASSERT(dstPoint.fX + srcRect.width() <= dst->width() &&
             dstPoint.fY + srcRect.height() <= dst->height());

    return !dst->isSameAs(src) && NULL != dst->asRenderTarget() && NULL != src->asTexture();
}

bool GrDrawTarget::onCopySurface(GrSurface* dst,
                                 GrSurface* src,
                                 const SkIRect& srcRect,
                                 const SkIPoint& dstPoint) {
    if (!GrDrawTarget::onCanCopySurface(dst, src, srcRect, dstPoint)) {
        return false;
    }

    GrRenderTarget* rt = dst->asRenderTarget();
    GrTexture* tex = src->asTexture();

    GrDrawTarget::AutoStateRestore asr(this, kReset_ASRInit);
    this->drawState()->setRenderTarget(rt);
    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(srcRect.fLeft - dstPoint.fX),
                        SkIntToScalar(srcRect.fTop - dstPoint.fY));
    matrix.postIDiv(tex->width(), tex->height());
    this->drawState()->addColorTextureEffect(tex, matrix);
    SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.fX,
                                        dstPoint.fY,
                                        srcRect.width(),
                                        srcRect.height());
    this->drawSimpleRect(dstRect);
    return true;
}

void GrDrawTarget::initCopySurfaceDstDesc(const GrSurface* src, GrTextureDesc* desc) {
    // Make the dst of the copy be a render target because the default copySurface draws to the dst.
    desc->fOrigin = kDefault_GrSurfaceOrigin;
    desc->fFlags = kRenderTarget_GrTextureFlagBit | kNoStencil_GrTextureFlagBit;
    desc->fConfig = src->config();
}

///////////////////////////////////////////////////////////////////////////////

void GrDrawTargetCaps::reset() {
    f8BitPaletteSupport = false;
    fNPOTTextureTileSupport = false;
    fTwoSidedStencilSupport = false;
    fStencilWrapOpsSupport = false;
    fHWAALineSupport = false;
    fShaderDerivativeSupport = false;
    fGeometryShaderSupport = false;
    fDualSourceBlendingSupport = false;
    fBufferLockSupport = false;
    fPathRenderingSupport = false;
    fDstReadInShaderSupport = false;
    fReuseScratchTextures = true;

    fMaxRenderTargetSize = 0;
    fMaxTextureSize = 0;
    fMaxSampleCount = 0;

    memset(fConfigRenderSupport, 0, sizeof(fConfigRenderSupport));
}

GrDrawTargetCaps& GrDrawTargetCaps::operator=(const GrDrawTargetCaps& other) {
    f8BitPaletteSupport = other.f8BitPaletteSupport;
    fNPOTTextureTileSupport = other.fNPOTTextureTileSupport;
    fTwoSidedStencilSupport = other.fTwoSidedStencilSupport;
    fStencilWrapOpsSupport = other.fStencilWrapOpsSupport;
    fHWAALineSupport = other.fHWAALineSupport;
    fShaderDerivativeSupport = other.fShaderDerivativeSupport;
    fGeometryShaderSupport = other.fGeometryShaderSupport;
    fDualSourceBlendingSupport = other.fDualSourceBlendingSupport;
    fBufferLockSupport = other.fBufferLockSupport;
    fPathRenderingSupport = other.fPathRenderingSupport;
    fDstReadInShaderSupport = other.fDstReadInShaderSupport;
    fReuseScratchTextures = other.fReuseScratchTextures;

    fMaxRenderTargetSize = other.fMaxRenderTargetSize;
    fMaxTextureSize = other.fMaxTextureSize;
    fMaxSampleCount = other.fMaxSampleCount;

    memcpy(fConfigRenderSupport, other.fConfigRenderSupport, sizeof(fConfigRenderSupport));

    return *this;
}

SkString GrDrawTargetCaps::dump() const {
    SkString r;
    static const char* gNY[] = {"NO", "YES"};
    r.appendf("8 Bit Palette Support       : %s\n", gNY[f8BitPaletteSupport]);
    r.appendf("NPOT Texture Tile Support   : %s\n", gNY[fNPOTTextureTileSupport]);
    r.appendf("Two Sided Stencil Support   : %s\n", gNY[fTwoSidedStencilSupport]);
    r.appendf("Stencil Wrap Ops  Support   : %s\n", gNY[fStencilWrapOpsSupport]);
    r.appendf("HW AA Lines Support         : %s\n", gNY[fHWAALineSupport]);
    r.appendf("Shader Derivative Support   : %s\n", gNY[fShaderDerivativeSupport]);
    r.appendf("Geometry Shader Support     : %s\n", gNY[fGeometryShaderSupport]);
    r.appendf("Dual Source Blending Support: %s\n", gNY[fDualSourceBlendingSupport]);
    r.appendf("Buffer Lock Support         : %s\n", gNY[fBufferLockSupport]);
    r.appendf("Path Rendering Support      : %s\n", gNY[fPathRenderingSupport]);
    r.appendf("Dst Read In Shader Support  : %s\n", gNY[fDstReadInShaderSupport]);
    r.appendf("Reuse Scratch Textures      : %s\n", gNY[fReuseScratchTextures]);
    r.appendf("Max Texture Size            : %d\n", fMaxTextureSize);
    r.appendf("Max Render Target Size      : %d\n", fMaxRenderTargetSize);
    r.appendf("Max Sample Count            : %d\n", fMaxSampleCount);

    static const char* kConfigNames[] = {
        "Unknown",  // kUnknown_GrPixelConfig
        "Alpha8",   // kAlpha_8_GrPixelConfig,
        "Index8",   // kIndex_8_GrPixelConfig,
        "RGB565",   // kRGB_565_GrPixelConfig,
        "RGBA444",  // kRGBA_4444_GrPixelConfig,
        "RGBA8888", // kRGBA_8888_GrPixelConfig,
        "BGRA8888", // kBGRA_8888_GrPixelConfig,
    };
    GR_STATIC_ASSERT(0 == kUnknown_GrPixelConfig);
    GR_STATIC_ASSERT(1 == kAlpha_8_GrPixelConfig);
    GR_STATIC_ASSERT(2 == kIndex_8_GrPixelConfig);
    GR_STATIC_ASSERT(3 == kRGB_565_GrPixelConfig);
    GR_STATIC_ASSERT(4 == kRGBA_4444_GrPixelConfig);
    GR_STATIC_ASSERT(5 == kRGBA_8888_GrPixelConfig);
    GR_STATIC_ASSERT(6 == kBGRA_8888_GrPixelConfig);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kConfigNames) == kGrPixelConfigCnt);

    SkASSERT(!fConfigRenderSupport[kUnknown_GrPixelConfig][0]);
    SkASSERT(!fConfigRenderSupport[kUnknown_GrPixelConfig][1]);
    for (size_t i = 0; i < SK_ARRAY_COUNT(kConfigNames); ++i)  {
        if (i != kUnknown_GrPixelConfig) {
            r.appendf("%s is renderable: %s, with MSAA: %s\n",
                     kConfigNames[i],
                     gNY[fConfigRenderSupport[i][0]],
                     gNY[fConfigRenderSupport[i][1]]);
        }
    }
    return r;
}
