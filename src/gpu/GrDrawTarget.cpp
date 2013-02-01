
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "GrDrawTarget.h"
#include "GrRenderTarget.h"
#include "GrTexture.h"
#include "GrVertexBuffer.h"

#include "SkStrokeRec.h"

SK_DEFINE_INST_COUNT(GrDrawTarget)

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
        GrAssert(di.fDevBounds == &di.fDevBoundsStorage);
        fDevBoundsStorage = di.fDevBoundsStorage;
        fDevBounds = &fDevBoundsStorage;
    } else {
        fDevBounds = NULL;
    }
    return *this;
}

#if GR_DEBUG
bool GrDrawTarget::DrawInfo::isInstanced() const {
    if (fInstanceCount > 0) {
        GrAssert(0 == fIndexCount % fIndicesPerInstance);
        GrAssert(0 == fVertexCount % fVerticesPerInstance);
        GrAssert(fIndexCount / fIndicesPerInstance == fInstanceCount);
        GrAssert(fVertexCount / fVerticesPerInstance == fInstanceCount);
        // there is no way to specify a non-zero start index to drawIndexedInstances().
        GrAssert(0 == fStartIndex);
        return true;
    } else {
        GrAssert(!fVerticesPerInstance);
        GrAssert(!fIndicesPerInstance);
        return false;
    }
}
#endif

void GrDrawTarget::DrawInfo::adjustInstanceCount(int instanceOffset) {
    GrAssert(this->isInstanced());
    GrAssert(instanceOffset + fInstanceCount >= 0);
    fInstanceCount += instanceOffset;
    fVertexCount = fVerticesPerInstance * fInstanceCount;
    fIndexCount = fIndicesPerInstance * fInstanceCount;
}

void GrDrawTarget::DrawInfo::adjustStartVertex(int vertexOffset) {
    fStartVertex += vertexOffset;
    GrAssert(fStartVertex >= 0);
}

void GrDrawTarget::DrawInfo::adjustStartIndex(int indexOffset) {
    GrAssert(this->isIndexed());
    fStartIndex += indexOffset;
    GrAssert(fStartIndex >= 0);
}

////////////////////////////////////////////////////////////////////////////////

#define DEBUG_INVAL_BUFFER 0xdeadcafe
#define DEBUG_INVAL_START_IDX -1

GrDrawTarget::GrDrawTarget() : fClip(NULL) {
    fDrawState = &fDefaultDrawState;
    // We assume that fDrawState always owns a ref to the object it points at.
    fDefaultDrawState.ref();
    GeometrySrcState& geoSrc = fGeoSrcStateStack.push_back();
#if GR_DEBUG
    geoSrc.fVertexCount = DEBUG_INVAL_START_IDX;
    geoSrc.fVertexBuffer = (GrVertexBuffer*)DEBUG_INVAL_BUFFER;
    geoSrc.fIndexCount = DEBUG_INVAL_START_IDX;
    geoSrc.fIndexBuffer = (GrIndexBuffer*)DEBUG_INVAL_BUFFER;
#endif
    geoSrc.fVertexSrc = kNone_GeometrySrcType;
    geoSrc.fIndexSrc  = kNone_GeometrySrcType;
}

GrDrawTarget::~GrDrawTarget() {
    GrAssert(1 == fGeoSrcStateStack.count());
    SkDEBUGCODE(GeometrySrcState& geoSrc = fGeoSrcStateStack.back());
    GrAssert(kNone_GeometrySrcType == geoSrc.fIndexSrc);
    GrAssert(kNone_GeometrySrcType == geoSrc.fVertexSrc);
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
    GrAssert(NULL != fDrawState);
    if (NULL == drawState) {
        drawState = &fDefaultDrawState;
    }
    if (fDrawState != drawState) {
        fDrawState->unref();
        drawState->ref();
        fDrawState = drawState;
    }
}

