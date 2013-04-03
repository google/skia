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

GrGpuGL::ProgramCache::ProgramCache(const GrGLContext& gl)
    : fCount(0)
    , fCurrLRUStamp(0)
    , fGL(gl)
#ifdef PROGRAM_CACHE_STATS
    , fTotalRequests(0)
    , fCacheMisses(0)
#endif
{
}

GrGpuGL::ProgramCache::~ProgramCache() {
    // dump stats
#ifdef PROGRAM_CACHE_STATS
    SkDebugf("--- Program Cache ---\n");
    SkDebugf("Total requests: %d\n", fTotalRequests);
    SkDebugf("Cache misses: %d\n", fCacheMisses);
    SkDebugf("Cache miss %%: %f\n", (fTotalRequests > 0)
                                    ? (float)fCacheMisses/(float)fTotalRequests : 0.0f);
    SkDebugf("---------------------\n");
#endif
}

void GrGpuGL::ProgramCache::abandon() {
    for (int i = 0; i < fCount; ++i) {
        GrAssert(NULL != fEntries[i].fProgram.get());
        fEntries[i].fProgram->abandon();
        fEntries[i].fProgram.reset(NULL);
    }
    fCount = 0;
}

GrGLProgram* GrGpuGL::ProgramCache::getProgram(const GrGLProgramDesc& desc,
                                               const GrEffectStage* stages[]) {
    Entry newEntry;
    newEntry.fKey.setKeyData(desc.asKey());
#ifdef PROGRAM_CACHE_STATS
    ++fTotalRequests;
#endif

    Entry* entry = fHashCache.find(newEntry.fKey);
    if (NULL == entry) {
#ifdef PROGRAM_CACHE_STATS
        ++fCacheMisses;
#endif
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

bool GrGpuGL::flushGraphicsState(DrawType type, const GrDeviceCoordTexture* dstCopy) {
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
        GrGLProgramDesc desc;
        GrGLProgramDesc::Build(this->getDrawState(),
                               kDrawPoints_DrawType == type,
                               blendOpts,
                               srcCoeff,
                               dstCoeff,
                               this,
                               dstCopy,
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
        fCurrentProgram->setData(this, color, coverage, dstCopy, &fSharedGLProgramState);
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

void GrGpuGL::setupGeometry(const DrawInfo& info, size_t* indexOffsetInBytes) {

    GrGLsizei stride = this->getDrawState().getVertexSize();

    size_t vertexOffsetInBytes = stride * info.startVertex();

    const GeometryPoolState& geoPoolState = this->getGeomPoolState();

    GrGLVertexBuffer* vbuf;
    switch (this->getGeomSrc().fVertexSrc) {
        case kBuffer_GeometrySrcType:
            vbuf = (GrGLVertexBuffer*) this->getGeomSrc().fVertexBuffer;
            break;
        case kArray_GeometrySrcType:
        case kReserved_GeometrySrcType:
            this->finalizeReservedVertices();
            vertexOffsetInBytes += geoPoolState.fPoolStartVertex * this->getGeomSrc().fVertexSize;
            vbuf = (GrGLVertexBuffer*) geoPoolState.fPoolVertexBuffer;
            break;
        default:
            vbuf = NULL; // suppress warning
            GrCrash("Unknown geometry src type!");
    }

    GrAssert(NULL != vbuf);
    GrAssert(!vbuf->isLocked());
    vertexOffsetInBytes += vbuf->baseOffset();

    GrGLIndexBuffer* ibuf = NULL;
    if (info.isIndexed()) {
        GrAssert(NULL != indexOffsetInBytes);

        switch (this->getGeomSrc().fIndexSrc) {
        case kBuffer_GeometrySrcType:
            *indexOffsetInBytes = 0;
            ibuf = (GrGLIndexBuffer*)this->getGeomSrc().fIndexBuffer;
            break;
        case kArray_GeometrySrcType:
        case kReserved_GeometrySrcType:
            this->finalizeReservedIndices();
            *indexOffsetInBytes = geoPoolState.fPoolStartIndex * sizeof(GrGLushort);
            ibuf = (GrGLIndexBuffer*) geoPoolState.fPoolIndexBuffer;
            break;
        default:
            ibuf = NULL; // suppress warning
            GrCrash("Unknown geometry src type!");
        }

        GrAssert(NULL != ibuf);
        GrAssert(!ibuf->isLocked());
        *indexOffsetInBytes += ibuf->baseOffset();
    }
    GrGLAttribArrayState* attribState =
        fHWGeometryState.bindArrayAndBuffersToDraw(this, vbuf, ibuf);

    uint32_t usedAttribArraysMask = 0;
    const GrVertexAttrib* vertexAttrib = this->getDrawState().getVertexAttribs();
    int vertexAttribCount = this->getDrawState().getVertexAttribCount();
    for (int vertexAttribIndex = 0; vertexAttribIndex < vertexAttribCount;
         ++vertexAttribIndex, ++vertexAttrib) {

        usedAttribArraysMask |= (1 << vertexAttribIndex);
        GrVertexAttribType attribType = vertexAttrib->fType;
        attribState->set(this,
                         vertexAttribIndex,
                         vbuf,
                         GrGLAttribTypeToLayout(attribType).fCount,
                         GrGLAttribTypeToLayout(attribType).fType,
                         GrGLAttribTypeToLayout(attribType).fNormalized,
                         stride,
                         reinterpret_cast<GrGLvoid*>(
                         vertexOffsetInBytes + vertexAttrib->fOffset));
    }

    attribState->disableUnusedAttribArrays(this, usedAttribArraysMask);
}
