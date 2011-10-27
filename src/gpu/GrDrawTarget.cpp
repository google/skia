
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "GrDrawTarget.h"
#include "GrGpuVertex.h"
#include "GrTexture.h"
#include "GrVertexBuffer.h"
#include "GrIndexBuffer.h"

namespace {

// recursive helper for creating mask with all the tex coord bits set for
// one stage
template <int N>
int stage_mask_recur(int stage) {
    return GrDrawTarget::StageTexCoordVertexLayoutBit(stage, N) |
           stage_mask_recur<N+1>(stage);
}
template<> 
int stage_mask_recur<GrDrawState::kNumStages>(int) { return 0; }

// mask of all tex coord indices for one stage
int stage_tex_coord_mask(int stage) {
    return stage_mask_recur<0>(stage);
}

// mask of all bits relevant to one stage
int stage_mask(int stage) {
    return stage_tex_coord_mask(stage) |
           GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(stage);
}

// recursive helper for creating mask of with all bits set relevant to one
// texture coordinate index
template <int N>
int tex_coord_mask_recur(int texCoordIdx) {
    return GrDrawTarget::StageTexCoordVertexLayoutBit(N, texCoordIdx) |
           tex_coord_mask_recur<N+1>(texCoordIdx);
}
template<> 
int tex_coord_mask_recur<GrDrawState::kMaxTexCoords>(int) { return 0; }

// mask of all bits relevant to one texture coordinate index
int tex_coord_idx_mask(int texCoordIdx) {
    return tex_coord_mask_recur<0>(texCoordIdx);
}

bool check_layout(GrVertexLayout layout) {
    // can only have 1 or 0 bits set for each stage.
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        int stageBits = layout & stage_mask(s);
        if (stageBits && !GrIsPow2(stageBits)) {
            return false;
        }
    }
    return true;
}

int num_tex_coords(GrVertexLayout layout) {
    int cnt = 0;
    // figure out how many tex coordinates are present
    for (int t = 0; t < GrDrawState::kMaxTexCoords; ++t) {
        if (tex_coord_idx_mask(t) & layout) {
            ++cnt;
        }
    }
    return cnt;
}

} //unnamed namespace

size_t GrDrawTarget::VertexSize(GrVertexLayout vertexLayout) {
    GrAssert(check_layout(vertexLayout));

    size_t vecSize = (vertexLayout & kTextFormat_VertexLayoutBit) ?
                        sizeof(GrGpuTextVertex) :
                        sizeof(GrPoint);

    size_t size = vecSize; // position
    size += num_tex_coords(vertexLayout) * vecSize;
    if (vertexLayout & kColor_VertexLayoutBit) {
        size += sizeof(GrColor);
    }
    if (vertexLayout & kCoverage_VertexLayoutBit) {
        size += sizeof(GrColor);
    }
    if (vertexLayout & kEdge_VertexLayoutBit) {
        size += 4 * sizeof(GrScalar);
    }
    return size;
}

////////////////////////////////////////////////////////////////////////////////

/**
 * Functions for computing offsets of various components from the layout
 * bitfield.
 * 
 * Order of vertex components:
 * Position
 * Tex Coord 0
 * ...
 * Tex Coord GrDrawState::kMaxTexCoords-1
 * Color
 * Coverage
 */

int GrDrawTarget::VertexStageCoordOffset(int stage, GrVertexLayout vertexLayout) {
    GrAssert(check_layout(vertexLayout));
    if (StagePosAsTexCoordVertexLayoutBit(stage) & vertexLayout) {
        return 0;
    }
    int tcIdx = VertexTexCoordsForStage(stage, vertexLayout);
    if (tcIdx >= 0) {

        int vecSize = (vertexLayout & kTextFormat_VertexLayoutBit) ?
                                    sizeof(GrGpuTextVertex) :
                                    sizeof(GrPoint);
        int offset = vecSize; // position
        // figure out how many tex coordinates are present and precede this one.
        for (int t = 0; t < tcIdx; ++t) {
            if (tex_coord_idx_mask(t) & vertexLayout) {
                offset += vecSize;
            }
        }
        return offset;
    }

    return -1;
}

int  GrDrawTarget::VertexColorOffset(GrVertexLayout vertexLayout) {
    GrAssert(check_layout(vertexLayout));

    if (vertexLayout & kColor_VertexLayoutBit) {
        int vecSize = (vertexLayout & kTextFormat_VertexLayoutBit) ?
                                    sizeof(GrGpuTextVertex) :
                                    sizeof(GrPoint);
        return vecSize * (num_tex_coords(vertexLayout) + 1); //+1 for pos
    }
    return -1;
}

int  GrDrawTarget::VertexCoverageOffset(GrVertexLayout vertexLayout) {
    GrAssert(check_layout(vertexLayout));

    if (vertexLayout & kCoverage_VertexLayoutBit) {
        int vecSize = (vertexLayout & kTextFormat_VertexLayoutBit) ?
                                    sizeof(GrGpuTextVertex) :
                                    sizeof(GrPoint);

        int offset = vecSize * (num_tex_coords(vertexLayout) + 1);
        if (vertexLayout & kColor_VertexLayoutBit) {
            offset += sizeof(GrColor);
        }
        return offset;
    }
    return -1;
}

