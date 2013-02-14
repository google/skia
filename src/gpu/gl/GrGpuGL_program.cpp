/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGpuGL.h"

#include "GrEffect.h"
#include "GrGLEffect.h"

typedef GrGLUniformManager::UniformHandle UniformHandle;
static const UniformHandle kInvalidUniformHandle = GrGLUniformManager::kInvalidUniformHandle;

#define SKIP_CACHE_CHECK    true
#define GR_UINT32_MAX   static_cast<uint32_t>(-1)

GrGpuGL::ProgramCache::ProgramCache(const GrGLContextInfo& gl)
    : fCount(0)
    , fCurrLRUStamp(0)
    , fGL(gl) {
}

void GrGpuGL::ProgramCache::abandon() {
    for (int i = 0; i < fCount; ++i) {
        GrAssert(NULL != fEntries[i].fProgram.get());
        fEntries[i].fProgram->abandon();
        fEntries[i].fProgram.reset(NULL);
    }
    fCount = 0;
}

GrGLProgram* GrGpuGL::ProgramCache::getProgram(const GrGLProgram::Desc& desc,
                                               const GrEffectStage* stages[]) {
    Entry newEntry;
    newEntry.fKey.setKeyData(desc.asKey());

    Entry* entry = fHashCache.find(newEntry.fKey);
    if (NULL == entry) {
        newEntry.fProgram.reset(GrGLProgram::Create(fGL, desc, stages));
        if (NULL == newEntry.fProgram.get()) {
            return NULL;
        }
        if (fCount < kMaxEntries) {
            entry = fEntries + fCount;
            ++fCount;
        } else {
            GrAssert(kMaxEntries == fCount);
            entry = fEntries;
            for (int i = 1; i < kMaxEntries; ++i) {
                if (fEntries[i].fLRUStamp < entry->fLRUStamp) {
                    entry = fEntries + i;
                }
            }
            fHashCache.remove(entry->fKey, entry);
        }
        *entry = newEntry;
        fHashCache.insert(entry->fKey, entry);
    }

    entry->fLRUStamp = fCurrLRUStamp;
    if (GR_UINT32_MAX == fCurrLRUStamp) {
        // wrap around! just trash our LRU, one time hit.
        for (int i = 0; i < fCount; ++i) {
            fEntries[i].fLRUStamp = 0;
        }
    }
    ++fCurrLRUStamp;
    return entry->fProgram;
}

////////////////////////////////////////////////////////////////////////////////

void GrGpuGL::abandonResources(){
    INHERITED::abandonResources();
    fProgramCache->abandon();
    fHWProgramID = 0;
}

////////////////////////////////////////////////////////////////////////////////

#define GL_CALL(X) GR_GL_CALL(this->glInterface(), X)

void GrGpuGL::flushPathStencilMatrix() {
    const SkMatrix& viewMatrix = this->getDrawState().getViewMatrix();
    const GrRenderTarget* rt = this->getDrawState().getRenderTarget();
    SkISize size;
    size.set(rt->width(), rt->height());
    const SkMatrix& vm = this->getDrawState().getViewMatrix();

    if (fHWPathStencilMatrixState.fRenderTargetOrigin != rt->origin() ||
        fHWPathStencilMatrixState.fViewMatrix.cheapEqualTo(viewMatrix) ||
        fHWPathStencilMatrixState.fRenderTargetSize!= size) {
        // rescale the coords from skia's "device" coords to GL's normalized coords,
        // and perform a y-flip if required.
        SkMatrix m;
        if (kBottomLeft_GrSurfaceOrigin == rt->origin()) {
            m.setScale(SkIntToScalar(2) / rt->width(), SkIntToScalar(-2) / rt->height());
            m.postTranslate(-SK_Scalar1, SK_Scalar1);
        } else {
            m.setScale(SkIntToScalar(2) / rt->width(), SkIntToScalar(2) / rt->height());
            m.postTranslate(-SK_Scalar1, -SK_Scalar1);
        }
        m.preConcat(vm);

        // GL wants a column-major 4x4.
        GrGLfloat mv[]  = {
            // col 0
            SkScalarToFloat(m[SkMatrix::kMScaleX]),
            SkScalarToFloat(m[SkMatrix::kMSkewY]),
            0,
            SkScalarToFloat(m[SkMatrix::kMPersp0]),

            // col 1
            SkScalarToFloat(m[SkMatrix::kMSkewX]),
            SkScalarToFloat(m[SkMatrix::kMScaleY]),
            0,
            SkScalarToFloat(m[SkMatrix::kMPersp1]),

            // col 2
            0, 0, 0, 0,

            // col3
            SkScalarToFloat(m[SkMatrix::kMTransX]),
            SkScalarToFloat(m[SkMatrix::kMTransY]),
            0.0f,
            SkScalarToFloat(m[SkMatrix::kMPersp2])
        };
        GL_CALL(MatrixMode(GR_GL_PROJECTION));
        GL_CALL(LoadMatrixf(mv));
        fHWPathStencilMatrixState.fViewMatrix = vm;
        fHWPathStencilMatrixState.fRenderTargetSize = size;
        fHWPathStencilMatrixState.fRenderTargetOrigin = rt->origin();
    }
}