bool GrDrawTarget::reserveVertexSpace(GrVertexLayout vertexLayout,
                                      int vertexCount,
                                      void** vertices) {
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    bool acquired = false;
    if (vertexCount > 0) {
        GrAssert(NULL != vertices);
        this->releasePreviousVertexSource();
        geoSrc.fVertexSrc = kNone_GeometrySrcType;

        acquired = this->onReserveVertexSpace(GrDrawState::VertexSize(vertexLayout),
                                              vertexCount,
                                              vertices);
    }
    if (acquired) {
        geoSrc.fVertexSrc = kReserved_GeometrySrcType;
        geoSrc.fVertexCount = vertexCount;
        geoSrc.fVertexLayout = vertexLayout;
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
        GrAssert(NULL != indices);
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

bool GrDrawTarget::reserveVertexAndIndexSpace(GrVertexLayout vertexLayout,
                                              int vertexCount,
                                              int indexCount,
                                              void** vertices,
                                              void** indices) {
    this->willReserveVertexAndIndexSpace(GrDrawState::VertexSize(vertexLayout), vertexCount, indexCount);
    if (vertexCount) {
        if (!this->reserveVertexSpace(vertexLayout, vertexCount, vertices)) {
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

bool GrDrawTarget::geometryHints(size_t vertexSize,
                                 int32_t* vertexCount,
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
#if GR_DEBUG
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
#if GR_DEBUG
            geoSrc.fIndexBuffer = (GrIndexBuffer*)DEBUG_INVAL_BUFFER;
#endif
            break;
        default:
            GrCrash("Unknown Index Source Type.");
            break;
    }
}

void GrDrawTarget::setVertexSourceToArray(GrVertexLayout vertexLayout,
                                          const void* vertexArray,
                                          int vertexCount) {
    this->releasePreviousVertexSource();
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    geoSrc.fVertexSrc = kArray_GeometrySrcType;
    geoSrc.fVertexLayout = vertexLayout;
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

void GrDrawTarget::setVertexSourceToBuffer(GrVertexLayout vertexLayout,
                                           const GrVertexBuffer* buffer) {
    this->releasePreviousVertexSource();
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    geoSrc.fVertexSrc    = kBuffer_GeometrySrcType;
    geoSrc.fVertexBuffer = buffer;
    buffer->ref();
    geoSrc.fVertexLayout = vertexLayout;
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
#if GR_DEBUG
    newState.fVertexCount  = ~0;
    newState.fVertexBuffer = (GrVertexBuffer*)~0;
    newState.fIndexCount   = ~0;
    newState.fIndexBuffer = (GrIndexBuffer*)~0;
#endif
}

void GrDrawTarget::popGeometrySource() {
    // if popping last element then pops are unbalanced with pushes
    GrAssert(fGeoSrcStateStack.count() > 1);

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
#if GR_DEBUG
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
            maxValidVertex = geoSrc.fVertexBuffer->sizeInBytes() / GrDrawState::VertexSize(geoSrc.fVertexLayout);
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
                maxValidIndex = geoSrc.fIndexBuffer->sizeInBytes() / sizeof(uint16_t);
                break;
        }
        if (maxIndex > maxValidIndex) {
            GrCrash("Index reads outside valid index range.");
        }
    }

    GrAssert(NULL != drawState.getRenderTarget());
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        if (drawState.isStageEnabled(s)) {
            const GrEffectRef& effect = *drawState.getStage(s).getEffect();
            int numTextures = effect->numTextures();
            for (int t = 0; t < numTextures; ++t) {
                GrTexture* texture = effect->texture(t);
                GrAssert(texture->asRenderTarget() != drawState.getRenderTarget());
            }
        }
    }
#endif
    if (NULL == drawState.getRenderTarget()) {
        return false;
    }
    return true;
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
        this->onDraw(info);
    }
}

void GrDrawTarget::stencilPath(const GrPath* path, const SkStrokeRec& stroke, SkPath::FillType fill) {
    // TODO: extract portions of checkDraw that are relevant to path stenciling.
    GrAssert(NULL != path);
    GrAssert(fCaps.pathStencilingSupport());
    GrAssert(!stroke.isHairlineStyle());
    GrAssert(!SkPath::IsInverseFillType(fill));
    this->onStencilPath(path, stroke, fill);
}

////////////////////////////////////////////////////////////////////////////////

// Some blend modes allow folding a partial coverage value into the color's
// alpha channel, while others will blend incorrectly.
bool GrDrawTarget::canTweakAlphaForCoverage() const {
    /**
     * The fractional coverage is f
     * The src and dst coeffs are Cs and Cd
     * The dst and src colors are S and D
     * We want the blend to compute: f*Cs*S + (f*Cd + (1-f))D
     * By tweaking the source color's alpha we're replacing S with S'=fS. It's
     * obvious that that first term will always be ok. The second term can be
     * rearranged as [1-(1-Cd)f]D. By substituting in the various possibilities
     * for Cd we find that only 1, ISA, and ISC produce the correct depth
     * coefficient in terms of S' and D.
     */
    GrBlendCoeff dstCoeff = this->getDrawState().getDstBlendCoeff();
    return kOne_GrBlendCoeff == dstCoeff ||
           kISA_GrBlendCoeff == dstCoeff ||
           kISC_GrBlendCoeff == dstCoeff ||
           this->getDrawState().isCoverageDrawing();
}

namespace {
GrVertexLayout default_blend_opts_vertex_layout() {
    GrVertexLayout layout = 0;
    return layout;
}
}

GrDrawTarget::BlendOptFlags
GrDrawTarget::getBlendOpts(bool forceCoverage,
                           GrBlendCoeff* srcCoeff,
                           GrBlendCoeff* dstCoeff) const {

    GrVertexLayout layout;
    if (kNone_GeometrySrcType == this->getGeomSrc().fVertexSrc) {
        layout = default_blend_opts_vertex_layout();
    } else {
        layout = this->getVertexLayout();
    }

    const GrDrawState& drawState = this->getDrawState();

    GrBlendCoeff bogusSrcCoeff, bogusDstCoeff;
    if (NULL == srcCoeff) {
        srcCoeff = &bogusSrcCoeff;
    }
    *srcCoeff = drawState.getSrcBlendCoeff();

    if (NULL == dstCoeff) {
        dstCoeff = &bogusDstCoeff;
    }
    *dstCoeff = drawState.getDstBlendCoeff();

    if (drawState.isColorWriteDisabled()) {
        *srcCoeff = kZero_GrBlendCoeff;
        *dstCoeff = kOne_GrBlendCoeff;
    }

    bool srcAIsOne = drawState.srcAlphaWillBeOne(layout);
    bool dstCoeffIsOne = kOne_GrBlendCoeff == *dstCoeff ||
                         (kSA_GrBlendCoeff == *dstCoeff && srcAIsOne);
    bool dstCoeffIsZero = kZero_GrBlendCoeff == *dstCoeff ||
                         (kISA_GrBlendCoeff == *dstCoeff && srcAIsOne);

    bool covIsZero = !drawState.isCoverageDrawing() &&
                     !(layout & GrDrawState::kCoverage_VertexLayoutBit) &&
                     0 == drawState.getCoverage();
    // When coeffs are (0,1) there is no reason to draw at all, unless
    // stenciling is enabled. Having color writes disabled is effectively
    // (0,1). The same applies when coverage is known to be 0.
    if ((kZero_GrBlendCoeff == *srcCoeff && dstCoeffIsOne) || covIsZero) {
        if (drawState.getStencil().doesWrite()) {
            return kDisableBlend_BlendOptFlag |
                   kEmitTransBlack_BlendOptFlag;
        } else {
            return kSkipDraw_BlendOptFlag;
        }
    }

    // check for coverage due to constant coverage, per-vertex coverage,
    // edge aa or coverage stage
    bool hasCoverage = forceCoverage ||
                       0xffffffff != drawState.getCoverage() ||
                       (layout & GrDrawState::kCoverage_VertexLayoutBit) ||
                       (layout & GrDrawState::kEdge_VertexLayoutBit);
    for (int s = drawState.getFirstCoverageStage();
         !hasCoverage && s < GrDrawState::kNumStages;
         ++s) {
        if (drawState.isStageEnabled(s)) {
            hasCoverage = true;
        }
    }

    // if we don't have coverage we can check whether the dst
    // has to read at all. If not, we'll disable blending.
    if (!hasCoverage) {
        if (dstCoeffIsZero) {
            if (kOne_GrBlendCoeff == *srcCoeff) {
                // if there is no coverage and coeffs are (1,0) then we
                // won't need to read the dst at all, it gets replaced by src
                return kDisableBlend_BlendOptFlag;
            } else if (kZero_GrBlendCoeff == *srcCoeff) {
                // if the op is "clear" then we don't need to emit a color
                // or blend, just write transparent black into the dst.
                *srcCoeff = kOne_GrBlendCoeff;
                *dstCoeff = kZero_GrBlendCoeff;
                return kDisableBlend_BlendOptFlag | kEmitTransBlack_BlendOptFlag;
            }
        }
    } else if (drawState.isCoverageDrawing()) {
        // we have coverage but we aren't distinguishing it from alpha by request.
        return kCoverageAsAlpha_BlendOptFlag;
    } else {
        // check whether coverage can be safely rolled into alpha
        // of if we can skip color computation and just emit coverage
        if (this->canTweakAlphaForCoverage()) {
            return kCoverageAsAlpha_BlendOptFlag;
        }
        if (dstCoeffIsZero) {
            if (kZero_GrBlendCoeff == *srcCoeff) {
                // the source color is not included in the blend
                // the dst coeff is effectively zero so blend works out to:
                // (c)(0)D + (1-c)D = (1-c)D.
                *dstCoeff = kISA_GrBlendCoeff;
                return  kEmitCoverage_BlendOptFlag;
            } else if (srcAIsOne) {
                // the dst coeff is effectively zero so blend works out to:
                // cS + (c)(0)D + (1-c)D = cS + (1-c)D.
                // If Sa is 1 then we can replace Sa with c
                // and set dst coeff to 1-Sa.
                *dstCoeff = kISA_GrBlendCoeff;
                return  kCoverageAsAlpha_BlendOptFlag;
            }
        } else if (dstCoeffIsOne) {
            // the dst coeff is effectively one so blend works out to:
            // cS + (c)(1)D + (1-c)D = cS + D.
            *dstCoeff = kOne_GrBlendCoeff;
            return  kCoverageAsAlpha_BlendOptFlag;
        }
    }
    return kNone_BlendOpt;
}

bool GrDrawTarget::willUseHWAALines() const {
    // there is a conflict between using smooth lines and our use of
    // premultiplied alpha. Smooth lines tweak the incoming alpha value
    // but not in a premul-alpha way. So we only use them when our alpha
    // is 0xff and tweaking the color for partial coverage is OK
    if (!fCaps.hwAALineSupport() ||
        !this->getDrawState().isHWAntialiasState()) {
        return false;
    }
    BlendOptFlags opts = this->getBlendOpts();
    return (kDisableBlend_BlendOptFlag & opts) &&
           (kCoverageAsAlpha_BlendOptFlag & opts);
}

bool GrDrawTarget::canApplyCoverage() const {
    // we can correctly apply coverage if a) we have dual source blending
    // or b) one of our blend optimizations applies.
    return this->getCaps().dualSourceBlendingSupport() ||
           kNone_BlendOpt != this->getBlendOpts(true);
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

void GrDrawTarget::drawRect(const GrRect& rect,
                            const SkMatrix* matrix,
                            const GrRect* srcRects[],
                            const SkMatrix* srcMatrices[]) {
    GrVertexLayout layout = 0;
    uint32_t explicitCoordMask = 0;

    if (NULL != srcRects) {
        for (int s = 0; s < GrDrawState::kNumStages; ++s) {
            int numTC = 0;
            if (NULL != srcRects[s]) {
                layout |= GrDrawState::StageTexCoordVertexLayoutBit(s, numTC);
                explicitCoordMask |= (1 << s);
                ++numTC;
            }
        }
    }

    GrDrawState::AutoViewMatrixRestore avmr;
    if (NULL != matrix) {
        avmr.set(this->drawState(), *matrix, explicitCoordMask);
    }

    AutoReleaseGeometry geo(this, layout, 4, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return;
    }

    int stageOffsets[GrDrawState::kNumStages];
    int vsize = GrDrawState::VertexSizeAndOffsetsByStage(layout, stageOffsets,  NULL, NULL, NULL);
    geo.positions()->setRectFan(rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, vsize);

    for (int i = 0; i < GrDrawState::kNumStages; ++i) {
        if (explicitCoordMask & (1 << i)) {
            GrAssert(0 != stageOffsets[i]);
            GrPoint* coords = GrTCast<GrPoint*>(GrTCast<intptr_t>(geo.vertices()) +
                                                stageOffsets[i]);
            coords->setRectFan(srcRects[i]->fLeft, srcRects[i]->fTop,
                               srcRects[i]->fRight, srcRects[i]->fBottom,
                               vsize);
            if (NULL != srcMatrices && NULL != srcMatrices[i]) {
                srcMatrices[i]->mapPointsWithStride(coords, vsize, 4);
            }
        } else {
            GrAssert(0 == stageOffsets[i]);
        }
    }

    this->drawNonIndexed(kTriangleFan_GrPrimitiveType, 0, 4);
}

void GrDrawTarget::clipWillBeSet(const GrClipData* clipData) {
}

////////////////////////////////////////////////////////////////////////////////

GrDrawTarget::AutoStateRestore::AutoStateRestore() {
    fDrawTarget = NULL;
}

GrDrawTarget::AutoStateRestore::AutoStateRestore(GrDrawTarget* target,
                                                 ASRInit init) {
    fDrawTarget = NULL;
    this->set(target, init);
}

GrDrawTarget::AutoStateRestore::~AutoStateRestore() {
    if (NULL != fDrawTarget) {
        fDrawTarget->setDrawState(fSavedState);
        fSavedState->unref();
    }
}

void GrDrawTarget::AutoStateRestore::set(GrDrawTarget* target, ASRInit init) {
    GrAssert(NULL == fDrawTarget);
    fDrawTarget = target;
    fSavedState = target->drawState();
    GrAssert(fSavedState);
    fSavedState->ref();
    if (kReset_ASRInit == init) {
        // calls the default cons
        fTempState.init();
    } else {
        GrAssert(kPreserve_ASRInit == init);
        // calls the copy cons
        fTempState.set(*fSavedState);
    }
    target->setDrawState(fTempState.get());
}

////////////////////////////////////////////////////////////////////////////////

GrDrawTarget::AutoReleaseGeometry::AutoReleaseGeometry(
                                         GrDrawTarget*  target,
                                         GrVertexLayout vertexLayout,
                                         int vertexCount,
                                         int indexCount) {
    fTarget = NULL;
    this->set(target, vertexLayout, vertexCount, indexCount);
}

GrDrawTarget::AutoReleaseGeometry::AutoReleaseGeometry() {
    fTarget = NULL;
}

GrDrawTarget::AutoReleaseGeometry::~AutoReleaseGeometry() {
    this->reset();
}

bool GrDrawTarget::AutoReleaseGeometry::set(GrDrawTarget*  target,
                                            GrVertexLayout vertexLayout,
                                            int vertexCount,
                                            int indexCount) {
    this->reset();
    fTarget = target;
    bool success = true;
    if (NULL != fTarget) {
        fTarget = target;
        success = target->reserveVertexAndIndexSpace(vertexLayout,
                                                     vertexCount,
                                                     indexCount,
                                                     &fVertices,
                                                     &fIndices);
        if (!success) {
            fTarget = NULL;
            this->reset();
        }
    }
    GrAssert(success == (NULL != fTarget));
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

void GrDrawTarget::Caps::print() const {
    static const char* gNY[] = {"NO", "YES"};
    GrPrintf("8 Bit Palette Support       : %s\n", gNY[fInternals.f8BitPaletteSupport]);
    GrPrintf("NPOT Texture Tile Support   : %s\n", gNY[fInternals.fNPOTTextureTileSupport]);
    GrPrintf("Two Sided Stencil Support   : %s\n", gNY[fInternals.fTwoSidedStencilSupport]);
    GrPrintf("Stencil Wrap Ops  Support   : %s\n", gNY[fInternals.fStencilWrapOpsSupport]);
    GrPrintf("HW AA Lines Support         : %s\n", gNY[fInternals.fHWAALineSupport]);
    GrPrintf("Shader Derivative Support   : %s\n", gNY[fInternals.fShaderDerivativeSupport]);
    GrPrintf("Geometry Shader Support     : %s\n", gNY[fInternals.fGeometryShaderSupport]);
    GrPrintf("FSAA Support                : %s\n", gNY[fInternals.fFSAASupport]);
    GrPrintf("Dual Source Blending Support: %s\n", gNY[fInternals.fDualSourceBlendingSupport]);
    GrPrintf("Buffer Lock Support         : %s\n", gNY[fInternals.fBufferLockSupport]);
    GrPrintf("Path Stenciling Support     : %s\n", gNY[fInternals.fPathStencilingSupport]);
    GrPrintf("Max Texture Size            : %d\n", fInternals.fMaxTextureSize);
    GrPrintf("Max Render Target Size      : %d\n", fInternals.fMaxRenderTargetSize);
}