int  GrDrawTarget::VertexEdgeOffset(GrVertexLayout vertexLayout) {
    GrAssert(check_layout(vertexLayout));

    // edge pts are after the pos, tex coords, and color
    if (vertexLayout & kEdge_VertexLayoutBit) {
        int vecSize = (vertexLayout & kTextFormat_VertexLayoutBit) ?
                                    sizeof(GrGpuTextVertex) :
                                    sizeof(GrPoint);
        int offset = vecSize * (num_tex_coords(vertexLayout) + 1); //+1 for pos
        if (vertexLayout & kColor_VertexLayoutBit) {
            offset += sizeof(GrColor);
        }
        if (vertexLayout & kCoverage_VertexLayoutBit) {
            offset += sizeof(GrColor);
        }
        return offset;
    }
    return -1;
}

int GrDrawTarget::VertexSizeAndOffsetsByIdx(
        GrVertexLayout vertexLayout,
        int texCoordOffsetsByIdx[GrDrawState::kMaxTexCoords],
        int* colorOffset,
        int* coverageOffset,
        int* edgeOffset) {
    GrAssert(check_layout(vertexLayout));

    int vecSize = (vertexLayout & kTextFormat_VertexLayoutBit) ?
                                                    sizeof(GrGpuTextVertex) :
                                                    sizeof(GrPoint);
    int size = vecSize; // position

    for (int t = 0; t < GrDrawState::kMaxTexCoords; ++t) {
        if (tex_coord_idx_mask(t) & vertexLayout) {
            if (NULL != texCoordOffsetsByIdx) {
                texCoordOffsetsByIdx[t] = size;
            }
            size += vecSize;
        } else {
            if (NULL != texCoordOffsetsByIdx) {
                texCoordOffsetsByIdx[t] = -1;
            }
        }
    }
    if (kColor_VertexLayoutBit & vertexLayout) {
        if (NULL != colorOffset) {
            *colorOffset = size;
        }
        size += sizeof(GrColor);
    } else {
        if (NULL != colorOffset) {
            *colorOffset = -1;
        }
    }
    if (kCoverage_VertexLayoutBit & vertexLayout) {
        if (NULL != coverageOffset) {
            *coverageOffset = size;
        }
        size += sizeof(GrColor);
    } else {
        if (NULL != coverageOffset) {
            *coverageOffset = -1;
        }
    }
    if (kEdge_VertexLayoutBit & vertexLayout) {
        if (NULL != edgeOffset) {
            *edgeOffset = size;
        }
        size += 4 * sizeof(GrScalar);
    } else {
        if (NULL != edgeOffset) {
            *edgeOffset = -1;
        }
    }
    return size;
}

int GrDrawTarget::VertexSizeAndOffsetsByStage(
        GrVertexLayout vertexLayout,
        int texCoordOffsetsByStage[GrDrawState::kNumStages],
        int* colorOffset,
        int* coverageOffset,
        int* edgeOffset) {
    GrAssert(check_layout(vertexLayout));

    int texCoordOffsetsByIdx[GrDrawState::kMaxTexCoords];
    int size = VertexSizeAndOffsetsByIdx(vertexLayout,
                                         (NULL == texCoordOffsetsByStage) ?
                                               NULL :
                                               texCoordOffsetsByIdx,
                                         colorOffset,
                                         coverageOffset,
                                         edgeOffset);
    if (NULL != texCoordOffsetsByStage) {
        for (int s = 0; s < GrDrawState::kNumStages; ++s) {
            int tcIdx;
            if (StagePosAsTexCoordVertexLayoutBit(s) & vertexLayout) {
                texCoordOffsetsByStage[s] = 0;
            } else if ((tcIdx = VertexTexCoordsForStage(s, vertexLayout)) >= 0) {
                texCoordOffsetsByStage[s] = texCoordOffsetsByIdx[tcIdx];
            } else {
                texCoordOffsetsByStage[s] = -1;
            }
        }
    }
    return size;
}

////////////////////////////////////////////////////////////////////////////////

bool GrDrawTarget::VertexUsesStage(int stage, GrVertexLayout vertexLayout) {
    GrAssert(stage < GrDrawState::kNumStages);
    GrAssert(check_layout(vertexLayout));
    return !!(stage_mask(stage) & vertexLayout);
}

bool GrDrawTarget::VertexUsesTexCoordIdx(int coordIndex,
                                         GrVertexLayout vertexLayout) {
    GrAssert(coordIndex < GrDrawState::kMaxTexCoords);
    GrAssert(check_layout(vertexLayout));
    return !!(tex_coord_idx_mask(coordIndex) & vertexLayout);
}