bool GrGpuGL::flushGraphicsState(DrawType type) {
    const GrDrawState& drawState = this->getDrawState();

    // GrGpu::setupClipAndFlushState should have already checked this and bailed if not true.
    GrAssert(NULL != drawState.getRenderTarget());

    if (kStencilPath_DrawType == type) {
        this->flushPathStencilMatrix();
    } else {
        this->flushMiscFixedFunctionState();

        GrBlendCoeff srcCoeff;
        GrBlendCoeff dstCoeff;
        GrDrawState::BlendOptFlags blendOpts = drawState.getBlendOpts(false, &srcCoeff, &dstCoeff);
        if (GrDrawState::kSkipDraw_BlendOptFlag & blendOpts) {
            return false;
        }

        const GrEffectStage* stages[GrDrawState::kNumStages];
        for (int i = 0; i < GrDrawState::kNumStages; ++i) {
            stages[i] = drawState.isStageEnabled(i) ? &drawState.getStage(i) : NULL;
        }
        GrGLProgram::Desc desc;
        GrGLProgram::BuildDesc(this->getDrawState(),
                               kDrawPoints_DrawType == type,
                               blendOpts,
                               srcCoeff,
                               dstCoeff,
                               this,
                               &desc);

        fCurrentProgram.reset(fProgramCache->getProgram(desc, stages));
        if (NULL == fCurrentProgram.get()) {
            GrAssert(!"Failed to create program!");
            return false;
        }
        fCurrentProgram.get()->ref();

        GrGLuint programID = fCurrentProgram->programID();
        if (fHWProgramID != programID) {
            GL_CALL(UseProgram(programID));
            fHWProgramID = programID;
        }

        fCurrentProgram->overrideBlend(&srcCoeff, &dstCoeff);
        this->flushBlend(kDrawLines_DrawType == type, srcCoeff, dstCoeff);

        GrColor color;
        GrColor coverage;
        if (blendOpts & GrDrawState::kEmitTransBlack_BlendOptFlag) {
            color = 0;
            coverage = 0;
        } else if (blendOpts & GrDrawState::kEmitCoverage_BlendOptFlag) {
            color = 0xffffffff;
            coverage = drawState.getCoverage();
        } else {
            color = drawState.getColor();
            coverage = drawState.getCoverage();
        }
        fCurrentProgram->setData(this, color, coverage, &fSharedGLProgramState);
    }
    this->flushStencil(type);
    this->flushScissor();
    this->flushAAState(type);

    GrIRect* devRect = NULL;
    GrIRect devClipBounds;
    if (drawState.isClipState()) {
        this->getClip()->getConservativeBounds(drawState.getRenderTarget(), &devClipBounds);
        devRect = &devClipBounds;
    }
    // This must come after textures are flushed because a texture may need
    // to be msaa-resolved (which will modify bound FBO state).
    this->flushRenderTarget(devRect);

    return true;
}