int GrDrawTarget::VertexTexCoordsForStage(int stage,
                                          GrVertexLayout vertexLayout) {
    GrAssert(stage < GrDrawState::kNumStages);
    GrAssert(check_layout(vertexLayout));
    int bit = vertexLayout & stage_tex_coord_mask(stage);
    if (bit) {
        // figure out which set of texture coordates is used
        // bits are ordered T0S0, T0S1, T0S2, ..., T1S0, T1S1, ...
        // and start at bit 0.
        GR_STATIC_ASSERT(sizeof(GrVertexLayout) <= sizeof(uint32_t));
        return (32 - Gr_clz(bit) - 1) / GrDrawState::kNumStages;
    }
    return -1;
}

////////////////////////////////////////////////////////////////////////////////

void GrDrawTarget::VertexLayoutUnitTest() {
    // not necessarily exhaustive
    static bool run;
    if (!run) {
        run = true;
        for (int s = 0; s < GrDrawState::kNumStages; ++s) {

            GrAssert(!VertexUsesStage(s, 0));
            GrAssert(-1 == VertexStageCoordOffset(s, 0));
            GrVertexLayout stageMask = 0;
            for (int t = 0; t < GrDrawState::kMaxTexCoords; ++t) {
                stageMask |= StageTexCoordVertexLayoutBit(s,t);
            }
            GrAssert(1 == GrDrawState::kMaxTexCoords ||
                     !check_layout(stageMask));
            GrAssert(stage_tex_coord_mask(s) == stageMask);
            stageMask |= StagePosAsTexCoordVertexLayoutBit(s);
            GrAssert(stage_mask(s) == stageMask);
            GrAssert(!check_layout(stageMask));
        }
        for (int t = 0; t < GrDrawState::kMaxTexCoords; ++t) {
            GrVertexLayout tcMask = 0;
            GrAssert(!VertexUsesTexCoordIdx(t, 0));
            for (int s = 0; s < GrDrawState::kNumStages; ++s) {
                tcMask |= StageTexCoordVertexLayoutBit(s,t);
                GrAssert(VertexUsesStage(s, tcMask));
                GrAssert(sizeof(GrPoint) == VertexStageCoordOffset(s, tcMask));
                GrAssert(VertexUsesTexCoordIdx(t, tcMask));
                GrAssert(2*sizeof(GrPoint) == VertexSize(tcMask));
                GrAssert(t == VertexTexCoordsForStage(s, tcMask));
                for (int s2 = s + 1; s2 < GrDrawState::kNumStages; ++s2) {
                    GrAssert(-1 == VertexStageCoordOffset(s2, tcMask));
                    GrAssert(!VertexUsesStage(s2, tcMask));
                    GrAssert(-1 == VertexTexCoordsForStage(s2, tcMask));

                #if GR_DEBUG
                    GrVertexLayout posAsTex = tcMask | StagePosAsTexCoordVertexLayoutBit(s2);
                #endif
                    GrAssert(0 == VertexStageCoordOffset(s2, posAsTex));
                    GrAssert(VertexUsesStage(s2, posAsTex));
                    GrAssert(2*sizeof(GrPoint) == VertexSize(posAsTex));
                    GrAssert(-1 == VertexTexCoordsForStage(s2, posAsTex));
                    GrAssert(-1 == VertexEdgeOffset(posAsTex));
                }
                GrAssert(-1 == VertexEdgeOffset(tcMask));
                GrAssert(-1 == VertexColorOffset(tcMask));
                GrAssert(-1 == VertexCoverageOffset(tcMask));
            #if GR_DEBUG
                GrVertexLayout withColor = tcMask | kColor_VertexLayoutBit;
            #endif
                GrAssert(-1 == VertexCoverageOffset(withColor));
                GrAssert(2*sizeof(GrPoint) == VertexColorOffset(withColor));
                GrAssert(2*sizeof(GrPoint) + sizeof(GrColor) == VertexSize(withColor));
            #if GR_DEBUG
                GrVertexLayout withEdge = tcMask | kEdge_VertexLayoutBit;
            #endif
                GrAssert(-1 == VertexColorOffset(withEdge));
                GrAssert(2*sizeof(GrPoint) == VertexEdgeOffset(withEdge));
                GrAssert(4*sizeof(GrPoint) == VertexSize(withEdge));
            #if GR_DEBUG
                GrVertexLayout withColorAndEdge = withColor | kEdge_VertexLayoutBit;
            #endif
                GrAssert(2*sizeof(GrPoint) == VertexColorOffset(withColorAndEdge));
                GrAssert(2*sizeof(GrPoint) + sizeof(GrColor) == VertexEdgeOffset(withColorAndEdge));
                GrAssert(4*sizeof(GrPoint) + sizeof(GrColor) == VertexSize(withColorAndEdge));
            #if GR_DEBUG
                GrVertexLayout withCoverage = tcMask | kCoverage_VertexLayoutBit;
            #endif
                GrAssert(-1 == VertexColorOffset(withCoverage));
                GrAssert(2*sizeof(GrPoint) == VertexCoverageOffset(withCoverage));
                GrAssert(2*sizeof(GrPoint) + sizeof(GrColor) == VertexSize(withCoverage));
            #if GR_DEBUG
                GrVertexLayout withCoverageAndColor = tcMask | kCoverage_VertexLayoutBit |
                                                      kColor_VertexLayoutBit;
            #endif
                GrAssert(2*sizeof(GrPoint) == VertexColorOffset(withCoverageAndColor));
                GrAssert(2*sizeof(GrPoint) + sizeof(GrColor) == VertexCoverageOffset(withCoverageAndColor));
                GrAssert(2*sizeof(GrPoint) + 2 * sizeof(GrColor) == VertexSize(withCoverageAndColor));
            }
            GrAssert(tex_coord_idx_mask(t) == tcMask);
            GrAssert(check_layout(tcMask));

            int stageOffsets[GrDrawState::kNumStages];
            int colorOffset;
            int edgeOffset;
            int coverageOffset;
            int size;
            size = VertexSizeAndOffsetsByStage(tcMask,
                                               stageOffsets, &colorOffset,
                                               &coverageOffset, &edgeOffset);
            GrAssert(2*sizeof(GrPoint) == size);
            GrAssert(-1 == colorOffset);
            GrAssert(-1 == coverageOffset);
            GrAssert(-1 == edgeOffset);
            for (int s = 0; s < GrDrawState::kNumStages; ++s) {
                GrAssert(VertexUsesStage(s, tcMask));
                GrAssert(sizeof(GrPoint) == stageOffsets[s]);
                GrAssert(sizeof(GrPoint) == VertexStageCoordOffset(s, tcMask));
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

#define DEBUG_INVAL_BUFFER 0xdeadcafe
#define DEBUG_INVAL_START_IDX -1

GrDrawTarget::GrDrawTarget() {
#if GR_DEBUG
    VertexLayoutUnitTest();
#endif
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
    int popCnt = fGeoSrcStateStack.count() - 1;
    while (popCnt) {
        this->popGeometrySource();
        --popCnt;
    }
    this->releasePreviousVertexSource();
    this->releasePreviousIndexSource();
}

void GrDrawTarget::setClip(const GrClip& clip) {
    clipWillBeSet(clip);
    fClip = clip;
}

const GrClip& GrDrawTarget::getClip() const {
    return fClip;
}

void GrDrawTarget::setTexture(int stage, GrTexture* tex) {
    GrAssert(stage >= 0 && stage < GrDrawState::kNumStages);
    fCurrDrawState.fTextures[stage] = tex;
}

const GrTexture* GrDrawTarget::getTexture(int stage) const {
    GrAssert(stage >= 0 && stage < GrDrawState::kNumStages);
    return fCurrDrawState.fTextures[stage];
}

GrTexture* GrDrawTarget::getTexture(int stage) {
    GrAssert(stage >= 0 && stage < GrDrawState::kNumStages);
    return fCurrDrawState.fTextures[stage];
}

void GrDrawTarget::setRenderTarget(GrRenderTarget* target) {
    fCurrDrawState.fRenderTarget = target;
}

const GrRenderTarget* GrDrawTarget::getRenderTarget() const {
    return fCurrDrawState.fRenderTarget;
}

GrRenderTarget* GrDrawTarget::getRenderTarget() {
    return fCurrDrawState.fRenderTarget;
}

void GrDrawTarget::setViewMatrix(const GrMatrix& m) {
    fCurrDrawState.fViewMatrix = m;
}

void GrDrawTarget::preConcatViewMatrix(const GrMatrix& matrix) {
    fCurrDrawState.fViewMatrix.preConcat(matrix);
}

void GrDrawTarget::postConcatViewMatrix(const GrMatrix& matrix) {
    fCurrDrawState.fViewMatrix.postConcat(matrix);
}

const GrMatrix& GrDrawTarget::getViewMatrix() const {
    return fCurrDrawState.fViewMatrix;
}

bool GrDrawTarget::getViewInverse(GrMatrix* matrix) const {
    // Mike:  Can we cache this somewhere?
    // Brian: Sure, do we use it often?

    GrMatrix inverse;
    if (fCurrDrawState.fViewMatrix.invert(&inverse)) {
        if (matrix) {
            *matrix = inverse;
        }
        return true;
    }
    return false;
}

void GrDrawTarget::setSamplerState(int stage, const GrSamplerState& state) {
    GrAssert(stage >= 0 && stage < GrDrawState::kNumStages);
    fCurrDrawState.fSamplerStates[stage] = state;
}

void GrDrawTarget::enableState(uint32_t bits) {
    fCurrDrawState.fFlagBits |= bits;
}

void GrDrawTarget::disableState(uint32_t bits) {
    fCurrDrawState.fFlagBits &= ~(bits);
}

void GrDrawTarget::setBlendFunc(GrBlendCoeff srcCoeff,
                                GrBlendCoeff dstCoeff) {
    fCurrDrawState.fSrcBlend = srcCoeff;
    fCurrDrawState.fDstBlend = dstCoeff;
#if GR_DEBUG
    switch (dstCoeff) {
    case kDC_BlendCoeff:
    case kIDC_BlendCoeff:
    case kDA_BlendCoeff:
    case kIDA_BlendCoeff:
        GrPrintf("Unexpected dst blend coeff. Won't work correctly with"
                 "coverage stages.\n");
        break;
    default:
        break;
    }
    switch (srcCoeff) {
    case kSC_BlendCoeff:
    case kISC_BlendCoeff:
    case kSA_BlendCoeff:
    case kISA_BlendCoeff:
        GrPrintf("Unexpected src blend coeff. Won't work correctly with"
                 "coverage stages.\n");
        break;
    default:
        break;
    }
#endif
}

void GrDrawTarget::setColor(GrColor c) {
    fCurrDrawState.fColor = c;
}

void GrDrawTarget::setColorFilter(GrColor c, SkXfermode::Mode mode) {
    fCurrDrawState.fColorFilterColor = c;
    fCurrDrawState.fColorFilterXfermode = mode;
}

void GrDrawTarget::setAlpha(uint8_t a) {
    this->setColor((a << 24) | (a << 16) | (a << 8) | a);
}

void GrDrawTarget::saveCurrentDrawState(SavedDrawState* state) const {
    state->fState = fCurrDrawState;
}

void GrDrawTarget::restoreDrawState(const SavedDrawState& state) {
    fCurrDrawState = state.fState;
}

void GrDrawTarget::copyDrawState(const GrDrawTarget& srcTarget) {
    fCurrDrawState = srcTarget.fCurrDrawState;
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

        acquired = this->onReserveVertexSpace(vertexLayout,
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

bool GrDrawTarget::geometryHints(GrVertexLayout vertexLayout,
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
    const GeometrySrcState& geoSrc = this->getGeomSrc();
    // if popping last element then pops are unbalanced with pushes
    GrAssert(fGeoSrcStateStack.count() > 1);
    
    this->geometrySourceWillPop(fGeoSrcStateStack.fromBack(1));
    this->releasePreviousVertexSource();
    this->releasePreviousIndexSource();
    fGeoSrcStateStack.pop_back();
}

////////////////////////////////////////////////////////////////////////////////

void GrDrawTarget::drawIndexed(GrPrimitiveType type, int startVertex,
                               int startIndex, int vertexCount,
                               int indexCount) {
#if GR_DEBUG
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    int maxVertex = startVertex + vertexCount;
    int maxValidVertex;
    switch (geoSrc.fVertexSrc) {
        case kNone_GeometrySrcType:
            GrCrash("Attempting to draw indexed geom without vertex src.");
        case kReserved_GeometrySrcType: // fallthrough
        case kArray_GeometrySrcType:
            maxValidVertex = geoSrc.fVertexCount;
            break;
        case kBuffer_GeometrySrcType:
            maxValidVertex = geoSrc.fVertexBuffer->sizeInBytes() /
                             VertexSize(geoSrc.fVertexLayout);
            break;
    }
    if (maxVertex > maxValidVertex) {
        GrCrash("Indexed drawing outside valid vertex range.");
    }
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
        GrCrash("Indexed drawing outside valid index range.");
    }
#endif
    if (indexCount > 0) {
        this->onDrawIndexed(type, startVertex, startIndex,
                            vertexCount, indexCount);
    }
}


void GrDrawTarget::drawNonIndexed(GrPrimitiveType type,
                                  int startVertex,
                                  int vertexCount) {
#if GR_DEBUG
    GeometrySrcState& geoSrc = fGeoSrcStateStack.back();
    int maxVertex = startVertex + vertexCount;
    int maxValidVertex;
    switch (geoSrc.fVertexSrc) {
        case kNone_GeometrySrcType:
            GrCrash("Attempting to draw non-indexed geom without vertex src.");
        case kReserved_GeometrySrcType: // fallthrough
        case kArray_GeometrySrcType:
            maxValidVertex = geoSrc.fVertexCount;
            break;
        case kBuffer_GeometrySrcType:
            maxValidVertex = geoSrc.fVertexBuffer->sizeInBytes() /
            VertexSize(geoSrc.fVertexLayout);
            break;
    }
    if (maxVertex > maxValidVertex) {
        GrCrash("Non-indexed drawing outside valid vertex range.");
    }
#endif
    if (vertexCount > 0) {
        this->onDrawNonIndexed(type, startVertex, vertexCount);
    }
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
     * rearranged as [1-(1-Cd)f]D. By substituing in the various possbilities
     * for Cd we find that only 1, ISA, and ISC produce the correct depth
     * coeffecient in terms of S' and D.
     */
    return kOne_BlendCoeff == fCurrDrawState.fDstBlend||
           kISA_BlendCoeff == fCurrDrawState.fDstBlend ||
           kISC_BlendCoeff == fCurrDrawState.fDstBlend;
}


bool GrDrawTarget::srcAlphaWillBeOne() const {
    const GrVertexLayout& layout = this->getGeomSrc().fVertexLayout;

    // Check if per-vertex or constant color may have partial alpha
    if ((layout & kColor_VertexLayoutBit) ||
        0xff != GrColorUnpackA(fCurrDrawState.fColor)) {
        return false;
    }
    // Check if color filter could introduce an alpha
    // (TODO: Consider being more aggressive with regards to detecting 0xff
    // final alpha from color filter).
    if (SkXfermode::kDst_Mode != fCurrDrawState.fColorFilterXfermode) {
        return false;
    }
    // Check if a color stage could create a partial alpha
    for (int s = 0; s < fCurrDrawState.fFirstCoverageStage; ++s) {
        if (StageWillBeUsed(s, layout, fCurrDrawState)) {
            GrAssert(NULL != fCurrDrawState.fTextures[s]);
            GrPixelConfig config = fCurrDrawState.fTextures[s]->config();
            if (!GrPixelConfigIsOpaque(config)) {
                return false;
            }
        }
    }
    return true;
}

GrDrawTarget::BlendOptFlags
GrDrawTarget::getBlendOpts(bool forceCoverage,
                           GrBlendCoeff* srcCoeff,
                           GrBlendCoeff* dstCoeff) const {

    const GrVertexLayout& layout = this->getGeomSrc().fVertexLayout;

    GrBlendCoeff bogusSrcCoeff, bogusDstCoeff;
    if (NULL == srcCoeff) {
        srcCoeff = &bogusSrcCoeff;
    }
    *srcCoeff = fCurrDrawState.fSrcBlend;

    if (NULL == dstCoeff) {
        dstCoeff = &bogusDstCoeff;
    }
    *dstCoeff = fCurrDrawState.fDstBlend;

    // We don't ever expect source coeffecients to reference the source
    GrAssert(kSA_BlendCoeff != *srcCoeff &&
             kISA_BlendCoeff != *srcCoeff &&
             kSC_BlendCoeff != *srcCoeff &&
             kISC_BlendCoeff != *srcCoeff);
    // same for dst
    GrAssert(kDA_BlendCoeff != *dstCoeff &&
             kIDA_BlendCoeff != *dstCoeff &&
             kDC_BlendCoeff != *dstCoeff &&
             kIDC_BlendCoeff != *dstCoeff);

    if (SkToBool(kNoColorWrites_StateBit & fCurrDrawState.fFlagBits)) {
        *srcCoeff = kZero_BlendCoeff;
        *dstCoeff = kOne_BlendCoeff;
    }

    bool srcAIsOne = this->srcAlphaWillBeOne();
    bool dstCoeffIsOne = kOne_BlendCoeff == *dstCoeff ||
                         (kSA_BlendCoeff == *dstCoeff && srcAIsOne);
    bool dstCoeffIsZero = kZero_BlendCoeff == *dstCoeff ||
                         (kISA_BlendCoeff == *dstCoeff && srcAIsOne);


    // When coeffs are (0,1) there is no reason to draw at all, unless
    // stenciling is enabled. Having color writes disabled is effectively
    // (0,1).
    if ((kZero_BlendCoeff == *srcCoeff && dstCoeffIsOne)) {
        if (fCurrDrawState.fStencilSettings.doesWrite()) {
            if (fCaps.fShaderSupport) {
                return kDisableBlend_BlendOptFlag |
                       kEmitTransBlack_BlendOptFlag;
            } else {
                return kDisableBlend_BlendOptFlag;
            }
        } else {
            return kSkipDraw_BlendOptFlag;
        }
    }

    // check for coverage due to edge aa or coverage texture stage
    bool hasCoverage = forceCoverage ||
                       fCurrDrawState.fEdgeAANumEdges > 0 ||
                       (layout & kCoverage_VertexLayoutBit) ||
                       (layout & kEdge_VertexLayoutBit);
    for (int s = fCurrDrawState.fFirstCoverageStage;
         !hasCoverage && s < GrDrawState::kNumStages;
         ++s) {
        if (StageWillBeUsed(s, layout, fCurrDrawState)) {
            hasCoverage = true;
        }
    }

    // if we don't have coverage we can check whether the dst
    // has to read at all. If not, we'll disable blending.
    if (!hasCoverage) {
        if (dstCoeffIsZero) {
            if (kOne_BlendCoeff == *srcCoeff) {
                // if there is no coverage and coeffs are (1,0) then we
                // won't need to read the dst at all, it gets replaced by src
                return kDisableBlend_BlendOptFlag;
            } else if (kZero_BlendCoeff == *srcCoeff &&
                       fCaps.fShaderSupport) {
                // if the op is "clear" then we don't need to emit a color
                // or blend, just write transparent black into the dst.
                *srcCoeff = kOne_BlendCoeff;
                *dstCoeff = kZero_BlendCoeff;
                return kDisableBlend_BlendOptFlag |
                       kEmitTransBlack_BlendOptFlag;
            }
        }
    } else {
        // check whether coverage can be safely rolled into alpha
        // of if we can skip color computation and just emit coverage
        if (this->canTweakAlphaForCoverage()) {
            return kCoverageAsAlpha_BlendOptFlag;
        }
        // We haven't implemented support for these optimizations in the
        // fixed pipe (which is on its deathbed)
        if (fCaps.fShaderSupport) {
            if (dstCoeffIsZero) {
                if (kZero_BlendCoeff == *srcCoeff) {
                    // the source color is not included in the blend
                    // the dst coeff is effectively zero so blend works out to:
                    // (c)(0)D + (1-c)D = (1-c)D.
                    *dstCoeff = kISA_BlendCoeff;
                    return  kEmitCoverage_BlendOptFlag;
                } else if (srcAIsOne) {
                    // the dst coeff is effectively zero so blend works out to:
                    // cS + (c)(0)D + (1-c)D = cS + (1-c)D.
                    // If Sa is 1 then we can replace Sa with c 
                    // and set dst coeff to 1-Sa.
                    *dstCoeff = kISA_BlendCoeff;
                    return  kCoverageAsAlpha_BlendOptFlag;
                }
            } else if (dstCoeffIsOne) {
                // the dst coeff is effectively one so blend works out to:
                // cS + (c)(1)D + (1-c)D = cS + D.
                *dstCoeff = kOne_BlendCoeff;
                return  kCoverageAsAlpha_BlendOptFlag;
            }
        }
    }
    return kNone_BlendOpt;
}

bool GrDrawTarget::willUseHWAALines() const {
    // there is a conflict between using smooth lines and our use of
    // premultiplied alpha. Smooth lines tweak the incoming alpha value
    // but not in a premul-alpha way. So we only use them when our alpha
    // is 0xff and tweaking the color for partial coverage is OK
    if (!fCaps.fHWAALineSupport ||
        !(kHWAntialias_StateBit & fCurrDrawState.fFlagBits)) {
        return false;
    }
    BlendOptFlags opts = this->getBlendOpts();
    return (kDisableBlend_BlendOptFlag & opts) &&
           (kCoverageAsAlpha_BlendOptFlag & opts);
}

bool GrDrawTarget::canApplyCoverage() const {
    // we can correctly apply coverage if a) we have dual source blending
    // or b) one of our blend optimizations applies.
    return this->getCaps().fDualSourceBlendingSupport ||
           kNone_BlendOpt != this->getBlendOpts(true);
}

bool GrDrawTarget::drawWillReadDst() const {
    return SkToBool((kDisableBlend_BlendOptFlag | kSkipDraw_BlendOptFlag) &
                    this->getBlendOpts());
}

///////////////////////////////////////////////////////////////////////////////

void GrDrawTarget::setEdgeAAData(const GrDrawState::Edge* edges, int numEdges) {
    GrAssert(numEdges <= GrDrawState::kMaxEdges);
    memcpy(fCurrDrawState.fEdgeAAEdges, edges,
           numEdges * sizeof(GrDrawState::Edge));
    fCurrDrawState.fEdgeAANumEdges = numEdges;
}


////////////////////////////////////////////////////////////////////////////////

void GrDrawTarget::drawRect(const GrRect& rect, 
                            const GrMatrix* matrix,
                            StageBitfield stageEnableBitfield,
                            const GrRect* srcRects[],
                            const GrMatrix* srcMatrices[]) {
    GrVertexLayout layout = GetRectVertexLayout(stageEnableBitfield, srcRects);

    AutoReleaseGeometry geo(this, layout, 4, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return;
    }

    SetRectVertices(rect, matrix, srcRects, 
                    srcMatrices, layout, geo.vertices());

    drawNonIndexed(kTriangleFan_PrimitiveType, 0, 4);
}

GrVertexLayout GrDrawTarget::GetRectVertexLayout(StageBitfield stageEnableBitfield, 
                                                 const GrRect* srcRects[]) {
    GrVertexLayout layout = 0;

    for (int i = 0; i < GrDrawState::kNumStages; ++i) {
        int numTC = 0;
        if (stageEnableBitfield & (1 << i)) {
            if (NULL != srcRects && NULL != srcRects[i]) {
                layout |= StageTexCoordVertexLayoutBit(i, numTC);
                ++numTC;
            } else {
                layout |= StagePosAsTexCoordVertexLayoutBit(i);
            }
        }
    }
    return layout;
}

void GrDrawTarget::clipWillBeSet(const GrClip& clip) {
}

void GrDrawTarget::SetRectVertices(const GrRect& rect,
                                   const GrMatrix* matrix, 
                                   const GrRect* srcRects[], 
                                   const GrMatrix* srcMatrices[],
                                   GrVertexLayout layout, 
                                   void* vertices) {
#if GR_DEBUG
    // check that the layout and srcRects agree
    for (int i = 0; i < GrDrawState::kNumStages; ++i) {
        if (VertexTexCoordsForStage(i, layout) >= 0) {
            GR_DEBUGASSERT(NULL != srcRects && NULL != srcRects[i]);
        } else {
            GR_DEBUGASSERT(NULL == srcRects || NULL == srcRects[i]);
        }
    }
#endif

    int stageOffsets[GrDrawState::kNumStages];
    int vsize = VertexSizeAndOffsetsByStage(layout, stageOffsets,
                                            NULL, NULL, NULL);

    GrTCast<GrPoint*>(vertices)->setRectFan(rect.fLeft, rect.fTop,
                                            rect.fRight, rect.fBottom,
                                            vsize);
    if (NULL != matrix) {
        matrix->mapPointsWithStride(GrTCast<GrPoint*>(vertices), vsize, 4);
    }

    for (int i = 0; i < GrDrawState::kNumStages; ++i) {
        if (stageOffsets[i] > 0) {
            GrPoint* coords = GrTCast<GrPoint*>(GrTCast<intptr_t>(vertices) +
                                                stageOffsets[i]);
            coords->setRectFan(srcRects[i]->fLeft, srcRects[i]->fTop,
                               srcRects[i]->fRight, srcRects[i]->fBottom,
                               vsize);
            if (NULL != srcMatrices && NULL != srcMatrices[i]) {
                srcMatrices[i]->mapPointsWithStride(coords, vsize, 4);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

GrDrawTarget::AutoStateRestore::AutoStateRestore() {
    fDrawTarget = NULL;
}

GrDrawTarget::AutoStateRestore::AutoStateRestore(GrDrawTarget* target) {
    fDrawTarget = target;
    if (NULL != fDrawTarget) {
        fDrawTarget->saveCurrentDrawState(&fDrawState);
    }
}

GrDrawTarget::AutoStateRestore::~AutoStateRestore() {
    if (NULL != fDrawTarget) {
        fDrawTarget->restoreDrawState(fDrawState);
    }
}

void GrDrawTarget::AutoStateRestore::set(GrDrawTarget* target) {
    if (target != fDrawTarget) {
        if (NULL != fDrawTarget) {
            fDrawTarget->restoreDrawState(fDrawState);
        }
        if (NULL != target) {
            target->saveCurrentDrawState(&fDrawState);
        }
        fDrawTarget = target;
    }
}

////////////////////////////////////////////////////////////////////////////////

GrDrawTarget::AutoDeviceCoordDraw::AutoDeviceCoordDraw(GrDrawTarget* target, 
                                                       int stageMask) {
    GrAssert(NULL != target);

    fDrawTarget = target;
    fViewMatrix = target->getViewMatrix();
    fStageMask = stageMask;
    if (fStageMask) {
        GrMatrix invVM;
        if (fViewMatrix.invert(&invVM)) {
            for (int s = 0; s < GrDrawState::kNumStages; ++s) {
                if (fStageMask & (1 << s)) {
                    fSamplerMatrices[s] = target->getSamplerMatrix(s);
                }
            }
            target->preConcatSamplerMatrices(fStageMask, invVM);
        } else {
            // sad trombone sound
            fStageMask = 0;
        }
    }
    target->setViewMatrix(GrMatrix::I());
}

GrDrawTarget::AutoDeviceCoordDraw::~AutoDeviceCoordDraw() {
    fDrawTarget->setViewMatrix(fViewMatrix);
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        if (fStageMask & (1 << s)) {
            fDrawTarget->setSamplerMatrix(s, fSamplerMatrices[s]);
        }
    }
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
        if (vertexCount > 0) {
            success = target->reserveVertexSpace(vertexLayout, 
                                                 vertexCount,
                                                 &fVertices);
            if (!success) {
                this->reset();
            }
        }
        if (success && indexCount > 0) {
            success = target->reserveIndexSpace(indexCount, &fIndices);
            if (!success) {
                this->reset();
            }
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

void GrDrawTarget::Caps::print() const {
    static const char* gNY[] = {"NO", "YES"};
    GrPrintf("8 Bit Palette Support       : %s\n", gNY[f8BitPaletteSupport]);
    GrPrintf("NPOT Texture Support        : %s\n", gNY[fNPOTTextureSupport]);
    GrPrintf("NPOT Texture Tile Support   : %s\n", gNY[fNPOTTextureTileSupport]);
    GrPrintf("NPOT Render Target Support  : %s\n", gNY[fNPOTRenderTargetSupport]);
    GrPrintf("Two Sided Stencil Support   : %s\n", gNY[fTwoSidedStencilSupport]);
    GrPrintf("Stencil Wrap Ops  Support   : %s\n", gNY[fStencilWrapOpsSupport]);
    GrPrintf("HW AA Lines Support         : %s\n", gNY[fHWAALineSupport]);
    GrPrintf("Shader Support              : %s\n", gNY[fShaderSupport]);
    GrPrintf("Shader Derivative Support   : %s\n", gNY[fShaderDerivativeSupport]);
    GrPrintf("Geometry Shader Support     : %s\n", gNY[fGeometryShaderSupport]);
    GrPrintf("FSAA Support                : %s\n", gNY[fFSAASupport]);
    GrPrintf("Dual Source Blending Support: %s\n", gNY[fDualSourceBlendingSupport]);
    GrPrintf("Buffer Lock Support         : %s\n", gNY[fBufferLockSupport]);
    GrPrintf("Min Render Target Width     : %d\n", fMinRenderTargetWidth);
    GrPrintf("Min Render Target Height    : %d\n", fMinRenderTargetHeight);
    GrPrintf("Max Texture Size            : %d\n", fMaxTextureSize);
    GrPrintf("Max Render Target Size      : %d\n", fMaxRenderTargetSize);
}