void GrGpuGL::setupGeometry(const DrawInfo& info, int* startIndexOffset) {

    int newColorOffset;
    int newCoverageOffset;
    int newTexCoordOffset;
    int newEdgeOffset;

    GrVertexLayout currLayout = this->getDrawState().getVertexLayout();

    GrGLsizei newStride = GrDrawState::VertexSizeAndOffsets(currLayout,
                                                            &newTexCoordOffset,
                                                            &newColorOffset,
                                                            &newCoverageOffset,
                                                            &newEdgeOffset);
    int oldColorOffset;
    int oldCoverageOffset;
    int oldTexCoordOffset;
    int oldEdgeOffset;

    GrGLsizei oldStride = GrDrawState::VertexSizeAndOffsets(fHWGeometryState.fVertexLayout,
                                                            &oldTexCoordOffset,
                                                            &oldColorOffset,
                                                            &oldCoverageOffset,
                                                            &oldEdgeOffset);

    int extraVertexOffset;
    this->setBuffers(info.isIndexed(), &extraVertexOffset, startIndexOffset);

    size_t vertexOffset = (info.startVertex() + extraVertexOffset) * newStride;

    // all the Pointers must be set if any of these are true
    bool allOffsetsChange =  fHWGeometryState.fArrayPtrsDirty ||
                             vertexOffset != fHWGeometryState.fVertexOffset ||
                             newStride != oldStride;

    if (allOffsetsChange) {
        int idx = GrGLProgram::PositionAttributeIdx();
        GL_CALL(VertexAttribPointer(idx, 2, GR_GL_FLOAT, false, newStride, (GrGLvoid*)vertexOffset));
        fHWGeometryState.fVertexOffset = vertexOffset;
    }

    if (newTexCoordOffset > 0) {
        GrGLvoid* texCoordOffset = (GrGLvoid*)(vertexOffset + newTexCoordOffset);
        int idx = GrGLProgram::TexCoordAttributeIdx();
        if (oldTexCoordOffset <= 0) {
            GL_CALL(EnableVertexAttribArray(idx));
            GL_CALL(VertexAttribPointer(idx, 2, GR_GL_FLOAT, false, newStride, texCoordOffset));
        } else if (allOffsetsChange || newTexCoordOffset != oldTexCoordOffset) {
            GL_CALL(VertexAttribPointer(idx, 2, GR_GL_FLOAT, false, newStride, texCoordOffset));
        }
    } else if (oldTexCoordOffset > 0) {
        GL_CALL(DisableVertexAttribArray(GrGLProgram::TexCoordAttributeIdx()));
    }

    if (newColorOffset > 0) {
        fSharedGLProgramState.fConstAttribColor = GrColor_ILLEGAL;
        GrGLvoid* colorOffset = (int8_t*)(vertexOffset + newColorOffset);
        int idx = GrGLProgram::ColorAttributeIdx();
        if (oldColorOffset <= 0) {
            GL_CALL(EnableVertexAttribArray(idx));
            GL_CALL(VertexAttribPointer(idx, 4, GR_GL_UNSIGNED_BYTE, true, newStride, colorOffset));
        } else if (allOffsetsChange || newColorOffset != oldColorOffset) {
            GL_CALL(VertexAttribPointer(idx, 4, GR_GL_UNSIGNED_BYTE, true, newStride, colorOffset));
        }
    } else if (oldColorOffset > 0) {
        GL_CALL(DisableVertexAttribArray(GrGLProgram::ColorAttributeIdx()));
    }

    if (newCoverageOffset > 0) {
        fSharedGLProgramState.fConstAttribColor = GrColor_ILLEGAL;
        GrGLvoid* coverageOffset = (int8_t*)(vertexOffset + newCoverageOffset);
        int idx = GrGLProgram::CoverageAttributeIdx();
        if (oldCoverageOffset <= 0) {
            GL_CALL(EnableVertexAttribArray(idx));
            GL_CALL(VertexAttribPointer(idx, 4, GR_GL_UNSIGNED_BYTE,
                                        true, newStride, coverageOffset));
        } else if (allOffsetsChange || newCoverageOffset != oldCoverageOffset) {
            GL_CALL(VertexAttribPointer(idx, 4, GR_GL_UNSIGNED_BYTE,
                                        true, newStride, coverageOffset));
        }
    } else if (oldCoverageOffset > 0) {
        GL_CALL(DisableVertexAttribArray(GrGLProgram::CoverageAttributeIdx()));
    }

    if (newEdgeOffset > 0) {
        GrGLvoid* edgeOffset = (int8_t*)(vertexOffset + newEdgeOffset);
        int idx = GrGLProgram::EdgeAttributeIdx();
        if (oldEdgeOffset <= 0) {
            GL_CALL(EnableVertexAttribArray(idx));
            GL_CALL(VertexAttribPointer(idx, 4, GR_GL_FLOAT, false, newStride, edgeOffset));
        } else if (allOffsetsChange || newEdgeOffset != oldEdgeOffset) {
            GL_CALL(VertexAttribPointer(idx, 4, GR_GL_FLOAT, false, newStride, edgeOffset));
        }
    } else if (oldEdgeOffset > 0) {
        GL_CALL(DisableVertexAttribArray(GrGLProgram::EdgeAttributeIdx()));
    }

    fHWGeometryState.fVertexLayout = currLayout;
    fHWGeometryState.fArrayPtrsDirty = false;
}
